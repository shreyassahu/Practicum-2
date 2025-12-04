#include "ls.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "config.h"

int handleLS(int socket_desc, char* remote_file_path) {
  char buffer[BUFFER_SIZE];
  snprintf(buffer, sizeof(buffer), "LS %s", remote_file_path);

  if(send(socket_desc, buffer, strlen(buffer), 0) < 0) {
    printf("Couldn't send command\n");
    return -1;
  }

  memset(buffer, '\0', sizeof(buffer));
  ssize_t bytes = recv(socket_desc, buffer, sizeof(buffer) - 1, 0);
  if(bytes <= 0) {
    printf("Error receiving response\n");
    return -1;
  }

  printf("%s", buffer);
  return 0;
}