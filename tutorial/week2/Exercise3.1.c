#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct String {
	char* array;
	int size;
} String;

int my_str_len(char* string) {

    int length = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        length++;
    }
    return length;

}

char* my_mem_cpy(char* orig) {
    
    char* array = malloc(sizeof(char) * my_str_len(orig));
    for (int i = 0; orig[i] != '\0'; i++) {
        array[i] = orig[i];
    }
    return array;

}

String* init_defStr(char* string) {
    
    String* str = malloc(sizeof(String)); // Always malloc() during init
    str->array = my_mem_cpy(string);
    str->size = my_str_len(string); // Avoid using strcpy()

    return str;

}

void append_defStr(String* orig, String extra) {
 
    orig->size += extra.size;
    strcat(orig->array, extra.array);

}

int main(int argc, char** argv) {

    String* orig = init_defStr("CSSE");
    String* extra = init_defStr("2310");
      
    append_defStr(orig, *extra);

    printf("%s\n", orig->array);
    printf("%d\n", orig->size);

    return 0;

}

