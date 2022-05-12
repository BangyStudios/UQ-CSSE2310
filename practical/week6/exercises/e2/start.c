#include <stdio.h>
#include "f.h"
#include "g.h"

int main(int argc, char** argv) {
    printf("Starting ...");
    printf("%d %s", F(), G());
    return 0;
}
