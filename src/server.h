#ifndef SERVER_H
#define SERVER_H

#include "http.h"

int new_server(int port);
void connection_loop(int server_fd);
void *client_connection(void *args);
http_request read_http_request(int client_fd);

#endif // SERVER_H