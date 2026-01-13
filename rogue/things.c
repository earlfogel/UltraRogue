/*
 * Contains functions for dealing with things like
 * potions and scrolls
 *
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * inv_name:
 *	return the name of something as it would appear in an
 *	inventory.
 */
char *
inv_name (struct object *obj, bool drop)
{
    char *pb;

    switch(obj->o_type) {
	case SCROLL:
	    sprintf(prbuf, "A %s%sscroll ", 
		    obj->o_flags & CANRETURN ? "claimed " : "", 
		    blesscurse(obj->o_flags));
	    pb = &prbuf[strlen(prbuf)];
	    if (s_know[obj->o_which] || (obj->o_flags & ISPOST))
		sprintf(pb, "of %s", s_magic[obj->o_which].mi_name);
	    else if (s_guess[obj->o_which])
		sprintf(pb, "called %s", s_guess[obj->o_which]);
	    else
		sprintf(pb, "titled '%s'", s_names[obj->o_which]);
        when POTION:
	    sprintf(prbuf, "A %s%spotion ", 
		    obj->o_flags & CANRETURN ? "claimed " : "", 
		    blesscurse(obj->o_flags));
	    pb = &prbuf[strlen(prbuf)];
	    if (p_know[obj->o_which] || (obj->o_flags & ISPOST))
		sprintf(pb, "of %s(%s)", p_magic[obj->o_which].mi_name,
		    p_colors[obj->o_which]);
	    else if (p_guess[obj->o_which])
		sprintf(pb, "called %s(%s)", p_guess[obj->o_which],
		    p_colors[obj->o_which]);
	    else
		sprintf(prbuf, "A%s %s potion",
			obj->o_flags & CANRETURN ? " claimed" : 
			vowelstr(p_colors[obj->o_which]),
			p_colors[obj->o_which]);
	when FOOD:
	    if (obj->o_count == 1)
		sprintf(prbuf, "A%s %s", 
			obj->o_flags & CANRETURN ? " claimed" : 
			vowelstr(fd_data[obj->o_which].mi_name), 
			fd_data[obj->o_which].mi_name);
	    else
		sprintf(prbuf, "%d %s%ss", obj->o_count,
			obj->o_flags & CANRETURN ? "claimed " : "", 
			fd_data[obj->o_which].mi_name);
	when WEAPON:
	    if (obj->o_count > 1)
		sprintf(prbuf, "%d ", obj->o_count);
	    else
		strcpy(prbuf, "A ");
	    pb = &prbuf[strlen(prbuf)];
	    if ((obj->o_flags & ISKNOW) && (obj->o_flags & ISZAPPED))
		sprintf(pb, "charged%s ", charge_str(obj));
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & CANRETURN)
		sprintf(pb, "claimed ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISPOISON)
		sprintf(pb, "poisoned ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISVORPED && difficulty <= 3)
		sprintf(pb, "vorpal ");
	    else if (obj->o_flags & ISSILVER)
		sprintf(pb, "silver ");
	    pb = &prbuf[strlen(prbuf)];
	    if ((obj->o_flags & ISKNOW) || (obj->o_flags & ISPOST))
		sprintf(pb, "%s %s", num(obj->o_hplus, obj->o_dplus),
		    weaps[obj->o_which].w_name);
	    else
		sprintf(pb, "%s", weaps[obj->o_which].w_name);
	    if (obj->o_count > 1)
		strcat(prbuf, "s");
	when ARMOR:
	    if ((obj->o_flags & ISKNOW) || (obj->o_flags & ISPOST))
		sprintf(prbuf, "%s%s %s",
		    obj->o_flags & CANRETURN ? "claimed " : "", 
		    num(armors[obj->o_which].a_class - obj->o_ac, 0),
		    armors[obj->o_which].a_name);
	    else
		sprintf(prbuf, "%s%s", 
		    obj->o_flags & CANRETURN ? "claimed " : "", 
		    armors[obj->o_which].a_name);
	when ARTIFACT:
	    sprintf(prbuf, "the %s", arts[obj->o_which].ar_name);
	    if (obj->o_flags & CANRETURN)
		strcat(prbuf, " (claimed)");
	when STICK:
	    sprintf(prbuf, "A %s%s%s ", 
		obj->o_flags & CANRETURN ? "claimed " : "", 
		blesscurse(obj->o_flags), ws_type[obj->o_which]);
	    pb = &prbuf[strlen(prbuf)];
	    if (ws_know[obj->o_which] || (obj->o_flags & ISPOST))
		sprintf(pb, "of %s%s(%s)", ws_magic[obj->o_which].mi_name,
		    charge_str(obj), ws_made[obj->o_which]);
	    else if (ws_guess[obj->o_which])
		sprintf(pb, "called %s(%s)", ws_guess[obj->o_which],
		    ws_made[obj->o_which]);
	    else
		sprintf(&prbuf[2], "%s%s %s", 
		    obj->o_flags & CANRETURN ? "claimed " : "", 
		    ws_made[obj->o_which],
		    ws_type[obj->o_which]);
        when RING:
	    if (r_know[obj->o_which] || (obj->o_flags & ISPOST))
		sprintf(prbuf, "A%s%s ring of %s(%s)", 
		    obj->o_flags & CANRETURN ? " claimed" : "", ring_num(obj),
		    r_magic[obj->o_which].mi_name, r_stones[obj->o_which]);
	    else if (r_guess[obj->o_which])
		sprintf(prbuf, "A%s ring called %s(%s)",
		    obj->o_flags & CANRETURN ? " claimed" : "",
		    r_guess[obj->o_which], r_stones[obj->o_which]);
	    else
		sprintf(prbuf, "A%s %s ring", 
		    obj->o_flags & CANRETURN ? " claimed" : 
		    vowelstr(r_stones[obj->o_which]),
		    r_stones[obj->o_which]);
	otherwise:
	    debug("Picked up something funny.");
	    sprintf(prbuf, "Something bizarre %s", unctrl(obj->o_type));
    }

    /* Is it marked? */
    if (obj->o_mark[0]) {
	pb = &prbuf[strlen(prbuf)];
	sprintf(pb, " <%s>", obj->o_mark);
    }

    /* Is it owned? */
    if (obj->o_flags & ISOWNED) {
        char *c = strstr(prbuf, "claimed");
        if (c)
            *c = 'C';  /* change 'claimed' to 'Claimed' */
    }

    /* Is it time to reveal blessed/cursed status? */
    if (game_over && (obj->o_type == FOOD
	    || (obj->o_type == ARTIFACT && wizard)
	    || (obj->o_type == RING && (obj->o_ac != 11 || wizard)) )) {
	if (obj->o_flags & ISCURSED)
	    strcat(prbuf, " (cursed)");
	else if (obj->o_flags & ISBLESSED)
	    strcat(prbuf, " (blessed)");
#if 0
	if (wizard)
	    sprintf(prbuf + strlen(prbuf), " ac=%d", obj->o_ac);
#endif
    }

    if (obj == cur_armor)
	strcat(prbuf, " (being worn)");
    if (obj == cur_weapon)
	strcat(prbuf, " (weapon in hand)");
    if      (obj == cur_ring[LEFT_1])  strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_2])  strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_3])  strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_4])  strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[RIGHT_1]) strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_2]) strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_3]) strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_4]) strcat(prbuf, " (on right hand)");
    if (obj->o_flags & IS2PROT)
	strcat(prbuf, " [Protected]");
    else if (obj->o_flags & ISPROT)
	strcat(prbuf, " [protected]");
    if (drop && isupper(prbuf[0]))
	prbuf[0] = tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    if (!drop)
	strcat(prbuf, ".");
    return prbuf;
}

