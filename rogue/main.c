 /*
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    More recent code by Earl Fogel, all rights released.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdio.h>
#include "curses.h"
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include "mach_dep.h"
#include "rogue.h"

#if 0
#include <mcheck.h>
#endif


int fd_score = -1;		/* file descriptor the score file */

int 
main (argc, argv, envp)
int argc;
char **argv;
char **envp;
{
    char *env;
    struct linked_list *item;
    struct object *obj;
    int lowtime, wpt=0, i, j, hpadd, dmadd;
    bool alldone,predef;
    time_t now;

#ifndef _WIN32
    (void) signal(SIGQUIT, SIG_IGN); 		/* ignore quit for now */
#endif

#if 0
    mtrace();	/* glibc malloc debugging */
#endif

    /*
     * get home and options from environment
     */
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((env = getenv("HOMEPATH")) != NULL)  /* windows */
	strcpy(home, env);
    else
	strcpy(home, ".");
    strcat(home, "/");

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    /*
     * Open score file.
     * If we can't access a central score file,
     * use the home directory instead.
     */
    strcpy(score_file, SCOREDIR);
    fd_score = open(score_file, O_RDWR);
    if (fd_score < 0) {
	strcpy(score_file, home);
	strcat(score_file, ".rog_score");
#ifdef _WIN32
	fd_score = open(score_file, O_RDWR|O_CREAT);
#else
	fd_score = open(score_file, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
#endif
    }

    if ((env = getenv("SROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0') {
	    if (getenv("USERNAME")) 
		strcpy(whoami, getenv("USERNAME"));
	    else if (getenv("USER")) 
		strcpy(whoami, getenv("USER"));
    }
    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
		"candleberry", "caprifig", "dewberry", "elderberry",
		"gooseberry", "guanabana", "hagberry", "ilama", "imbu",
		"jaboticaba", "jujube", "litchi", "mombin", "pitanga",
		"prickly pear", "rambutan", "sapodilla", "soursop",
		"sweetsop", "whortleberry"
	};

	srand48((long) (getpid()+255));
	strcpy(fruit, funfruit[rnd(sizeof(funfruit)/sizeof(funfruit[0]))]);
     }

     /* put a copy of fruit in the right place */
     fd_data[1].mi_name = ALLOC(LINELEN);
     strcpy(fd_data[1].mi_name, fruit);

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0) {
	waswizard = TRUE;
	score(0, SCOREIT, 0);
	exit(0);
    }

    /*
     * Check to see if player is a wizard
     */
#ifdef EARL
    if ((argc >= 2 && strcmp(argv[argc-1], "-w") == 0)) {
	wizard = TRUE;
	canwizard = TRUE;
    }
#endif
    if (argc >= 2 && argv[argc-1][0] == '\0') {
	wizard = TRUE;
	canwizard = TRUE;
    }

    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    seed = dnum;
    srand48(seed);
    game_id = rnd(INT_MAX);

    init_things();			/* Set up probabilities of things */
    init_fd();				/* Set up food probabilities */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
    init_names();			/* Set up names of scrolls */
    setup();

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    keypad(cw,TRUE);		/* for arrow keys */
    keypad(hw,TRUE);		/* for arrow keys */

    if (argc >= 2 && argv[1][0] != '-'
#ifndef EARL
      && !wizard
#endif
      ) {
	if (!restore(argv[1])) { /* Note: restore returns on error only */
	    exit(1);
	}
    }

   if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    usleep(250000);
    if (wizard)
	usleep(250000);

    /*
     * choose which monsters may appear
     */
    if (argc >= 2 && strcmp(argv[1], "-mc") == 0) {
	init_monsters('c');  /* classic */
    } else if (argc >= 2 && strcmp(argv[1], "-mr") == 0) {
	init_monsters('r');  /* random */
    } else if (argc >= 2 && strcmp(argv[1], "-ma") == 0) {
	init_monsters('a');  /* all */
    } else if (argc >= 2 && strcmp(argv[1], "-mm") == 0) {
	init_monsters('m');  /* more monsters */
    } else {
	init_monsters('+');  /* classic plus */
    }

    /*
     * choose level of difficulty
     */
    if (argc >= 2 && strcmp(argv[1], "-easy") == 0) {
	difficulty--;
    } else if (argc >= 2 && strcmp(argv[1], "-hard") == 0) {
	difficulty++;
    }

    predef = geta_player();
    waswizard = wizard;
re_roll:
    if(!predef)
	init_player();			/* Roll up the rogue */
    else
	goto get_food;			/* Using a pre-rolled rogue */
    /*
     * Give the rogue his weaponry.  
     */
    alldone = FALSE;
    do {
	int ch = 0;
	i = rnd(16);	/* number of acceptable weapons */
	switch(i) {
	    case 0: ch = 25; wpt = MACE;
	    when 1: ch = 25; wpt = SWORD;
	    when 2: ch = 15; wpt = TWOSWORD;
	    when 3: ch = 10; wpt = SPEAR;
	    when 4: ch = 20; wpt = TRIDENT;
	    when 5: ch = 20; wpt = SPETUM;
	    when 6: ch = 20; wpt = BARDICHE;
	    when 7: ch = 15; wpt = SPIKE;
	    when 8: ch = 15; wpt = BASWORD;
	    when 9: ch = 20; wpt = HALBERD;
	    when 10:ch = 20; wpt = BATTLEAXE;
	    when 11:ch = 20; wpt = GLAIVE;
	    when 12:ch = 20; wpt = LPIKE;
	    when 13:ch = 20; wpt = BRSWORD;
	    when 14:ch = 20; wpt = CRYSKNIFE;
	    when 15:ch = 20; wpt = CLAYMORE;
	}
	if(rnd(100) < ch) {		/* create this weapon */
	    alldone = TRUE;
	}
    } while(!alldone);
    hpadd = rnd(2) + 1;
    dmadd = rnd(2) + 1;
    if (player.t_ctype == C_FIGHTER) {
	if (rnd(100) > 50)
	    wpt = TWOSWORD;
	else
	    wpt = CLAYMORE;
	hpadd = hpadd - 1;
    }
    /*
     * Find out what the armor is.
     */
    i = rnd(100) + 1;
    j = 0;
    while (armors[j].a_prob < i)
	j++;
    if (difficulty >= 2 && j == CRYSTAL_ARMOR)
	j = j - 2;
    if (difficulty > 2 && j == MITHRIL)
	j = j - 2;
    /*
     * See if this rogue is acceptable to the player.
     */
    if(!puta_player(j,wpt,hpadd,dmadd))
 	goto re_roll;
    /*
     * It's OK. Add this stuff to the rogue's pack.
     */
    item = spec_item(WEAPON, wpt, hpadd, dmadd);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * And a suit of armor
     */
    item = spec_item(ARMOR, j, 0, 0);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[j].a_wght;
    add_pack(item, TRUE);
    cur_armor = obj;
    /*
     * And some food too
     */
get_food:
    item = spec_item(FOOD, 0, 0, 0);
    obj = OBJPTR(item);
    obj->o_weight = things[TYP_FOOD].mi_wght;
    add_pack(item, TRUE);

    resurrect = pstats.s_const;
    new_level(FALSE);			/* Draw current level */

    /* more tweaks based on difficulty level */
    tweak_settings(TRUE);

    /*
     * Start up daemons and fuses
     */
    start_daemon(DAEMON_DOCTOR, &player, AFTER);
    light_fuse(FUSE_SWANDER, 0, WANDERTIME, AFTER);
    start_daemon(DAEMON_STOMACH, 0, AFTER);
    start_daemon(DAEMON_RUNNERS, 0, AFTER);
    playit();
    /* notreached */
    return 0;
}

