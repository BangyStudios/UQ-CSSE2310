#include "intcommon.h"

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <tinyexpr.h>

/*
 *  Returns whether the port argument is valid or not
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      1 - if the port is valid
 *      0 - if the port is invalid
 */
int check_arg_port(char* port) {
    int temp;
    if (sscanf(port, "%d", &temp) == 1) {
        if (temp >= PORT_MIN || temp < PORT_MAX) {
            return 1;
        }
    }
    return 0;
}

/*
 *  Gets and returns the maximum number of threads from the arguments.
 *  Exits on usage error.
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      0 - not specified (unlimited)
 *      maxThreads - the maximum specified
 */
int get_arg_maxthreads(int argc, char** argv) {
    int maxThreads = 0;
    if (argc > 2) {
        for (int i = 0; i < strlen(argv[2]); i++) { // Contains non-numerical
            if (isalpha(argv[2][i])) {
                fprintf(stderr, "Usage: intserver portnum [maxThreads]\n");
                exit(1);
            }
        }
        if (sscanf(argv[2], "%d", &maxThreads) == 1) {
            if (maxThreads > 0) {
                return maxThreads;
            }
        }
        fprintf(stderr, "Usage: intserver portnum [maxThreads]\n");
        exit(1);
    }
    return maxThreads;
}

/*
 *  Gets a socket information struct pointer with localhost and given port.
 *  Exits on failure.
 *  Params:
 *      char* port - argument string for port
 *  Returns (struct sockaddr*):
 *      struct sockaddr* - socket address struct
 */
struct sockaddr* socket_getaddrinfo(char* port) {
    struct addrinfo hints; // Construct a hints struct to get address info
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* ai = 0;
    if ((getaddrinfo("localhost", port, &hints, &ai))) { // Get addrinfo
        fprintf(stderr, "intserver: unable to open socket for listening\n");
        exit(3);
    }
    return ai;
}

/*
 *  Binds a socket to some port/address specified by socket_create().
 *  Exits on failure
 *  Params:
 *      int sockfd - argument string for port
 *      struct addrinfo* ai - an address informations struct pointer
 *  Returns (int):
 *      port - the acquired port number
 */
