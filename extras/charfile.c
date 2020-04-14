/*
 * Delete entries from saved character file
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<time.h>

#define I_APPEAL        1
#define I_ARM           2
#define I_CHAR          3
#define I_DEX           4
#define I_HITS          5
#define I_INTEL         6
#define I_STR           7
#define I_WEAP          8
#define I_WEAPENCH      9
#define I_WELL          10
#define I_WIS           11
#define MAXPATT         100              /* max player attributes */
#define MAXPDEF         100              /* max saved characters */

#define ROGDEFS		".rog_defs";

/* character types (I_CHAR) */
#define C_FIGHTER       0
#define C_MAGICIAN      1
#define C_CLERIC        2
#define C_THIEF         3
#define C_MONSTER       4

int
main(argc, argv)
int argc;
char **argv;
{
    int def_array[MAXPDEF][MAXPATT];        /* Pre-defined chars */
    int i, j, k;
    FILE *inf, *outf;
    int num = 0;
    char file[200];
    char *class;

    /*
     * if we can't access a central score file,
     * try the user's home directory instead.
     */
    if (getenv("HOME") != NULL) {
	strcpy(file, getenv("HOME"));
    } else if (getenv("HOMEPATH") != NULL) {
	strcpy(file, getenv("HOMEPATH"));
    }
    strcat(file, "/.rog_defs");

    if (access(file, R_OK|W_OK) != F_OK) {
	fprintf(stderr, "Unable to access character file: %s\n", file);
	exit(1);
    }

    if (argc > 2 || argc < 2
     || (argc > 1 && (argv[1][0] < '0' || argv[1][0] > '9'))
     ) {
	fprintf(stderr, "Usage: %s [n]\n", *argv);
	fprintf(stderr, " E.g. 'charfile 3' delete 3rd entry in character file\n");
	exit(1);
    }

    if (argc == 2) 
	num = atoi(argv[1]);
    
    /*
     * Open file
     */
    if ((inf=fopen(file,"r")) == NULL) {
	fprintf(stderr, "Unable to read %s\n", file);
	perror(file);
	exit(0);
    }

    /*
     * Read the file
     */
    read(fileno(inf), (char *) def_array, sizeof def_array);
    fclose(inf);

    /*
     * Show what we have
     */
    for (i=0; i<MAXPDEF; i++) {
	if (def_array[i][I_INTEL] > 0) {
	    switch(def_array[i][I_CHAR]) {
		case C_FIGHTER:
		    class = "fighter";
		    break;
		case C_MAGICIAN:
		    class = "magician";
		    break;
		case C_CLERIC:
		    class = "cleric";
		    break;
		case C_THIEF:
		    class = "thief";
		    break;
		default:
		    class = "";
		    break;
	    }
	    printf("Int: %d  Str: %d  Wis: %d  Dex: %d  Const: %d  Charisma: %d\n",
		def_array[i][I_INTEL],
		def_array[i][I_STR],
		def_array[i][I_WIS],
		def_array[i][I_DEX],
		def_array[i][I_WELL],
		def_array[i][I_APPEAL]);
	    printf("%s  Armor: %d   Weapon: %d (+%d,+%d)    Hit Points: %d\n\n",
		class,
		def_array[i][I_ARM],
		def_array[i][I_WEAP],
		def_array[i][I_WEAPENCH]/10,
		def_array[i][I_WEAPENCH]%10,
		def_array[i][I_HITS]);
	}
    }

    if (num < 1 || num > 10)
	exit(0);

    /*
     * Delete one
     */
    for (i=0; i<MAXPDEF; i++) {
	if (i == num-1) {
	    if (def_array[i][I_INTEL] > 0) {
		printf("Delete entry %d\n", num);
		for (j=i; j<MAXPDEF-1; j++) {
		    for (k=0; k<MAXPATT; k++) {
			def_array[j][k] = def_array[j+1][k];
		    }
		}
		for (k=0; k<MAXPATT; k++) {  /* clear last entry */
		    def_array[j][k] = 0;
		}
	    }
	}
    }

    /*
     * Update the file
     */
    if ((outf=fopen(file,"w")) == NULL) {
	fprintf(stderr, "Unable to write %s\n", file);
	perror(file);
	exit(0);
    }
    write(fileno(outf), (char *) def_array, sizeof def_array);
    fclose(outf);
}

