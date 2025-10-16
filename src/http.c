#include "http.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

char *build_http_respones(int status_code, char *content_type, header *headers, size_t headers_size, char *body)
{
  const char *content = body ? body : "";
  const char *status_text = http_status_code(status_code);
  const char *ctype = content_type ? content_type : "text/plain";
  size_t body_len = strlen(content);

  int has_content_type = 0;
  int has_content_length = 0;

  size_t total = (size_t)snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", status_code, status_text);

  if (headers && headers_size > 0)
  {
    for (size_t i = 0; i < headers_size; i++)
    {
      if (!headers[i].key || !headers[i].value)
        continue;
      if (strcasecmp(headers[i].key, "Content-Type") == 0)
        has_content_type = 1;
      else if (strcasecmp(headers[i].key, "Content-Length") == 0)
        has_content_length = 1;
      total += (size_t)snprintf(NULL, 0, "%s: %s\r\n", headers[i].key, headers[i].value);
    }
  }

  if (!has_content_type)
  {
    total += (size_t)snprintf(NULL, 0, "Content-Type: %s\r\n", ctype);
  }
  if (!has_content_length)
  {
    total += (size_t)snprintf(NULL, 0, "Content-Length: %zu\r\n", body_len);
  }

  total += 2; // between header and body
  total += body_len;

  char *response = (char *)malloc(total + 1);
  if (!response)
  {
    return NULL;
  }

  size_t offset = 0;
  offset += (size_t)sprintf(response, "HTTP/1.1 %d %s\r\n", status_code, status_text);

  if (headers && headers_size > 0)
  {
    for (size_t i = 0; i < headers_size; i++)
    {
      if (!headers[i].key || !headers[i].value)
        continue;
      offset += (size_t)sprintf(response + offset, "%s: %s\r\n", headers[i].key, headers[i].value);
    }
  }

  if (!has_content_type)
  {
    offset += (size_t)sprintf(response + offset, "Content-Type: %s\r\n", ctype);
  }
  if (!has_content_length)
  {
    offset += (size_t)sprintf(response + offset, "Content-Length: %zu\r\n", body_len);
  }

  memcpy(response + offset, "\r\n", 2);
  offset += 2;

  if (body_len > 0)
  {
    memcpy(response + offset, content, body_len);
    offset += body_len;
  }
  response[offset] = '\0';
  return response;
}

char *http_status_code(int code)
{
  switch (code)
  {
  case 100:
    return "Continue";
    break;
  case 101:
    return "Switching Protocols";
    break;
  case 102:
    return "Processing";
    break;
  case 103:
    return "Early Hints";
    break;
  case 200:
    return "OK";
    break;
  case 201:
    return "Created";
    break;
  case 202:
    return "Accepted";
    break;
  case 203:
    return "Non-Authoritative Information";
    break;
  case 204:
    return "No Content";
    break;
  case 205:
    return "Reset Content";
    break;
  case 206:
    return "Partial Content";
    break;
  case 207:
    return "Multi-Status";
    break;
  case 208:
    return "Already Reported";
    break;
  case 218:
    return "This is fine (Apache Web Server)";
    break;
  case 226:
    return "IM Used";
    break;
  case 300:
    return "Multiple Choices";
    break;
  case 301:
    return "Moved Permanently";
    break;
  case 302:
    return "Found";
    break;
  case 303:
    return "See Other";
    break;
  case 304:
    return "Not Modified";
    break;
  case 306:
    return "Switch Proxy";
    break;
  case 307:
    return "Temporary Redirect";
    break;
  case 308:
    return "Resume Incomplete";
    break;
  case 400:
    return "Bad Request";
    break;
  case 401:
    return "Unauthorized";
    break;
  case 402:
    return "Payment Required";
    break;
  case 403:
    return "Forbidden";
    break;
  case 404:
    return "Not Found";
    break;
  case 405:
    return "Method Not Allowed";
    break;
  case 406:
    return "Not Acceptable";
    break;
  case 407:
    return "Proxy Authentication Required";
    break;
  case 408:
    return "Request Timeout";
    break;
  case 409:
    return "Conflict";
    break;
  case 410:
    return "Gone";
    break;
  case 411:
    return "Length Required";
    break;
  case 412:
    return "Precondition Failed";
    break;
  case 413:
    return "Request Entity Too Large";
    break;
  case 414:
    return "Request-URI Too Long";
    break;
  case 415:
    return "Unsupported Media Type";
    break;
  case 416:
    return "Requested Range Not Satisfiable";
    break;
  case 417:
    return "Expectation Failed";
    break;
  case 418:
    return "I'm a teapot";
    break;
  case 419:
    return "Page Expired (Laravel Framework)";
    break;
  case 420:
    return "Method Failure (Spring Framework)";
    break;
  case 421:
    return "Misdirected Request";
    break;
  case 422:
    return "Unprocessable Entity";
    break;
  case 423:
    return "Locked";
    break;
  case 424:
    return "Failed Dependency";
    break;
  case 426:
    return "Upgrade Required";
    break;
  case 428:
    return "Precondition Required";
    break;
  case 429:
    return "Too Many Requests";
    break;
  case 431:
    return "Request Header Fields Too Large";
    break;
  case 440:
    return "Login Time-out";
    break;
  case 444:
    return "Connection Closed Without Response";
    break;
  case 449:
    return "Retry With";
    break;
  case 450:
    return "Blocked by Windows Parental Controls";
    break;
  case 451:
    return "Unavailable For Legal Reasons";
    break;
  case 494:
    return "Request Header Too Large";
    break;
  case 495:
    return "SSL Certificate Error";
    break;
  case 496:
    return "SSL Certificate Required";
    break;
  case 497:
    return "HTTP Request Sent to HTTPS Port";
    break;
  case 498:
    return "Invalid Token (Esri)";
    break;
  case 499:
    return "Client Closed Request";
    break;
  case 500:
    return "Internal Server Error";
    break;
  case 501:
    return "Not Implemented";
    break;
  case 502:
    return "Bad Gateway";
    break;
  case 503:
    return "Service Unavailable";
    break;
  case 504:
    return "Gateway Timeout";
    break;
  case 505:
    return "HTTP Version Not Supported";
    break;
  case 506:
    return "Variant Also Negotiates";
    break;
  case 507:
    return "Insufficient Storage";
    break;
  case 508:
    return "Loop Detected";
    break;
  case 509:
    return "Bandwidth Limit Exceeded";
    break;
  case 510:
    return "Not Extended";
    break;
  case 511:
    return "Network Authentication Required";
    break;
  case 520:
    return "Unknown Error";
    break;
  case 521:
    return "Web Server Is Down";
    break;
  case 522:
    return "Connection Timed Out";
    break;
  case 523:
    return "Origin Is Unreachable";
    break;
  case 524:
    return "A Timeout Occurred";
    break;
  case 525:
    return "SSL Handshake Failed";
    break;
  case 526:
    return "Invalid SSL Certificate";
    break;
  case 527:
    return "Railgun Listener to Origin Error";
    break;
  case 530:
    return "Origin DNS Error";
    break;
  case 598:
    return "Network Read Timeout Error";
    break;
  }
  return "";
}