/*
 * drop:
 *	put something down
 */
int 
drop (struct linked_list *item)
{
    char ch='.';
    struct linked_list *obj, *nobj;
    struct object *op;

    if (item == NULL) {
	ch = mvwinch(stdscr, hero.y, hero.x);
	if (ch != FLOOR && ch != PASSAGE && ch != POOL) {
	    msg("There is something there already.");
	    return(FALSE);
	}
	if ((obj = get_item("drop", 0)) == NULL)
	    return(FALSE);
    }
    else {
	obj = item;
    }
    op = (struct object *) ldata(obj);
    if (!dropcheck(op))
	return(FALSE);

    /*
     * If it is a scare monster scroll, curse it
     */
    if (op->o_type == SCROLL && op->o_which == S_SCARE) {
	if (op->o_flags & ISBLESSED)
	    op->o_flags &= ~ISBLESSED;
	else op->o_flags |= ISCURSED;
    }

    /*
     * Take it out of the pack
     */
    if (count > 1 && op->o_count > count) {  /* drop some of them */
        nobj = new_item(sizeof *op);
        op->o_count -= count+1;
        op = (struct object *) ldata(nobj);
        *op = *((struct object *) ldata(obj));
        op->o_count = count+1;
        obj = nobj;
	count = 0;
    } else {
	detach(pack, obj);
	inpack--;
	freeletter(obj);
    }

    if(ch == POOL) {
	msg("Your %s sinks out of sight.",inv_name(op,TRUE));
	discard(obj);
    }
    else if (levtype == POSTLEV) {
	op->o_pos = hero;	/* same place as hero */
	if (item == NULL)	/* if item wasn't sold */
	    msg("Thanks for your donation to the fiend's flea market.");
        if (op->o_type != ARTIFACT)
	    fall(obj,FALSE);
	else {
	    msg("You'll be sorry you ever got rid of that!");
	    has_artifact &= ~(1 << op->o_which);
	    discard(obj);
	}
    }
    else {
	/*
	 * Link it into the level object list
	 */
	attach(lvl_obj, obj);
	mvaddch(hero.y, hero.x, op->o_type);
	op->o_pos = hero;
	msg("Dropped %s.", inv_name(op, TRUE));
    }
    if (op->o_type == ARTIFACT && levtype != POSTLEV)
	has_artifact &= ~(1 << op->o_which);
    updpack(FALSE);
    return (TRUE);
}

