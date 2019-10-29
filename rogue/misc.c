#include "curses.h"
#include "rogue.h"
#include <ctype.h>

/*
 * all sorts of miscellaneous routines
 *
 */

/*
 * tr_name:
 *	print the name of a trap
 */

char *
tr_name (ch)
int ch;
{
    char *s = "";

    switch (ch)
    {
	case TRAPDOOR:
	    s = "You found a trapdoor.";
	when BEARTRAP:
	    s = "You found a beartrap.";
	when SLEEPTRAP:
	    s = "You found a sleeping gas trap.";
	when ARROWTRAP:
	    s = "You found an arrow trap.";
	when TELTRAP:
	    s = "You found a teleport trap.";
	when DARTTRAP:
	    s = "You found a poison dart trap.";
	when POOL:
	    s = "You found a shimmering pool.";
	when MAZETRAP:
	    s = "You found a maze entrance.";
	when FIRETRAP:
	    s = "You found a fire trap.";
	when POISONTRAP:
	    s = "You found a poison pool trap.";
	when LAIR:
	    s = "You found a monster lair.";
	when RUSTTRAP:
	    s = "You found a rust trap.";
    }
    return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */

void 
look (wakeup)
bool wakeup;
{
    int x, y;
    char ch, och;
    int oldx, oldy;
    bool inpass, horiz, vert, do_light = FALSE, do_blank = FALSE;
    int passcount = 0;
    struct room *rp;
    int ey, ex;

    /* Are we moving vertically or horizontally? */
    if (runch == 'h' || runch == 'l') horiz = TRUE;
    else horiz = FALSE;
    if (runch == 'j' || runch == 'k') vert = TRUE;
    else vert = FALSE;

    getyx(cw, oldy, oldx);	/* Save current position */

    /* Blank out the floor around our last position and check for
     * moving out of a corridor in a maze.
     */
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) &&
	!(oldrp->r_flags & HASFIRE) && off(player, ISBLIND))
	do_blank = TRUE;
    for (x = player.t_oldpos.x - 1; x <= player.t_oldpos.x + 1; x++)
	for (y = player.t_oldpos.y - 1; y <= player.t_oldpos.y + 1; y++) {
	    ch = show(y, x);
	    if (do_blank && (y != hero.y || x != hero.x) && ch == FLOOR)
		mvwaddch(cw, y, x, ' ');
		
	    /* Moving out of a corridor? */
	    if (levtype == MAZELEV &&
		(ch != '|' && ch != '-') &&      /* Not a wall */
		((vert && x != player.t_oldpos.x && y==player.t_oldpos.y) ||
		 (horiz && y != player.t_oldpos.y && x==player.t_oldpos.x)))
		do_light = TRUE;	/* Just came to a turn */
	}

    inpass = ((rp = roomin(&hero)) == NULL); /* Are we in a passage? */

    /* Are we coming out of a wall into a corridor in a maze? */
    och = show(player.t_oldpos.y, player.t_oldpos.x);
    ch = show(hero.y, hero.x);
    if (levtype == MAZELEV && (och == '|' || och == '-' || och == SECRETDOOR) &&
	(ch != '|' && ch != '-' && ch != SECRETDOOR))
	do_light = off(player, ISBLIND); /* Light it up if not blind */

    /* Look around the player */
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y++)
	{
	    if (y <= 0 || y >= LINES - 2)
		continue;
	    if (isalpha(mvwinch(mw, y, x)))
	    {
		struct linked_list *it;
		struct thing *tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		if (it == NULL) 
		    continue;
		tp = (struct thing *) ldata(it);
		tp->t_oldch = mvinch(y, x);
		if (isatrap(tp->t_oldch)) {
		    struct trap *trp = trap_at(y, x);

		    tp->t_oldch = (trp->tr_flags & ISFOUND) ? tp->t_oldch
							    : trp->tr_show;
		}
		if (tp->t_oldch == FLOOR && (rp->r_flags & ISDARK)
		    && !(rp->r_flags & HASFIRE) && off(player, ISBLIND))
			tp->t_oldch = ' ';
	    }

	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if player is in a passage and
	     * check for maze turns
	     */
	    if (off(player, ISBLIND))
	    {
		if ((y == hero.y && x == hero.x)
		    || (inpass && (ch == '-' || ch == '|')))
			continue;

		/* Are we at a crossroads in a maze? */
		if (levtype == MAZELEV &&
		    (ch != '|' && ch != '-') &&      /* Not a wall */
		    ((vert && x != hero.x && y == hero.y) ||
		     (horiz && y != hero.y && x == hero.x)))
		    do_light = TRUE;	/* Just came to a turn */
	    }
	    else if (y != hero.y || x != hero.x)
		continue;

	    wmove(cw, y, x);
	    waddch(cw, ch);
	    if (door_stop && !firstmove && running)
	    {
		switch (runch)
		{
		    case 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch)
		{
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount++;
			break;
		    case FLOOR:
			/* Stop by new passages in a maze (floor next to us) */
			if ((levtype == MAZELEV) &&
			    ((horiz && x == hero.x && y != hero.y) ||
			     (vert && y == hero.y && x != hero.x)))
				running = FALSE;
		    case '|':
		    case '-':
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;

    /* Do we have to light up the area (just stepped into a new corridor)? */
    if (do_light && wakeup && 	/* wakeup will be true on a normal move */
	rp != NULL && !(rp->r_flags & ISDARK) &&   /* We have some light */
	!ce(hero, player.t_oldpos))  /* Don't do anything if we didn't move */
	    light(&hero);

    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    if (wakeup) {
	player.t_oldpos = hero; /* Don't change if we didn't move */
	oldrp = rp;
    }
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

int 
secretdoor (y, x)
int y;
int x;
{
    int i;
    struct room *rp;
    coord *cpp;
    static coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++)
	if (inroom(rp, cpp)) {
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return('-');
	    else
		return('|');
	}
    return('p');
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *
find_obj (y, x)
int y;
int x;
{
    struct linked_list *obj;
    struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    return NULL;
}

/*
 * eat:
 *	Player wants to eat something
 */

void 
eat ()
{
    struct linked_list *item;
    struct object *obj;
    int amount;
    /* adjust food for different screen sizes */
    float scale = (float) (LINES * COLS) / (30.0 * 80.0);

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_which) {
	case FD_RATION:
	    amount = HUNGERTIME + rnd(400) - 200;
	    if (rnd(100) > 70 && !(obj->o_flags & ISBLESSED)) {
		msg("Yuk, this food tastes awful.");
		pstats.s_exp++;
		check_level();
	    }
	    else
		msg("Yum, that tasted good.");
	when FD_FRUIT:
	    amount = 200 + rnd(HUNGERTIME);
	    msg("My, that was a yummy %s.", fruit);
	when FD_CRAM:
	    amount = rnd(HUNGERTIME / 2) + 600;
	    msg("The cram tastes dry in your mouth.");
	when FD_CAKES:
	    amount = (HUNGERTIME / 3) + rnd(600);
	    msg("Yum, the honey cakes tasted good.");
	when FD_LEMBA:
	    amount = (HUNGERTIME / 2) + rnd(900);
	    quaff(P_HEALING, FALSE);
	when FD_MIRUVOR:
	    amount = (HUNGERTIME / 3) + rnd(500);
	    quaff(P_HEALING, FALSE);
	    quaff(P_RESTORE, FALSE);
	otherwise:
	    msg("What a strange thing to eat!");
	    amount = HUNGERTIME;
    }
    food_left += (int) (amount * scale);
    if (food_left < 0) {
	food_left = 10;  /* workaround bug */
    }
    if (obj->o_flags & ISBLESSED) {
	food_left += 2 * amount;
	msg("You have a tingling feeling in your mouth.");
    }
    else if (food_left > scale * STOMACHSIZE) {
	food_left = scale * STOMACHSIZE;
	msg("You feel satiated and too full to move.");
	no_command = HOLDTIME;
    }
    hungry_state = F_OK;
    updpack(TRUE);
    if (obj == cur_weapon)
	cur_weapon = NULL;
    del_pack(item);
}

/*
 * Used to modify the player's strength
 * it keeps track of the highest it has been, just in case
 */

void 
chg_str (amt, both, lost)
int amt;
bool both;
bool lost;
{
    int ring_str;		/* ring strengths */
    struct stats *ptr;		/* for speed */

    ptr = &pstats;
    ring_str = ring_value(R_ADDSTR) + (on(player, POWERSTR) ? 10 : 0) +
			(on(player, SUPERHERO) ? 10 : 0);
    ptr->s_str -= ring_str;
    ptr->s_str += amt;

    if (ptr->s_str < 3) {
	ptr->s_str = 3;
        lost = FALSE;
    }
    else if (ptr->s_str > 25)
	ptr->s_str = 25;

    if (both)
	max_stats.s_str = ptr->s_str;
    if (lost)
	lost_str -= amt;

    ptr->s_str += ring_str;
    if (ptr->s_str < 0)
	ptr->s_str = 0;

    updpack(TRUE);
}

/*
 * Used to modify the player's dexterity
 * it keeps track of the highest it has been, just in case
 */

void 
chg_dext (amt, both, lost)
int amt;
bool both;
bool lost;
{
    int ring_dext;		/* ring strengths */
    struct stats *ptr;		/* for speed */

    ptr = &pstats;
    ring_dext = ring_value(R_ADDHIT) + (on(player, POWERDEXT) ? 10 : 0) +
			(on(player, SUPERHERO) ? 5 : 0);
    ptr->s_dext -= ring_dext;
    ptr->s_dext += amt;

    if (ptr->s_dext < 3) {
	ptr->s_dext = 3;
        lost = FALSE;
    }
    else if (ptr->s_dext > 25)
	ptr->s_dext = 25;

    if (both)
	max_stats.s_dext = ptr->s_dext;
    if (lost)
	lost_dext -= amt;

    ptr->s_dext += ring_dext;
    if (ptr->s_dext < 0)
	ptr->s_dext = 0;
}

/*
 * add_haste:
 *	add a haste to the player
 */

void 
add_haste (blessed)
bool blessed;
{
    short hasttime;

    if (blessed) hasttime = 10;
    else hasttime = 6;

    if (on(player, ISSLOW)) { /* Is person slow? */
	extinguish_fuse(FUSE_NOSLOW);
	noslow(NULL);

	if (blessed) hasttime = 4;
	else return;
    }

    if (on(player, ISHASTE)) {
	msg("You faint from exhaustion.");
	no_command += rnd(hasttime);
	lengthen_fuse(FUSE_NOHASTE, rnd(hasttime) + (roll(1,4) * hasttime));
    }
    else {
	turn_on(player, ISHASTE);
	light_fuse(FUSE_NOHASTE, 0, roll(hasttime, hasttime), AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

void 
aggravate ()
{
    struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/*
 * calm:
 *	calm the monsters around you
 */
void 
calm (blessed)
bool blessed;
{
    struct linked_list *mi;
    struct thing *tp;

    for (mi = mlist; mi != NULL; mi = next(mi)) {
	tp = THINGPTR(mi);
	if ((blessed && roomin(&hero) == roomin(&tp->t_pos)) ||
	    (abs(hero.x - tp->t_pos.x) < 10 && abs(hero.y - tp->t_pos.y) < 10)
	) {
	    tp->t_dest = &(tp->t_pos);
	    turn_off(*tp, ISRUN);
	    if (blessed && !save_throw(VS_MAGIC, tp)) {
		turn_off(*tp, ISMEAN);
	    }
	}
    }
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr (str)
char *str;
{
    switch (*str)
    {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}

/* 
 * see if the object is one of the currently used items
 */
int 
is_current (obj)
struct object *obj;
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || 
	obj == cur_ring[LEFT_1] || obj == cur_ring[LEFT_2] ||
	obj == cur_ring[LEFT_3] || obj == cur_ring[LEFT_4] ||
	obj == cur_ring[RIGHT_1] || obj == cur_ring[RIGHT_2] ||
	obj == cur_ring[RIGHT_3] || obj == cur_ring[RIGHT_4]) {
	msg("That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
int 
get_dir ()
{
    msg("Which direction? ");

    switch (readchar())
    {
	case 'h': case'H': delta.y =  0; delta.x = -1;
	when 'j': case'J': delta.y =  1; delta.x =  0;
	when 'k': case'K': delta.y = -1; delta.x =  0;
	when 'l': case'L': delta.y =  0; delta.x =  1;
	when 'y': case'Y': delta.y = -1; delta.x = -1;
	when 'u': case'U': delta.y = -1; delta.x =  1;
	when 'b': case'B': delta.y =  1; delta.x = -1;
	when 'n': case'N': delta.y =  1; delta.x =  1;
	otherwise: return FALSE;
    }
    msg("");

    if (on(player, ISHUH) && rnd(100) > 80)
	do
	{
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}


/*
 * Maze_view:
 *	Returns true if the player can see the specified location within
 *	the confines of a maze (within one column or row)
 */

bool 
maze_view (y, x)
int y;
int x;
{
    int start, goal, delta, ycheck, xcheck, absy, absx;
    bool row;

    /* Get the absolute value of y and x differences */
    absy = hero.y - y;
    absx = hero.x - x;
    if (absy < 0) absy = -absy;
    if (absx < 0) absx = -absx;

    /* Must be within one row or column */
    if (absy > 1 && absx > 1) return(FALSE);

    if (absy <= 1) {		/* Go along row */
	start = hero.x;
	goal = x;
	row = TRUE;
	ycheck = hero.y;
    }
    else {			/* Go along column */
	start = hero.y;
	goal = y;
	row = FALSE;
	xcheck = hero.x;
    }
    if (start <= goal) delta = 1;
    else delta = -1;

    while (start != goal) {
	if (row) xcheck = start;
	else ycheck = start;
	switch (winat(ycheck, xcheck)) {
	    case '|':
	    case '-':
	    case WALL:
	    case DOOR:
	    case SECRETDOOR:
		return(FALSE);
	}
	start += delta;
    }
    return(TRUE);
}

/*
 * listens: listen for monsters less than 5 units away
 */
void 
listens ()
{
    struct linked_list *item;
    struct thing *tp;
    int thief_bonus = -50;
    int mcount = 0;

    if (player.t_ctype == C_THIEF)
	thief_bonus = 10;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = (struct thing *) ldata(item);
	if (DISTANCE(hero.y, hero.x, tp->t_pos.y, tp->t_pos.x) < 64
	    && rnd(100) < (thief_bonus + 2*pstats.s_dext + 5*pstats.s_lvl)) {
 	    msg("You hear a%s %s nearby.", 
		vowelstr(monsters[tp->t_index].m_name),
		monsters[tp->t_index].m_name);
	    mcount ++;
	}
    }
    if (mcount == 0)
	msg("You hear nothing.");
}
