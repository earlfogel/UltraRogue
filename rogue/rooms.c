/*
 * Draw the nine rooms on the screen
 *
 * @(#)rooms.c	3.8 (Berkeley) 6/15/81
 */

#include "curses.h"
#include "rogue.h"

void 
do_rooms ()
{
    int i, j, cnt=0;
    struct room *rp;
    struct linked_list *item;
    struct thing *tp;
    int left_out;
    coord top;
    coord bsze;
    coord mp;

    /*
     * bsze is the maximum room size
     */
    bsze.x = COLS/3;
    bsze.y = (LINES-2)/3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	rp->r_nexits = rp->r_flags = rp->r_fires = 0;
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
	rooms[rnd_room()].r_flags |= ISGONE;
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++)
    {
	bool has_gold=FALSE;

	/*
	 * Find upper left corner of box that this room goes in
	 */
	top.x = (i%3)*bsze.x;
	top.y = i/3*bsze.y + 1;
	if (rp->r_flags & ISGONE)
	{
	    /*
	     * Place a gone room.  Make certain that there is a blank line
	     * for passage drawing.
	     */
	    do
	    {
		rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
		rp->r_pos.y = top.y + rnd(bsze.y-2) + 1;
		rp->r_max.x = -COLS;
		rp->r_max.x = -LINES;
	    } until(rp->r_pos.y > 0 && rp->r_pos.y < LINES-2);
	    continue;
	}
	if (rnd(10) < level-1)
	    rp->r_flags |= ISDARK;
	/*
	 * Find a place and size for a random room
	 */
	do
	{
	    rp->r_max.x = rnd(bsze.x - 4) + 4;
	    rp->r_max.y = rnd(bsze.y - 4) + 4;
	    rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
	    rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
	} until (rp->r_pos.y != 0);

	/* Draw the room */
	draw_room(rp);

	/*
	 * Put the gold in
	 */
	if (rnd(100) < 50 && (!has_artifact || level >= max_level))
	{
	    struct linked_list *item;
	    struct object *cur;
	    coord tp;

	    has_gold = TRUE;	/* This room has gold in it */

	    item = spec_item(GOLD, NULL, NULL, NULL);
	    cur = OBJPTR(item);

	    /* Put the gold into the level list of items */
	    attach(lvl_obj, item);

	    /* Put it somewhere */
	    rnd_pos(rp, &tp);
	    mvaddch(tp.y, tp.x, GOLD);
	    cur->o_pos = tp;
	    if (roomin(&tp) != rp) {
		endwin();
		abort();
	    }
	}

	/*
	 * Put the monster in
	 */
	if (rnd(100) < (has_gold ? 80 : 25))
	{
	    item = new_item(sizeof *tp);
	    tp = THINGPTR(item);
	    do
	    {
		rnd_pos(rp, &mp);
	    } until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);
	    new_monster(item, randmonster(FALSE, FALSE), &mp, FALSE);
	    /*
	     * See if we want to give it a treasure to carry around.
	     */
	    if (rnd(100) < monsters[tp->t_index].m_carry)
		attach(tp->t_pack, new_thing());

	    /*
	     * If it has a fire, mark it
	     */
	    if (on(*tp, HASFIRE)) {
		rp->r_flags |= HASFIRE;
		rp->r_fires++;
	    }
		
	    /*
	     * If it carries gold, give it some
	     */
	    if (on(*tp, CARRYGOLD)) {
		struct object *cur;

		item = spec_item(GOLD, NULL, NULL, NULL);
		cur = OBJPTR(item);
		cur->o_count = GOLDCALC + GOLDCALC + GOLDCALC;
		cur->o_pos = tp->t_pos;
		attach(tp->t_pack, item);
	    }

	    /* swarms and flocks of monsters */
	    if (difficulty > 2 && off(*tp, ISFRIENDLY)) {
		cnt = 0;
		j = rnd(7);
		if (on(*tp, ISSWARM) && j < 2)
		    cnt = roll(2, 4);
		else if (on(*tp, ISFLOCK) && j < 2)
		    cnt = roll(1, 4);
		if (level < 5 && difficulty <= 3) cnt /= 2;
		for (j = 1; j <= cnt; j++) {
		    struct thing  *mp = creat_mons(tp, tp->t_index, FALSE);
		    if (mp != NULL) {
			turn_off(*mp, ISFRIENDLY);
		    }
		}
	    }
	}
    }
}

/*
 * Draw a box around a room
 */

void 
draw_room (struct room *rp)
{
    int j, k;

    move(rp->r_pos.y, rp->r_pos.x+1);
    vert(rp->r_max.y-2);				/* Draw left side */
    move(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x);
    horiz(rp->r_max.x);					/* Draw bottom */
    move(rp->r_pos.y, rp->r_pos.x);
    horiz(rp->r_max.x);					/* Draw top */
    vert(rp->r_max.y-2);				/* Draw right side */
    /*
     * Put the floor down
     */
    for (j = 1; j < rp->r_max.y-1; j++)
    {
	move(rp->r_pos.y + j, rp->r_pos.x+1);
	for (k = 1; k < rp->r_max.x-1; k++)
	    addch(FLOOR);
    }
}

/*
 * horiz:
 *	draw a horizontal line
 */

void 
horiz (int cnt)
{
    while (cnt--)
	addch('-');
}

/*
 * vert:
 *	draw a vertical line
 */

void 
vert (int cnt)
{
    int x, y;

    getyx(stdscr, y, x);
    x--;
    while (cnt--) {
	move(++y, x);
	addch('|');
    }
}

/*
 * rnd_pos:
 *	pick a random spot in a room
 */

void 
rnd_pos (struct room *rp, coord *cp)
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x-2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y-2) + 1;
}
