#include "intcommon.h"

#include <csse2310a3.h>

/*
 *  Gets and returns the verbosity flag from the arguments, exits if wrong
 *  usage.
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      1 - verbose
 *      0 - no verbose
 */
int get_arg_verbose(int argc, char** argv) {
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) { // -v exists in wrong position
            fprintf(stderr, "Usage: intclient [-v] portnum [jobfile]\n");
            exit(1);
        }
    }
    if (!strcmp(argv[1], "-v")) { // -v exists in correct position
        return 1;
    } else { // -v does not exist in argument
        return 0;
    }
}

/*
 *  Gets and returns the port number from the arguments
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      port - the porn number
 */
char* get_arg_port(int argc, char** argv, int verbose) {
    if (verbose == 1) {
        return argv[2];
    } else {
        return argv[1];
    }
}

/*
 *  Gets jobfile arguments and count
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (StringArray*):
 *      StringArray* argFiles - a StringArray pointer
 */
StringArray* get_arg_files(int argc, char** argv) {
    // Variable name "a" is used only to save line space
    StringArray* argFiles = malloc(sizeof(StringArray)); // Creates StringArray
    argFiles->size = 0; // Init args size
    argFiles->strings = NULL; // Init args
    int firstIndex;
    if (get_arg_verbose(argc, argv)) { // There is a -v at index 1
        firstIndex = 3;
    } else { // There is no -v at index 1
        firstIndex = 2;
    }
    for (int i = firstIndex; i < argc; i++) { // For each non -v argument
        argFiles->strings = realloc(argFiles->strings,
                sizeof(char*) * (argFiles->size + 1));
        argFiles->strings[argFiles->size++] = strdup(argv[i]); // + to arg list
    }
    return argFiles;
}

/*
 *  Gets a socket information struct pointer with localhost and given port.
 *  Exits if unable to get port.
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
        fprintf(stderr, "intclient: unable to connect to port %s\n", 
                port);
        exit(2);
    }
    return ai;
}

/*
 *  Connects to some server at port/address specified by socket_create()
 *  Exits on failure. 
 *  Params:
 *      int sockfd - socket file descriptor
 *      struct addrinfo* ai - an address informations struct pointer
 *      char* port - argument string for port
 *  Returns (void):
 */
