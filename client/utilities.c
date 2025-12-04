#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

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