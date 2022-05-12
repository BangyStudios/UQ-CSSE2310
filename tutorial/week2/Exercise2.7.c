// Dynamic array creation and manipulation

#include<stdio.h>

int main(int argc, char** argv) {

    int *array = NULL;
    
    array = malloc(sizeof(int) * 10);
    printf("The initial array has been created of size 10\n");
    
    for (int i = 0; i < 10; i++) {   
        array[i] = i + 1;
        printf("%d has been added to initial array\n", array[i]);
    }

    array = realloc(array, sizeof(int) * 11); // old values moved to new alloc
    printf("The initial array has been resized to 11\n");
    array[10] = 11;

    int sum;
    for (int i = 0; i < 11; i++) {
        sum = sum + array[i];    
        printf("%d has been added to sum\n", array[i]);
    }

    printf("The final sum is %d\n", sum);
    
    free(array);

    return 0;
}

