/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certain object is
 */

void 
whatis (struct linked_list *what)
{
    struct object *obj;
    struct linked_list *item;

    if (what == NULL) {		/* do we need to ask which one? */
	if ((item = get_item("identify", 0)) == NULL)
	    return;
    }
    else
	item = what;
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    switch (obj->o_type)
    {
        case SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which])
	    {
		FREE(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which])
	    {
		FREE(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    if (ws_guess[obj->o_which])
	    {
		FREE(ws_guess[obj->o_which]);
		ws_guess[obj->o_which] = NULL;
	    }
        when WEAPON:
        case ARMOR:
        when RING:
	    r_know[obj->o_which] = TRUE;
	    if (r_guess[obj->o_which])
	    {
		FREE(r_guess[obj->o_which]);
		r_guess[obj->o_which] = NULL;
	    }
    }
    if (what == NULL)
	msg(inv_name(obj, FALSE));
}

/*
 * create_obj:
 *	Create any object for wizard, scroll, magician, or cleric
 */
void 
create_obj (int which_item, int which_type, bool cursed)
{
    struct linked_list *item;
    struct object *obj;
    int wh, whc, msz;
    char ch, newitem, newtype, *pt;
    WINDOW *thiswin;
    int tsize;
    int pres_item = 1;

    thiswin = cw;
    if (which_item == 0) {
	bool nogood = TRUE;
	thiswin = hw;
	wclear(hw);
	wprintw(hw,"Item\tKey\n\n");
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_RING].mi_name,RING,
		things[TYP_STICK].mi_name,STICK);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_POTION].mi_name,POTION,
		things[TYP_SCROLL].mi_name,SCROLL);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_ARMOR].mi_name,ARMOR,
		things[TYP_WEAPON].mi_name,WEAPON);
	wprintw(hw,"%s\t %c\n",things[TYP_FOOD].mi_name,FOOD);
	if (wizard) {
	    wprintw(hw,"%s %c\n",things[TYP_ARTIFACT].mi_name, ARTIFACT);
	    waddstr(hw,"monster\t m");
	}
	wprintw(hw,"\n\nWhat do you want to create? ");
	draw(hw);
	do {
	    ch = wgetch(hw);
	    if (ch == ESCAPE) {
		restscr(cw);
		return;
	    }
	    switch (ch) {
		case RING:
		case STICK:	
		case POTION:
		case SCROLL:	
		case ARMOR:	
		case WEAPON:
		case FOOD:
		    nogood = FALSE;
		    break;
		case 'm':
		    if (wizard) 
			nogood = FALSE;
		    break;
		case ARTIFACT:
		    if (wizard)
			nogood = FALSE;
		    break;
		default:
		    nogood = TRUE;
	    }
	} while (nogood);
	newitem = ch;
    }
    else
	newitem = which_item;

    pt = "those";
    msz = 0;
    if(newitem == 'm') {
	makemon();		/* make monster and be done with it */
	return;
    }
    if(newitem == GOLD)
	pt = "gold";
    switch(newitem) {
	case POTION:	whc = TYP_POTION;	tsize = msz = MAXPOTIONS;
	when SCROLL:	whc = TYP_SCROLL;	tsize = msz = MAXSCROLLS;
	when FOOD:	whc = TYP_FOOD;		tsize = msz = MAXFOODS;
	when WEAPON:	whc = TYP_WEAPON;	tsize = msz = MAXWEAPONS;
	when ARMOR:	whc = TYP_ARMOR;	tsize = msz = MAXARMORS;
	when RING:	whc = TYP_RING;		tsize = msz = MAXRINGS;
	when STICK:	whc = TYP_STICK;	tsize = msz = MAXSTICKS;
	when ARTIFACT:	whc = TYP_ARTIFACT;	tsize = msz = MAXARTIFACT;
	otherwise:
	    if (thiswin == hw)
		restscr(cw);
	    mpos = 0;
	    msg("Even wizards can't create %s !!",pt);
	    return;
    }
    if(msz == 1) {		/* if only one type of item */
	ch = 1;
	if (thiswin == hw)
	    restscr(cw);
    }
    else if (which_type == 0) {
	struct magic_item *wmi;
	struct init_artifact *war;
	char wmn;
	int ii;
	char numstr[20];

	mpos = 0;
	wmi = NULL;
	war = NULL;
	wmn = 0;
	switch(newitem) {
		case POTION:	wmi = &p_magic[0];
		when SCROLL:	wmi = &s_magic[0];
		when RING:	wmi = &r_magic[0];
		when STICK:	wmi = &ws_magic[0];
		when FOOD:	wmi = &fd_data[0];
		when ARTIFACT:	war = &arts[0];
		when WEAPON:	wmn = 1;
		when ARMOR:	wmn = 2;
	}
	wclear(hw);
	thiswin = hw;
	while (msz > 0) {
	    int left_limit;
	    int num_lines = 2*(LINES-3);

	    wclear(hw);
	    touchwin(hw);
	    if (msz < num_lines) 
		left_limit = (msz + 1)/2;
	    else 
		left_limit = num_lines / 2;

	    wmove(hw, 2, 0);
	    for (ii = 0 ; ii < left_limit; ii++) {
		sprintf(numstr,"%d", pres_item);
	        waddstr(hw, numstr);
	        waddstr(hw,") ");
		if (wmi != NULL) {
	            waddstr(hw, wmi->mi_name);
	            wmi++;
		}
		else if (war != NULL) {
		    waddstr(hw, war->ar_name);
		    war++;
		}
	        else if(wmn == 1)
		    waddstr(hw, weaps[pres_item - 1].w_name);
	        else if(wmn == 2)
		    waddstr(hw, armors[pres_item - 1].a_name);
		waddstr(hw, "\n");
		pres_item++;
	    }
	    for (ii = 0 ; ii < left_limit && pres_item <= tsize; ii++) {
		sprintf(numstr,"%d", pres_item);
	        mvwaddstr(hw, ii+2, COLS/2, numstr);
	        waddstr(hw,") ");
		if (wmi != NULL) {
	            waddstr(hw, wmi->mi_name);
	            wmi++;
		}
		else if (war != NULL) {
		    waddstr(hw, war->ar_name);
		    war++;
		}
	        else if(wmn == 1)
		    waddstr(hw, weaps[pres_item - 1].w_name);
	        else if(wmn == 2)
		    waddstr(hw, armors[pres_item - 1].a_name);
		waddstr(hw, "\n");
		pres_item++;
	    }
	    if ((msz -= num_lines) > 0) {
		mvwaddstr(hw, LINES-1, 0, morestr);
		draw(hw);
		(void) wgetch(hw);
	    }
	}
	sprintf(prbuf,"Which %s? ",things[whc].mi_name);
	mvwaddstr(hw,LINES - 1, 0, prbuf);
	draw(hw);
	do {
	    numstr[0] = '\0';
	    ch = get_string(numstr, hw);
	    if (ch == QUIT) {
	        restscr(cw);
	        msg("");
	        return;
	    }
	    newtype = atoi(numstr) - 1;
	    if (newtype >= tsize) {
		mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
		mvwaddstr(hw,LINES - 1, 0, prbuf);
		draw(hw);
	    }
	} until (newtype < tsize);
        if (thiswin == hw)			/* restore screen if need be */
	    restscr(cw);
    }
    else 
	newtype = which_type;
    item = new_item(sizeof *obj);	/* get some memory */
    obj = OBJPTR(item);
    if (newitem == ARTIFACT) {
	new_artifact(newtype, obj);
	if (add_pack(item, FALSE) == FALSE) {
	    obj->o_pos = hero;
	    fall(item, TRUE);
	}
	return;
    }
    obj->o_type = newitem;		/* store the new items */
    obj->o_mark[0] = '\0';
    obj->o_which = newtype;
    obj->o_group = 0;
    obj->o_count = 1;
    obj->o_flags = 0;
    obj->o_dplus = obj->o_hplus = 0;
    obj->o_damage = obj->o_hurldmg = "0d0";
    obj->o_weight = 0;
    obj->o_mark[0] = '\0';
    wh = obj->o_which;
    mpos = 0;
    if (!wizard)
	whc = (cursed ? -1 : 1) * rnd(4);
    else			/* wizard gets to choose */
	whc = getbless();
    if (whc < 0)
	obj->o_flags |= ISCURSED;
    switch (obj->o_type) {
	case WEAPON:
	case ARMOR:
	    if (obj->o_type == WEAPON) {
		init_weapon(obj, wh);
		obj->o_hplus += whc;
		obj->o_dplus += whc;
	    }
	    else {				/* armor here */
		obj->o_weight = armors[wh].a_wght;
		obj->o_ac = armors[wh].a_class - whc;
	    }
	when RING:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    r_know[wh] = TRUE;
	    obj->o_ac = whc;
	    obj->o_weight = things[TYP_RING].mi_wght;
	when STICK:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    ws_know[wh] = TRUE;
	    fix_stick(obj);
	when SCROLL:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    obj->o_weight = things[TYP_SCROLL].mi_wght;
	    s_know[wh] = TRUE;
	when POTION:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    obj->o_weight = things[TYP_POTION].mi_wght;
	    p_know[wh] = TRUE;
    }
    mpos = 0;
    obj->o_flags |= ISKNOW;
    if (add_pack(item, FALSE) == FALSE) {
	obj->o_pos = hero;
	fall(item, TRUE);
    }
}

