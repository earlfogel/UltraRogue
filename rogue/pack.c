#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * Routines to deal with the pack
 */

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of gettting it off the ground.
 */
bool 
add_pack (struct linked_list *item, bool silent)
{
    struct linked_list *ip, *lp=NULL;
    struct object *obj, *op=NULL;
    bool exact, from_floor;

    if (item == NULL)
    {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return(FALSE);
    }
    else
	from_floor = FALSE;
    obj = (struct object *) ldata(item);
    /*
     * If it is gold, just add its value to rogue's purse and get rid
     * of it.
     */
    if (obj->o_type == GOLD) {
	struct linked_list *mitem;
	struct thing *tp;

	if (!silent) {
	    msg("You found %d gold pieces.", obj->o_count);
	}

	/* First make sure no greedy monster is after this gold.
	 * If so, make the monster run after the rogue instead.
	 */
	 for (mitem = mlist; mitem != NULL; mitem = next(mitem)) {
	    tp = (struct thing *) ldata(mitem);
	    if (tp->t_dest == &obj->o_pos) tp->t_dest = &hero;
	}

	purse += obj->o_count;
	if (from_floor) {
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x,
		(roomin(&hero) == NULL ? PASSAGE : FLOOR));
	    discard(item);
	}
	return(TRUE);
    }

    /*
     * see if player can carry any more weight
     */
    if (itemweight(obj) + pstats.s_pack > pstats.s_carry) {
	msg("Too much for you to carry.");
	if (!silent && levtype != POSTLEV) {
	    msg("You moved onto %s", inv_name(obj, TRUE));
	}
	return FALSE;
    }
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
    if (obj->o_group)
    {
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group)
	    {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)
		{
		    detach(lvl_obj, item);
		    mvaddch(hero.y, hero.x,
			(roomin(&hero) == NULL ? PASSAGE : FLOOR));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }

    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISCURSED)
	{
	    msg("The scroll turns to dust as you pick it up.");
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
	    return(TRUE);
	}

    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL)
    {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else
    {
	/*
	 * Search for an object which is exactly the same
	 */
	while (ip != NULL && op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
	    {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    /*
     * Check if there is room
     */
    if (ip == NULL || !exact || !ISMULT(obj->o_type)) {
	if (inpack >= maxpack-1) {
	    msg("You can't carry anything else.");
	    if (!silent && levtype != POSTLEV) {
		obj = (struct object *) ldata(item);
		msg("You moved onto %s.", inv_name(obj, TRUE));
	    }
	    return(FALSE);
	}
    }
    inpack++;
    getletter(item);
    if (from_floor)
    {
	detach(lvl_obj, item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
    if (ip == NULL)
    {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else
	{
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else
    {
	/*
	 * If we found an exact match.  If it is food,
	 * increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type))
	{
	    op->o_count += obj->o_count;
	    inpack--;			/* adjust for previous addition */
	    freeletter(item);
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = (struct object *) ldata(item);
    if (!silent)
    {
	msg("You now have %s (%c).", inv_name(obj, 1), pack_char(obj));
    }
    obj->o_flags &= ~ISPOST;
    if (levtype == POSTLEV)
	switch(obj->o_type) {
	    case POTION:
		p_know[obj->o_which] = TRUE;
	    when STICK:
		ws_know[obj->o_which] = TRUE;
	    when SCROLL:
		s_know[obj->o_which] = TRUE;
	    when RING:
		r_know[obj->o_which] = TRUE;
	}
    if (obj->o_type == ARTIFACT) {
	has_artifact |= (1 << obj->o_which);
	picked_artifact |= (1 << obj->o_which);
	if ( !(obj->art_stats.ar_flags & ISUSED) ) {
	    obj->art_stats.ar_flags |= ISUSED;
	    pstats.s_exp += arts[obj->o_which].ar_worth / 10;
	    check_level();
	}
    }
    updpack(FALSE);
    return(TRUE);
}

/*
 * inventory:
 *	list what is in the pack
 */
int 
inventory (struct linked_list *list, int type)
{
    struct object *obj;
    char ch;
    int n_objs;
    char inv_temp[LINELEN];

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list))
    {
	obj = (struct object *) ldata(list);
	if (obj->o_type == GOLD)
		continue;	/* occurs when inventorying floor as wizard */

	switch (n_objs++)
	{
	    /*
	     * For the first thing in the inventory, just save the string
	     * in case there is only one.
	     */
	    case 0:
		if (list->l_letter < 'a' || list->l_letter > 'z')
		    list->l_letter = ch;

		sprintf(inv_temp, "%c) %s", list->l_letter, inv_name(obj, FALSE));
		break;
	    /*
	     * If there is more than one, clear the screen, print the
	     * saved message and fall through to ...
	     */
	    case 1:
		if (slow_invent)
		    msg(inv_temp);
		else
		{
		    wclear(hw);
		    waddstr(hw, inv_temp);
		    waddch(hw, '\n');
		}
		__attribute__ ((fallthrough));
	    /*
	     * Print the line for this object
	     */
	    default:
		if (list->l_letter < 'a' || list->l_letter > 'z')
		    list->l_letter = ch;

		if (slow_invent)
		    msg("%c) %s", list->l_letter, inv_name(obj, FALSE));
		else
		    wprintw(hw, "%c) %s\n", list->l_letter, inv_name(obj, FALSE));
	}
    }
    if (n_objs == 0)
    {
	msg(type == 0 ? "You are empty handed." :
			"You don't have anything appropriate.");
	return FALSE;
    }
    if (n_objs == 1)
    {
	msg(inv_temp);
	return TRUE;
    }
    if (!slow_invent)
    {
	mvwaddstr(hw, LINES-1, 0, spacemsg);
	draw(hw);
	(void) wgetch(hw);
	clearok(cw, TRUE);
	touchwin(cw);
    }
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
void 
pick_up (int ch)
{
    switch(ch)
    {
	case GOLD:
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case ARTIFACT:
	case RING:
	case STICK:
	    add_pack(NULL, FALSE);
	    break;
	default:
	    debug("Where did you pick that up???");
	    break;
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
void 
picky_inven ()
{
    struct linked_list *item;
    char ch, mch;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE));
    else
    {
	msg("Which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (item->l_letter == mch)
	    {
		msg("%c) %s",item->l_letter,inv_name((struct object *) ldata(item), FALSE));
		return;
	    }
	msg("'%s' not in pack.", unctrl(mch));
    }
}


/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item (char *purpose, int type)
{
    struct linked_list *obj, *pit, *savepit=NULL;
    struct object *pob;
    char ch, och, anr;
    int cnt, itemcount = 0;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else  {
	/* see if we have any of the type requested */
	if(type != 0 && type != CALLABLE && type != MARKABLE) {
	    pit = pack;
	    for(ch = 'a' ; pit != NULL ; pit = next(pit), ch++) {
		pob = OBJPTR(pit);
		if ((type == pob->o_type) || 
			(type == STICK && pob->o_type == WEAPON && 
			((pob->o_flags & ISZAPPED) != 0))) {
		    ++itemcount;
		    savepit = pit;	/* save in case of only 1 */
		}
	    }
	    if (itemcount == 0 && type != WEAPON) {
		msg("You have nothing to %s.",purpose);
		after = FALSE;
		return NULL;
	    }
	}
	for (;;) {
	    msg("%s what? (* for list): ",purpose);
	    ch = readchar();
	    save_ch = ' ';  /* no type-ahead */
	    mpos = 0;
	    if (ch == ESCAPE) {		/* abort if escape hit */
		after = FALSE;
		msg("");		/* clear display */
		return NULL;
	    } else if (ch == '-' && cur_weapon != NULL
		    && strcmp(purpose, "wield") == 0) {
		msg("You are no longer wielding %s.", inv_name(cur_weapon, TRUE));
		cur_weapon = NULL;
		return(NULL);
	    }
	    if (ch == '*') {
		if (itemcount == 1) {
		    struct object *opb = OBJPTR(savepit);
		    mpos = 0;
		    msg("%c) %s",savepit->l_letter,inv_name(opb,FALSE));
		    continue;
		} else {
		    wclear(hw);
		    pit = pack;		/* point to pack */
		    cnt = 0;
		    for(ch = 'a'; pit != NULL ; pit = next(pit), ch++) {
			pob = OBJPTR(pit);
			if(type==0          || type==CALLABLE || 
			   type == MARKABLE || type==pob->o_type) {
			  wprintw(hw,"%c) %s\n\r",pit->l_letter,inv_name(pob,FALSE));
			  if (++cnt >= LINES - 2 && next(pit) != NULL) {
			    cnt = 0;
			    dbotline(hw, spacemsg);
			    wclear(hw);
			  }
			}
		    }
		    wmove(hw, LINES - 1,0);
		    wprintw(hw,"%s what? ",purpose);
		    draw(hw);		/* write screen */
		    anr = FALSE;
		}
		do {
		    ch = wgetch(hw);
		    if (isupper(ch))
			ch = tolower(ch);
		    if (isalpha(ch) || ch == ESCAPE) {
			anr = TRUE; 
		    } else if (ch == '-' && cur_weapon != NULL
			    && strcmp(purpose, "wield") == 0) {
			msg("You are no longer wielding %s.", inv_name(cur_weapon, TRUE));
			cur_weapon = NULL;
			return(NULL);
		    }
		} while(!anr);		/* do till we got it right */
		restscr(cw);		/* redraw orig screen */
		if(ch == ESCAPE) {
		    after = FALSE;
		    msg("");		/* clear top line */
		    return NULL;	/* all done if abort */
		}
		/* ch has item to get from pack */
	    }
	    for(obj = pack,och = 'a'; obj != NULL;obj = next(obj),och++)
		if (ch == obj->l_letter)
		    break;
	    if (obj == NULL) {
		msg("Not in pack.");
		continue;
	    }
	    else {
		struct object *opb;
		opb = OBJPTR(obj);
		if((type==WEAPON || type==0 || type==MARKABLE || type==CALLABLE)
			|| (type==STICK && opb->o_type==STICK)
			|| (type==STICK && opb->o_type==WEAPON)
			|| (type == opb->o_type))
			return obj;
		else {
			mpos = 0;
			msg("You can't %s that.",purpose);
			after = FALSE;
			return NULL;
		}
	    }
	}
    }
    return NULL;
}

int 
pack_char (struct object *obj)
{
    struct linked_list *item;
    char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return item->l_letter;
	else
	    c++;
    msg("Help!  Item in pack not in pack!");
    return 0;
}

/* 
 * del_pack:
 *	Take something out of the hero's pack
 */
void 
del_pack (struct linked_list *what)
{
	struct object *op;

	op = OBJPTR(what);
	cur_null(op);		/* check for current stuff */
	if (op->o_count > 1) {
	    op->o_count--;
	}
	else {
	    detach(pack,what);
	    freeletter(what);
	    discard(what);
	    inpack--;
	}
	updpack(FALSE);
}

/*
 * cur_null:
 *	This updates cur_weapon etc for dropping things
 */
void 
cur_null (struct object *op)
{
	if (op == cur_weapon)
	    cur_weapon = NULL;
	else if (op == cur_armor)
	    cur_armor = NULL;
	else if (op == cur_ring[LEFT_1])
	    cur_ring[LEFT_1] = NULL;
	else if (op == cur_ring[LEFT_2])
	    cur_ring[LEFT_2] = NULL;
	else if (op == cur_ring[LEFT_3])
	    cur_ring[LEFT_3] = NULL;
	else if (op == cur_ring[LEFT_4])
	    cur_ring[LEFT_4] = NULL;
	else if (op == cur_ring[RIGHT_1])
	    cur_ring[RIGHT_1] = NULL;
	else if (op == cur_ring[RIGHT_2])
	    cur_ring[RIGHT_2] = NULL;
	else if (op == cur_ring[RIGHT_3])
	    cur_ring[RIGHT_3] = NULL;
	else if (op == cur_ring[RIGHT_4])
	    cur_ring[RIGHT_4] = NULL;
}

/*
 * idenpack:
 *	Identify all the items in the pack
 */
void 
idenpack ()
{
	struct linked_list *pc;

	for (pc = pack ; pc != NULL ; pc = next(pc))
		whatis(pc);
}

#define SIZE(array)	(sizeof (array) / sizeof (*(array)))

char pack_letters[] = "zyxwvutsrqponmlkjihgfedcba";
char *pack_index	= pack_letters + SIZE(pack_letters) - 1;
char *pack_end	= pack_letters + SIZE(pack_letters) - 1;

void 
getletter (struct linked_list *item)
{
	if (item != NULL) {
		if (pack_index > pack_letters && islower(pack_index[-1]))
			item->l_letter = *--pack_index;
		else
			item->l_letter = '?';
	}
}

void 
freeletter (struct linked_list *item)
{
	if (item != NULL && islower(item->l_letter))
		if (pack_index < pack_end)
			*pack_index++ = item->l_letter;
}

/*
 * Print out the item on the floor.  Used by the move command.
 */
void 
show_floor ()
{
	struct linked_list *item;
	struct object *obj;

	item = find_obj(hero.y, hero.x);
	if (item != NULL) {
		addmsg("You moved onto ");
		obj = (struct object *) ldata(item);
		if (obj->o_type == GOLD)
			msg("%d gold pieces.", obj->o_count);
		else {
			addmsg(inv_name(obj, TRUE));
			addmsg(".");
			endmsg();
		}
	}
}
