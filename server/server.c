#include "server.h"
#include "config.h"

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Entry point of the server. 
 *        -Creates and binds TCP socket
 *        -Listens for incoming client connections
 *        -Creates a new thread for each client request
 * 
 * @return 0 on success, -1 on error
 */
int main(void)
{
  int client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  pthread_t tid;


  // Creates root dir in server folder, name mentioned in config
  mkdir(ROOT_DIR, 0755);


  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_desc < 0){
    printf("Error while creating socket\n");
    return -1;
  }

  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = INADDR_ANY; 

  printf("Socket created successfully\n");

  // Bind to the set port and IP:
  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    printf("Couldn't bind to the port\n");
    close(socket_desc);
    return -1;
  }

  printf("Done with binding\n");

  // Listen for clients:
  if(listen(socket_desc, 1) < 0){
    printf("Error while listening\n");
    close(socket_desc);
    return -1;
  }
  printf("\nListening for incoming connections.....\n");

  while(1) {

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
    
    if (client_sock < 0){
      printf("Can't accept\n");
      close(client_sock);
      continue;
    }
    printf("Client connected at IP: %s and port: %i\n", 
          inet_ntoa(client_addr.sin_addr), 
          ntohs(client_addr.sin_port));

    int *client_sock_ptr = malloc(sizeof(int));
    *client_sock_ptr = client_sock;

    // Thread created and handled for each client request
    if(pthread_create(&tid, NULL, handleClient, client_sock_ptr) != 0) {
      printf("Failed to create thread\n");
      close(client_sock);
      free(client_sock_ptr);
      continue;
    }
    pthread_detach(tid);
  }

  close(socket_desc);
  return 0;
}