#include "parsing.h"
#include "http.h"

#include <string.h>

http_method parse_http_method(const char *s) {
  if (!s)
    return HTTP_METHOD_UNKNOWN;
  if (strcmp(s, "GET") == 0)
    return HTTP_METHOD_GET;
  if (strcmp(s, "HEAD") == 0)
    return HTTP_METHOD_HEAD;
  if (strcmp(s, "POST") == 0)
    return HTTP_METHOD_POST;
  if (strcmp(s, "PUT") == 0)
    return HTTP_METHOD_PUT;
  if (strcmp(s, "DELETE") == 0)
    return HTTP_METHOD_DELETE;
  if (strcmp(s, "CONNECT") == 0)
    return HTTP_METHOD_CONNECT;
  if (strcmp(s, "OPTIONS") == 0)
    return HTTP_METHOD_OPTIONS;
  if (strcmp(s, "TRACE") == 0)
    return HTTP_METHOD_TRACE;
  if (strcmp(s, "PATCH") == 0)
    return HTTP_METHOD_PATCH;
  return HTTP_METHOD_UNKNOWN;
}

char *http_status_code(int code) {
  switch (code) {
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