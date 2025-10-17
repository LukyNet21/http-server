#include "http.h"
#include "parsing.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static inline int http_is_tchar(int ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
         (ch >= '0' && ch <= '9') || ch == '!' || ch == '#' || ch == '$' ||
         ch == '%' || ch == '&' || ch == '\'' || ch == '*' || ch == '+' ||
         ch == '-' || ch == '.' || ch == '^' || ch == '_' || ch == '`' ||
         ch == '|' || ch == '~';
}

char *build_http_respones(int status_code, char *content_type, header *headers,
                          size_t headers_size, char *body) {
  const char *content = body ? body : "";
  const char *status_text = http_status_code(status_code);
  const char *ctype = content_type ? content_type : "text/plain";
  size_t body_len = strlen(content);

  int has_content_type = 0;
  int has_content_length = 0;

  size_t total =
      (size_t)snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", status_code, status_text);

  if (headers && headers_size > 0) {
    for (size_t i = 0; i < headers_size; i++) {
      if (!headers[i].key || !headers[i].value)
        continue;
      if (strcasecmp(headers[i].key, "Content-Type") == 0)
        has_content_type = 1;
      else if (strcasecmp(headers[i].key, "Content-Length") == 0)
        has_content_length = 1;
      total += (size_t)snprintf(NULL, 0, "%s: %s\r\n", headers[i].key,
                                headers[i].value);
    }
  }

  if (!has_content_type) {
    total += (size_t)snprintf(NULL, 0, "Content-Type: %s\r\n", ctype);
  }
  if (!has_content_length) {
    total += (size_t)snprintf(NULL, 0, "Content-Length: %zu\r\n", body_len);
  }

  total += 2; // between header and body
  total += body_len;

  char *response = (char *)malloc(total + 1);
  if (!response) {
    return NULL;
  }

  size_t offset = 0;
  offset +=
      (size_t)sprintf(response, "HTTP/1.1 %d %s\r\n", status_code, status_text);

  if (headers && headers_size > 0) {
    for (size_t i = 0; i < headers_size; i++) {
      if (!headers[i].key || !headers[i].value)
        continue;
      offset += (size_t)sprintf(response + offset, "%s: %s\r\n", headers[i].key,
                                headers[i].value);
    }
  }

  if (!has_content_type) {
    offset += (size_t)sprintf(response + offset, "Content-Type: %s\r\n", ctype);
  }
  if (!has_content_length) {
    offset +=
        (size_t)sprintf(response + offset, "Content-Length: %zu\r\n", body_len);
  }

  memcpy(response + offset, "\r\n", 2);
  offset += 2;

  if (body_len > 0) {
    memcpy(response + offset, content, body_len);
    offset += body_len;
  }
  response[offset] = '\0';
  return response;
}