/*
 * endit:
 *	Exit the program abnormally.
 */

void 
endit ()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

void 
fatal (s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    printf("\n");	/* So the cursor doesn't stop at the end of the line */
    printf("%s\n", s);
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 *
 * Concatenate four short random numbers to make a long one.
 * This is required on Windows, where RAND_MAX is quite small.
 *
 * see http://forums.codeguru.com/showthread.php?534679-Generating-big-random-numbers-in-C
 */

int 
rnd (range)
int range;
{
    return (range == 0 ? 0 : ((((rand() & 255)<<8 | (rand() & 255))<<8 | (rand() & 255))<<7 | (rand() & 127)) % range);
}

/*
 * roll:
 *	roll a number of dice
 */

int 
roll (number, sides)
int number;
int sides;
{
    int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}

void 
setup ()
{

#ifndef _WIN32
#ifdef BSD4
    void  tstop();
    signal(SIGTSTP, (sig_t)tstop);
#endif
#ifdef USGV5
    int  tstp();
    signal(SIGTSTP, tstp);
#endif

    signal(SIGHUP, (sig_t)quit);
    signal(SIGINT, (sig_t)quit);
#endif

    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
    nonl();
    curs_set(0);			/* hide cursor */
}

/*
 * make some adjustments based on difficulty level
 */
void
tweak_settings (bool first_time)
{
    int i;
    int potion, scroll;
    struct linked_list *item;
    struct object *obj;

    /* urogue -easy */
    if (difficulty < 2) {
	if (LINES > maxpack + 2)
	    maxpack += 2;  /* pack can hold 2 more items */

	if (first_time) {
	    /* players start out with more stuff */
	    if (player.t_ctype == C_THIEF || player.t_ctype == C_FIGHTER) {
		potion = P_TFIND; /* magic detection */
		scroll = S_IDENT; /* identify */
	    } else {
		potion = P_HEALING; /* healing */
		scroll = S_LIGHT;   /* light */
	    }
	    p_know[potion] = TRUE;
	    s_know[scroll] = TRUE;

	    item = spec_item(SCROLL, scroll, 0, 0);
	    obj = (struct object *) ldata(item);
	    obj->o_flags |= ISKNOW;
	    obj->o_flags |= ISBLESSED;
	    obj->o_weight = 0;
	    add_pack(item, TRUE);

	    item = spec_item(POTION, potion, 0, 0);
	    obj = (struct object *) ldata(item);
	    obj->o_flags |= ISKNOW;
	    obj->o_weight = 0;
	    add_pack(item, TRUE);
	}

	for(i=0; i < MAXSCROLLS; i++) {
	    if (s_magic[i].mi_curse > 10)
		s_magic[i].mi_curse -= 10;  /* less chance of cursed scrolls */
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_curse > 10)
		p_magic[i].mi_curse -= 10;  /* less chance of cursed potions */
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_curse > 10 && r_magic[i].mi_curse < 100)
		r_magic[i].mi_curse -= 10;  /* less chance of cursed rings */
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_curse > 10)
		ws_magic[i].mi_curse -= 10;  /* less chance of cursed wands */
	}

    /* urogue -hard */
    } else if (difficulty > 2) {
	for(i=0; i < MAXSCROLLS; i++) {
	    if (s_magic[i].mi_bless > 5)
		s_magic[i].mi_bless -= 5;  /* less chance of blessed scrolls */
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_bless > 5)
		p_magic[i].mi_bless -= 5;  /* less chance of blessed potions */
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_bless > 5)
		r_magic[i].mi_bless -= 5;  /* less chance of blessed rings */
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_bless > 5)
		ws_magic[i].mi_bless -= 5;  /* less chance of blessed wands */
	}
    }
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

void 
playit ()
{

    /*
     * set up defaults for slow terminals
     */
    char_type = player.t_ctype;

    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit();
}

