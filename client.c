/*
 * client.c -- TCP Socket Client
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if(argc < 3) {
    printf("The minimum number of arguments is 4\n");
    printf("Allowed commands are:\n");
    printf("rfs WRITE local-file-path remote-file-path\n");
    printf("rfs remote-file-path local-file-path\n");
    printf("rfs RM remote-file-path");
    return -1;
  }

  char* command = argv[1];
  char* remoteFileName;
  char* localFileName;

  if(strcmp(command, "WRITE") != 0 &&
    strcmp(command, "GET") != 0 &&
    strcmp(command, "RM") != 0) {
      printf("Enter valid command/n");
    }

  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];
  char fullCommand[2000];
  
  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));
  memset(fullCommand, '\0', sizeof(fullCommand));

  if(strcmp(command, "RM") == 0) {
    if(argc < 3) {
        printf("RM requires remote-file-path\n");
        return -1;
    }
    snprintf(fullCommand, sizeof(fullCommand), "%s %s", command, argv[2]);
  } 
  else {
    if(argc < 4) {
        printf("%s requires two file paths\n", command);
        return -1;
    }
    snprintf(fullCommand, sizeof(fullCommand), "%s %s %s", command, argv[2], argv[3]);
  }
    
  printf("Sending command: %s\n", fullCommand);
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Unable to create socket\n");
    close(socket_desc);
    return -1;
  }
  
  printf("Socket created successfully\n");
  
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("10.110.59.141");
  
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    close(socket_desc);
    return -1;
  }
  printf("Connected with server successfully\n");

  // Send the message to server:
  if(send(socket_desc, fullCommand, strlen(fullCommand), 0) < 0){
    printf("Unable to send message\n");
    close(socket_desc);
    return -1;
  }
  
  // Receive the server's response:
  if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
    printf("Error while receiving server's msg\n");
    close(socket_desc);
    return -1;
  }
  
  printf("Server's response: %s\n",server_message);
  
  // Close the socket:
  close(socket_desc);
  
  return 0;
}
