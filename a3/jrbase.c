#include "jrcommon.h"
#include<csse2310a3.h>

/*
 * Gets and returns the verbosity flag from the arguments
 */
int get_verbosity(int argc, char** argv) {
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) { // -v exists in wrong position
            fprintf(stderr, "Usage: jobrunner [-v] jobfile [jobfile ...]\n");
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
 * Gets jobfile arguments and count
 */
StringArray* get_arg_files(int argc, char** argv) {
    // Variable name "a" is used only to save line space
    StringArray* argFiles = malloc(sizeof(StringArray)); // Creates StringArray
    argFiles->size = 0; // Init args size
    argFiles->strings = NULL; // Init args 
    int firstIndex;
    if (get_verbosity(argc, argv)) { // There is a -v at index 1
        firstIndex = 2;
    } else { // There is no -v at index 1
        firstIndex = 1;
    }
    for (int i = firstIndex; i < argc; i++) { // For each non -v argument
        argFiles->strings = realloc(argFiles->strings, 
                sizeof(char*) * (argFiles->size + 1));
        argFiles->strings[argFiles->size++] = strdup(argv[i]); // + to arg list
    }
    return argFiles;
}

/*
 * Stores a line in a jobfile into a Job without verifying syntax
 */
Job* read_job_line(int fileIndex, int lineIndex, int jobIndex, char* line) {
    Job* job = malloc(sizeof(Job));
    char** linePart = split_by_commas(line);
    job->fileIndex = fileIndex; // Add file number in argFiles
    job->lineIndex = lineIndex; // Add line number in file
    job->jobIndex = jobIndex; // Add job number in jobs
    job->valid = 1; // Innocent until proven guilty
    job->program = malloc(sizeof(char*));
    job->stdIn = malloc(sizeof(char*));
    job->stdOut = malloc(sizeof(char*));
    job->timeout = 0;
    job->argc = 1; // Init argv size
    job->argv = malloc(sizeof(char*) * job->argc); // Init argv
    int linePartIndex = 0;
    while (linePart[linePartIndex]) {
        if (linePartIndex == 0) { // Add program to job & argv
            job->program = strdup(linePart[linePartIndex]);
            job->argv[0] = strdup(linePart[linePartIndex]); 
        } else if (linePartIndex == 1) { // Add stdin to job
            job->stdIn = strdup(linePart[linePartIndex]);
        } else if (linePartIndex == 2) { // Add stdout to job
            job->stdOut = strdup(linePart[linePartIndex]);
        } else if (linePartIndex == 3) { // Add timeout to job
            char* timeout = linePart[linePartIndex];
            job->timeout = atoi(linePart[linePartIndex]);
            for (int i = 0; timeout[i] != '\0'; i++) {
                if (isalpha(timeout[i])) { // If non alphabetical
                    job->timeout = -1;
                    break;
                }
            }
        } else { // Is an argv
            job->argv = realloc(job->argv, sizeof(char*) * (job->argc + 1));
            job->argv[job->argc] = strdup(linePart[3 + job->argc]); // +argv
            job->argc++;
        }
        linePartIndex++; // Increment index in linePart
    }
    job->argv = realloc(job->argv, sizeof(char*) * (job->argc + 1));
    job->argv[job->argc] = NULL; // Append NULL pointer to argv for exec
    free(line);
    free(linePart);
    return job; // Return job
}

/*
 * Returns 1 if char* is empty, 0 otherwise.
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
 * Stores every line in every jobfile into a JobArray without verifying syntax
 */
JobArray* read_job_file(StringArray* argFiles) {
    JobArray* jobs = malloc(sizeof(JobArray));
    jobs->jobs = NULL;
    jobs->size = 0;
    int jobIndex = 1; // Unique incrementing job number starting from 1
    int lineIndex;
    for (int fileIndex = 0; fileIndex < argFiles->size; fileIndex++) {
        FILE* jobfile = fopen(argFiles->strings[fileIndex], "r");
        if (jobfile == NULL) { // If file cannot be opened
            fprintf(stderr, "jobrunner: file \"%s\" can not be opened\n", 
                    argFiles->strings[fileIndex]);
            exit(2);
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
            job = read_job_line(fileIndex, lineIndex++, jobIndex++, line);
            jobs->jobs = realloc(jobs->jobs, 
                    sizeof(Job) * (jobs->size + 1));
            jobs->jobs[jobs->size++] = job;
        }
    }
    return jobs;
}

/*
 * Parses (verifies) the syntax of every Job (line) in a JobArray without 
 * verifying runnability
 */
void parse_job_array(StringArray* argFiles, JobArray* jobs) {
    for (int i = 0; i < jobs->size; i++) { // Loop over JobArray
        Job* job = jobs->jobs[i];
        if (!strcmp(job->program, "") || !strcmp(job->stdIn, "") || 
                !strcmp(job->stdOut, "")) { // Mandatory empty
            fprintf(stderr, "jobrunner: invalid job specification on line " 
                    "%d of \"%s\"\n", job->lineIndex, 
                    argFiles->strings[job->fileIndex]);
            exit(3);
        } else if (job->timeout < 0) { // Timeout invalid
            fprintf(stderr, "jobrunner: invalid job specification on line " 
                    "%d of \"%s\"\n", job->lineIndex, 
                    argFiles->strings[job->fileIndex]);
            exit(3);
        }
    }
}

/*
 * Accepts arguments and calls other functions to complete task
 */
int main(int argc, char** argv) {
    if (argc < 2) { // If no arguments
        fprintf(stderr, "Usage: jobrunner [-v] jobfile [jobfile ...]\n");
        exit(1);
    }
    int verbose = get_verbosity(argc, argv); // Get verbosity setting
    StringArray* argFiles = get_arg_files(argc, argv); // Get argv jobfile list
    if (!argFiles->size) {
        fprintf(stderr, "Usage: jobrunner [-v] jobfile [jobfile ...]\n");
        exit(1);
    }
    JobArray* jobs = read_job_file(argFiles); // Read jobfiles to JobArray
    parse_job_array(argFiles, jobs);
    handle(jobs, verbose); // Handle in jrhandler.c
    return 0;
}
