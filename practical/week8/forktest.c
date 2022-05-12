#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char* argv) {

    if (!fork()) {
        fprintf(stdout, "Yes daddy!\n");
    } else {
        fprintf(stdout, "Hi, I like children a lot!\n");
        while (wait(NULL) >= 0);
        fprintf(stdout, "The child has died and I'm now sad.\n");
    }

}
