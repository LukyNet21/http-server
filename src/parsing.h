#ifndef PARSINHG_H
#define PARSINHG_H

#include "http.h"

char *http_status_code(int code) ;
http_method parse_http_method(const char *s);

#endif // PARSINHG_H