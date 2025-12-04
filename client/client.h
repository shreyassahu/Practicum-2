#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

void printUsage();
int connectToServer();
int handleWrite(int socket_desc, char* local_file_path, char* remote_file_path);
int handleGet(int socket_desc, char* remote_file_path, char* local_file_path);
int handleRM(int socket_desc, char* remote_file_path);
int handleLS(int socket_desc, char* remote_file_path);
int createDirectories(char* file_path);

#endif

