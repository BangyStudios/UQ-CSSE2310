#include <csse2310a3.h>                                                                                
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    char* line;
    line = read_line(stdin);
    while(line) {
        char** processed = split_by_commas(line);
        int i = 0;
        while(processed[i]) {
            printf("%d: %s\n", i, processed[i]);
            i++;
        }   

        free(processed);
        free(line);
        line=read_line(stdin);
    }   
    return 0;
}