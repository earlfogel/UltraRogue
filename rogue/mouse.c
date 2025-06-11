/*
 * mouse movement
 *
 */

#ifdef MOUSE

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

bool firststep = FALSE;
static int nsearch = 9;
static bool reachable;

int check_doors(struct room *room, int minx, int maxx, int miny, int maxy);
int check_possible_doors(struct room *room, int minx, int maxx, int miny, int maxy, coord dest);

#define STEPS(y1, x1, y2, x2) (abs(x2 - x1) + abs(y2 - y1))
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
char *name[] = {"north", "east", "south", "west"};
struct wall {
    bool door;	/* true if there is a door in this wall */
    int min;
    int max;
    int pos;
    int minpos;
    int maxpos;
} walls[4];

char 
do_mousemove (dest, prev)
coord dest;
coord prev;
{
    char ch = ' ';
    static char oldch = ' ';
    static coord indoor = {0,0};
    static coord prevdest = {0,0};
    static coord mydest = {0,0};
    static int nturns = 0;	/* count turns, in case we get stuck in a loop */
    int nsecret = 0;
    int x, y;
    int curdist, bestdist;
    int nmoves = 0;
    int ndoors = 0;
    int npdoors = 0;
    int nmonst = 0;
    coord pos;
    coord best = {hero.x, hero.y};
    int minx, miny, maxx, maxy;
    struct room *myroom = roomin(&hero);

    if (count < 1) {
	mousemove = FALSE;
	return('.');	/* so we don't reset prev */
    }

    if (firststep) {
	nturns = 0;
	oldch = ch;
	mydest.x = dest.x;
	mydest.y = dest.y;
    }

    if (dest.x != prevdest.x || dest.y != prevdest.y) {
	indoor.x = indoor.y = 0;
	prev.x = prev.y = 0;  /* local variables */
	prevdest.x = dest.x;
	prevdest.y = dest.y;
    } else {
	firststep = FALSE;
    }

    if (myroom != NULL) {
	minx = myroom->r_pos.x;
	miny = myroom->r_pos.y;
	maxx = myroom->r_pos.x + myroom->r_max.x - 1;
	maxy = myroom->r_pos.y + myroom->r_max.y - 1;
    }

    /*
     * If we're in a room and the destination is elsewere
     */
    if (myroom != NULL
	&& myroom != roomin(&dest)
	&& myroom != roomin(&mydest)
	&& winat(hero.y, hero.x) != DOOR
	&& off(player, CANINWALL)) {

	ndoors = check_doors(myroom, minx, maxx, miny, maxy);
	npdoors = check_possible_doors(myroom, minx, maxx, miny, maxy, dest);

	/*
	 * if there may be an unknown door between us and
	 * our destination, head towards it
	 */
	if (npdoors > 0) {
	    if (dest.x > maxx) mydest.x = maxx;
	    if (dest.x < minx) mydest.x = minx;
	    if (dest.y > maxy) mydest.y = maxy;
	    if (dest.y < miny) mydest.y = miny;
	}
    }

    /*
     * choose the exit nearest to our destination
     */
    if (myroom != NULL
	&& myroom != roomin(&dest)
	&& myroom != roomin(&mydest)
	&& winat(hero.y, hero.x) != DOOR
	&& off(player, CANINWALL)) {
	struct room *destroom = roomin(&dest);
	coord exit, bestdoor;
	int i, doordist;
	int mindx, mindy, maxdx, maxdy;

	if (destroom == NULL) {
	    mindx = maxdx = dest.x;
	    mindy = maxdy = dest.y;
	} else {
	    mindx = destroom->r_pos.x;
	    mindy = destroom->r_pos.y;
	    maxdx = destroom->r_pos.x + destroom->r_max.x - 1;
	    maxdy = destroom->r_pos.y + destroom->r_max.y - 1;
	}
	bestdoor.y = 0;
	bestdoor.x = 0;
	bestdist = (COLS*COLS + LINES*LINES) * 32;  /* a big number */
	for (i = 0; i < myroom->r_nexits; i++) {  /* for each door */
	    exit = myroom->r_exit[i];
	    if (mvwinch(cw, exit.y, exit.x) == DOOR
		|| (exit.x == minx && mvwinch(cw,exit.y,minx-1) == PASSAGE)
		|| (exit.x == maxx && mvwinch(cw,exit.y,maxx+1) == PASSAGE)
		|| (exit.y == miny && mvwinch(cw,miny-1,exit.x) == PASSAGE)
		|| (exit.y == maxy && mvwinch(cw,maxy+1,exit.x) == PASSAGE)
	    ) {
		if (exit.x == indoor.x && exit.y == indoor.y
		    && (ndoors > 1 || npdoors > 1))
		    continue; /* don't pick the door we came in */
		doordist = STEPS(hero.y, hero.x, exit.y, exit.x)
			    + STEPS(exit.y, exit.x, dest.y, dest.x);
		if (mindy > maxy+1 && exit.y != maxy) {
		    if (exit.y == miny)
			doordist *= 8;
		    else
			doordist *= 4;
		}
		if (maxdy < miny-1 && exit.y != miny) {
		    if (exit.y == maxy)
			doordist *= 8;
		    else
			doordist *= 4;
		}
		if (mindx > maxx+1 && exit.x != maxx) {
		    if (exit.x == minx)
			doordist *= 8;
		    else
			doordist *= 4;
		}
		if (maxdx < minx-1 && exit.x != minx) {
		    if (exit.x == maxx)
			doordist *= 8;
		    else
			doordist *= 4;
		}
		if (doordist < bestdist) {
		    bestdist = doordist;
		    bestdoor = exit;
		}
	    }
	}
	if (bestdoor.x > 0 && bestdoor.y > 0) {
	    mydest.x = bestdoor.x;
	    mydest.y = bestdoor.y;
	} else {	/* no known doors */
	    if (dest.x > maxx) mydest.x = maxx;
	    if (dest.x < minx) mydest.x = minx;
	    if (dest.y > maxy) mydest.y = maxy;
	    if (dest.y < miny) mydest.y = miny;
	}
    } else if (winat(hero.y, hero.x) == DOOR || on(player, CANINWALL)) {
	mydest.x = dest.x;
	mydest.y = dest.y;
    }

    /*
     * now pick our next move
     * - at this point, mydest may be our final destination or
     *   an intermediate destination
     */
    curdist = STEPS(mydest.y, mydest.x, hero.y, hero.x) + 1;
    bestdist = curdist;
    for (x = hero.x - 1; x <= hero.x + 1; x++) {
	for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    coord tryp;
	    tryp.x = x; tryp.y = y;
	    if (mvwinch(cw, y, x) == SECRETDOOR) {
		nsecret++;
	    }
	    if (!(x == mydest.x && y == mydest.y)) {
		if (x < 0 || x > COLS || y < 1 || y > LINES - 2 ||
		    (x == hero.x && y == hero.y) ||
		    (!step_ok(y, x, MONSTOK, &player)
			/* && !(winat(y, x) == SECRETDOOR) */ ))
		    continue;  /* skip invalid moves */
		if (!ISWEARING(R_LEVITATION) && off(player, CANFLY) && isatrap(show(y, x)))
		    continue;  /* avoid traps */
		if (winat(y, x) == POST)
		    continue;  /* avoid trading posts */
		if (x == prev.x && y == prev.y)
		    continue;  /* don't reverse course */
		if ((winat(hero.y, hero.x) == PASSAGE || levtype == MAZELEV
		 || (winat(hero.y, hero.x) == DOOR && winat(y,x) == PASSAGE))
		    && off(player, CANINWALL)
		    && (y != hero.y && x != hero.x)
		    && !isalpha(show(prev.y, prev.x))
		    && !firststep)
		    continue;  /* don't diagonal */
		if (winat(hero.y, hero.x) == DOOR
		    && roomin(&prev)
		    && roomin(&tryp) == roomin(&prev)
		    && !firststep)
		    continue;  /* don't back into room */
		if (isalpha(show(y, x))) {  /* eek, a monster */
		    nmonst++;  /* we'll try to avoid it */
		    continue;
		}
	    } else {  /* we found it */
		if ((winat(y, x) == SECRETDOOR)) {
		    return('s');  /* we'll search for it */
		} else if (!step_ok(y, x, MONSTOK, &player)
			|| isalpha(show(y, x))) {
		    count = 1;
		    return(' ');
		}
	    }
	    nmoves++;
	    if (STEPS(mydest.y, mydest.x, y, x) < bestdist
		|| nmoves == 1) {
		best.x = x;
		best.y = y;
		bestdist = STEPS(mydest.y, mydest.x, y, x);
	    }
	    /*
	     * if this move goes into the room we want, take it
	     */
	    pos.x = x; pos.y = y;
	    if (roomin(&pos) && !roomin(&hero)
		&& roomin(&pos) == roomin(&dest)) {
		best.x = x;
		best.y = y;
		bestdist = 1;
	    }
	}
    }
    if (winat(hero.y,hero.x) == DOOR
	&& winat(prev.y,prev.x) == PASSAGE
	) { /* enter a room */
	indoor.x = hero.x;
	indoor.y = hero.y;
    }
    if (winat(hero.y,hero.x) == DOOR
	&& winat(prev.y,prev.x) != PASSAGE
	) { /* leave a room */
	    int i;
	    indoor.x = 0;
	    indoor.y = 0;
	    mydest.x = dest.x;
	    mydest.y = dest.y;
	    for (i = 0; i < 4; i++) {
		walls[i].door = FALSE;
	    }
	}
    if (nmoves < 1) {	/* dead end */
	if (nmonst > 0) {	/* stop if there's a monster in the way */
	    ch = '.';
	    count = 1;
	    after = FALSE;
	} else if (nsecret > 0) {
	    return('s');  /* there is a secret door */
	} else if (prev.y > 0 && levtype == NORMLEV && nsearch-- > 0) {
	    /* maybe there's a secret door */
	    return('s');
	}
    } else if (bestdist < curdist || nmoves == 1
	|| (ndoors + npdoors == 1)
	|| indoor.x > 0 || firststep
	|| winat(hero.y, hero.x) == PASSAGE
	|| winat(hero.y, hero.x) == DOOR
	|| levtype == MAZELEV) {
	if (best.x < hero.x && best.y == hero.y)
	    ch = 'h';
	else if (best.x == hero.x && best.y > hero.y)
	    ch = 'j';
	else if (best.x == hero.x && best.y < hero.y)
	    ch = 'k';
	else if (best.x > hero.x && best.y == hero.y)
	    ch = 'l';
	else if (best.x < hero.x && best.y < hero.y)
	    ch = 'y';
	else if (best.x > hero.x && best.y < hero.y)
	    ch = 'u';
	else if (best.x < hero.x && best.y > hero.y)
	    ch = 'b';
	else if (best.x > hero.x && best.y > hero.y)
	    ch = 'n';
	moving = TRUE;
	firststep = FALSE;
	if (reachable) {
	    if (count < 50 && bestdist < curdist && rnd(9)>0)
		count++;  /* don't stop just yet */
	} else {
	    if (bestdist > curdist)
		count--;  /* give up sooner */
	}
	/* stop if a move will take us into an unexplored room */
	if (doorstop && myroom == NULL
		&& mvwinch(cw, best.y, best.x) == DOOR
		&& mvwinch(cw, best.y+1, best.x) != '|'
		&& mvwinch(cw, best.y-1, best.x) != '|'
		&& mvwinch(cw, best.y, best.x+1) != '-'
		&& mvwinch(cw, best.y, best.x-1) != '-'
		&& mvwinch(cw, dest.y, dest.x) == ' '
		&& off(player, CANINWALL)) {
debug("entering a new room");
	    count = 1;	/* stop to look around */
	}
	if (ch != oldch) {
	    nturns++;
	    oldch = ch;
	}
    }
    if (hero.y == dest.y && hero.x == dest.x) {  /* we made it */
	count = 1;
	ch = ' ';
	after = FALSE;
	mousemove = FALSE;
    }

    if (nturns > 30) {
	ch = ' ';
	after = FALSE;
	mousemove = FALSE;
debug("stuck in a loop");
    }
    return(ch);
}