void socket_connect(int sockfd, struct addrinfo* ai, char* port) {
    if (connect(sockfd, (struct sockaddr*)ai->ai_addr, 
            sizeof(struct sockaddr))) {
        fprintf(stderr, "intclient: unable to connect to port %s\n", port);
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
    socket_connect(sockfd, ai, port);
    return sockfd;
}

/*
 *  Parses header struct to single char* line
 *  Params:
 *      HttpHeader* header
 *  Returns (int):
 *      char* line - the parsed line
 */
char* http_header_toline(HttpHeader* header) {
    char* line;
    asprintf(&line, "%s: %s", header->name, header->value);
    return line;
} 

/*
 *  Constructs a HTTP header address
 *  Params:
 *      int mode - 0:validate, 1:integrate;
 *      Job* job - a Job pointer
 *  Returns (char*):
 *      address - the HTTP address
 */
char* http_address_construct(int mode, Job* job) {
    char* address;
    if (!mode) {
        asprintf(&address, "/validate/%s", job->function);
    } else if (mode == 1) {
        asprintf(&address, "/integrate/%lf/%lf/%d/%d/%s", job->lower, 
                job->upper, job->segments, job->threads, job->function);
    } else {
        fprintf(stderr, "http_address_construct: invalid mode\n");
    }
    return address;
}

/*
 *  Constructs a HTTP header as a char*
 *  Params:
 *      char* method - 
 *      char* address -
 *      char* verbose - 
 *      char* body
 *  Returns (char*):
 */
char* http_request_construct(char* method, char* address, char* body, 
        int verbose) {
    // Stage
    char* protocol = "HTTP/1.1";
    HttpHeader** headers = malloc(sizeof(HttpHeader*) * 2);
    HttpHeader* header0 = malloc(sizeof(HttpHeader));
    HttpHeader* header1 = malloc(sizeof(HttpHeader));
    // Construct verbosity header
    header0->name = "X-Verbose";
    if (verbose) {
        header0->value = "yes";
    } else {
        header0->value = "no";
    }
    headers[0] = header0;
    // Construct content-length header
    header1->name = "Content-Length";
    header1->value = "0";
    headers[1] = header1;
    // Construct request string
    char* request;
    asprintf(&request, "%s %s %s\n%s\n%s\n\n%s", method, address, 
            protocol, http_header_toline(headers[0]), 
            http_header_toline(headers[1]), body);
    return request;
}

/*
 *  Sends string to sock socket and prints response
 *  Params:
 *      Socket* sock - a socket database object
 *      char* data - a string
 *  Returns (void):
 */
char* socket_sendreceive(int sockfd, char* data) {
    // Send
    int len = strlen(data);
    if (send(sockfd, (char*)&len, sizeof(int), 0) < 0) { // Send metadata
        fprintf(stderr, "socket_send: send() failed\n");
    }
    if (send(sockfd, data, strlen(data), 0) < 0) { // Send data
        fprintf(stderr, "socket_send: send() failed\n");
    }
    // Receive
    if (recv(sockfd, &len, sizeof(int), 0) < 0) { // Receive metadata
        fprintf(stderr, "client_handler: recv() failed\n");
    }
    char buffer[len];
    if (recv(sockfd, buffer, len, 0) < 0) { // Receive data
        fprintf(stderr, "client_handler: recv() failed\n");
    }
    char* received;
    asprintf(&received, "%s", buffer);
    close(sockfd);
    return received;
}

/*
 *  Parses a numeric value and stores it into a Job* struct
 *  Params:
 *      Job* job - job to parse value to
 *      char* linePart - string to parse
 *      int type - lower 1, upper 2, segments 3 or threads 4
 *  Returns (int):
 *      0 - success
 *      1 - failure
 */
int parse_line_part(Job* job, char* linePart, int type) {
    int tempWhole, tempStatus;
    double tempDecimal;
    for (int i = 0; i < strlen(linePart); i++) { // Contains non-numerical
        if (isalpha(linePart[i])) {
            return 1;
        }
    }
    if (type == 1) { // Parse lower bound
        tempStatus = sscanf(linePart, "%lf", &tempDecimal);
        if (tempStatus == 1) {
            job->lower = tempDecimal;
            return 0;
        } else {
            return 1;
        }
    } else if (type == 2) { // Parse upper bound
        tempStatus = sscanf(linePart, "%lf", &tempDecimal);
        if (tempStatus == 1) {
            job->upper = tempDecimal;
            return 0;
        } else {
            return 1;
        }
    } else if (type == 3) { // Parse segments
        tempStatus = sscanf(linePart, "%d", &tempWhole);
        if (tempStatus == 1 && tempWhole < INT_MAX) {
            job->segments = tempWhole;
            return 0;
        } else {
            return 1;
        }
    } else if (type == 4) { // Parse threads
        tempStatus = sscanf(linePart, "%d", &tempWhole);
        if (tempStatus == 1 && tempWhole < INT_MAX) {
            job->threads = tempWhole;
            return 0;
        } else {
            return 1;
        }
    } else {
        fprintf(stderr, "parse_line_part: invalid field\n"); // Impossible
        return 0;
    }
}

/*
 *  Stores a line in a jobfile into a Job without verifying syntax
 *  Params:
 *      int fileIndex - index of the file in files from 0
 *      int lineIndex - index of the line in file
 *      int jobIndex - index of the line in files
 *      char* line - the line string
 *  Returns (Job*):
 *      Job* job - a Job pointer
 */
Job* read_job_line(int stdin, int fileIndex, int lineIndex, int jobIndex, 
        char* line) {
    Job* job = malloc(sizeof(Job));
    char** linePart = split_by_commas(line);
    job->fileIndex = fileIndex; // Add file number in argFiles
    job->lineIndex = lineIndex; // Add line number in file
    job->jobIndex = jobIndex; // Add job number in jobs
    job->valid = 1; // Innocent until proven guilty
    job->function = malloc(sizeof(char*));
    int linePartIndex = 0;
    while (linePart[linePartIndex]) {
        if (linePartIndex == 0) { // Add program to job & argv
            job->function = strdup(linePart[linePartIndex]);
        } else { // Add lower to job
            if (parse_line_part(job, linePart[linePartIndex], linePartIndex)) {
                fprintf(stderr, "intclient: syntax error on line %d\n",
                        lineIndex);
                job->valid = 0;
                break;
            }
        }
        linePartIndex++; // Increment index in linePart
    }
    if (!stdin) {
        free(line);
    }
    free(linePart);
    if (linePartIndex != 5) {
        fprintf(stderr, "intclient: syntax error on line %d\n", lineIndex);
        job->valid = 0;
    }
    return job; // Return job
}

/*
 *  Returns 1 if char* is empty, 0 otherwise.
 *  Params:
 *      char* line - the line string
 *  Returns (int):
 *      1 - line is empty
 *      0 - line is not empty
 */
int is_empty(char* line) {
    if (line[0] == '\0') {
        return 1;
    } else {
        for (int i = 0; i < strlen(line); i++) {
            if (!isspace(line[i])) {
                return 0;
            }
        }
        return 1;
    }
}

/*
 *  Returns whether there is a valid job or not in given JobArray*
 *  Params:
 *      JobArray* job - an array containing all Job*
 *  Returns (int):
 *      0 - at least 1 valid job
 *      1 - no valid jobs
 */
int empty_job_array(JobArray* jobs) {
    for (int i = 0; i < jobs->size; i++) {
        if (jobs->jobs[i]->valid) {
            return 0;
        }
    }
    return 1;
}

/*
 *  Stores every line in every jobfile into a JobArray without verifying syntax
 *  and exits with status code 0 if empty.
 *  Params:
 *      StringArray* argFiles - a struct containing all the file path(s)
 *  Returns (JobArray*):
 *      JobArray* jobs - a JobArray pointer
 */
JobArray* read_job_file(StringArray* argFiles) {
    if (argFiles->size > 1) { // There exists more than 1 jobfile
        fprintf(stderr, "Usage: intclient [-v] portnum [jobfile]\n");
        exit(1);
    }
    JobArray* jobs = malloc(sizeof(JobArray));
    jobs->jobs = NULL;
    jobs->size = 0;
    int jobIndex = 1; // Unique incrementing job number starting from 1
    int lineIndex;
    for (int fileIndex = 0; fileIndex < argFiles->size; fileIndex++) {
        FILE* jobfile = fopen(argFiles->strings[fileIndex], "r");
        if (jobfile == NULL) { // If file cannot be opened
            fprintf(stderr, "intclient: unable to open \"%s\" for reading\n",
                    argFiles->strings[fileIndex]);
            exit(4);
        }
        Job* job;
        char* line;
        lineIndex = 1;
        while ((line = read_line(jobfile))) { // Loop over lines in a jobfile
            if (is_empty(line)) {
                continue;
            } else if (line[0] == '#') {
                continue;
            }
            job = read_job_line(0, fileIndex, lineIndex++, jobIndex++, line);
            jobs->jobs = realloc(jobs->jobs,
                    sizeof(Job) * (jobs->size + 1));
            jobs->jobs[jobs->size++] = job;
        }
    }
    return jobs;
}

/*
 *  Checks whether a given string contains a space
 *  Params:
 *      char* string - a string
 *  Returns (int):
 *      0 - does not contain a space
 *      1 - does contain a space
 */
int contains_space(char* string) {
    for (int i = 0; i < strlen(string); i++) {
        if (isspace(string[i])) {
            return 1;
        }
    }
    return 0;
}

/*
 *  Checks whether function is valid on server-side
 *  Params:
 *      JobArray jobs - a Job pointers
 *      port - port of socket
 *  Returns (int):
 *      1 - valid
 *      0 - invalid
 */
int check_job_server(Job* job, char* port) {
    int sockfd = socket_create(port);
    char* address = http_address_construct(0, job);
    char* request = http_request_construct("GET", address, "", 0);
    char* received = socket_sendreceive(sockfd, request);
    int status;
    char* statusExplanation;
    HttpHeader** headers;
    char* body;
    parse_HTTP_response(received, strlen(received), &status, 
            &statusExplanation, &headers, &body);
    if (status == 200) {
        return 1;
    } else {
        return 0;
    }
}

/*
 *  Checks the syntax of every Job (line) in a JobArray without.
 *  Prints error message and continues if invalid.
 *  Params:
 *      StringArray* argFiles - a struct containing all the file path(s)
 *      JobArray jobs - a struct containing all the Job pointers
 *      port - port of socket
 *  Returns (void):
 */
void check_job_array(StringArray* argFiles, JobArray* jobs, char* port) {
    for (int i = 0; i < jobs->size; i++) { // Loop over JobArray
        Job* job = jobs->jobs[i];
        if (contains_space(job->function)) { // Function contains space
            fprintf(stderr, "intclient: spaces not permitted in expression "
                    "(line %d)\n", job->lineIndex);
            job->valid = 0;
            continue;
        } else if (job->upper < job->lower) { // Bounds inverted
            fprintf(stderr, "intclient: upper bound must be greater than "
                    "lower bound (line %d)\n", job->lineIndex);
            job->valid = 0;
            continue;
        } else if (job->segments < 1) { // Negative segments
            fprintf(stderr, "intclient: segments must be a positive integer "
                    "(line %d)\n", job->lineIndex);
            job->valid = 0;
            continue;
        } else if (job->threads < 0) { // Negative threads
            fprintf(stderr, "intclient: threads must be a positive integer "
                    "(line %d)\n", job->lineIndex);
            job->valid = 0;
            continue;
        } else if (job->segments % job->threads) { // Segments not divisible
            fprintf(stderr, "intclient: segments must be an integer multiple "
                    "of threads (line %d)\n", job->lineIndex);
            job->valid = 0;
            continue;
        } else if (!check_job_server(job, port)) {
            fprintf(stderr, "intclient: bad expression \"%s\"" 
                    " (line %d)\n", job->function, job->lineIndex);
            job->valid = 0;
            continue;
        }
    }
}

/*
 *  Main logic of intclient for reading from jobfiles. Exits when done.
 *  Params:
 *      int verbose - verbosity
 *      char* port - port number
 *  Returns (void):
 */
void run_jobfile(StringArray* argFiles, int verbose, char* port) {
    // Stage
    int status;
    char* statusExplanation;
    char* body;
    HttpHeader** headers;
    // Read jobfile
    JobArray* jobs = read_job_file(argFiles); // Read jobfiles to JobArray
    int sockfd = socket_create(port);
    check_job_array(argFiles, jobs, port);
    if (empty_job_array(jobs)) {
        exit(0);
    }
    for (int i = 0; i < jobs->size; i++) { // For all jobs
        Job* job = jobs->jobs[i];
        if (job->valid) {
            char* address = http_address_construct(1, job);
            char* request = http_request_construct("GET", address, "", 
                    verbose);
            sockfd = socket_create(port);
            char* received = socket_sendreceive(sockfd, request);
            parse_HTTP_response(received, strlen(received), &status, 
                    &statusExplanation, &headers, &body);
            fprintf(stdout, "The integral of %s from %lf to %lf is "
                    "%s\n", job->function, job->lower, job->upper, body);
        } else {
            continue;
        }
    }
    exit(0); // Exit when done.
}

/*
 *  Main logic of intclient for reading from stdin. Exits when done.
 *  Params:
 *      int verbose - verbosity
 *      char* port - port number
 *  Returns (void):
 */
void run_stdin(int verbose, char* port) {
    // Stage
    int status;
    char* statusExplanation;
    char* body;
    HttpHeader** headers;
    // Accept input
    char* input;
    int sockfd = socket_create(port);
    while (1) {
        if (scanf("%s", input)) {
            Job* job = read_job_line(1, 0, 1, 1, input);
            fprintf("%s\n", job->function);
            if (check_job_server(job, port)) {
                char* address = http_address_construct(1, job);
                char* request = http_request_construct("GET", address, "", 
                        verbose);
                int sockfd = socket_create(port);
                char* received = socket_sendreceive(sockfd, request);
                parse_HTTP_response(received, strlen(received), &status, 
                        &statusExplanation, &headers, &body);
                fprintf(stdout, "The integral of %s from %lf to %lf is %s\n", 
                        job->function, job->lower, job->upper, body);
            } else {
                fprintf(stderr, "intclient: bad expression \"%s\"" 
                        " (line %d)\n", job->function, job->lineIndex);
                job->valid = 0;
                exit(0);
            }
        }
    }
}

/*
 *  Main logic of intclient
 *  Params:
 *      int argc - size of argv
 *      int argv - user-input arguments
 *  Returns (int):
 *      0 - ok
 *      1 - usage error
 *      n - unexpected
 */
int main(int argc, char** argv) {
    // Stage
    if (argc < 2) { // If no arguments
        fprintf(stderr, "Usage: intclient [-v] portnum [jobfile]\n");
        exit(1);
    }
    int verbose = get_arg_verbose(argc, argv); // Get verbosity setting
    char* port = get_arg_port(argc, argv, verbose); // Get port number
    // From stdin or jobfile(s)?
    StringArray* argFiles = get_arg_files(argc, argv); // Get jobfile list
    if (argFiles->size) {
        run_jobfile(argFiles, verbose, port);
    } else {
        run_stdin(verbose, port);
    }
    return 0;
}
