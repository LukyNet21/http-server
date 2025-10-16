int new_server(int port);
void connection_loop(int server_fd);
void *client_connection(void *args);

