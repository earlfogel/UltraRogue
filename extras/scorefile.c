/*
 * Delete entries from top score file
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<time.h>
#include 	"tunable.h"

#define SCOREFILE "/usr/local/lib/urogue/LIB/scorefile";

int
main(argc, argv)
int argc;
char **argv;
{
    static struct sc_ent {
        long sc_score;
        char sc_name[76];
        long sc_gold;
        int sc_flags;
        int sc_level;
        short sc_artifacts;
        short sc_monster;
        int sc_game_id;
    } top_ten[10];
    struct sc_ent *scp;
    int i;
    struct sc_ent *sc2;
    FILE *inf, *outf;

    int num = 0;
    char *scorefile = SCOREFILE;
    char buf[200];

    /*
     * if we can't access a central score file,
     * try the user's home directory instead.
     */
#ifdef _WIN32
    if (getenv("APPDATA") != NULL) {
	strcpy(buf, getenv("APPDATA"));
	strcat(buf, "/urogue/rogue.score");
	scorefile = buf;
    }
#else
    if (access(scorefile, R_OK|W_OK) != 0) {
	if (getenv("HOME") != NULL) {
	    strcpy(buf, getenv("HOME"));
	    strcat(buf, "/.rog_score");
	    scorefile = buf;
	}
    }
#endif

    if (argc > 2 || (argc > 1 && (argv[1][0] < '0' || argv[1][0] > '9'))) {
	fprintf(stderr, "Usage: %s [n]\n", *argv);
	fprintf(stderr, " E.g. 'scorefile 3' delete 3rd entry in scorefile\n");
	exit(1);
    }

    if (argc == 2) 
	num = atoi(argv[1]);
    
    /*
     * Open file
     */
    if ((inf=fopen(scorefile,"r")) == NULL) {
	fprintf(stderr, "Unable to read %s\n", scorefile);
	perror(scorefile);
	exit(0);
    }

    /*
     * Initialize list
     */
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
        scp->sc_score = 0L;
	scp->sc_name[0] = '\0';
        scp->sc_gold = 0L;
        scp->sc_flags = 0;
        scp->sc_level = 0;
        scp->sc_monster = 0;
        scp->sc_artifacts = 0;
        scp->sc_game_id = 0;
    }

    /*
     * Read top scores
     */
    read(fileno(inf), (char *) top_ten, sizeof top_ten);
    fclose(inf);

    /*
     * Show what we have
     */
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	if (scp->sc_score > 0) {
	    printf("%10ld %10s\n", scp->sc_score, scp->sc_name);
	}
    }

    if (argc < 2 || num < 1 || num > 10)
	exit(0);

    /*
     * Delete one
     */
    i = 1;
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	if (i == num) {
	    if (scp->sc_score > 0) {
		printf("Delete entry %d\n", num);
		sc2 = scp;
		while (sc2 < &top_ten[10]) {
		    *sc2 = sc2[+1];
		    sc2++;
		}
		sc2->sc_score = 0L;
		sc2->sc_name[0] = '\0';
		break;
	    }
	}
	i++;
    }

    /*
     * Update the file
     */
    if ((outf=fopen(scorefile,"w")) == NULL) {
	fprintf(stderr, "Unable to write %s\n", scorefile);
	perror(scorefile);
	exit(0);
    }
    write(fileno(outf), (char *) top_ten, sizeof top_ten);
    fclose(outf);

}

