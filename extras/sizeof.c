/* Show size of various things */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#define long int64_t

int main(void) {
        printf("short %lu bytes\n", sizeof (short int));
        printf("int   %lu bytes\n", sizeof (int));
        printf("long  %lu bytes\n", sizeof (long));
        printf("char* %lu bytes\n", sizeof (char *));
        printf("int64_t %lu bytes\n", sizeof (int64_t));
        printf("\n");
        printf("RAND_MAX %d\n", RAND_MAX);
        printf("INT_MAX %d\n", INT_MAX);
        printf("LONG_MAX %ld\n", LONG_MAX);
}
