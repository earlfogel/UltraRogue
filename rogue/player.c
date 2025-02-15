/*
 * This file contains functions for dealing with special player abilities
 * @(#)player.c	7.0 (Indian Hill) 10/5/82
 */

#include <ctype.h>
#include "curses.h"
#include "rogue.h"

/* Constitution bonus */

int 
const_bonus ()	/* Hit point adjustment for changing levels */
{
    if (pstats.s_const > 6 && pstats.s_const <= 12) 
	return(0);
    if (pstats.s_const > 12) 
	return(pstats.s_const-12);
    if (pstats.s_const > 3) 
	return(-1);
    return(-2);
}

/* Routines for thieves */

/*
 * gsense:
 *	Sense gold
 */

void 
gsense ()
{
    /* Only thieves can do this */
    if (player.t_ctype != C_THIEF) {
	msg("You seem to have no gold sense.");
	return;
    }

    if (lvl_obj != NULL) {
	struct linked_list *gitem;
	struct object *cur;
	int gtotal = 0;

	wclear(hw);
	for (gitem = lvl_obj; gitem != NULL; gitem = next(gitem)) {
	    cur = (struct object *) ldata(gitem);
	    if (cur->o_type == GOLD) {
		gtotal += cur->o_count;
		mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, GOLD);
	    }
	}
	if (gtotal) {
	    s_know[S_GFIND] = TRUE;
	    msg("You sense gold!");
	    overlay(hw,cw);
	    return;
	}
    }
    msg("You can sense no gold on this level.");
}


/*
 * steal:
 *	Steal in direction given in delta
 */

void 
steal ()
{
    struct linked_list *item;
    struct thing *tp;
    char *mname;
    coord new_pos;
    short thief_bonus = -50;

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES-3 ||
	new_pos.x < 0 || new_pos.x > COLS-1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("Nothing to steal from.");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
	debug("Steal from what @ %d,%d?", new_pos.y, new_pos.x);
	return;
    }
    tp = THINGPTR(item);
    mname = monsters[tp->t_index].m_name;

    /* Can player steal something unnoticed? */
    if (player.t_ctype == C_THIEF) thief_bonus = 10;

    if (rnd(100) <
	(thief_bonus + 2*pstats.s_dext + 5*pstats.s_lvl -
	 5*(tp->t_stats.s_lvl - 3))) {
	struct linked_list *s_item, *pack_ptr;
	short count = 0;

	s_item = NULL;	/* Start stolen goods out as nothing */

	/* Find a good item to take */
	for (pack_ptr=tp->t_pack; pack_ptr != NULL; pack_ptr=next(pack_ptr))
	    if (rnd(++count) == 0) s_item = pack_ptr;

	/* if we have a merchant, get an item */
	if (s_item == NULL && tp->t_index == nummonst)
	    s_item = new_thing();

	/* if we have a shop keeper, get an item from the floor */
	count = 0;
	if (s_item == NULL && lvl_obj != NULL && tp->t_index == nummonst+2) {
	    for (pack_ptr=lvl_obj; pack_ptr != NULL; pack_ptr=next(pack_ptr)) {
		if (rnd(++count) == 0) s_item = pack_ptr;
	    }
	}

	/* Find anything? */
	if (s_item == NULL) {
	    /* are they standing on something we can steal? */
	    if ((s_item = find_obj(new_pos.y, new_pos.x)) != NULL) {
		/* temporarily add it to the monster's pack
		 * so we can steal it */
		detach(lvl_obj, s_item);
		attach(tp->t_pack, s_item);
		tp->t_oldch = (roomin(&tp->t_pos) == NULL ? PASSAGE : FLOOR);
	    } else {
		msg("The %s has nothing to steal.", mname);
		return;
	    }
	}

	if (tp->t_index == nummonst+2) {
	    /* Take it from the shop */
	    if (lvl_obj) detach(lvl_obj, s_item);
	} else {
	    /* Take it from monster */
	    if (tp->t_pack) detach(tp->t_pack, s_item);
	}

	/* Give it to player */
	if (add_pack(s_item, FALSE) == FALSE) {
	   (OBJPTR(s_item))->o_pos = hero;
	   fall(s_item, TRUE);
	}

	/* Get points for stealing */
	pstats.s_exp += tp->t_stats.s_exp/2;

	/*
	 * Do adjustments if player went up a level
	 */
	check_level();
    }

    else {

	/* Annoy monster (maybe) */
	if (rnd(35) >= pstats.s_dext + thief_bonus && on(*tp, ISFRIENDLY)) {
	    msg("Your attempt fails and the %s notices.", monsters[tp->t_index].m_name);
	    turn_off(*tp, ISFRIENDLY);
	    turn_on(*tp, ISMEAN);
	    runto(&new_pos, &hero);
	} else {
	    msg("Your attempt fails.");
	}
    }
}


