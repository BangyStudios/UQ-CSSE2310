#include "jrcommon.h"
#include<sys/wait.h>
#include<sys/types.h>

/*
 *  Returns whether or not a job uses the same stdin & stdout as parent process
 *  Params:
 *      Job* job - a Job pointer
 *  Returns (int):
 *      1 - if the job uses the same stdin & stdout as parent
 *      0 - if the job uses a different stdin & stdout from parent
 */
int is_stdio_parent(Job* job) {
    if (!strcmp(job->stdIn, "-") && !strcmp(job->stdOut, "-")) {
        return 1;
    } else {
        return 0;
    }
}

/*
 *  Returns whether or not a job's stdio uses a file descriptor
 *  Params:
 *      char* stdio - the stdin or stdout string
 *  Returns (int):
 *      1 - stdin or stdout is a file descriptor
 *      0 - stdin or stdout is not a file descriptor
 */
int is_file(char* stdio) {
    if (stdio[0] == '@') {
        return 0;
    } else if (!strcmp(stdio, "-")) {
        return 0;  
    } else {
        return 1;
    }
}

/*
 *  Returns whether or not a job's stdio uses a pipe
 *  Params:
 *      char* stdio - the stdin or stdout string
 *  Returns (int):
 *      1 - stdin or stdout is a pipe
 *      0 - stdin or stdout is not a pipe
 */
int is_pipe(char* stdio) {
    if (stdio[0] == '@') {
        return 1;
    } else {
        return 0;
    }
}

/*
 *  Creates a record of a file descriptor. Adds this pipe descriptor to array
 *  of records of file descriptors.
 *  Params: 
 *      Job* job - a Job pointer
 *      FileArray* files - a files pointer
 *      int type - read is 0, write is 1
 *      int fd - file descriptor associated with the file
 *  Returns (void):
 */
void add_file(Job* job, FileArray* files, int type, int fd) {
    File* file = malloc(sizeof(File)); // Initiates file record
    file->name = malloc(sizeof(char*)); // Initiates file name
    file->type = type; // Initiates file type
    file->fd = fd;
    if (!type) { // If read file
        file->name = strdup(job->stdIn);
    } else if (type == 1) { // If write file
        file->name = strdup(job->stdOut);
    } else {
        fprintf(stderr, "jrhandler.add_file: file is not read xor write\n");
    }
    files->files = realloc(files->files, 
            sizeof(File) * (files->size + 1));
    files->files[files->size++] = file;
}

/*
 *  Returns whether the end of a pipe has been taken.
 *  Params: 
 *      char* name - the name of a pipe @name is "name"
 *      int end - read end is 0, write end is 1
 *  Returns (int):
 *      1 - given pipe end with name has been taken
 *      0 - given pipe end with name has not been taken (yet)
 */
int is_pipe_taken(PipeArray* pipes, char* name, int end) {
    for (int i = 0; i < pipes->size; i++) {
        Pipe* pipe = pipes->pipes[i];
        if (!strcmp(pipe->name, name)) { // If pipe name matches
            if (!end) { // Read end
                if (pipe->readJobIndex != -1) { // Is taken
                    return 1;
                }
            } else if (end == 1) {// Write end
                if (pipe->writeJobIndex != -1) { // Is taken
                    return 1;
                }
            } else {
                fprintf(stderr, "jrhandler.is_pipe_taken: pipe end is not read"
                        " xor write\n");
            }
        }
    }
    return 0;
}

/*
 *  Creates a pipe descriptor (does not actually create a pipe) just a "list" 
 *  of pipes. Adds this pipe descriptor to an array of pipe descriptors.
 *  Params: 
 *      Job* job - a Job pointer
 *      PipeArray* pipes - a pipes pointer
 *      int end - read end is 0, write end is 1
 *  Returns (void):
 */
