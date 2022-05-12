#include <stdio.h>

int main (int argc, char** argv) {

	if (argc != 4) {
		printf("Argument vector length mismatch\n");
	}

	int num1 = atoi(argv[2]);

	int num2 = atoi(argv[3]);

	int numout = 0;

	if (num1 < 1 || num2 < 1) {
		printf("Non-positive integer\n");
	}

	printf("The operator is '%s' \n", argv[1]);

	if (strcmp(argv[1], "+") == 0) {
		numout = num1 + num2;
	} else if (strcmp(argv[1], "-") == 0) {
		numout = num1 - num2;
	} else if (strcmp(argv[1], "*") == 0) {
		numout = num1 * num2;
	} else if (strcmp(argv[1], "/") == 0) {
		numout = num1 / num2;
	} else {
		printf("Unknown operator\n");
	}

	printf("The number is %d \n" , numout);
	
	return 0;
}
