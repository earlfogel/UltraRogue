/* Show size of various things */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
        printf("short %d bytes\n", sizeof (short int));
        printf("int   %d bytes\n", sizeof (int));
        printf("long  %d bytes\n", sizeof (long int));
        printf("char* %d bytes\n", sizeof (char *));
        printf("\n");
        printf("RAND_MAX %ld\n", RAND_MAX);
        printf("INT_MAX %ld\n", INT_MAX);
        printf("LONG_MAX %ld\n", LONG_MAX);
}
