#include "client.h"
#include "config.h"

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





