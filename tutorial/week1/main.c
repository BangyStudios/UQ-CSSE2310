#include <stdio.h>

// CSSE2310/CSSE7231 assessment items are worth
// A1: 15%, A2: 10%, A3: 20%, A4: 20%, Final Exam (FE): 35%

struct marks {
    int assignments[4];
    int exam;
};

// Calculate a CSSE2310/7231 final mark 
double calc_marks(struct marks values);

int main(int argc, char** argv) {
    struct marks inputs;
    inputs.assignments[0] = 30;	// assignment marks out of 60
    inputs.assignments[1] = 43;
    inputs.assignments[2] = 32;
    inputs.assignments[3] = 41;
    inputs.exam = 56;		// final exam out of 70

    printf("Final mark: %f\n", calc_marks(inputs));
    return 0;
}

// Calculate a CSSE2310/7231 final mark 
double calc_marks(struct marks values) {
    double totalA = 0;
    for (int i = 0; i < 4; i++) {
	totalA += values.assignments[i];
	// same as totalA = totalA + values.asignments[i];
    }
    double totalE = values.exam/2.0;
    double total = totalA + totalE;
    // Check 40% hurdles
    double assignmentHurdle = 0.4 * 65;
    double examHurdle = 0.4 * 35;
    int applyGradeCap;
    if (totalA < assignmentHurdle || totalE < examHurdle) {
	applyGradeCap = 1;
    } else {
	applyGradeCap = 0;
    }
    return (applyGradeCap == 1 && total > 49) ? 49 : total;
}