http_request parse_http_request_headers(char *inp) {
  http_request req;
  size_t pos;
  memset(&req, 0, sizeof(req));
  req.method = HTTP_METHOD_UNKNOWN;

  if (!inp)
    return req;

  size_t inp_size = strlen(inp);

  // Parse method
  size_t len = 0;
  while (inp[len] && inp[len] != ' ')
    len++;

  if (len == 0)
    return req;

  char *method = (char *)malloc(len + 1);
  if (!method)
    return req;

  memcpy(method, inp, len);
  method[len] = '\0';
  req.method_str = method;
  req.method = parse_http_method(req.method_str);
  pos = len;

  // Parse request target
  if (inp[pos] != ' ')
    return req;
  pos++;

  len = 0;
  while (inp[pos + len] && inp[pos + len] != ' ')
    len++;

  if (len == 0)
    return req;

  char *target = (char *)malloc(len + 1);
  if (!target)
    return req;

  memcpy(target, inp + pos, len);
  target[len] = '\0';
  req.target = target;
  pos += len;

  if (inp[pos] != ' ')
    return req;
  pos++;

  // Parse version
  if (strncmp(inp + pos, "HTTP/", 5) != 0) {
    return req;
  }
  pos += 5;
  if (sscanf(inp + pos, "%d.%d", &req.http_major, &req.http_minor) != 2) {
    return req;
  }

  size_t v = pos;
  while (v < inp_size && isdigit((unsigned char)inp[v]))
    v++;
  if (inp[v] != '.')
    return req;
  v++;
  while (v < inp_size && isdigit((unsigned char)inp[v]))
    v++;
  if (v + 1 >= inp_size)
    return req;
  if (inp[v] != '\r' || inp[v + 1] != '\n')
    return req;
  pos = v + 2;

  // Headers
  http_headers headers = parse_http_headers(inp, &pos);
  req.headers = headers;

  req.keep_alive =
      (req.http_major > 1 || (req.http_major == 1 && req.http_minor >= 1)) ? 1
                                                                           : 0;
  // Get headers
  for (size_t hi = 0; hi < req.headers.headers_count; hi++) {
    const char *k =
        req.headers.headers[hi].key ? req.headers.headers[hi].key : "";
    const char *v =
        req.headers.headers[hi].value ? req.headers.headers[hi].value : "";
    if (strcasecmp(k, "Connection") == 0) {
      if (strcasecmp(v, "close") == 0)
        req.keep_alive = 0;
      else if (strcasecmp(v, "keep-alive") == 0)
        req.keep_alive = 1;
    } else if (strcasecmp(k, "Host") == 0 && !req.host) {
      size_t vlen = strlen(v);
      char *h = (char *)malloc(vlen + 1);
      if (h) {
        memcpy(h, v, vlen);
        h[vlen] = '\0';
      }
      req.host = h;
    } else if (strcasecmp(k, "Content-Type") == 0 && !req.content_type) {
      size_t vlen = strlen(v);
      char *ct = (char *)malloc(vlen + 1);
      if (ct) {
        memcpy(ct, v, vlen);
        ct[vlen] = '\0';
      }
      req.content_type = ct;
    } else if (strcasecmp(k, "Content-Length") == 0) {
      sscanf(v, "%ld", &req.content_length);
    }
  }

  // Request path and query
  const char *q = strchr(req.target, '?');
  if (q) {
    size_t p_len = (size_t)(q - req.target);
    req.path = (char *)malloc(p_len + 1);
    if (req.path) {
      memcpy(req.path, req.target, p_len);
      req.path[p_len] = '\0';
    }
    size_t q_len = strlen(q + 1);
    req.query = (char *)malloc(q_len + 1);
    if (req.query) {
      memcpy(req.query, q + 1, q_len);
      req.query[q_len] = '\0';
    }
  } else {
    size_t p_len = strlen(req.target);
    req.path = (char *)malloc(p_len + 1);
    if (req.path) {
      memcpy(req.path, req.target, p_len);
      req.path[p_len] = '\0';
    }
    req.query = NULL;
  }

  // Query parameters
  if (req.query) {
    req.query_params = parse_http_query_params(req.query);
  } else {
    req.query_params.params = NULL;
    req.query_params.params_count = 0;
  }

  req.body = NULL;

  return req;
}

void http_request_debug_print(const http_request *r) {
  if (!r) {
    printf("(http_request) NULL\n");
    return;
  }

  const char *method_s = r->method_str ? r->method_str : "";
  printf("HTTP Request:\n");
  printf("  method: %s (%d)\n", method_s, (int)r->method);
  printf("  target: %s\n", r->target ? r->target : "");
  printf("  path: %s\n", r->path ? r->path : "");
  printf("  query: %s\n", r->query ? r->query : "");
  printf("  version: %d.%d\n", r->http_major, r->http_minor);
  printf("  host: %s\n", r->host ? r->host : "");
  printf("  content_type: %s\n", r->content_type ? r->content_type : "");
  printf("  content_length: %zu\n", r->content_length);
  printf("  keep_alive: %d\n", r->keep_alive);

  printf("  headers (%zu):\n", r->headers.headers_count);
  for (size_t i = 0; i < r->headers.headers_count; i++) {
    const char *k = r->headers.headers && r->headers.headers[i].key
                        ? r->headers.headers[i].key
                        : "";
    const char *v = r->headers.headers && r->headers.headers[i].value
                        ? r->headers.headers[i].value
                        : "";
    printf("    %s: %s\n", k, v);
  }

  printf("  query_params (%zu):\n", r->query_params.params_count);
  for (size_t i = 0; i < r->query_params.params_count; i++) {
    const char *k = r->query_params.params && r->query_params.params[i].key
                        ? r->query_params.params[i].key
                        : "";
    const char *v = r->query_params.params && r->query_params.params[i].value
                        ? r->query_params.params[i].value
                        : "";
    printf("    %s=%s\n", k, v);
  }

  if (r->body && r->content_length > 0) {
    printf("  body: %s\n", r->body);
  } else {
    printf("  body: \n");
  }
}

void http_headers_free(http_headers *h) {
  for (size_t i = 0; i < h->headers_count; i++) {
    free(h->headers[i].key);
    free(h->headers[i].value);
  }
  free(h->headers);
}

void query_params_free(query_params *p) {
  for (size_t i = 0; i < p->params_count; i++) {
    free(p->params[i].key);
    free(p->params[i].value);
  }
  free(p->params);
}

