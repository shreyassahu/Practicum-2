#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <time.h> 

extern pthread_mutex_t file_mutex;

void* handleClient(void *args);
int handleWrite(int client_sock, char* remote_path, long file_size);
int handleGet(int client_sock, char* remote_path); 
int handleRM(int client_sock, char* remote_file_path);
int handleLS(int client_sock, char* remote_path);
void sendResponse(int client_sock, char* message);
void sendError(int client_sock, char* message);
int getNextVersion(char* file_path); 
int createDirectories(char* file_path);

#endif