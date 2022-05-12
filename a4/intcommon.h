#ifndef INTCOMMON
#define INTCOMMON

#include <csse2310a4.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_MAX 65536
#define PORT_MIN 0
#define SIZE_BUFFER 4096

/*
 * This struct stores a list of strings and size of said list
 */
typedef struct StringArray {
    int size;
    char** strings;
} StringArray;

/*
 * This struct stores a list of the attributes of a single job
 */
typedef struct Job {
    int fileIndex;
    int lineIndex;
    int jobIndex;
    int valid;
    pid_t pid;
    int status;
    char* function;
    double lower;
    double upper;
    int segments;
    int threads;
} Job;

/*
 * This struct stores a list of job lines and the size of said list
 */
typedef struct JobArray {
    int size;
    Job** jobs;
} JobArray;

// Function prototypes from intclient.c

// End function prototypes from intclient.c

// Function prototypes from intserver.c

/*
 *  Gets a socket information struct pointer with localhost and given port
 *  Params:
 *      char* port - argument string for port
 *  Returns (struct sockaddr*):
 *      struct sockaddr* - socket address struct
 */
struct sockaddr* socket_getaddrinfo(char* port);

// End function prototypes from intserver.c

#endif