char 
do_mouseclick (dest)
coord dest;
{
    char ch = ' ';

    if (hero.y == dest.y && hero.x == dest.x) {
	if (winat(hero.y, hero.x) == STAIRS) {
	    if (is_carrying(TR_WAND)) {
		ch = '<';  /* up level */
	    } else {
		ch = '>';  /* down level */
	    }
	} else if (winat(hero.y, hero.x) == POOL
		|| winat(hero.y, hero.x) == POISONTRAP) {
	    ch = 'D';  /* dip it */
	} else if (winat(hero.y, hero.x) == GOLD
		|| winat(hero.y, hero.x) == POTION
		|| winat(hero.y, hero.x) == SCROLL
		|| winat(hero.y, hero.x) == FOOD
		|| winat(hero.y, hero.x) == WEAPON
		|| winat(hero.y, hero.x) == ARMOR
		|| winat(hero.y, hero.x) == RING
		|| winat(hero.y, hero.x) == ARTIFACT
		|| winat(hero.y, hero.x) == STICK) {
	    ch = ',';  /* pick it up */
	}
    } else if (isalpha(show(dest.y, dest.x))
	&& DISTANCE(dest.y, dest.x, hero.y, hero.x) < 4) {
	ch = 'F';   /* fight monster */
    } else if (dest.y > 0 && dest.y < LINES - 2) {
	/*
	 * walk towards the mouse
	 */
	count = LINES + COLS;  /* upper limit */
	if (levtype == MAZELEV && off(player, CANINWALL))
	    count /= 2;
	mousemove = TRUE;
	firststep = TRUE;
	nsearch = 6;
	ch = ' ';  /* do nothing, for now */
    }
    return(ch);
}

