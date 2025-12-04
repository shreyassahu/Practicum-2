#include "write.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "config.h"


int handleWrite(int socket_desc, char* local_file_path, char* remote_file_path) {
  FILE *fptr = fopen(local_file_path, "rb");

  if(fptr == NULL) {
    printf("Couldn't open file\n");
    return -1;
  }

  // Retrieve the size of the file
  fseek(fptr, 0, SEEK_END);
  long file_size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  char buffer[BUFFER_SIZE];
  snprintf(buffer, sizeof(buffer), "WRITE %s %ld", remote_file_path, file_size);

  if(send(socket_desc, buffer, strlen(buffer), 0) < 0) {
    printf("Couldn't send command");
    fclose(fptr);
    close(socket_desc);
    return -1;
  }

  // Initial handshake response from server ready to write
  char server_response[256];
  memset(server_response, '\0', sizeof(server_response));
  if(recv(socket_desc, server_response, sizeof(server_response), 0) < 0) {
    printf("Error while receiving server's message\n");
    fclose(fptr);
    close(socket_desc);
    return -1;
  }

  if(strcmp(server_response, "READY") != 0) {
    printf("Server error: %s", server_response);
    fclose(fptr);
    close(socket_desc);
    return -1;
  }

  // Sending file data
  size_t bytes;
  while((bytes = fread(buffer, 1, BUFFER_SIZE, fptr)) > 0) {
    if(send(socket_desc, buffer, bytes, 0) < 0) {
      printf("Couldn't send data from file");
      fclose(fptr);
      close(socket_desc);
      return -1;
    }
  }

  fclose(fptr);
  memset(server_response, '\0', sizeof(server_response));
  if(recv(socket_desc, server_response, sizeof(server_response), 0) > 0) {
    printf("Final response from server: %s", server_response);
  }

  return 0;

}