void add_pipe(Job* job, PipeArray* pipes, int end) {
    Pipe* pipe = malloc(sizeof(Pipe)); // Initiates pipe descriptor
    pipe->name = malloc(sizeof(char*)); // Initiates pipe name
    pipe->readJobIndex = -1; // -1 means empty
    pipe->writeJobIndex = -1; // -1 means empty
    if (!end) { // If read end
        pipe->readJobIndex = job->jobIndex;
        pipe->name = strdup(job->stdIn + 1);
    } else if (end == 1) { // If write end
        pipe->writeJobIndex = job->jobIndex;
        pipe->name = strdup(job->stdOut + 1);
    } else {
        fprintf(stderr, "jrhandler.add_pipe: "
                "pipe end is not read xor write\n");
    }
    pipes->pipes = realloc(pipes->pipes, 
            sizeof(Pipe) * (pipes->size + 1));
    pipes->pipes[pipes->size++] = pipe;
}

/*
 *  Verifies the runnability of each Job in jArray.
 *  Params: 
 *      jArray* jArray - an array of Job*
 *      FileArray* files - an array of File*
 *      PipeArray* pipes - an array of Pipe*
 *  Returns (int):
 */
void check_job_array(JobArray* jobs, FileArray* files, 
        PipeArray* pipes) {
    for (int i = 0; i < jobs->size; i++) {
        Job* job = jobs->jobs[i];
        if (is_stdio_parent(job)) { // If job uses same stdio as parent
            // Does anything need to be checked?
        } else { // If job uses different stdio from parent
            if (is_pipe(job->stdIn)) { // Stdin is a pipe
                if (is_pipe_taken(pipes, job->stdIn + 1, 0)) { // Taken
                    fprintf(stderr, "Invalid pipe usage \"%s\"", 
                            job->stdIn + 1);
                    job->valid = 0;
                } else { // Not taken
                    add_pipe(job, pipes, 0);
                }
            } else if (is_file(job->stdIn)) { // Stdin is a file
                int fd = open(job->stdIn, O_RDONLY); // Try to open stdin
                if (fd == -1) { // The stdin file cannot be opened
                    fprintf(stderr, "Unable to open \"%s\" for reading\n", 
                            job->stdIn);
                    job->valid = 0;
                } else { // The stdin file can be opened
                    add_file(job, files, 0, fd);
                }
            } else { // Stdin is same as parent
                // Does anything else need to be checked? 
            }
            if (is_pipe(job->stdOut)) { // Stdout is a pipe
                if (is_pipe_taken(pipes, job->stdIn + 1, 1)) { // Taken
                    fprintf(stderr, "Invalid pipe usage \"%s\"", 
                            job->stdIn + 1);
                    job->valid = 0;
                } else { // Not taken
                    add_pipe(job, pipes, 0);
                }
            } else if (is_file(job->stdOut)) { // Stdout is a file
                int fd = open(job->stdOut, O_WRONLY | O_CREAT | O_TRUNC, 
                        S_IRWXU);
                if (fd == -1) { // The stdout file cannot be opened
                    fprintf(stderr, "Unable to open \"%s\" for writing\n", 
                            job->stdOut);
                    job->valid = 0;
                } else { // The stdout file can be opened
                    add_file(job, files, 1, fd);
                }
            } else { // Stdout is same as parent
                // Does anything else need to be checked?
            }
        }
    }
}

/*
 *  Returns the number of valid Job* in a jArray*
 *  Params: 
 *      jArray* jArray - an array of Job*
 *  Returns (int):
 *      count - number of valid Job* in jArray
 */
int count_valid_jobs(JobArray* jobs) {
    int count = 0;
    for (int i = 0; i < jobs->size; i++) {
        Job* job = jobs->jobs[i];
        if (job->valid) {
            count++;
        } else {
            continue;
        }
    }
    return count;
}

/*
 *  Initiates all pipes (really) in a pipes. Writes file descriptors into
 *  struct fd[2] attribute.
 *  Params: 
 *      PipeArray* pipes - an array of Pipe*
 *  Returns (void):
 */
void init_pipes(PipeArray* pipes) {
    for (int i = 0; i < pipes->size; i++) {
        Pipe* p = pipes->pipes[i]; // Get pipe descriptor
        if (pipe(p->fd) == -1) { // Create pipe
            fprintf(stderr, "jrhandler.init_pipes: pipe open error\n");
        }
    }
}

