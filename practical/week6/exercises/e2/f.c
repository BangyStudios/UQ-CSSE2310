
#include <stdlib.h>	// srand and rand come from here
#include <unistd.h>	// this is a POSIX header not a C99 one

int F(void) {
    srand(getuid());
    return rand()%17;
}
