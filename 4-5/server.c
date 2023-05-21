#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *handle_client(void *arg) {
    int newsockfd = *(int *) arg;
    char buffer[256];
    int n;

    while (1) {
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n <= 0) {
            break;
        }
        printf("Nechesov: From Petrov I received %s\n", buffer);
    }

    close(newsockfd);
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int sockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        int *newsockfd = malloc(sizeof(int));
        *newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (*newsockfd < 0) {
            error("ERROR on accept");
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, newsockfd) != 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    close(sockfd);
    return 0;
}
