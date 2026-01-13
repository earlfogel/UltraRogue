 /*
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    More recent code by Earl Fogel, no rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
 */

#ifdef _WIN32
#define _POSIX
#define sig_t __p_sig_fn_t
#include <windows.h>
#include <wchar.h>
#include <direct.h>
#undef max
#undef min
#endif

#include <stdio.h>
#include "curses.h"
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include "rogue.h"

#if 0
#include <mcheck.h>
#endif


int 
main (int argc, char **argv)
{
    char *env;
    struct linked_list *item;
    struct object *obj;
    int lowtime, wpt=0, i, j, hpadd, dmadd;
    bool alldone, predef=0;
    char monster_flag = 'r';
    time_t now;
    char *restore_file = NULL;
    struct stat sb;
    bool show_welcome = FALSE;
    char char_file[LINELEN];

    (void) signal(SIGQUIT, SIG_IGN); 		/* ignore quit for now */

#if 0
    mtrace();	/* glibc malloc debugging */
#endif

#ifdef _WIN32
    /* use %APPDATA%/urogue */
    if ((env = getenv("APPDATA")) != NULL) {
	strcpy(file_name, env);
	strcat(file_name, "/urogue");
	if (stat(file_name, &sb) != 0)
	    _mkdir(file_name);
    }
    if (stat(file_name, &sb) == 0 && S_ISDIR(sb.st_mode))
	strcpy(home, file_name);
    else
	strcpy(home, ".");
    strcat(home, "/");
#else
    /* get home from environment */
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else
	strcpy(home, ".");
    strcat(home, "/");
#endif

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    if ((env = getenv("SROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0') {
	    if (getenv("USERNAME")) 
		strcpy(whoami, getenv("USERNAME"));
	    else if (getenv("USER")) 
		strcpy(whoami, getenv("USER"));
    }
    if (stat(file_name, &sb) == 0 && S_ISREG(sb.st_mode))
	restore_file = file_name;

    /* check for a character file */
    strcpy(char_file, home);
    strcat(char_file, ROGDEFS);
    if (stat(char_file, &sb) != 0) {
	show_welcome = TRUE;
    }

    /*
     * Parse command-line options
     * Anything remaining after this should be the path to a saved game.
     */
    while (--argc > 0 && (*++argv)[0] == '-') {
	switch (argv[0][1]) {
	case 'd':  /* -debug: debug mode */
	    wizard = canwizard = TRUE;
	    break;
	case 's':   /* -score: print score and exit */
	    score(0, SCOREIT, 0);
	    exit(0);
	case 'm':   /* -mc, -mr, -ma: choose classic, random or all monsters */
	    monster_flag = (char) argv[0][2];
	    break;
	case 'e':   /* -easy */
	    if (strcmp(argv[0], "-easy") == 0) {
		difficulty--;
		mindifficulty = difficulty;
	    } else {
		usage();
		exit(1);
	    }
	    break;
	case 'h':   /* -hard */
	    if (strcmp(argv[0], "-hard") == 0) {
		difficulty++;
		mindifficulty = difficulty;
	    } else {
		usage();
		exit(1);
	    }
	    break;
	case 'n':   /* DON'T restore a saved game */
	    restore_file = NULL;
	    break;
	case 'v':
	   printf("UltraRogue version %s\n", release);
	   exit(0);
	default:
	    usage();
	    exit(1);
	}
    }
    if (argc>0)
	restore_file = argv[0];
#if 0
printf("wizard=%d, monster_flag=%c, difficulty=%d\n", wizard, monster_flag, difficulty);
exit(0);
#endif

    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    seed = dnum;
    srandom(seed);
    game_id = rnd(INT_MAX-1) + 1;

    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
		"candleberry", "caprifig", "dewberry", "elderberry",
		"gooseberry", "guanabana", "hagberry", "ilama", "imbu",
		"jaboticaba", "jujube", "litchi", "mombin", "pitanga",
		"prickly pear", "rambutan", "sapodilla", "soursop",
		"sweetsop", "whortleberry"
	};

	strcpy(fruit, funfruit[rnd(sizeof(funfruit)/sizeof(funfruit[0]))]);
     }
    /* put a copy of fruit in the right place */
    fd_data[1].mi_name = ALLOC(LINELEN);
    strcpy(fd_data[1].mi_name, fruit);

    init_things();			/* Set up probabilities of things */
    init_fd();				/* Set up food probabilities */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    init_names();			/* Set up names of scrolls */

    initscr();				/* Start up cursor package */

#ifdef PDCURSES
    {
	int pdc_lines = LINES;
       	int pdc_cols = COLS;

	if ((env = getenv("PDC_LINES")) != NULL)
	    pdc_lines = atoi(env);
	if ((env = getenv("PDC_COLS")) != NULL)
	    pdc_cols = atoi(env);
	if (pdc_lines != LINES || pdc_cols != COLS) {
	    resize_term(pdc_lines,pdc_cols);
if (wizard) {
printf("LINES=%d COLS=%d Curses version: %s\n", LINES, COLS, curses_version());
fflush(stdout);
}
	}
    }
#endif

    setup();

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    keypad(cw,TRUE);		/* for arrow keys */
    keypad(hw,TRUE);		/* for arrow keys */
#ifdef MOUSE
    if (use_mouse)
	mousemask(BUTTON1_RELEASED, NULL);  /* for mouse buttons */
#endif


    /*
     * Restore saved game
     */
    if (restore_file) {
	if (!restore(restore_file)) /* Note: restore returns on error only */
	    exit(1);
    }

   if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    usleep(250000);
    if (wizard)
	usleep(250000);

    init_monsters(monster_flag);

    if (difficulty <= 3)
	predef = geta_player();

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
    /* you start with weaker armor in harder games */
    if (difficulty == 2 && j >= MITHRIL)
	j -= 2;
    if (difficulty > 2 && j >= PLATE_ARMOR)
	j -= 3;
    if (difficulty > 3 && j >= BANDED_MAIL)
	j -= 5;
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

    /* tweaks based on difficulty level */
    tweak_settings(TRUE, 2);

    /*
     * Start up daemons and fuses
     */
    start_daemon(DAEMON_DOCTOR, &player, AFTER);
    light_fuse(FUSE_SWANDER, 0, WANDERTIME, AFTER);
    start_daemon(DAEMON_STOMACH, 0, AFTER);
    start_daemon(DAEMON_RUNNERS, 0, AFTER);

    new_level(FALSE);			/* Draw current level */

    if (show_welcome) {
	msg("Welcome to urogue!  Press '?' for help.");
    }

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
fatal (char *s)
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
 *	Pick a random number.
 *
 * On Windows, RAND_MAX is quite small, so we concatenate four short
 * random numbers to make a long one.
 *
 * I.e. here's a walk-through of the 32 bits after each pass,
 * where the numbers 1 to 4 show which bits are set on each call to rand:
 *
 * start       00000000 00000000 00000000 00000000
 * rand & 255  00000000 00000000 00000000 11111111
 * shift 8     00000000 00000000 11111111 00000000
 * rand & 255  00000000 00000000 11111111 22222222
 * shift 8     00000000 11111111 22222222 00000000 
 * rand & 255  00000000 11111111 22222222 33333333 
 * shift 7     01111111 12222222 23333333 30000000 
 * rand & 127  01111111 12222222 23333333 34444444 
 *
 * This leave a zero in the highest order (left-most) bit, so we don't
 * generate negative numbers.
 *
 * see http://forums.codeguru.com/showthread.php?534679-Generating-big-random-numbers-in-C
 * and http://c-faq.com/lib/randrange.html
 */

int 
rnd (int range)
{
#ifndef _WIN32
	return (range == 0 ? 0 : (random() & 0x7fffffff) % range);
#else
    return (range == 0 ? 0 : ((((rand() & 255)<<8 | (rand() & 255))<<8 | (rand() & 255))<<7 | (rand() & 127)) % range);
#endif
}

/*
 * roll:
 *	roll a number of dice
 */

int 
roll (int number, int sides)
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
#   ifdef BSD4
    void  tstop();
    signal(SIGTSTP, (sig_t)tstop);
#   endif
#   ifdef USGV5
    int  tstp();
    signal(SIGTSTP, tstp);
#   endif
#endif
    signal(SIGHUP, (sig_t)quit);
    signal(SIGINT, (sig_t)quit);

    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
    nonl();
    if (showcursor)
	curs_set(1);	/* show cursor */
    else
	curs_set(0);	/* hide cursor */
}

