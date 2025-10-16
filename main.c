#include "src/server.h"
#include <unistd.h>

int main()
{
  int server_fd = new_server(8080);
  connection_loop(server_fd);
  close(server_fd);
  return 0;
}
