#include "client.h"
#include "config.h"

/**
 * @brief Prints the allowed commands
 * 
 */
void printUsage() {
  printf("The minimum number of arguments is 4\n");
  printf("Allowed commands are:\n");
  printf("rfs WRITE local-file-path remote-file-path\n");
  printf("rfs remote-file-path local-file-path\n");
  printf("rfs RM remote-file-path\n");
}

/**
 * @brief Connects the client to the server
 * 
 * @return int socket descriptor if successful, else -1
 */
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

/**
 * @brief Handles the write command for client
 * 
 * @param socket_desc socket descriptor
 * @param local_file_path client data path
 * @param remote_file_path server data path
 * @return int 0 success, -1 error
 */
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

/**
 * @brief Handles the get command for the client
 * 
 * @param socket_desc socket descriptor
 * @param remote_file_path server data path
 * @param local_file_path client output data path
 * @return int 0 success, -1 error
 */
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

/**
 * @brief Handles the RM command for client
 * 
 * @param socket_desc socket descriptor
 * @param remote_file_path file to be deleted in server
 * @return int 0 success, -1 error
 */
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

/**
 * @brief Handles the LS command for the client
 * 
 * @param socket_desc socket descriptor
 * @param remote_file_path server file path for LS
 * @return int 0 success, -1 error
 */
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

/**
 * @brief Creates directory in client folder, used in GET command
 * 
 * @param file_path client output directory
 * @return int 0
 */
int createDirectories(char* file_path) {
    char tmp[512];
    strncpy(tmp, file_path, sizeof(tmp));
    
    char *last_slash = strrchr(tmp, '/');
    if (!last_slash) return 0;  
    
    *last_slash = '\0';  
    
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
    
    return 0;
}

