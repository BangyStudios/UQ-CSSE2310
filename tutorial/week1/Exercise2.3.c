#include<stdio.h>

int get_largest(int arg1, int arg2) {

    int same = (arg1 == arg2) ? 1 : 0;
    int largest_arg = (arg1 > arg2 && arg1 != arg2) ? 1 : 2;
    if (!same) {
        if (largest_arg = 1) {
            return arg1;
        } else if (largest_arg = 2) {
            return arg2;
        } else {
            printf("Some weird crap is going on here :(");
        } 
    } else {
        printf("The arguments (%d, %d) are the same\n", arg1, arg2);
    }
}

int main(int argc, char** argv) {

    printf("%d\n", get_largest(1, 3));
    printf("%d\n", get_largest(50,1));
    printf("%d\n", get_largest(5, 5));
    return 0;

}