/*
 * if destination is unreachable, pick another
 */
coord
fix_mousedest (dest)
    coord dest;
{
    int x, y;
    int radius = 1;
    int bestdist = DISTANCE(hero.y, hero.x, dest.y, dest.x) + 1; /* upper limit */
    coord best = {-1,-1};
    int testdist;
    coord test;

    /*
     * if destination is unreachable, move it
     */
    if (!step_ok(dest.y, dest.x, MONSTOK, &player)
	&& !(mvwinch(stdscr, dest.y, dest.x) == SECRETDOOR)) {
#if 0
	while (radius < (COLS + LINES)/6) {
#endif
	while (radius < 8) {
	    for (x = dest.x - radius; x <= dest.x + radius; x++) {
		if (x < 0 || x > COLS - 1)
		    continue;
		for (y = dest.y - radius; y <= dest.y + radius; y++) {
		    if (y < 1 || y > LINES - 3)
			continue;
		    if (abs(dest.x - x) < radius && abs(dest.y - y) < radius)
			continue;	/* skip interior */
		    if (step_ok(y, x, MONSTOK, &player)) {
			test.x = x;
			test.y = y;
			testdist = DISTANCE(y, x, dest.y, dest.x);
			if (roomin(&hero) != NULL && roomin(&hero) == roomin(&test))
			    testdist /= 2;
			if (testdist < bestdist) {
			    best.x = x;
			    best.y = y;
			    bestdist = testdist;
			}
		    }
		}
	    }
	    radius++;
	}
	if (best.x > -1 && best.y > -1) {
	    dest.x = best.x;
	    dest.y = best.y;
	}
    }

    /* we fixed it */
    if (step_ok(dest.y, dest.x, MONSTOK, &player)
	|| mvwinch(stdscr, dest.y, dest.x) == SECRETDOOR) {
	reachable = TRUE;
    } else {
	count /= 2;	/* give up sooner */
	reachable = FALSE;
    }

    return(dest);
}

int
check_doors(struct room *myroom, int minx, int maxx, int miny, int maxy)
{
    int i;
    int ndoors = 0;

    /*
     * check for doors we know
     */
    for (i = 0; i < 4; i++) {
	walls[i].door = FALSE;
    }
    for (i = 0; i < myroom->r_nexits; i++) {
	coord exit = myroom->r_exit[i];
	/* if it looks like a door */
	if (mvwinch(cw, exit.y, exit.x) == DOOR
	    || (exit.x == minx && mvwinch(cw,exit.y,minx-1) == PASSAGE)
	    || (exit.x == maxx && mvwinch(cw,exit.y,maxx+1) == PASSAGE)
	    || (exit.y == miny && mvwinch(cw,miny-1,exit.x) == PASSAGE)
	    || (exit.y == maxy && mvwinch(cw,maxy+1,exit.x) == PASSAGE)
	   ) {
	    if (exit.x == minx)
		walls[WEST].door = TRUE;
	    else if (exit.x == maxx)
		walls[EAST].door = TRUE;
	    else if (exit.y == miny)
		walls[NORTH].door = TRUE;
	    else if (exit.y == maxy)
		walls[SOUTH].door = TRUE;
	    ndoors++;	/* count doors */
	}
    }
    return(ndoors);
}

int
check_possible_doors(struct room *myroom, int minx, int maxx, int miny, int maxy, coord dest)
{
    int i, j;
    int npdoors = 0;

    /*
     * check for unknown patches of wall that may contain doors
     */
    if (myroom->r_flags & ISDARK) {
	for (i = 0; i < 4; i++) {
	    if (i == NORTH || i == SOUTH) {
		walls[i].min = minx + 1;
		walls[i].max = maxx - 1;
		walls[i].minpos = 7;
		walls[i].maxpos = LINES - 8;
	    } else {
		walls[i].min = miny + 1;
		walls[i].max = maxy - 1;
		walls[i].minpos = 6;
		walls[i].maxpos = COLS - 6;
	    }
	}
	walls[NORTH].pos = miny;
	walls[SOUTH].pos = maxy;
	walls[WEST].pos = minx;
	walls[EAST].pos = maxx;

	/*
	 * but only walls between us and our destination
	 */
	for (i = 0; i < 4; i++) {
	    if (i == NORTH) {
		if (dest.y >= miny) continue;
		if (dest.x <= minx) continue;
		if (dest.x >= maxx) continue;
	    }
	    if (i == SOUTH) {
		if (dest.y <= maxy) continue;
		if (dest.x <= minx) continue;
		if (dest.x >= maxx) continue;
	    }
	    if (i == EAST) {
		if (dest.x <= maxx) continue;
		if (dest.y <= miny) continue;
		if (dest.y >= maxy) continue;
	    }
	    if (i == WEST) {
		if (dest.x >= maxx) continue;
		if (dest.y <= miny) continue;
		if (dest.y >= maxy) continue;
	    }

	    int minpos = walls[i].minpos;
	    int maxpos = walls[i].maxpos;

	    if (((i == NORTH || i == WEST) && walls[i].pos > minpos)
	     || ((i == EAST || i == SOUTH) && walls[i].pos < maxpos)) {
		if (!walls[i].door) {
		    for (j = walls[i].min; j <= walls[i].max; j++) {
			if (((i == NORTH || i == SOUTH)
				&& mvwinch(cw, walls[i].pos, j) == ' ')
			 || ((i == EAST || i == WEST)
				&& mvwinch(cw, j, walls[i].pos) == ' ')
				) {
			    npdoors++;
			    break;
			}
		    }
		}
	    }
	}
    } 
    return (npdoors);
}

#endif
