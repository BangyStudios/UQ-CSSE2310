#include<stdio.h>

struct Person {
    char* name;
    int age;
    float mass;
    float height;
}; 

float get_bmi(float mass, float height) {
	return (mass / (height * height));
}

int main(void) {
	printf("%f\n", get_bmi(75, 1.82));
	
	return 0;
}

	