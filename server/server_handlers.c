#include "server.h"
#include "config.h"

void* handleClient(void *args) {
  int client_sock = *(int*)args;
  free(args);

  char buffer[BUFFER_SIZE];
  memset(buffer, '\0', sizeof(buffer));

  ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer), 0);
  if(bytes_read <= 0) {
    close(client_sock);
    return NULL;
  }

  printf("Received command from client: %s\n", buffer);

  char remote_path[256];
  long file_size = 0;

  if(sscanf(buffer, "WRITE %s %ld", remote_path, &file_size) == 2) {
    handleWrite(client_sock, remote_path, file_size);
  }

  else if(sscanf(buffer, "GET %s", remote_path) == 1) { 
    handleGet(client_sock, remote_path);
  }

  else if(sscanf(buffer, "RM %s", remote_path) == 1) {
    handleRM(client_sock, remote_path);
  }

  else if(sscanf(buffer, "LS %s", remote_path) == 1) {
    handleLS(client_sock, remote_path);
  }

  close(client_sock);
  printf("Client Disconnected\n");
  return NULL;
}

int handleWrite(int client_sock, char* remote_path, long file_size) {

  char buffer[BUFFER_SIZE];
  sendResponse(client_sock, "READY");
  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_DIR, remote_path);
  createDirectories(full_path);

  pthread_mutex_lock(&file_mutex);

  struct stat st;
  if(stat(full_path, &st) == 0) {
    int version = getNextVersion(full_path);
    char version_path[1024];
    snprintf(version_path, sizeof(version_path), "%s.v%d", full_path, version);
    rename(full_path, version_path);
    printf("Saved previous version as %s\n", version_path);
  }

  FILE *fptr = fopen(full_path, "wb");

  if(fptr == NULL) {
    pthread_mutex_unlock(&file_mutex);
    sendError(client_sock, "Failed to create file");
    return -1;
  }

  long total_bytes_received = 0;
  while(total_bytes_received < file_size) {
    long remaining_bytes = file_size - total_bytes_received;
    size_t bytes_to_read = (remaining_bytes < BUFFER_SIZE) ? remaining_bytes : BUFFER_SIZE;

    ssize_t bytes = recv(client_sock, buffer, bytes_to_read, 0);
    if(bytes <= 0) {
      fclose(fptr);
      pthread_mutex_unlock(&file_mutex);
      sendError(client_sock, "Coudn't write to file");
      return -1;
    }

    fwrite(buffer, 1, bytes, fptr);
    total_bytes_received += bytes;
  }

  fclose(fptr);
  pthread_mutex_unlock(&file_mutex);
  sendResponse(client_sock, "Data written successfully");
  return 0;
  
}

int handleGet(int client_sock, char* remote_path) {

  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_DIR, remote_path);

  pthread_mutex_lock(&file_mutex);

  FILE *fptr = fopen(full_path, "rb");
  if(fptr == NULL) {
    pthread_mutex_unlock(&file_mutex);
    sendError(client_sock, "File not found");
    return -1;
  }

  // Retrieve size of file
  fseek(fptr, 0, SEEK_END);
  long file_size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  char buffer[BUFFER_SIZE];
  snprintf(buffer, sizeof(buffer), "SIZE %ld\n", file_size);
  send(client_sock, buffer, strlen(buffer), 0);

  ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer), 0);
  if(bytes_read <= 0) {
    close(client_sock);
    return -1;
  }

  buffer[bytes_read] = '\0';

  if(strcmp(buffer, "READY") != 0) {
    sendError(client_sock, "Didn't establish successful connection\n");
    fclose(fptr);
    return -1;
  }


  size_t bytes;
  while((bytes = fread(buffer, 1, BUFFER_SIZE, fptr)) > 0) {
    send(client_sock, buffer, bytes, 0);
  }

  fclose(fptr);
  pthread_mutex_unlock(&file_mutex);

  printf("Data from file sent.\n");
  return 0;
}

int handleRM(int client_sock, char* remote_file_path) {
  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_DIR, remote_file_path);

  pthread_mutex_lock(&file_mutex);

  if(remove(full_path) == 0) {
    pthread_mutex_unlock(&file_mutex);
    printf("Deleted file successfully\n");
    sendResponse(client_sock, "Deleted file successfully");
    return 0;
  }
  else {
    pthread_mutex_unlock(&file_mutex);
    sendError(client_sock, "Couldn't delete file");
    return -1;
  }
}

int handleLS(int client_sock, char* remote_path) {

  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_DIR, remote_path);

  pthread_mutex_lock(&file_mutex);

  char response[BUFFER_SIZE];
  int offset = 0;
  struct stat st;

  // Check current version
  if(stat(full_path, &st) == 0) {
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
    offset += snprintf(response + offset, sizeof(response) - offset,
                       "[current] %s - %s (%ld bytes)\n", full_path, time_str, st.st_size);
  }

  // Check all versions
  int version = 1;
  char version_path[1024];
  while(1) {
    snprintf(version_path, sizeof(version_path), "%s.v%d", full_path, version);
    if(stat(version_path, &st) != 0) {
      break;  // No more versions
    }
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
    offset += snprintf(response + offset, sizeof(response) - offset,
                       "[v%d] %s - %s (%ld bytes)\n", version, version_path, time_str, st.st_size);
    version++;
  }

  pthread_mutex_unlock(&file_mutex);

  if(offset == 0) {
    sendError(client_sock, "File not found");
    return -1;
  }

  send(client_sock, response, strlen(response), 0);
  return 0;
}

int getNextVersion(char* file_path) {
  int version = 1;
  char version_path[1024];
  struct stat st;

  while(1) {
    snprintf(version_path, sizeof(version_path), "%s.v%d", file_path, version);
    if(stat(version_path, &st) != 0) {
      return version;
    }
    version += 1;
  }
}

void sendResponse(int client_sock, char* message) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s", message);
  send(client_sock, buffer, strlen(buffer), 0);
}

void sendError(int client_sock, char* message) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Error: %s", message);
  send(client_sock, buffer, strlen(buffer), 0);
}

int createDirectories(char *path) {
    char tmp[512];
    strncpy(tmp, path, sizeof(tmp));
    
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