void http_request_free(http_request *r) {
  free(r->method_str);
  free(r->target);
  free(r->path);
  free(r->query);
  free(r->host);
  free(r->content_type);
  free(r->body);
  http_headers_free(&(r->headers));
  query_params_free(&(r->query_params));
}

http_headers parse_http_headers(char *s, size_t *n) {
  http_headers result = {.headers = NULL, .headers_count = 0};
  if (!s)
    return result;

  const size_t s_size = strlen(s);
  size_t i = n ? *n : 0;

  size_t cap = 16;
  header *arr = (header *)calloc(cap, sizeof(header));
  if (!arr)
    return result;

  size_t count = 0;
  int store_disabled = 0;

  while (i + 1 < s_size) {
    // End of headers
    if (s[i] == '\r' && s[i + 1] == '\n') {
      i += 2;
      break;
    }

    // Parse field-name
    size_t name_start = i;
    while (i < s_size && http_is_tchar((unsigned char)s[i]))
      i++;
    size_t name_end = i;
    if (name_start == name_end)
      break;
    if (i >= s_size || s[i] != ':')
      break;
    i++;

    // Skip whitespace
    while (i < s_size && (s[i] == ' ' || s[i] == '\t'))
      i++;

    // Parse value
    size_t value_start = i;
    while (i + 1 < s_size && !(s[i] == '\r' && s[i + 1] == '\n'))
      i++;
    size_t value_end = i;

    while (value_end > value_start &&
           (s[value_end - 1] == ' ' || s[value_end - 1] == '\t')) {
      value_end--;
    }

    if (!store_disabled && count >= cap) {
      size_t new_cap = cap * 2;
      header *new_arr = (header *)realloc(arr, new_cap * sizeof(header));
      if (!new_arr) {
        store_disabled = 1;
      } else {
        arr = new_arr;
        cap = new_cap;
      }
    }

    if (store_disabled) {
      if (i + 1 < s_size && s[i] == '\r' && s[i + 1] == '\n')
        i += 2;
      continue;
    }

    // Key
    size_t klen = name_end - name_start;
    char *k = (char *)malloc(klen + 1);
    if (!k) {
      if (i + 1 < s_size && s[i] == '\r' && s[i + 1] == '\n')
        i += 2;
      continue;
    }
    memcpy(k, s + name_start, klen);
    k[klen] = '\0';

    // Value
    size_t vlen = value_end - value_start;
    char *v = (char *)malloc(vlen + 1);
    if (!v) {
      free(k);
      if (i + 1 < s_size && s[i] == '\r' && s[i + 1] == '\n')
        i += 2;
      continue;
    }
    if (vlen)
      memcpy(v, s + value_start, vlen);
    v[vlen] = '\0';

    arr[count].key = k;
    arr[count].value = v;
    count++;

    if (i + 1 < s_size && s[i] == '\r' && s[i + 1] == '\n')
      i += 2;
  }

  result.headers = arr;
  result.headers_count = count;

  if (n)
    *n = i;

  return result;
}

query_params parse_http_query_params(char *s) {
  query_params result = {.params = NULL, .params_count = 0};
  if (!s || !*s)
    return result;

  size_t cap = 16;
  query_param *arr = (query_param *)calloc(cap, sizeof(query_param));
  if (!arr)
    return result;

  const char *p = s;
  const char *end = s + strlen(s);
  size_t count = 0;

  while (p < end) {
    if (count >= cap) {
      size_t new_cap = cap * 2;
      query_param *new_arr =
          (query_param *)realloc(arr, new_cap * sizeof(query_param));
      if (!new_arr) {
        break;
      }
      arr = new_arr;
      cap = new_cap;
    }

    const char *pair_start = p;
    const char *amp = strchr(pair_start, '&');
    size_t pair_len =
        amp ? (size_t)(amp - pair_start) : (size_t)(end - pair_start);

    const char *eq = memchr(pair_start, '=', pair_len);
    size_t klen = 0, vlen = 0;
    const char *vstart = NULL;
    if (eq) {
      klen = (size_t)(eq - pair_start);
      vstart = eq + 1;
      vlen = pair_len - klen - 1;
    } else {
      klen = pair_len;
      vstart = NULL;
      vlen = 0;
    }

    char *k = (char *)malloc(klen + 1);
    if (!k)
      break;
    if (klen)
      memcpy(k, pair_start, klen);
    k[klen] = '\0';

    char *v = (char *)malloc(vlen + 1);
    if (!v) {
      free(k);
      break;
    }
    if (vlen && vstart)
      memcpy(v, vstart, vlen);
    v[vlen] = '\0';

    arr[count].key = k;
    arr[count].value = v;
    count++;

    if (!amp)
      break;
    p = amp + 1;
  }

  result.params = arr;
  result.params_count = count;
  return result;
}