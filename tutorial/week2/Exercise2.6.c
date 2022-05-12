#include<stdio.h>

int* setValue(int* arg) {
    int num = 5;
    *arg = num;
    return arg;
}

int main(void) {
    int num = 10;

    printf("Original: %d\n", num);
    setValue(&num);
    printf("Changed: %d\n", num);

    return 0;
}
