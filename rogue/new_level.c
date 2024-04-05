#include <errno.h>
#include <string.h>
#include "curses.h"
#include "rogue.h"
#include "state.h"

/*
 * new_level:
 *	Dig and draw a new level
 *
 */

void 
new_level (
    LEVTYPE ltype		/* designates type of level to create */
)
{
    int rm, i, cnt;
    char ch = TRAPDOOR;
    struct linked_list *item;
    struct thing *tp;
    coord stairs;

    /*
     * reset windows, player, monsters, objects
     */
    cleanup_old_level();

    if (level > max_level)
	max_level = level;

    levtype = ltype;
    if (ltype == THRONE) {
	do_throne();			/* do monster throne stuff */
    }
    else if (ltype == POSTLEV) {
	do_post();			/* do post stuff */
    }
    else if (ltype == MAZELEV) {
	do_maze();
	no_food++;
    }
    else {
	do_rooms();			/* Draw rooms */
	do_passages();			/* Draw passages */
	no_food++;
    }

    if (difficulty < 2) {
	if (hungry_state != F_OK)
	    no_food = 4;	/* let there be food */
    }

    /*
     * Place the staircase down.
     */
    cnt = 0;
    do {
        rm = rnd_room();
	rnd_pos(&rooms[rm], &stairs);
    } until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);
    addch(STAIRS);

    if (ltype == POSTLEV) {
	if (monsters[nummonst+2].m_wander) {
	    item = new_item(sizeof (struct thing));         /* Friendly Fiend */
	    new_monster(item, nummonst+2, &stairs, TRUE);
	}
    }
    else put_things(ltype);                     /* Place objects (if any) */

    /*
     * special surprise on the way back up
     * create Lucifer
     */
    if (has_artifact && level == 1) {
	struct thing *tp;
	item = new_item(sizeof (struct thing));
	new_monster(item, nummonst+1, &stairs, FALSE);
	tp = (struct thing *) ldata(item);
	turn_on(*tp, CANINWALL);
	turn_on(*tp, CANHUH);
	turn_on(*tp, CANSEE);
	turn_on(*tp, CANBLINK);
	turn_on(*tp, CANSNORE);
	turn_on(*tp, CANDISEASE);
	turn_on(*tp, NOCOLD);
	turn_on(*tp, TOUCHFEAR);
	turn_on(*tp, BMAGICHIT);
	turn_on(*tp, NOFIRE);
	turn_on(*tp, NOBOLT);
	turn_on(*tp, CANBLIND);
	turn_on(*tp, CANINFEST);
	turn_on(*tp, CANSMELL);
	turn_on(*tp, CANPARALYZE);
	turn_on(*tp, CANSTINK);
	turn_on(*tp, CANCHILL);
	turn_on(*tp, CANFRIGHTEN);
	turn_on(*tp, CANHOLD);
	turn_on(*tp, CANBRANDOM);
    }

    /*
     * maybe add a trading post 
     */
    if (level > 15 && rnd(11) == 7 && ltype == NORMLEV) {
	cnt = 0;
	do {
	    rm = rnd_room();
	    if (rooms[rm].r_flags & ISTREAS)
		continue;
	    rnd_pos(&rooms[rm], &stairs);
	} until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);
	addch(POST);
    }

    /*
     * Place the traps (except for trading post)
     */
    ntraps = 0;	/* No traps yet */
    if (levtype == NORMLEV) {
	if (rnd(10) < level) {
	    ntraps = rnd(level/4)+1;
	    if (ntraps > MAXTRAPS)
		ntraps = MAXTRAPS;
	    if (difficulty < 2) {
		ntraps = ntraps/2;  /* easy game has fewer traps */
		if (ntraps < 1)
		    ntraps = 1;
	    }
	    i = ntraps;
	    /*
	     * maybe a lair
	     */
	    if (level > 50 && rnd(wizard ? 3 : 10) == 0 && ltype == NORMLEV) {
		cnt = 0;
		do {
		    rm = rnd_room();
		    if (rooms[rm].r_flags & ISTREAS)
			continue;
		    rnd_pos(&rooms[rm], &stairs);
	        } until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);
	        addch(LAIR);
		i--;
		traps[i].tr_flags = 0;
		traps[i].tr_type = LAIR;
		traps[i].tr_show = FLOOR;
		traps[i].tr_pos = stairs;
	    }
	    while (i--)
	    {
		cnt = 0;
		do {
		    rm = rnd_room();
		    if (rooms[rm].r_flags & ISTREAS)
			continue;
		    rnd_pos(&rooms[rm], &stairs);
		} until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);

		traps[i].tr_flags = 0;
		switch(rnd(11)) {
		    case 0: ch = TRAPDOOR;
		    when 1: ch = BEARTRAP;
		    when 2: ch = SLEEPTRAP;
		    when 3: ch = ARROWTRAP;
		    when 4: ch = TELTRAP;
		    when 5: ch = DARTTRAP;
		    when 6: ch = POOL;
			    traps[i].tr_flags = ISFOUND;
		    when 7: ch = MAZETRAP;
		    when 8: ch = FIRETRAP;
		    when 9: ch = POISONTRAP;
		    when 10: ch = RUSTTRAP;
		}
		addch(ch);
		traps[i].tr_type = ch;
		traps[i].tr_show = FLOOR;
		traps[i].tr_pos = stairs;
	    }
	}
    }
    do {
	rm = rnd_room();
	if (levtype != THRONE && (rooms[rm].r_flags & ISTREAS))
	    continue;
	rnd_pos(&rooms[rm], &hero);
    } until( winat(hero.y, hero.x) == FLOOR &&
	     DISTANCE(hero.y, hero.x, stairs.y, stairs.x) > 16);
    oldrp = &rooms[rm];		/* Set the current room */
    player.t_oldpos = player.t_pos;	/* Set the current position */
    light(&hero);

    if (levtype != POSTLEV && levtype != THRONE) {
	if (on(player, BLESSMAP) && rnd(5) == 0) {
	    read_scroll(S_MAP, FALSE);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMAP);
	}
	if (on(player, BLESSGOLD) && rnd(5) == 0) {
	    read_scroll(S_GFIND, FALSE);
	    if (rnd(3) == 0)
		turn_off(player, BLESSGOLD);
	}
	if (on(player, BLESSFOOD) && rnd(5) == 0) {
	    read_scroll(S_FOODFIND, FALSE);
	    if (rnd(3) == 0)
		turn_off(player, BLESSFOOD);
	}
	if (on(player, BLESSMAGIC) && rnd(5) == 0) {
	    quaff(P_TFIND, FALSE);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMAGIC);
	}
	if (on(player, BLESSMONS) && rnd(5) == 0) {
	    quaff(P_MFIND, FALSE);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMONS);
	}
    }

    if (ISWEARING(R_ADORNMENT) || 
		(cur_armor != NULL && cur_armor->o_which == MITHRIL)) {
	bool greed = FALSE;
	for (item = mlist; item != NULL; item = next(item)) {
	    tp = (struct thing *) ldata(item);
	    if (on(*tp, ISGREED) && off(*tp,ISFRIENDLY)) {
		turn_on(*tp, ISRUN);
		turn_on(*tp, ISMEAN);
		tp->t_dest = &hero;
		greed = TRUE;
	    }
	}
	if (greed)
	    msg("An uneasy feeling comes over you.");
    }

    if (is_carrying(TR_PHIAL) && is_active(TR_PHIAL)) {
	if (roomin(&hero)->r_flags & ISDARK)
	    msg("The Phial banishes the darkness.");
        read_scroll(S_LIGHT, TRUE);
    }

    if (is_carrying(TR_SILMARIL) && is_active(TR_SILMARIL)
     && level < max_level) {
	msg("The Silmaril reveals all.");
        read_scroll(S_MAP, FALSE);
    }

    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
    status(TRUE);

    if (autosave == TRUE
	&& levtype == NORMLEV
	&& hungry_state < F_FAINT
	&& pstats.s_hpt > max_stats.s_hpt / 2
	&& off(player, HASINFEST)
	&& off(player, ISBLIND)
	&& off(player, PERMBLIND)
	&& off(player, HASDISEASE)) {
	char fname[200];
	FILE *savefd;

	strcpy(fname, home);
	strcat(fname, "rogue.asave");
        if ((savefd = fopen(fname, "wb")) == NULL) {
            msg("");
            msg("Autosave error: %s.%s", strerror(errno));    /* fake perror() */
        } else {
	    save_file(savefd);
	    fclose(savefd);
        }

    }
}

