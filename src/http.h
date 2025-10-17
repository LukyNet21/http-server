#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
  char *key;
  char *value;
} kv_pair;

typedef kv_pair header;
typedef kv_pair query_param;

typedef enum {
  HTTP_METHOD_GET,
  HTTP_METHOD_HEAD,
  HTTP_METHOD_POST,
  HTTP_METHOD_PUT,
  HTTP_METHOD_DELETE,
  HTTP_METHOD_CONNECT,
  HTTP_METHOD_OPTIONS,
  HTTP_METHOD_TRACE,
  HTTP_METHOD_PATCH,
  HTTP_METHOD_UNKNOWN
} http_method;

typedef struct {
  header *headers;
  size_t headers_count;
} http_headers;

typedef struct {
  query_param *params;
  size_t params_count;
} query_params;

typedef struct {
  http_method method;
  char *method_str;
  char *target;
  char *path;
  char *query;
  int http_major;
  int http_minor;
  http_headers headers;
  char *host;
  char *content_type;
  size_t content_length;
  char *body;
  int keep_alive;

  query_params query_params;
} http_request;

http_request parse_http_request_headers(char *inp);
char *build_http_respones(int status_code, char *content_type, header *headers,
                          size_t headers_size, char *body);
http_headers parse_http_headers(char *s, size_t *n);
query_params parse_http_query_params(char *s);

void http_request_free(http_request *r);
void http_headers_free(http_headers *h);
void query_params_free(query_params *p);

void http_request_debug_print(const http_request *r);

#endif // HTTP_H