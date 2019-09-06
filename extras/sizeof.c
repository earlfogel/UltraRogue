/* Show size of various things */

#include <stdio.h>

int main(void) {
        printf("short %d bytes\n", sizeof (short int));
        printf("int   %d bytes\n", sizeof (int));
        printf("long  %d bytes\n", sizeof (long int));
        printf("char* %d bytes\n", sizeof (char *));
}
