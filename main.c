#include "src/server.h"
#include <signal.h> 
#include <unistd.h>
#include <stdlib.h>

int server_fd;

void closeServer(int sig)
{
  close(server_fd);
  exit(sig);
}

int main()
{
  signal(SIGINT, closeServer);
  server_fd = new_server(8080);
  connection_loop(server_fd);
  close(server_fd);
  return 0;
}
