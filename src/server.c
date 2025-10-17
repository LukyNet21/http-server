#include "server.h"
#include "http.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int new_server(int port) {
  int server_fd;
  struct sockaddr_in server_addr;

  // create server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
      0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("socket binded to port %d\n", port);

  // listen for connections
  if (listen(server_fd, 10) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
  printf("server listening for connections\n");

  return server_fd;
}

void connection_loop(int server_fd) {
  while (1) {
    // accept connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int conn_fd;

    printf("waiting for connections...\n");

    if ((conn_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                          &client_len)) < 0) {
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

void *client_connection(void *args) {
  int client_fd = *((int *)args);

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  const int max_requests = 100;
  for (int i = 0; i < max_requests; i++) {
    http_request req = read_http_request(client_fd);

    if (!req.method_str && req.headers.headers_count == 0) {
      http_request_free(&req);
      break;
    }

    http_request_debug_print(&req);

    header resp_headers[2];
    size_t rhc = 0;
    resp_headers[rhc].key = "Connection";
    resp_headers[rhc].value = req.keep_alive ? "keep-alive" : "close";
    rhc++;
    if (req.keep_alive) {
      resp_headers[rhc].key = "Keep-Alive";
      resp_headers[rhc].value = "timeout=5, max=100";
      rhc++;
    }

    char *response = build_http_respones(200, "text/plain", resp_headers, rhc,
                                         "Hello, World!!!!!");
    if (response) {
      write(client_fd, response, strlen(response));
      free(response);
    }

    int keep = req.keep_alive;
    http_request_free(&req);
    if (!keep) {
      break;
    }
  }

  close(client_fd);
  return NULL;
}

http_request read_http_request(int client_fd) {
  http_request req;
  memset(&req, 0, sizeof(req));
  req.method = HTTP_METHOD_UNKNOWN;

  size_t cap = 8192;
  size_t len = 0;
  char *buff = (char *)malloc(cap + 1);
  if (!buff) {
    return req;
  }

  ssize_t header_end = -1;

  while (1) {
    if (cap - len < 1024) {
      size_t new_cap = cap * 2;
      char *nb = (char *)realloc(buff, new_cap + 1);
      if (!nb) {
        break;
      }
      buff = nb;
      cap = new_cap;
    }

    ssize_t n = read(client_fd, buff + len, cap - len);
    if (n <= 0) {
      break;
    }
    len += (size_t)n;
    buff[len] = '\0';

    if (header_end < 0) {
      for (size_t i = 0; header_end < 0 && i + 3 < len; i++) {
        if (buff[i] == '\r' && buff[i + 1] == '\n' && buff[i + 2] == '\r' &&
            buff[i + 3] == '\n') {
          header_end = (ssize_t)(i + 4);
        }
      }
    }

    if (header_end > 0) {
      break;
    }
  }

  size_t bytes_read = len;
  if (!buff || bytes_read == 0) {
    free(buff);
    return req;
  }

  buff[bytes_read] = '\0';
  req = parse_http_request_headers(buff);

  int skip_body =
      (req.method == HTTP_METHOD_GET || req.method == HTTP_METHOD_HEAD);
  size_t need_body = skip_body ? 0 : req.content_length;

  if (header_end > 0 && !skip_body && need_body > 0) {
    size_t have = bytes_read - (size_t)header_end;
    if (have < need_body) {
      size_t needed_total = (size_t)header_end + need_body;
      if (cap < needed_total + 1) {
        size_t new_cap = needed_total * 2;
        char *nb = (char *)realloc(buff, new_cap + 1);
        if (nb) {
          buff = nb;
          cap = new_cap;
        }
      }
      while (have < need_body) {
        ssize_t n =
            read(client_fd, buff + (size_t)header_end + have, need_body - have);
        if (n <= 0)
          break;
        have += (size_t)n;
      }
      bytes_read = (size_t)header_end + have;
      buff[bytes_read] = '\0';
    }

    if (need_body > 0) {
      req.body = (char *)malloc(need_body + 1);
      if (req.body) {
        memcpy(req.body, buff + (size_t)header_end, need_body);
        req.body[need_body] = '\0';
        req.content_length = need_body;
      }
    }
  }
  free(buff);
  return req;
}