int socket_bind(int sockfd, struct addrinfo* ai) {
    if (bind(sockfd, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        fprintf(stderr, "intserver: unable to open socket for listening\n");
        exit(3);
    } else {
        struct sockaddr_in ad;
        memset(&ad, 0, sizeof(struct sockaddr_in));
        socklen_t len = sizeof(struct sockaddr_in);
        if (getsockname(sockfd, (struct sockaddr*)&ad, &len)) {
            fprintf(stderr, 
                    "intserver: unable to open socket for listening\n");
            exit(3);
        } else {
            return ntohs(ad.sin_port);
        }
    }
}

/*
 *  Makes socket listen on some port/address specified by socket_create()
 *  Params:
 *      int sockfd - argument string for port
 *  Returns (void):
 */
void socket_listen(int sockfd) {
    // Try to listen on socket
    if (listen(sockfd, 10) < 0) {
        fprintf(stderr, "intserver: unable to open socket for listening\n");
        exit(3);
    }
}

/*
 *  Creates a socket and returns its associated file descriptor
 *  Params:
 *      char* port - argument string for port
 *  Returns (int):
 *      sock - the file descriptor of the socket created
 */
int socket_create(char* port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Try to create socket
    struct addrinfo* ai = socket_getaddrinfo(port);
    socket_bind(sockfd, ai);
    socket_listen(sockfd);
    return sockfd;
}

/*
 *  Returns whether string starts with another (sub)string
 *  Params:
 *      char* prefix
 *      char* string
 *  Returns (bool):
 *      true - string starts with prefix
 *      false - string doesn't start with prefix
 */
bool isprefix(char* prefix, char* string) {
    return strncmp(prefix, string, strlen(prefix)) == 0;
}

/*
 *  Determines whether a function is valid or not
 *  Params:
 *      char* function - i.e. sin(x)
 *  Returns (int):
 *      1 - function is valid
 *      0 - function is invalid
 */
int function_isvalid(char* function) {
    double x;
    te_variable variables[] = {{"x", &x}};
    int errorPosition;
    te_expr* fx = te_compile(function, variables, 1, &errorPosition);
    if (!fx) {
        return 0;
    } else {
        return 1;
    }
}

/*
 *  Integrates a function of given bound with respect to x using the
 *  trapezoidal method.
 *  Params:
 *      Job* job - a Job* pointer
 *  Returns (double):
 *      result - result of integration
 */
double function_integrate_trapezoidal(Job* job) {
    // Stage
    double x;
    te_variable variables[] = {{"x", &x}};
    int errorPosition;
    te_expr* fx = te_compile(job->function, variables, 1, &errorPosition);
    // Define variables
    double result = 0.0, segmentWidth;
    // Find segment width
    segmentWidth = (job->upper - job->lower) / job->segments;
    // Integrate
    x = job->lower;
    result += segmentWidth * te_eval(fx);
    for (int i = 1; i < job->segments - 1; i++) {
        x = i * segmentWidth;
        result += segmentWidth * 2 * te_eval(fx);
    }
    x = job->upper;
    result += segmentWidth * te_eval(fx); 
    // Return
    return result;
}

/*
 *  Constructs a HTTP response as a char* depending on function validity
 *  Params:
 *      int isvalid - 1:valid, 0:!valid;
 *  Returns (char*):
 *      message - response message
 */
char* http_respond_isvalid(int isvalid) {
    // Stage
    HttpHeader** headers = malloc(sizeof(HttpHeader*));
    HttpHeader* header0 = malloc(sizeof(HttpHeader));
    char* message;
    char* body = "";
    // Construct content-length header
    header0->name = "Content-Length";
    header0->value = "0";
    headers[0] = header0;
    if (isvalid) {
        int status = 200;
        char* statusExplanation = "OK";
        message = construct_HTTP_response(status, statusExplanation, headers, 
                body);
    } else {
        int status = 400;
        char* statusExplanation = "Bad Request";
        message = construct_HTTP_response(status, statusExplanation, headers, 
                body);
    }
    free(header0);
    free(headers);
    return message;
}

/*
 *  Constructs a HTTP response as a char* depending on function validity
 *  Params:
 *      int isvalid - 1:valid, 0:!valid;
 *  Returns (char*):
 *      message - response message
 */
char* http_respond_integrate(double result) {
    // Stage
    HttpHeader** headers = malloc(sizeof(HttpHeader*));
    HttpHeader* header0 = malloc(sizeof(HttpHeader));
    char* message;
    char* body;
    char* bodyLen;
    asprintf(&body, "%lf", result);
    asprintf(&bodyLen, "%d", strlen(body));
    // Construct content-length header
    header0->name = "Content-Length";
    header0->value = bodyLen;
    headers[0] = header0;
    int status = 200;
    char* statusExplanation = "OK";
    message = construct_HTTP_response(status, statusExplanation, headers, 
            body);
    free(header0);
    free(headers);
    return message;
}

/*
 *  Parses an HTTP request into a job or query
 *  Params:
 *      char* buffer - HTTP request as a char*
 *      int bufferLen - the length of the HTTP request as a char*
 *  Returns (char*):
 *      message - message back to the client
 */
char* http_request_handler(char* buffer, int bufferLen) {
    char* method = NULL;
    char* address = NULL;
    HttpHeader** headers = NULL;
    char* body = NULL;
    int verbose = 0;
    if (parse_HTTP_request(buffer, bufferLen, &method, &address, &headers, 
            &body) < 0) {
        fprintf(stderr, "http_request_parse: parse_HTTP_request() failed\n");
    }
    if (isprefix("/validate/", address)) { // Validate function
        char* function = split_by_char(address, '/', 0)[2];
        if (function_isvalid(function)) { // Function is valid
            return http_respond_isvalid(1);
        } else { // Function is not valid
            return http_respond_isvalid(0);
        }
    } else if (isprefix("/integrate/", address)) { // Integrate
        char** addressJob = split_by_char(address, '/', 0);
        Job* job = malloc(sizeof(Job));
        job->lower = atof(addressJob[2]);
        job->upper = atof(addressJob[3]);
        job->segments = atoi(addressJob[4]);
        job->threads = atoi(addressJob[5]);
        job->function = addressJob[6];
        return http_respond_integrate(function_integrate_trapezoidal(job));
    } else {
        fprintf(stderr, "http_request_parse: unknown HTTP address\n");
    }
    return NULL;
}

/*
 *  Receives and sends response back to client
 *  Params:
 *      void* clientfdPacked - packed client file descriptor from accept()
 *  Returns (void):
 */
void* client_handler(void* clientfdPacked) {
    // Stage
    int clientfd = *((int*)clientfdPacked);
    free(clientfdPacked);
    // Receive
    int len;
    if (recv(clientfd, &len, sizeof(int), 0) < 0) { // Receive metadata
        fprintf(stderr, "client_handler: recv() failed\n");
    }
    char buffer[len];
    if (recv(clientfd, buffer, len, 0) < 0) { // Receive data
        fprintf(stderr, "client_handler: recv() failed\n");
    }
    char* received;
    asprintf(&received, "%s", buffer);
    // Discard empty requests
    if (strlen(received) < 1) {
        close(clientfd);
        return NULL;
    }
    // Compute
    char* message = http_request_handler(received, strlen(received));
    // Send
    len = strlen(message);
    if (send(clientfd, (char*)&len, sizeof(int), 0) < 0) { // Send metadata
        fprintf(stderr, "socket_send: send() failed\n");
    }
    if (send(clientfd, message, strlen(message), 0) < 0) { // Send data
        fprintf(stderr, "socket_send: send() failed\n");
    }
    close(clientfd);
    return NULL;
}

/*
 *  Spawns threads to handle client accept(). Exits on error.
 *  Params:
 *      int - socket file descriptor
 *      char* maxPorts - argument string for port
 *  Returns (void):
 */
void client_handler_spawner(int sockfd, int maxThreads) {
    pthread_attr_t pthreadAttr;
    pthread_attr_init(&pthreadAttr);
    pthread_attr_setdetachstate(&pthreadAttr, PTHREAD_CREATE_DETACHED);
    while (1) {
        int clientfd;
        if ((clientfd = accept(sockfd, 0, 0)) < 0) {
            fprintf(stderr, "client_handler_spawner: error\n");
            exit(3);
        }
        pthread_t* pthread = malloc(sizeof(pthread_t));
        int* clientfdPacked = malloc(sizeof(int));
        *clientfdPacked = clientfd;
        pthread_create(pthread, &pthreadAttr, client_handler, 
                clientfdPacked);
    }
}

/*
 *  Main logic of intserver. Exits on wrong syntax.
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      0 - ok
 *      1 - usage error
 *      n - unexpected
 */
int main(int argc, char** argv) {
    if (argc < 2) { // If no arguments
        fprintf(stderr, "Usage: intserver portnum [maxThreads]");
        exit(1);
    }
    char* port = argv[1];
    if (!check_arg_port(port)) { // Port number is invalid
        fprintf(stderr, "Usage: intserver portnum [maxThreads]\n");
        exit(1);
    }
    int maxThreads = get_arg_maxthreads(argc, argv); // Get maxThreads
    int sockfd = socket_create(port);
    socket_listen(sockfd);
    client_handler_spawner(sockfd, maxThreads);
    return 0;
}