/* Routines for clerics */


void 
pray ()
{
    short i, num_prayers, which_prayer, pray_points;
    bool nohw = FALSE;
    int c, prayer_cost;
    bool boost = FALSE, blessed = FALSE;
    int maxboost;
    static int repeat_prayer = -1;
    int min_wisdom = 17;

    if (ring_blessed(R_WIZARD))
	min_wisdom--;
    else if (ring_cursed(R_WIZARD))
	min_wisdom++;

    if (player.t_ctype != C_CLERIC && pstats.s_wisdom < min_wisdom) {
	msg("You are not permitted to pray.");
	return;
    }

    /* Get the number of available prayers */
    if (pstats.s_wisdom >= min_wisdom) 
	num_prayers = (pstats.s_wisdom - min_wisdom + 2) / 2;
    else 
	num_prayers = 0;

    if (player.t_ctype == C_CLERIC)
	num_prayers += pstats.s_lvl;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
 		&& ISWEARING(R_WIZARD))
	num_prayers *= 2;

    pray_points = pstats.s_wisdom * pstats.s_lvl;
    if(ISWEARING(R_WIZARD))
	pray_points *= 2;

    if (difficulty > 2) {
	if (player.t_ctype == C_CLERIC) {
	    if (num_prayers > 1)
		num_prayers *= 2.0/3.0;
	    if (pray_points > 20)
		pray_points = 20 + (pray_points-20) * 2.0/3.0;
	} else {
	    pray_points /= 2;
	}
    } else if (difficulty == 2) {
	if (pray_points > 20)
	    pray_points = 20 + (pray_points-20) * 3.0/4.0;
    }

    maxboost = num_prayers - 3;

    if (num_prayers > maxprayers) 
	num_prayers = maxprayers;

    if (repeat_prayer >= 0) {
        which_prayer = repeat_prayer;
        goto do_prayer;
    }

    /* Prompt for prayer */
    msg("Which prayer are you offering [%d points left]? (* for list): ",
			pray_points - pray_time);

    c = readchar();
    if (c == ESCAPE) {
	msg("");
	after = FALSE;
	count = 0;
	return;
    }
    if (isupper(c)) {
	boost = TRUE;
	c = tolower(c);
    }
    which_prayer = (short) (c - 'a');
    if (which_prayer >= 0 && which_prayer < num_prayers) {
	nohw = TRUE;
	msg("");

    } else if (slow_invent) {
	char c;

	for (i=0; i<num_prayers; i++) {
	    msg("");
	    mvwaddch(cw, 0, 0, '[');
	    waddch(cw, (char) ((short) 'a' + i));
	    wprintw(cw, ", %d/%d] A prayer for ", cleric_spells[i].s_cost,
			pray_points - pray_time);
	    if (cleric_spells[i].s_type == TYP_POTION)
		waddstr(cw, p_magic[cleric_spells[i].s_which].mi_name);
	    else if (cleric_spells[i].s_type == TYP_SCROLL)
		waddstr(cw, s_magic[cleric_spells[i].s_which].mi_name);
	    else if (cleric_spells[i].s_type == TYP_STICK)
		waddstr(cw, ws_magic[cleric_spells[i].s_which].mi_name);
	    waddstr(cw, morestr);
	    draw(cw);
	    do {
		c = readchar();
	    } while (c != ' ' && c != ESCAPE);
	    if (c == ESCAPE)
		break;
	}
	msg("");
	mvwaddstr(cw, 0, 0, "Which prayer are you offering? ");
	draw(cw);
    }
    else {
	/* Set up for redraw */
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	/* Now display the possible prayers */
	wclear(hw);
	touchwin(hw);
	for (i=0; i<num_prayers; i++) {
	    mvwaddch(hw, i+2, 0, '[');
	    waddch(hw, (char) ((short) 'a' + i));
	    wprintw(hw, ", %2d] A prayer for ", cleric_spells[i].s_cost);
	    if (cleric_spells[i].s_type == TYP_POTION)
		waddstr(hw, p_magic[cleric_spells[i].s_which].mi_name);
	    else if (cleric_spells[i].s_type == TYP_SCROLL)
		waddstr(hw, s_magic[cleric_spells[i].s_which].mi_name);
	    else if (cleric_spells[i].s_type == TYP_STICK)
		waddstr(hw, ws_magic[cleric_spells[i].s_which].mi_name);
	}
	wmove(hw, 0, 0);
	wprintw(hw, "Which prayer are you offering (%d points left)? ",
				pray_points - pray_time);
	draw(hw);
    }

    if (!nohw) {
	c = wgetch(hw);
	if (isupper(c)) {
	    boost = TRUE;
	    c = tolower(c);
	}
	which_prayer = (short) (c - 'a');
	while (which_prayer < 0 || which_prayer >= num_prayers) {
	    if (c == ESCAPE) {
		after = FALSE;
		return;
	    }
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed prayers. ");
	    draw(hw);
	    c = wgetch(hw);
	    if (isupper(c)) {
		boost = TRUE;
		c = tolower(c);
	    }
	    which_prayer = (short) (c - 'a');
	}
    }