/*
 * do special checks for dropping or unweilding|unwearing|unringing
 */
int 
dropcheck (struct object *op)
{
    if (op == NULL)
	return TRUE;
    if (levtype == POSTLEV) {
	if ((op->o_flags & ISCURSED) && (op->o_flags & ISKNOW)) {
	    msg("The trader does not accept your shoddy merchandise.");
	    return(FALSE);
	}
    }
    if (op != cur_armor && op != cur_weapon &&
	op != cur_ring[LEFT_1] && op != cur_ring[LEFT_2] &&
	op != cur_ring[LEFT_3] && op != cur_ring[LEFT_4] &&
	op != cur_ring[RIGHT_1] && op != cur_ring[RIGHT_2] &&
	op != cur_ring[RIGHT_3] && op != cur_ring[RIGHT_4]) 
	    return TRUE;
    if (op->o_flags & ISCURSED) {
	msg("You can't.  It appears to be cursed.");
	return FALSE;
    }
    if (op == cur_weapon)
	cur_weapon = NULL;
    else if (op == cur_armor) {
	waste_time();
	cur_armor = NULL;
    }
    else if (op == cur_ring[LEFT_1] || op == cur_ring[LEFT_2] ||
	op == cur_ring[LEFT_3] || op == cur_ring[LEFT_4] ||
	op == cur_ring[RIGHT_1] || op == cur_ring[RIGHT_2] ||
	op == cur_ring[RIGHT_3] || op == cur_ring[RIGHT_4]) {
	if      (op == cur_ring[LEFT_1])  cur_ring[LEFT_1]  = NULL;
	else if (op == cur_ring[LEFT_2])  cur_ring[LEFT_2]  = NULL;
	else if (op == cur_ring[LEFT_3])  cur_ring[LEFT_3]  = NULL;
	else if (op == cur_ring[LEFT_4])  cur_ring[LEFT_4]  = NULL;
	else if (op == cur_ring[RIGHT_1]) cur_ring[RIGHT_1] = NULL;
	else if (op == cur_ring[RIGHT_2]) cur_ring[RIGHT_2] = NULL;
	else if (op == cur_ring[RIGHT_3]) cur_ring[RIGHT_3] = NULL;
	else if (op == cur_ring[RIGHT_4]) cur_ring[RIGHT_4] = NULL;
	switch (op->o_which) {
	    case R_ADDSTR:
		chg_str(-op->o_ac, FALSE, FALSE);
	    when R_ADDHIT:
		chg_dext(-op->o_ac, FALSE, FALSE);
	    when R_ADDINTEL:
		pstats.s_intel -= op->o_ac;
	    when R_ADDWISDOM:
		pstats.s_wisdom -= op->o_ac;
	    when R_SEEINVIS:
		if (find_slot(FUSE, FUSE_UNSEE) == NULL) {
		    turn_off(player, CANSEE);
		    msg("The tingling feeling leaves your eyes.");
		}
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    when R_LIGHT: {
		    if(roomin(&hero) != NULL) {
			    light(&hero);
			    mvwaddch(cw, hero.y, hero.x, PLAYER);
		    }
	        }
	}
    }
    return TRUE;
}

