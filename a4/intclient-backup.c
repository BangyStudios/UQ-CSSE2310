/*
 *  Receives string from sock socket
 *  Params:
 *      Socket* sock - a socket database object
 *  Returns (char*):
 *      buffer - string from the other socket
 */
char* socket_receive(int sockfd) {
    char* buffer = NULL;
    int bufferSize = 4096;
    int totalSize = 0;
    int currentSize = 0;
    int recvSize = 0;
    do {
        if (totalSize >= currentSize) {
            char* temp;
            currentSize += bufferSize;
            temp = realloc(buffer, currentSize * sizeof(char));
            if (temp == NULL) {
                fprintf(stderr, "Shit has hit the fan\n");
                break;
            }
            buffer = temp;
        }
        recvSize = recv(sockfd, buffer + totalSize, bufferSize, 0);
        if (recvSize > 0) {
            totalSize += recvSize;
        } else if (recvSize == 0) {
            break;
        }
    } while (recvSize > -1);
    return buffer;
}

/*
 *  Sends string to sock socket
 *  Params:
 *      Socket* sock - a socket database object
 *      char* message - a string
 *  Returns (void):
 */
void socket_send(int sockfd, char* message) {
    int sockfdw = dup(sockfd);
    FILE* stream = fdopen(sockfdw, "w");
    fputs(message, stream);
    fflush(stream);
    fclose(stream);
}

/*
 *  Receives string from sock socket
 *  Params:
 *      Socket* sock - a socket database object
 *  Returns (char*):
 *      buffer - string from the other socket
 */
char* socket_receive(int sockfd) {
    int sockfdr = dup(sockfd);
    FILE* stream = fdopen(sockfdr, "r");
    int bufferSize = 4096;
    int chunks = 1;
    char* buffer = malloc(sizeof(char) * bufferSize * chunks);
    while(fgets(buffer[bufferSize * (chunks - 1)], 
        bufferSize, stream) != NULL) {
        chunks++;
        buffer = realloc(buffer, sizeof(char) * bufferSize * chunks);
    }
    return buffer;
}
    for (int i = 0; i < jobs->size; i++) { // Print all jobs (should be only 1)
        fprintf(stdout, "|lineIndex=%d|function=%s|lower=%lf|upper=%lf|"
            "segments=%d|threads=%d|\n",
            jobs->jobs[i]->lineIndex,
            jobs->jobs[i]->function,
            jobs->jobs[i]->lower,
            jobs->jobs[i]->upper,
            jobs->jobs[i]->segments,
            jobs->jobs[i]->threads);
    }
    for (int i = 0; i < sockArray->size; i++) { // Print all sockets
        fprintf(stdout, "|socketfd=%d|port=%u|open=%d|\n",
            sockArray->sockets[i]->sockfd,
            sockArray->sockets[i]->port,
            sockArray->sockets[i]->open);
    }