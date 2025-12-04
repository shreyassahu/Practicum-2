#include "rm.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "config.h"

int handleRM(int socket_desc, char* remote_file_path) {
  char buffer[BUFFER_SIZE];
  snprintf(buffer, sizeof(buffer), "RM %s", remote_file_path);

  if(send(socket_desc, buffer, strlen(buffer), 0) < 0) {
    printf("Couldn't send command\n");
    return -1;
  }

  char server_response[256];
  memset(server_response, '\0', sizeof(server_response));

  if(recv(socket_desc, server_response, sizeof(server_response), 0) < 0) {
    printf("Error while receiving server's message\n");
    return -1;
  }

  printf("Server's response: %s", server_response);
  return 0;

}