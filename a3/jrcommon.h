#ifndef JRCOMMON
#define JRCOMMON

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<fcntl.h>
#include<errno.h>

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
    char* program;
    char* stdIn;
    char* stdOut;
    int timeout;
    int argc;
    char** argv;
} Job;

/*
 * This struct stores a list of job lines and the size of said list
 */
typedef struct JobArray {
    int size;
    Job** jobs;
} JobArray;

/*
 * This struct stores a list of the attributes of an open file
 */
typedef struct File {
    char* name; // Name/path of the file
    int type; // 0 is read, 1 is write
    int fd;
} File;

/*
 * This struct stores a list of file descriptors and the size of said list
 */
typedef struct FileArray {
    int size;
    File** files;
} FileArray;

/*
 * This struct stores a list of the attributes of a single pipe
 */
typedef struct Pipe {
    char* name;
    int readJobIndex;
    int writeJobIndex;
    int fd[2];
} Pipe;

/*
 * This struct stores a list of pipe descriptors and the size of said list
 */
typedef struct PipeArray {
    int size;
    Pipe** pipes;
} PipeArray;
    

// Function prototypes from jrbase.c

// End function prototypes from jrbase.c

// Function prototypes from jrhandler.c
int handle(JobArray* jobs, int verbose);
// End function prototypes from jrhandler.c

#endif
