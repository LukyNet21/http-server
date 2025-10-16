#include "server.h"
#include "http.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

int new_server(int port)
{
  int server_fd;
  struct sockaddr_in server_addr;

  // create server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  printf("socket created\n");

  // config socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // bind socket to port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("socket binded to port %d\n", port);

  // listen for connections
  if (listen(server_fd, 10) < 0)
  {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
  printf("server listening for connections\n");

  return server_fd;
}

void connection_loop(int server_fd)
{
  while (1)
  {
    // accept connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int conn_fd;

    printf("waiting for connections...\n");

    if ((conn_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                          &client_len)) < 0)
    {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    char *client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);
    printf("connection accepted from client %s:%d\n", client_ip, client_port);

    pthread_t thread;
    pthread_create(&thread, NULL, client_connection, (void *)&conn_fd);
    pthread_detach(thread);
  }
}

void *client_connection(void *args)
{
  int client_fd = *((int *)args);
  char buff[100];
  ssize_t bytes_read;

  memset(buff, 0, sizeof(buff));

  bytes_read = read(client_fd, buff, sizeof(buff) - 1);
  if (bytes_read <= 0)
  {
    printf("Client disconnected or error occurred\n");
    close(client_fd);
    return NULL;
  }

  header headers[] = {};

  buff[bytes_read] = '\0';
  printf("Message from client: %s", buff);

  char *response = build_http_respones(200, "text/plain", headers, 0, "Hello, World!!!!!");
  write(client_fd, response, strlen(response));
  free(response);

  close(client_fd);
  return NULL;
}