#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "html_builder.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>

int download(char *path, int fd) {
    int arch = open(path, O_RDONLY, 0);
    if (arch == -1)
        return 0;

    struct stat a;
    stat(path, &a);

    char *buffer = malloc(1000);
    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    sprintf(buffer, "%sContent-length: %ld\r\n\r\n", buffer, (intmax_t)a.st_size);
    write(fd, buffer, strlen(buffer));

    long length = a.st_size;
    char *srcp = mmap(0, length, PROT_READ, MAP_PRIVATE, arch, 0);

    while (length > 0)
        length -= send(fd, srcp, a.st_size, 0);


    munmap(srcp, a.st_size);
    close(arch);
    close(fd);
    return 0;
}


int main(int argc, char const *argv[]) {
    
    unsigned short port = (unsigned short) atoi(argv[1]);
    char *root = strdup(argv[2]);
    
    int skt = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    const void *optval_p = &optval;
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, optval_p, sizeof(int));
    
    struct sockaddr_in s;
    s.sin_family = AF_INET;
    s.sin_port = htons(port);
    s.sin_addr.s_addr = htons(INADDR_ANY);

    struct sockaddr_in s_client;

    bind(skt, (struct sockaddr *) &s, sizeof(s));
    listen(skt, 1024);

    char *buf = calloc(50000, sizeof(char));


    int client = 0;
    socklen_t sf = sizeof(s_client);

    while ((client = accept(skt, (struct sockaddr *) &s_client, &sf))) {

        ssize_t rd = read(client, buf, 20000);
        write(1, buf, rd);


        // deleting \r rest of GET
        strtok(buf, "\r");
        // jumping GET word
        strtok(buf, " ");
        // getting path request
        char *path = strtok(NULL, " ");

        if (path == NULL) continue;
        //css case
        if (strstr(path, ".style.css") != NULL) {

            char *css_path = strdup(argv[0]);
            *strrchr(css_path, '/') = NULL;
            strcat(css_path, "/style.css");
            int css_fd = open(css_path, O_RDONLY, 0);
            char *css_buf = calloc(2048, sizeof(char));
            read(css_fd, css_buf, 2048);
            write(client, css_buf, strlen(css_buf));

            close(client);
            close(css_fd);
            continue;
        }
        // .js case
        if (strstr(path, ".sorttable.js") != NULL) {
            char *css_path = strdup(argv[0]);
            *strrchr(css_path, '/') = NULL;
            strcat(css_path, "/sorttable.js");
            int css_fd = open(css_path, O_RDONLY, 0);
            char *css_buf = calloc(2048, sizeof(char));
            read(css_fd, css_buf, 2048);
            write(client, css_buf, strlen(css_buf));

            close(client);
            close(css_fd);
            continue;
        }
        //favicon.ico case
        if (strstr(path, "favicon.ico") != NULL) {
            close(client);
            continue;
        }

        for (int i = 0; i < 256; i++) {
            if (path[i] == '%' && path[i + 1] == '2' && path[i + 2] == '0') {
                path[i] = ' ';
                for (int j = i + 1; j < 256; j++) {
                    path[j] = path[j + 2];
                }
            }
        }

        if (strrchr(path, '/') == path + strlen(path) - 1) {
            char* rrpath = malloc(256);
            strcpy(rrpath, root);
            strcat(rrpath, path);
            
            DIR *dir_handler = opendir(rrpath);
            if (dir_handler != NULL)
            {
                char *page = build_html(path, root);
                write(client, page, strlen(page));
                close(client);
            }
            else
            {
                char *page = build_html("/", root);
                write(client, page, strlen(page));
                close(client);
            }
        }
        else
        {
            char *rpath = malloc(512);
            strcpy(rpath, root);
            strcat(rpath, path);
            download(rpath, client);
        }
    }
    return 0;
}