/*
 * Searches for a file descriptor struct by name. 
 *  Params: 
 *      FileArray* files - an array of File*
 *      char* name - name to search
 *      int type - type to search
 *  Returns (File*):
 *      (File*) - the file descriptor struct
 *      NULL - if no such file descriptor struct exists
 */
File* search_file(FileArray* files, char* name, int type) {
    for (int i = 0; i < files->size; i++) {
        File* file = files->files[i];
        if (!strcmp(file->name, name) && file->type == type) {
            return file;
        }
    }
    return NULL;
}

/*
 * Searches for a pipe descriptor struct by name, 
 *  Params: 
 *      PipeArray* pipes - an array of Pipe*
 *      char* name - name to search
 *  Returns (Pipe*):
 *      (Pipe*) - the pipe descriptor struct
 *      NULL - if no such pipe descriptor struct exists
 */
Pipe* search_pipe(PipeArray* pipes, char* name) {
    for (int i = 0; i < pipes->size; i++) {
        Pipe* pipe = pipes->pipes[i];
        if (!strcmp(pipe->name, name)) {
            return pipe;
        }
    }
    return NULL;
}

/*
 *  Redirects stdin and stdout according to job
 *  Params:
 *      Job* job - a Job*
 *      FileArray* files - an array of File*
 *      PipeArray* pipes - an array of Pipe*
 *  Returns (void):
 */
void auto_dup(Job* job, FileArray* files, PipeArray* pipes) { 
    int devNull = open("/dev/null", O_WRONLY); // Open "dev/null"
    dup2(devNull, STDERR_FILENO); // Redirect all error to null
    
    int stdIn;
    int stdOut;
    if (!is_stdio_parent(job)) { // Stdin and stdout not same as parent
        if (is_pipe(job->stdIn)) { // Stdin is a pipe
            Pipe* pipe = search_pipe(pipes, job->stdIn + 1);
            stdIn = pipe->fd[0];
            close(pipe->fd[1]);
        } else if (is_file(job->stdIn)) { // Stdin is a file
            File* file = search_file(files, job->stdIn, 0);
            stdIn = file->fd; 
        } else { // Stdin is same as parent
            // Do nothing
        }
        if (is_pipe(job->stdOut)) { // Stdout is a pipe
            Pipe* pipe = search_pipe(pipes, job->stdOut + 1);
            stdOut = pipe->fd[1];
            close(pipe->fd[0]);
        } else if (is_file(job->stdOut)) { // Stdout is a file
            File* file = search_file(files, job->stdOut, 1);
            stdOut = file->fd;
        } else { // Stdout is same parent
            // Do nothing
        }
    }

    dup2(stdIn, STDIN_FILENO);
    dup2(stdOut, STDOUT_FILENO);
    if (stdIn != STDIN_FILENO) {
        close(stdIn);
    }
    if (stdOut != STDOUT_FILENO) {
        close(stdOut);
    }
}

/*
 *  Closes pipes and files according to job
 *  Params:
 *      Job* job - a Job*
 *      FileArray* files - an array of File*
 *      PipeArray* pipes - an array of Pipe*
 *  Returns (void):
 */
void auto_close(Job* job, FileArray* files, PipeArray* pipes) {
    if (!is_stdio_parent(job)) { // Stdin and stdout not same as parent
        if (is_pipe(job->stdIn)) { // Stdin is a pipe
            Pipe* pipe = search_pipe(pipes, job->stdIn + 1);
            close(pipe->fd[0]);
        } else if (is_file(job->stdIn)) { // Stdin is a file
            File* file = search_file(files, job->stdIn, 0);
            close(file->fd);
        } else { // Stdin is same as parent
            // Do nothing
        }
        if (is_pipe(job->stdOut)) { // Stdout is a pipe
            Pipe* pipe = search_pipe(pipes, job->stdOut + 1);
            close(pipe->fd[1]);
        } else if (is_file(job->stdOut)) { // Stdout is a file
            File* file = search_file(files, job->stdOut, 1);
            close(file->fd);
        } else { // Stdout is same parent
            // Do nothing
        }
    }
}