do_prayer:

    prayer_cost = cleric_spells[which_prayer].s_cost;
    if (cleric_spells[which_prayer].s_blessed) {
	blessed = TRUE;
	boost = FALSE;
    }
    if (boost && which_prayer <= maxboost) {
	blessed = TRUE;
	prayer_cost *= 3;
    }
    if ((prayer_cost + pray_time) > pray_points) {
	msg("Your prayer fails.");
	count = 0;
	repeat_prayer = -1;
	return;
    }

    msg("Your prayer has been granted.");

    if (cleric_spells[which_prayer].s_type == TYP_POTION)
	quaff(	cleric_spells[which_prayer].s_which,
		blessed);
    else
	read_scroll(	cleric_spells[which_prayer].s_which,
			blessed);

    pray_time += prayer_cost;

    if (count == 0)
	repeat_prayer = -1;
    else if (repeat_prayer < 0) {
	repeat_prayer = which_prayer;
    }
}


/*
 * affect:
 *	cleric affecting undead
 */

void 
affect ()
{
    struct linked_list *item;
    struct thing *tp;
    char *mname;
    coord new_pos;

    if (player.t_ctype != C_CLERIC) {
	msg("Only clerics can affect undead.");
	return;
    }

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES-3 ||
	new_pos.x < 0 || new_pos.x > COLS-1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("Nothing to affect.");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
	debug("Affect what @ %d,%d?", new_pos.y, new_pos.x);
	return;
    }
    tp = (struct thing *) ldata(item);
    mname = monsters[tp->t_index].m_name;

    if (off(*tp, ISUNDEAD) || on(*tp, WASTURNED)) goto annoy;

    /* Can cleric kill it? */
    if (pstats.s_lvl >= 3 * tp->t_stats.s_lvl) {
	msg("You have destroyed the %s.", mname);
	killed(item, FALSE, TRUE);
	return;
    }

    /* Can cleric turn it? */
    if (rnd(100) + 1 >
	 (100 * ((2 * tp->t_stats.s_lvl) - pstats.s_lvl)) / pstats.s_lvl) {
	/* Make the monster flee */
	turn_on(*tp, WASTURNED);	/* No more fleeing after this */
	turn_on(*tp, ISFLEE);

	/* Let player know */
	msg("You have turned the %s.", mname);

	/* get points for turning monster */
	pstats.s_exp += tp->t_stats.s_exp/2;

	/* If monster was suffocating, stop it */
	if (on(*tp, DIDSUFFOCATE)) {
	    turn_off(*tp, DIDSUFFOCATE);
	    extinguish_fuse(FUSE_SUFFOCATE);
	}

	/* If monster held us, stop it */
	if (on(*tp, DIDHOLD) && (--hold_count == 0))
		turn_off(player, ISHELD);
	turn_off(*tp, DIDHOLD);
	return;
    }

    /* Otherwise -- no go */
annoy:
    msg("You do not affect the %s.", mname);

    /* Annoy monster */
   if (off(*tp, WASTURNED)) runto(&new_pos, &hero);
}


/* Routines for magicians */


