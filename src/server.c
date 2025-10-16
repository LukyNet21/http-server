#include "server.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    
    client_connection(conn_fd);
  }
}

void client_connection(int client_fd)
{
  char buff[100];
  char reply[200];
  char *msg = "[CLIENT SENT]";
  ssize_t bytes_read;

  while (1)
  {
    memset(buff, 0, sizeof(buff));

    bytes_read = read(client_fd, buff, sizeof(buff) - 1);
    if (bytes_read <= 0)
    {
      printf("Client disconnected or error occurred\n");
      break;
    }

    buff[bytes_read] = '\0';
    printf("Message from client: %s", buff);

    snprintf(reply, sizeof(reply), "%s %s", msg, buff);
    write(client_fd, reply, strlen(reply));

    if (strncmp(buff, "bye", 3) == 0)
    {
      printf("Client said bye, closing connection\n");
      break;
    }
  }

  close(client_fd);
}