/*
 * Pick a room that is really there
 */

int 
rnd_room ()
{
    int rm;

    if (levtype != NORMLEV)
	rm = 0;
    else do
	{
	    rm = rnd(MAXROOMS);
	} while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_things:
 *	put potions and scrolls on this level
 */

void 
put_things (
    LEVTYPE ltype		/* designates type of level to create */
)
{
    int i, rm, cnt;
    struct linked_list *item;
    struct object *cur;
    bool got_unique = FALSE;
    int length, width, maxobjects;
    coord tp;

    /*
     * Once you have found an artifact, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (has_artifact && level < max_level && ltype != THRONE)
	return;

    /* 
     * There is a chance that there is a treasure room on this level 
     * Increasing chance after level 15 
     */
    if (ltype != MAZELEV && rnd(30) < level - 14) {	
	int i, j;
	struct room *rp;

	/* Count the number of free spaces */
	i = 0;	/* 0 tries */
	do {
	    rp = &rooms[rnd_room()];
	    width = rp->r_max.y - 2;
	    length = rp->r_max.x - 2;
	} until ((width*length <= MAXTREAS) || (i++ > MAXROOMS*4));

	/* Mark the room as a treasure room */
	rp->r_flags |= ISTREAS;

	/* Make all the doors secret doors */
	for (i=0; i<rp->r_nexits; i++) {
	    cmov(rp->r_exit[i]);
	    addch(SECRETDOOR);
	}

	/* Put in the monsters and treasures */
	for (j=1; j<rp->r_max.y-1; j++)
	    for (i=1; i<rp->r_max.x-1; i++) {
		coord trp;

		trp.y = rp->r_pos.y+j;
		trp.x = rp->r_pos.x+i;

		/* Monsters */
		if ((rnd(100) < (MAXTREAS*100)/(width*length)) &&
		    (mvwinch(mw, rp->r_pos.y+j, rp->r_pos.x+i) == ' ')) {
		    struct thing *tp;

		    /* Make a monster */
		    item = new_item(sizeof *tp);
		    tp = THINGPTR(item);

		    /* 
		     * Put it there and aggravate it (unless it can escape) 
		     * only put one UNIQUE per treasure room at most
		     */
		    if (got_unique)
			new_monster(item,randmonster(FALSE, TRUE),&trp,FALSE);
		    else {
			new_monster(item,randmonster(FALSE, FALSE),&trp,FALSE);
			if (on(*tp, ISUNIQUE))
			    got_unique = TRUE;
		    }
		    turn_on(*tp, ISMEAN);
		    if (off(*tp, CANINWALL)) {
			tp->t_dest = &hero;
			turn_on(*tp, ISRUN);
		    }

		    /* If it is a mimic, undisguise it */
		    if (on(*tp, ISDISGUISE)) turn_off(*tp, ISDISGUISE);
		}

		/* Treasures */
		if ((rnd(100) < (MAXTREAS*100)/(width*length)) &&
		    (mvinch(rp->r_pos.y+j, rp->r_pos.x+i) == FLOOR)) {
		    item = new_thing();
		    attach(lvl_obj, item);
		    cur = (struct object *) ldata(item);
	
		    mvaddch(trp.y, trp.x, cur->o_type);
		    cur->o_pos = trp;
		}
	    }
    }

    /*
     * Do MAXOBJ attempts to put things on a level, maybe
     */
    maxobjects = ltype == THRONE ? rnd(3 * MAXOBJ) + 5 : MAXOBJ;
    for (i = 0; i < maxobjects; i++)
	if (rnd(100) < 40 || ltype == THRONE)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing();
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    /*
	     * Put it somewhere
	     */
	    cnt = 0;
	    do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &tp);
	    } until (winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
    /*
     * If player is really deep in the dungeon and hasn't found an
     * artifact yet, put it somewhere on the ground
     */
    if (make_artifact())
    {
	item = new_item(sizeof *cur);
	attach(lvl_obj, item);
	cur = (struct object *) ldata(item);
	new_artifact(-1, cur);
	cnt = 0;
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &tp);
	} until (winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	mvaddch(tp.y, tp.x, cur->o_type);
	cur->o_pos = tp;
    }
}

