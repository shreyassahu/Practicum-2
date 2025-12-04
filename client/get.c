#include "get.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "config.h"
#include "utilities.h"

int handleGet(int socket_desc, char* remote_file_path, char* local_file_path) {
  char buffer[BUFFER_SIZE];
  snprintf(buffer, sizeof(buffer), "GET %s", remote_file_path);

  if(send(socket_desc, buffer, strlen(buffer), 0) < 0) {
    printf("Couldn't send command");
    return -1;
  }

  char server_response[256];
  memset(server_response, '\0', sizeof(server_response));
  if(recv(socket_desc, server_response, sizeof(server_response), 0) < 0) {
    printf("Error while receiving server's message\n");
    close(socket_desc);
    return -1;
  } 

  char pattern[] = "SIZE";
  if(strstr(server_response, pattern) == NULL) {
    printf("Server error: %s", server_response);
    close(socket_desc);
    return -1;
  }

  long file_size = atol(server_response + 5);

  snprintf(buffer, sizeof(buffer), "READY");
  send(socket_desc, buffer, strlen(buffer), 0);

  createDirectories(local_file_path);
  FILE *fptr = fopen(local_file_path, "wb");
  if(fptr == NULL) {
    printf("Coundn't open file\n");
    return -1;
  }

  long total_bytes_received = 0;
  
  while(total_bytes_received < file_size) {
    long remaining_bytes = file_size - total_bytes_received;
    size_t bytes_to_read = (remaining_bytes < BUFFER_SIZE) ? remaining_bytes : BUFFER_SIZE;

    ssize_t bytes = recv(socket_desc, buffer, bytes_to_read, 0);
    if(bytes <= 0) {
      printf("Cound't receive data\n");
      fclose(fptr);
      return -1;
    }

    fwrite(buffer, 1, bytes, fptr);
    total_bytes_received += bytes;
  }

  fclose(fptr);
  printf("Data written successfully\n");
  return 0;
}