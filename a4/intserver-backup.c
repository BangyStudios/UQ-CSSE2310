/*
 *  Accepts client connections on a socket (blocking) and spawns threads
 *  Params:
 *      SocketArray* sockArray - socket database
 *      char* port - argument string for port
 *  Returns (void):
 */
void client_handler_spawner(Socket* sock, int maxThreads) {
    for (int threads = 0; threads < maxThreads; threads++) {
        int* sockfd;
        int acceptfd;
        pthread_t client;
        if ((acceptfd = accept(sock->sockfd, 0, 0) < 0) {
            fprintf(stderr, 
                "intserver: unable to open socket for listening\n");
            exit(3);
        } else {
            sockfd = malloc(sizeof(int));
            *sockfd = acceptfd;
        }
        if (pthread_create(&client, NULL, client_handler, (void*)threads) < 0) {
            fprintf(stderr, "Shit has hit the fan\n");
            exit(3);
        }
        pthread_join(client, NULL);
    }
}