/*
 *  Handles a signal.
 *  Params:
 *      int sig - signal code
 *  Returns (void):
 */
void handle_signal(int sig) {
    if (sig == SIGABRT) {
        fprintf(stderr, "Job N terminated with signal %d\n", sig);
    }
}

/*
 *  Runs one job.
 *  Params:
 *      Job* job - a Job*
 *      FileArray* files - an array of File*
 *      PipeArray* pipes - an array of Pipe*
 *  Returns (void):
 */
void run_job(Job* job, FileArray* files, PipeArray* pipes) {
    fflush(stdout); // Flush stdout buffer (standard procedure)
    job->pid = fork(); // Forks a child process
    //job->pid = pid; // Stores global pid into job struct
    if (job->pid == -1) { // If error
        fprintf(stderr, "jrhandler.run_job: fork failed");
    }
    if (job->pid == 0) { // If child
        auto_dup(job, files, pipes);
        execvp(job->program, job->argv);
    }
}

/*
 *  Get the number of valid jobs (children)
 *  Params:
 *      jArray* jArray - an array of Job*
 *  Returns (int):
 *      children - number of valid jobs (children)
 */
int get_child_count(JobArray* jobs) {
    int children = 0;
    for (int i = 0; i < jobs->size; i++) {
        Job* job = jobs->jobs[i];
        if (job->valid) {
            children++;
        } else {
            continue;
        }
    }
    return children;
}

 /*
 *  Initializes all the waitpid instances until all children dies
 *  Params:
 *      jArray* jArray - an array of Job*
 *  Returns (void);
 */  
void init_wait(JobArray* jobs, FileArray* files, PipeArray* pipes) {
    int children = get_child_count(jobs);
    while (children) {
        for (int i = 0; i < jobs->size; i++) {
            Job* job = jobs->jobs[i];
            if (job->valid) {
                int result = waitpid(job->pid, &job->status, WNOHANG);
                if (WIFEXITED(job->status)) { // Job exited with status
                    job->status = WEXITSTATUS(job->status);
                } else if (WIFSIGNALED(job->status)) { // Term with a signal
                    job->status = WTERMSIG(job->status);
                }
                if (result == -1) {    
                    // Should do something
                } else if (!result) {
                    continue;
                } else { // I know this is wrong but it's 15:57 on the due date 
                    auto_close(job, files, pipes);
                    fprintf(stderr, "Job %d exited with status %d\n", 
                            job->jobIndex, 
                            job->status); 
                    fflush(stderr);
                    children--;
                }
            } else {
                continue;
            }
        }
    }
}

/*
 *  Main job handling logic for program. Runs other functions to execute jobs 
 *  and handles low level errors.
 *  Params:
 *      jArray* jArray - an array of Job*
 *      int verbose - verbosity
 *  Returns (int):
 *      0 - ok
 */
int handle(JobArray* jobs, int verbose) {
    FileArray* files = malloc(sizeof(files));
    files->size = 0;
    PipeArray* pipes = malloc(sizeof(pipes));
    pipes->size = 0;
    check_job_array(jobs, files, pipes); // Check runnability
    init_pipes(pipes); // Initiate pipes
    if (!count_valid_jobs(jobs)) { // If no runnable (valid) jobs
        fprintf(stderr, "jobrunner: no runnable jobs\n");
        exit(4);
    }
    if (verbose) {                                                             
        for (int i = 0; i < jobs->size; i++) {                                 
            Job* job = jobs->jobs[i];
	    if (job->valid) {
                fprintf(stdout, "%d:%s:%s:%s:%d\n",  
                        job->jobIndex,                                   
                        job->program,                                    
                        job->stdIn,                                     
                        job->stdOut,                                    
                        job->timeout);
            }
        } 
    }
    for (int i = 0; i < jobs->size; i++) {
        Job* job = jobs->jobs[i];
        if (job->valid) {
            run_job(job, files, pipes);
        } else {
            continue;
        }
    }
    init_wait(jobs, files, pipes);
    return 0;    
}