/*
 * return a new thing
 */
struct linked_list *
new_thing ()
{
    struct linked_list *item;
    struct object *cur;
    int j, k;
    short blesschance, cursechance;

    item = new_item(sizeof *cur);
    cur = (struct object *) ldata(item);
    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    cur->o_weight = 0;
    cur->o_mark[0] = '\0';
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    blesschance = rnd(100);
    cursechance = rnd(100);
    switch (no_food > 3 ? 2 : pick_one(things, NUMTHINGS)) {
	case 0:
	    cur->o_type = POTION;
	    cur->o_which = pick_one(p_magic, MAXPOTIONS);
	    cur->o_weight = things[TYP_POTION].mi_wght;
	    if (cursechance < p_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < p_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	when 1:
	    cur->o_type = SCROLL;
	    cur->o_which = pick_one(s_magic, MAXSCROLLS);
	    cur->o_weight = things[TYP_SCROLL].mi_wght;
	    if (cursechance < s_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < s_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	when 2:
	    no_food = 0;
	    cur->o_type = FOOD;
	    cur->o_which = pick_one(fd_data, MAXFOODS);
	    cur->o_weight = 2;
	    cur->o_count += extras();
	when 3:
	    cur->o_type = WEAPON;
	    cur->o_which = rnd(MAXWEAPONS);
	    init_weapon(cur, cur->o_which);
	    if (cursechance < 10) {
		short bad=(rnd(10) < 1) ? 2 : 1;

		cur->o_flags |= ISCURSED;
		cur->o_hplus -= bad;
		cur->o_dplus -= bad;
	    }
	    else if (blesschance < 15) {
		short good=(rnd(10) < 1) ? 2 : 1;

		cur->o_hplus += good;
		cur->o_dplus += good;
	    }
	when 4:
	    cur->o_type = ARMOR;
	    for (j = 0; j < MAXARMORS; j++)
		if (blesschance < armors[j].a_prob)
		    break;
	    if (j == MAXARMORS) {
		debug("Picked a bad armor %d", blesschance);
		j = 0;
	    }
	    cur->o_which = j;
	    cur->o_ac = armors[j].a_class;
	    if (((k = rnd(100)) < 20) && j != MITHRIL) {
		cur->o_flags |= ISCURSED;
		cur->o_ac += rnd(3)+1;
	    }
	    else if (k < 28 || j == MITHRIL) 
		cur->o_ac -= rnd(3)+1;
	    if (j == MITHRIL)
		cur->o_flags |= ISPROT;
	    cur->o_weight = armors[j].a_wght;
	when 5:
	    cur->o_type = RING;
	    cur->o_which = pick_one(r_magic, MAXRINGS);
	    cur->o_weight = things[TYP_RING].mi_wght;
	    if (cursechance < r_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < r_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	    switch (cur->o_which) {
		case R_ADDSTR:
		case R_ADDWISDOM:
		case R_ADDINTEL:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_CARRYING:
		    cur->o_ac = rnd(2) + 1;	/* From 1 to 3 */
		    if (cur->o_flags & ISCURSED)
			cur->o_ac = -cur->o_ac;
		    if (cur->o_flags & ISBLESSED) cur->o_ac++;
		when R_RESURRECT:
		case R_TELCONTROL:
		case R_VREGEN:
		    cur->o_ac = 0;
		    if (cur->o_flags & ISCURSED)
			cur->o_ac = -1;
		    if (cur->o_flags & ISBLESSED) 
			cur->o_ac = 1;
		when R_DIGEST:
		    if (cur->o_flags & ISCURSED) cur->o_ac = -1;
		    else if (cur->o_flags & ISBLESSED) cur->o_ac = 2;
		    else cur->o_ac = 1;
	    }
	when 6:
	    cur->o_type = STICK;
	    cur->o_which = pick_one(ws_magic, MAXSTICKS);
	    fix_stick(cur);
	    if (cursechance < ws_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < ws_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	otherwise:
	    debug("Picked a bad kind of object");
	    wait_for(' ');
    }
    return item;
}

/*
 * provide a new item tailored to specification
 */
struct linked_list *
spec_item (int type, int which, int hit, int damage)
{
    struct linked_list *item;
    struct object *obj;

    item = new_item(sizeof *obj);
    obj = OBJPTR(item);
    obj->o_count = 1;
    obj->o_group = 0;
    obj->o_type = type;
    obj->o_which = which;
    obj->o_damage = obj->o_hurldmg = "0d0";
    obj->o_hplus = 0;
    obj->o_dplus = 0;
    obj->o_flags = 0;
    obj->o_mark[0] = '\0';
    obj->o_launch = 0;
    obj->o_weight = 0;

    /* Handle special characteristics */
    switch (type) {
	case WEAPON:
	    init_weapon(obj, which);
	    obj->o_hplus = hit;
	    obj->o_dplus = damage;
	    obj->o_ac = 10;

	    if (hit > 0 || damage > 0) obj->o_flags |= ISBLESSED;
	    else if (hit < 0 || damage < 0) obj->o_flags |= ISCURSED;

	when ARMOR:
	    obj->o_ac = armors[which].a_class - hit;
	    if (hit > 0) obj->o_flags |= ISBLESSED;
	    else if (hit < 0) obj->o_flags |= ISCURSED;

	when RING:
	    obj->o_ac = hit;
	    switch (obj->o_which) {
		case R_ADDSTR:
		case R_ADDWISDOM:
		case R_ADDINTEL:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_DIGEST:
		case R_RESURRECT:
		case R_TELCONTROL:
		    if (hit > 1) 
			obj->o_flags |= ISBLESSED;
		    else if (hit < 0) 
			obj->o_flags |= ISCURSED;
	    }

	when STICK:
	    fix_stick(obj);
	    obj->o_charges = hit;

	when GOLD:
	    obj->o_type = GOLD;
	    obj->o_count = GOLDCALC;
	    obj->o_ac = 11;
    }
    return(item);
}

/*
 * pick an item out of a list of nitems possible magic items
 */
int 
pick_one (struct magic_item *magic, int nitems)
{
    struct magic_item *end;
    int i;
    struct magic_item *start;

    /*
     * more magic detection when it is most needed
     */
    if (pstats.s_intel < 16 && pstats.s_wisdom < 18
     && magic == p_magic && rnd(14) == 0)
	return P_TFIND;

    start = magic;
    for (end = &magic[nitems], i = rnd(1000); magic < end; magic++) {
	if (i < magic->mi_prob)
	    break;
    }
    if (magic == end) {
	if (wizard) {
	    msg("bad pick_one: %d from %d items", i, nitems);
	    for (magic = start; magic < end; magic++)
		msg("%s: %d%%", magic->mi_name, magic->mi_prob);
	}
	magic = start;
    }
    return magic - start;
}


/* blesscurse returns whether, according to the flag, the object is
 * blessed, cursed, or neither
 */

char *
blesscurse (int flags)
{
    if (flags & ISKNOW)  {
	if (flags & ISCURSED) return("cursed ");
	if (flags & ISBLESSED) return("blessed ");
	return("normal ");
    }
    return("");
}

/*
 * extras:
 *	Return the number of extra items to be created
 */
int 
extras ()
{
	int i;

	i = rnd(100);
	if (i < 4)		/* 4% for 2 more */
	    return (2);
	else if (i < 11)	/* 7% for 1 more */
	    return (1);
	else			/* otherwise no more */
	    return (0);
}