/*
 * make some adjustments based on difficulty level
 */
void
tweak_settings (bool first_time, int old_difficulty)
{
    int i;
    int potion, scroll;
    struct linked_list *item;
    struct object *obj;

    /*
     * set things back to normal, at least temporarily
     */
    if (old_difficulty < 2) {	      /* from easy to normal */
	for(i=0; i < MAXSCROLLS; i++) {
	    if (s_magic[i].mi_curse > 0 && s_magic[i].mi_curse < 100)
		s_magic[i].mi_curse += 10;
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_curse > 0 && p_magic[i].mi_curse < 100)
		p_magic[i].mi_curse += 10;
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_curse > 0 && r_magic[i].mi_curse < 100)
		r_magic[i].mi_curse += 10;
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_curse > 0 && ws_magic[i].mi_curse < 100)
		ws_magic[i].mi_curse += 10;
	}
	maxpack = MAXPACK;
    } else if (old_difficulty > 2) {  /* from hard to normal */
	for(i=0; i < MAXSCROLLS; i++) {
	    if (s_magic[i].mi_bless > 0 && s_magic[i].mi_bless < 100)
		s_magic[i].mi_bless += 5;
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_bless > 0 && p_magic[i].mi_bless < 100)
		p_magic[i].mi_bless += 5;
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_bless > 0 && r_magic[i].mi_bless < 100)
		r_magic[i].mi_bless += 5;
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_bless > 0 && ws_magic[i].mi_bless < 100)
		ws_magic[i].mi_bless += 5;
	}
	s_magic[S_GENOCIDE].mi_prob += 1;
	s_magic[S_MAKEIT].mi_prob += 1;
	r_magic[R_WIZARD].mi_prob += 1;
    }

    /* normal difficulty */
    if (difficulty == 2) {
	if (first_time) {
	    if (player.t_ctype == C_THIEF || player.t_ctype == C_FIGHTER) {
		p_know[P_TFIND] = TRUE;
		s_know[S_IDENT] = TRUE;
	    }
	}

    /* urogue -easy */
    } else if (difficulty < 2) {
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
	    if (s_magic[i].mi_curse > 10 && s_magic[i].mi_curse < 100)
		s_magic[i].mi_curse -= 10;  /* less chance of cursed scrolls */
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_curse > 10 && p_magic[i].mi_curse < 100)
		p_magic[i].mi_curse -= 10;  /* less chance of cursed potions */
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_curse > 10 && r_magic[i].mi_curse < 100)
		r_magic[i].mi_curse -= 10;  /* less chance of cursed rings */
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_curse > 10 && ws_magic[i].mi_curse < 100)
		ws_magic[i].mi_curse -= 10;  /* less chance of cursed wands */
	}

    /* urogue -hard */
    } else if (difficulty > 2) {
	if (first_time) {
	    if (difficulty == 3 &&
		(player.t_ctype == C_THIEF || player.t_ctype == C_FIGHTER)) {
		p_know[P_TFIND] = TRUE;
		s_know[S_IDENT] = TRUE;
	    }
	}
	for(i=0; i < MAXSCROLLS; i++) {
	    if (s_magic[i].mi_bless > 5 && s_magic[i].mi_bless < 100)
		s_magic[i].mi_bless -= 5;  /* less chance of blessed scrolls */
	}
	for(i=0; i < MAXPOTIONS; i++) {
	    if (p_magic[i].mi_bless > 5 && p_magic[i].mi_bless < 100)
		p_magic[i].mi_bless -= 5;  /* less chance of blessed potions */
	}
	for(i=0; i < MAXRINGS; i++) {
	    if (r_magic[i].mi_bless > 5 && r_magic[i].mi_bless < 100)
		r_magic[i].mi_bless -= 5;  /* less chance of blessed rings */
	}
	for(i=0; i < MAXSTICKS; i++) {
	    if (ws_magic[i].mi_bless > 5 && ws_magic[i].mi_bless < 100)
		ws_magic[i].mi_bless -= 5;  /* less chance of blessed wands */
	}
	/*
	 * make these items more rare in hard games
	 * (which makes their immediate successors more common)
	 */
	s_magic[S_GENOCIDE].mi_prob -= 1;  /* 0.5% to 0.4 */
	s_magic[S_MAKEIT].mi_prob -= 1;    /* 0.5% to 0.4 */
	r_magic[R_WIZARD].mi_prob -= 1;    /* 0.4% to 0.3 */
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


void
usage ()
{
    char *usage =
	"urogue [options] [saved_game]\n"
	"\n"
	"Options:\n"
	"    -easy   make the game easier\n"
	"    -hard   make the game harder\n"
	"    -mc     use the classic monsters from urogue 1.0.2\n"
	"    -mr     use a random selection of monsters\n"
	"    -ma     use all 400+ monsters\n"
	"    -s      print top scores and exit\n"
	"    -v      print version string and exit\n"
	"    -d      debug mode\n";
    printf("%s", usage);
}
