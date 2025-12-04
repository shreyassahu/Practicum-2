#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "config.h"
#include "get.h"
#include "ls.h"
#include "rm.h"
#include "write.h"
#include "utilities.h"

void printUsage();
int connectToServer();

int main(int argc, char *argv[])
{
  if(argc < 3) {
    printUsage();
    return 1;
  }

  char* command = argv[1];


  // Handle write command
  if(strcmp(command, "WRITE") == 0) {
    if(argc < 4) {
      printf("WRITE requires valid file paths\n");
      printUsage();
      return 1;
    }

    char *local_file_path = argv[2];
    char *remote_file_path = argv[3];

    int socket_desc = connectToServer();
    if(socket_desc < 0) {
      printf("Couldn't connect to server\n");
      return 1;
    }

    int res = handleWrite(socket_desc, local_file_path, remote_file_path);
    close(socket_desc);
    return res;
  }

  // Handle GET command
  if(strcmp(command, "GET") == 0) {
    if(argc < 4) {
      printf("GET requires valid file paths\n");
      printUsage();
      return 1;
    }

    char* remote_file_path = argv[2];
    char* local_file_path = argv[3];

    int socket_desc = connectToServer();
    if(socket_desc < 0) {
      printf("Couldn't connect to server\n");
      return 1;
    }

    int res = handleGet(socket_desc, remote_file_path, local_file_path);
    close(socket_desc);
    return res;
  }

  // Handle RM command
  if(strcmp(command, "RM") == 0) {
    if(argc < 3) {
      printf("RM requires remote file path\n");
      printUsage();
      return 1;
    }

    char* remote_file_path = argv[2];

    int socket_desc = connectToServer();
    if(socket_desc < 0) {
      printf("Couldn't connect to server\n");
      return 1;
    }

    int res = handleRM(socket_desc, remote_file_path);
    close(socket_desc);
    return res;
  }

  if(strcmp(command, "LS") == 0) {
    if(argc < 3) {
      printf("LS requires remote file path\n");
      printUsage();
      return 1;
    }

    char* remote_file_path = argv[2];

    int socket_desc = connectToServer();
    if(socket_desc < 0) {
      printf("Couldn't connect to server\n");
      return 1;
    }

    int res = handleLS(socket_desc, remote_file_path);
    close(socket_desc);
    return res;
  } 

  return 1;
}

void printUsage() {
  printf("The minimum number of arguments is 4\n");
  printf("Allowed commands are:\n");
  printf("rfs WRITE local-file-path remote-file-path\n");
  printf("rfs remote-file-path local-file-path\n");
  printf("rfs RM remote-file-path\n");
}

int connectToServer() {
  int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_addr;

  if(socket_desc < 0){
    printf("Unable to create socket\n");
    close(socket_desc);
    return -1;
  }

  printf("Socket created successfully\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    close(socket_desc);
    return -1;
  }

  printf("Connected with server successfully\n");

  return socket_desc;
}