void 
cast ()
{
    int c, spell_cost;
    short i, num_spells, which_spell, avail_points;
    bool nohw = FALSE;
    bool boost = FALSE, blessed = FALSE;
    int maxboost;
    static int repeat_spell = -1;
    int min_intel = 16;

    if (ring_blessed(R_WIZARD))
	min_intel--;
    else if (ring_cursed(R_WIZARD))
	min_intel++;

    if (player.t_ctype != C_MAGICIAN && pstats.s_intel < min_intel) {
	msg("You are not permitted to cast spells.");
	return;
    }

    /* Get the number of available spells */
    if (pstats.s_intel >= min_intel) 
	num_spells = pstats.s_intel - min_intel + 1;
    else 
	num_spells = 0;

    if (player.t_ctype == C_MAGICIAN)
	num_spells += pstats.s_lvl;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
 		&& ISWEARING(R_WIZARD))
	num_spells *= 2;

    avail_points = pstats.s_intel * pstats.s_lvl;
    if (ISWEARING(R_WIZARD))
	avail_points *= 2;

    if (difficulty > 2) {
	if (player.t_ctype == C_MAGICIAN) {
	    if (num_spells > 1)
		num_spells *= 2.1/3.0;
	    if (avail_points > 20)
		avail_points = 20 + (avail_points-20) * 2.0/3.0;
	} else {
	    avail_points /= 2;
	}
    } else if (difficulty == 2) {
	if (avail_points > 20)
	    avail_points = 20 + (avail_points-20) * 3.0/4.0;
    }

    maxboost = num_spells - 3;

    if (num_spells > maxspells) 
	num_spells = maxspells;

    if (repeat_spell >= 0) {
	which_spell = repeat_spell;
	goto do_spell;
    }

    /* Prompt for spells */
    msg("Which spell are you casting [%d points left]? (* for list): ",
			avail_points - spell_power);

    c = readchar();
    if (c == ESCAPE) {
	msg("");
	after = FALSE;
	count = 0;
	return;
    }
    if (isupper(c)) {
	boost = TRUE;
	c = tolower(c);
    }
    which_spell = (short) (c - 'a');

    if (which_spell >= 0 && which_spell < num_spells) {
	nohw = TRUE;
	msg("");

    } else if (slow_invent) {
	char c;

	for (i=0; i<num_spells; i++) {
	    msg("");
	    mvwaddch(cw, 0, 0, '[');
	    waddch(cw, (char) ((short) 'a' + i));
	    wprintw(cw, ", %d/%d] A spell of ", magic_spells[i].s_cost,
			avail_points - spell_power);
	    if (magic_spells[i].s_type == TYP_POTION)
		waddstr(cw, p_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_SCROLL)
		waddstr(cw, s_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_STICK)
		waddstr(cw, ws_magic[magic_spells[i].s_which].mi_name);
	    waddstr(cw, morestr);
	    draw(cw);
	    do {
		c = readchar();
	    } while (c != ' ' && c != ESCAPE);
	    if (c == ESCAPE)
		break;
	}
	msg("");
	mvwaddstr(cw, 0, 0, "Which spell are you casting? ");
	draw(cw);
    }
    else {
	/* Set up for redraw */
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	/* Now display the possible spells */
	wclear(hw);
	touchwin(hw);
	for (i=0; i<num_spells; i++) {
	    mvwaddch(hw, i+2, 0, '[');
	    waddch(hw, (char) ((short) 'a' + i));
	    wprintw(hw, ", %2d] A spell of ", magic_spells[i].s_cost);
	    if (magic_spells[i].s_type == TYP_POTION)
		waddstr(hw, p_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_SCROLL)
		waddstr(hw, s_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_STICK)
		waddstr(hw, ws_magic[magic_spells[i].s_which].mi_name);
	}
	wmove(hw, 0, 0);
	wprintw(hw, "Which spell are you casting (%d points left)? ",
			avail_points - spell_power);
	draw(hw);
    }

    if (!nohw) {
	c = wgetch(hw);
	if (isupper(c)) {
	    boost = TRUE;
	    c = tolower(c);
	}
	which_spell = c - 'a';
	while (which_spell < 0 || which_spell >= num_spells) {
	    if (c == ESCAPE) {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed spells. ");
	    draw(hw);
	    c = wgetch(hw);
	    if (isupper(c)) {
		boost = TRUE;
		c = tolower(c);
	    }
	    which_spell = c - 'a';
	}
    }

do_spell:

    spell_cost = magic_spells[which_spell].s_cost;
    if (magic_spells[which_spell].s_blessed) {
	blessed = TRUE;
	boost = FALSE;
    }
    if (boost && which_spell <= maxboost) {
	blessed = TRUE;
	spell_cost *= 3;
    }

    if (spell_power + spell_cost > avail_points) {
	msg("Your attempt fails.");
	count = 0;
	repeat_spell = -1;
	return;
    }

    msg("Your spell is successful.");

    if (magic_spells[which_spell].s_type == TYP_POTION)
        quaff(	magic_spells[which_spell].s_which,
        	blessed);
    else if (magic_spells[which_spell].s_type == TYP_SCROLL)
        read_scroll(	magic_spells[which_spell].s_which,
        		blessed);
    else if (magic_spells[which_spell].s_type == TYP_STICK) {
	 if (get_dir())
	      do_zap(	TRUE, 
			magic_spells[which_spell].s_which,
			blessed);
	 else
	      return;
    }
    spell_power += spell_cost;

    if (count == 0)
	repeat_spell = -1;
    else if (repeat_spell < 0) {
	repeat_spell = which_spell;
    }
}