/*
 * getbless:
 *	Get a blessing for a wizards object
 */
int 
getbless ()
{
	char bless;

	msg("Blessing? (+,-,n)");
	bless = readchar();
	if (bless == '+')
		return (rnd(3) + 1);
	else if (bless == '-')
		return (-rnd(3) - 1);
	else
		return (0);
}

/*
 * make a monster for the wizard
 */
void 
makemon () 
{
    int i;
    short which_monst;
    int num_monst = nummonst+2, pres_monst=1, num_lines=2*(LINES-3);
    char monst_name[60];

    /* Print out the monsters */
    while (num_monst > 0) {
	int left_limit;

	if (num_monst < num_lines) left_limit = (num_monst+2)/2;
	else left_limit = num_lines/2;

	wclear(hw);
	touchwin(hw);

	/* Print left column */
	wmove(hw, 2, 0);
	for (i=0; i<left_limit; i++) {
	    sprintf(monst_name, "[%d] %s\n",
				pres_monst, monsters[pres_monst].m_name);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	/* Print right column */
	for (i=0; i<left_limit && pres_monst <= nummonst+2; i++) {
	    sprintf(monst_name, "[%d] %s",
				pres_monst, monsters[pres_monst].m_name);
	    wmove(hw, i+2, COLS/2);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	if ((num_monst -= num_lines) > 0) {
	    mvwaddstr(hw, LINES-1, 0, morestr);
	    draw(hw);
	    (void) wgetch(hw);
	}

	else {
	    mvwaddstr(hw, 0, 0, "Which monster");
	    waddstr(hw, " do you wish to create");
	    waddstr(hw, "? ");
	    draw(hw);
	}
    }

    do {
	monst_name[0] = '\0';
	i = get_string(monst_name, hw);
	if (i == QUIT) {
	    restscr(cw);
	    msg("");
	    return;
	}
	which_monst = atoi(monst_name);
	if (which_monst < 1 || which_monst > nummonst+2) {
	    mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
	    draw(hw);
	}
    } until (which_monst > 0 && which_monst <= nummonst+2);
    restscr(cw);
    creat_mons (&player, which_monst, TRUE);
    touchwin(cw);
}

/*
 * telport:
 *	Bamf the hero someplace else
 */

int 
teleport ()
{
    struct room *new_rp, *old_rp = roomin(&hero);
    int rm=0, which;
    coord c;
    bool is_lit = FALSE;	/* For saving room light state */
    bool rand_position = TRUE;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    if (ISWEARING(R_TELCONTROL)) {
	curs_set(1);  /* show cursor */
	msg("Where do you wish to teleport to? (* for help)");
	wmove(cw, hero.y, hero.x);
	draw(cw);
	which = readchar();
	while (which != ESCAPE 
		&& which != LINEFEED
		&& which != CARRIAGE_RETURN) {
	    switch(which) {
		case 'h': c.x--;
		when 'j': c.y++;
		when 'k': c.y--;
		when 'l': c.x++;
		when 'y': c.x--; c.y--;
		when 'u': c.x++; c.y--;
		when 'b': c.x--; c.y++;
		when 'n': c.x++; c.y++;
		when '*':
		    msg("Use h,j,k,l,y,u,b,n to position cursor, then hit return.");
	    }
	    c.y = max(c.y, 1);
	    c.y = min(c.y, LINES - 3);
	    c.x = max(c.x, 1);
	    c.x = min(c.x, COLS - 1);
	    wmove(cw, c.y, c.x);
	    draw(cw);
	    which = readchar();
	} 
	which = winat(c.y, c.x);
	if ((which == FLOOR || which == PASSAGE || which == DOOR) &&
	    ((ring_value(R_TELCONTROL) == 0 && rnd(10) < 6)
	    || (ring_value(R_TELCONTROL) > 0 && rnd(10) < 9))) {
	    rand_position = FALSE;
	    msg("You attempt succeeds.");
	    hero = c;
	    new_rp = roomin(&hero);
	}
	if (rand_position)
	    msg("Your attempt fails.");
	if (!showcursor) curs_set(0);  /* hide cursor */
    }
    if (rand_position) {
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &hero);
	} until(winat(hero.y, hero.x) == FLOOR);
	new_rp = &rooms[rm];
    }

    /* If hero gets moved, darken old room */
    if (old_rp && old_rp != new_rp) {
	if (!(old_rp->r_flags & ISDARK)) is_lit = TRUE;
	old_rp->r_flags |= ISDARK;	/* Fake darkness */
	light(&c);
	if (is_lit) old_rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);

    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	struct linked_list *ip, *nip;
	struct thing *mp;

	turn_off(player, ISHELD);
	hold_count = 0;
	for (ip = mlist; ip; ip = nip) {
	    mp = (struct thing *) ldata(ip);
	    nip = next(ip);
	    if (on(*mp, DIDHOLD)) {
		turn_off(*mp, DIDHOLD);
		turn_on(*mp, CANHOLD);
	    }
	    turn_off(*mp, DIDSUFFOCATE); /* Suffocation -- see below */
	}
    }

    /* Make sure player does not suffocate */
    extinguish_fuse(FUSE_SUFFOCATE);

    /* not trapped anymore */
    player.t_no_move = 0;

    count = 0;
    running = FALSE;
    return rm;
}

