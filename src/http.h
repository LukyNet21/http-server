#include <stddef.h>

char *build_http_respones(int status_code, char *content_type, header *headers, size_t headers_size, char *body);
char *http_status_code(int code);
