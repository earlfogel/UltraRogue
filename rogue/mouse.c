/*
 * mouse movement
 *
 */

#ifdef MOUSE

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

bool firststep = FALSE;
static int nsearch = 3;

char 
do_mousemove (dest, prev)
coord dest;
coord prev;
{
    char ch = ' ';
    static coord indoor = {0,0};
    static coord prevdest = {0,0};
    static coord mydest = {0,0};
    int nsecret = 0;

    if (count) {
	int x, y;
	int curdist, bestdist, bestx, besty;
	int nmoves = 0;
	int ndoors = 0;
	int nmonst = 0;
	coord pos;

	if (dest.x != prevdest.x || dest.y != prevdest.y) {
	    indoor.x = indoor.y = 0;
	    prev.x = prev.y = 0;  /* local variables */
	    prevdest.x = dest.x;
	    prevdest.y = dest.y;
	    mydest.x = dest.x;
	    mydest.y = dest.y;
	} else {
	    firststep = FALSE;
	}
	/*
	 * choose best direction for this step
	 */
	bestx = hero.x;
	besty = hero.y;
	/*
	 * if we're not in the same room as our destination,
	 * choose the exit nearest to that destination.
	 */
	if (roomin(&hero) != NULL
	    && off(player, CANINWALL)) {
	    int i;
	    for (i = 0; i < roomin(&hero)->r_nexits; i++) {
		ndoors++;	/* count doors */
	    }
	}
	if (roomin(&hero) != NULL
	    && roomin(&hero) != roomin(&dest)
	    && roomin(&hero) != roomin(&mydest)
	    && winat(hero.y, hero.x) != DOOR
	    && off(player, CANINWALL)) {
	    struct room *myroom = roomin(&hero);
	    struct room *destroom = roomin(&dest);
	    coord exit, bestdoor;
	    int i, doordist;
	    int minx = myroom->r_pos.x;
	    int miny = myroom->r_pos.y;
	    int maxx = myroom->r_pos.x + myroom->r_max.x - 1;
	    int maxy = myroom->r_pos.y + myroom->r_max.y - 1;
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
		if (winat(exit.y, exit.x) == DOOR
		    || (exit.x == minx && show(exit.y,minx-1) == PASSAGE)
		    || (exit.x == maxx && show(exit.y,maxx+1) == PASSAGE)
		    || (exit.y == miny && show(miny-1,exit.x) == PASSAGE)
		    || (exit.y == maxy && show(maxy+1,exit.x) == PASSAGE)
		) {
		    if (exit.x == indoor.x && exit.y == indoor.y
			&& ndoors > 1)
			continue; /* don't pick the door we came in */
		    if (winat(exit.y, exit.x) == ' ')
			continue;  /* only pick doors we know */
		    doordist = DISTANCE(hero.y, hero.x, exit.y, exit.x)
				+ DISTANCE(exit.y, exit.x, dest.y, dest.x);
		    if (mindy > maxy && exit.y != maxy) {
			if (exit.y == miny)
			    doordist *= 8;
			else
			    doordist *= 4;
		    }
		    if (maxdy < miny && exit.y != miny) {
			if (exit.y == maxy)
			    doordist *= 8;
			else
			    doordist *= 4;
		    }
		    if (mindx > maxx && exit.x != maxx) {
			if (exit.x == minx)
			    doordist *= 8;
			else
			    doordist *= 4;
		    }
		    if (maxdx < minx && exit.x != minx) {
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
	    mydest.x = bestdoor.x;
	    mydest.y = bestdoor.y;
	} else if (
	    (roomin(&hero) != NULL
		&& roomin(&hero) != roomin(&dest)
		&& roomin(&hero) != roomin(&mydest)
	    )
	    || winat(hero.y, hero.x) == DOOR
	    || on(player, CANINWALL)
	) {
	    mydest.x = dest.x;
	    mydest.y = dest.y;
	}
	curdist = DISTANCE(mydest.y, mydest.x, hero.y, hero.x) + 1;
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
		    if (!ISWEARING(R_LEVITATION) && off(player, CANFLY) && isatrap(mvwinch(cw, y, x)))
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
		if (DISTANCE(mydest.y, mydest.x, y, x) < bestdist
		    || nmoves == 1) {
		    bestx = x;
		    besty = y;
		    bestdist = DISTANCE(mydest.y, mydest.x, y, x);
		}
		/*
		 * if this move goes into the room we want, take it
		 */
		pos.x = x; pos.y = y;
		if (roomin(&pos) && !roomin(&hero)
		    && roomin(&pos) == roomin(&dest)) {
		    bestx = x;
		    besty = y;
		    bestdist = 1;
		}
	    }
	}
	if (winat(hero.y,hero.x) == DOOR
	    /* && winat(besty,bestx) != PASSAGE */
	    && winat(prev.y,prev.x) == PASSAGE
	    ) { /* enter a room */
	    if (1 || ndoors > 1) {
		indoor.x = hero.x;
		indoor.y = hero.y;
	    }
	}
	if (winat(hero.y,hero.x) == DOOR
	    && winat(prev.y,prev.x) != PASSAGE
	    ) { /* leave a room */
		indoor.x = 0;
		indoor.y = 0;
		mydest.x = dest.x;
		mydest.y = dest.y;
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
	} else if (bestdist < curdist || nmoves == 1 || ndoors == 1
	    || indoor.x > 0 || firststep
	    || winat(hero.y, hero.x) == PASSAGE
	    || winat(hero.y, hero.x) == DOOR
	    || levtype == MAZELEV) {
	    if (bestx < hero.x && besty == hero.y)
		ch = 'h';
	    else if (bestx == hero.x && besty > hero.y)
		ch = 'j';
	    else if (bestx == hero.x && besty < hero.y)
		ch = 'k';
	    else if (bestx > hero.x && besty == hero.y)
		ch = 'l';
	    else if (bestx < hero.x && besty < hero.y)
		ch = 'y';
	    else if (bestx > hero.x && besty < hero.y)
		ch = 'u';
	    else if (bestx < hero.x && besty > hero.y)
		ch = 'b';
	    else if (bestx > hero.x && besty > hero.y)
		ch = 'n';
	    firststep = FALSE;
	    if (count < 50 && bestdist < curdist && rnd(9)>0) {
		count++;  /* don't stop just yet */
	    }
	}
	if ((hero.y == dest.y && hero.x == dest.x)
	    ) {  /* we made it */
	    count = 1;
	    ch = ' ';
	    after = FALSE;
	    mousemove = FALSE;
	}
	moving = TRUE;
    } else {
	mousemove = FALSE;
	ch = '.';
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
	nsearch = 3;
	ch = ' ';  /* do nothing, for now */
    }
    return(ch);
}

#endif