/*
 * do_throne:
 *	Put a monster's throne room and monsters on the screen
 */
void 
do_throne ()
{
	coord mp;
	int save_level;
	int i;
	struct room *rp;
	struct thing *tp;
	struct linked_list *item;

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
	    rp->r_nexits = 0;			/* no exits */
	    rp->r_flags = ISGONE;		/* kill all rooms */
	}
	rp = &rooms[0];				/* point to only room */
	rp->r_flags = 0;			/* this room NOT gone */
	rp->r_max.x = 40;
	rp->r_max.y = 10;			/* 10 * 40 room */
	rp->r_pos.x = (COLS - rp->r_max.x) / 2;	/* center horizontal */
	rp->r_pos.y = 1;			/* 2nd line */
	draw_room(rp);				/* draw the only room */

	save_level = level;			/* save current level */
	if (max_level < 35 && difficulty <= 3)
	    level = min(2*level, level + roll(4,6));  /* weaker monsters */
	else
	    level = max(2*level, level + roll(4,6));

	for (i = roll(4,10); i >= 0; i--) {
	    if (max_level < 35 && difficulty <= 3 && rnd(2))  /* fewer monsters */
		continue;
	    item = new_item(sizeof *tp);
	    tp = THINGPTR(item);
	    do
	    {
		rnd_pos(rp, &mp);
	    } until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);
	    new_monster(item, randmonster(FALSE, FALSE), &mp, FALSE);
	    turn_on(*tp, CANSEE);
	    tp->t_stats.s_hpt *= 2;
	    tp->t_stats.s_lvl *= 2;
	    tp->t_stats.s_arm -= roll(2,3);
	}
	
	level = save_level + roll(2,3);		/* send the hero down */

	i = nummonst - roll(1, NUMUNIQUE);
	item = new_item(sizeof *tp);
	tp = THINGPTR(item);
	do
	{
	    rnd_pos(rp, &mp);
	} until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);
	new_monster(item, i, &mp, FALSE);
        turn_on(*tp, CANINWALL);
	turn_on(*tp, CANSEE);
	tp->t_stats.s_hpt *= 2;
	tp->t_stats.s_lvl *= 2;
	tp->t_stats.s_arm -= roll(2,3);
	msg("You have been summoned by a %s.", monsters[i].m_name);
	aggravate();
}

void
cleanup_old_level() {
    struct linked_list *item;
    struct thing *tp;

    /* Start player off right */
    turn_off(player, ISHELD);
    turn_off(player, ISFLEE);
    extinguish_fuse(FUSE_SUFFOCATE);
    hold_count = 0;
    trap_tries = 0;
    count = FALSE;

    /* clear windows */
    wclear(cw);
    wclear(mw);
    clear();

    /*
     * If we missed a UNIQUE, put it back in the monster table for next time.
     * Also empty any monsters' packs.
     */
    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	if (on(*tp, ISUNIQUE)) 
	    monsters[tp->t_index].m_normal = TRUE;
	free_list(tp->t_pack);	/* empty monster's pack */
    }

    /*
     * Free up the monsters and objects on the last level
     */
    free_list(mlist);
    free_list(lvl_obj);
}
