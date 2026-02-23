
/*
 * Read a scroll and let it happen
 *
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

void 
read_scroll (int which, bool blessed)
{
    struct object *obj, *nobj;
    struct linked_list *item=NULL, *nitem;
    int i,j;
    char ch, nch;
    bool cursed, is_scroll;
    char buf[LINELEN];
    int limit;

    cursed = FALSE;
    is_scroll = FALSE;

    if (which < 0) {
	item = get_item("read", SCROLL);
	if (item == NULL)
	    return;

	obj = (struct object *) ldata(item);
	if (obj->o_type != SCROLL) {
	    msg ("There's nothing on it to read!");
	    return;
	}
	if (on(player, ISBLIND)) {
	    msg("You can't see to read anything.");
	    return;
	}
	/* remove it from the pack */
	inpack--;
	detach(pack, item);
	freeletter(item);

	msg("As you read the scroll, it vanishes.");
	/*
	 * Calculate the effect it has on the poor player.
	 */
	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;

	which = obj->o_which;
	is_scroll = TRUE;
    }


    switch(which)
    {
	case S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give that power.
	     */
	    msg("Your hands begin to glow red.");
	    turn_on(player, CANHUH);
	when S_CURING:
	    /*
	     * A cure disease spell
	     */
	    if (on(player, HASINFEST) || on(player, HASDISEASE)) {
		if (on(player, HASDISEASE)) {
		    extinguish_fuse(FUSE_CURE_DISEASE);
		    cure_disease(NULL);
		}
		if (on(player, HASINFEST)) {
		    extinguish_fuse(FUSE_CURE_INFEST);
		    cure_infest(NULL);
		}
		if (is_scroll)
	    	    s_know[S_CURING] = TRUE;
	    }
	    else msg("Nothing happens.");
	when S_LIGHT:
	    if (blue_light(blessed, cursed) && is_scroll)
		s_know[S_LIGHT] = TRUE;
	when S_HOLD:
	    if (cursed) {
		/*
		 * This scroll aggravates all the monsters on the current
		 * level and sets them running towards the hero
		 */
		aggravate();
		if (off(player, ISDEAF)) 
		    msg("You hear a high pitched humming noise.");
	    }
	    else if (blessed) { /* Hold all monsters on level */
		if (mlist == NULL) msg("Nothing happens.");
		else {
		    struct linked_list *mon;
		    struct thing *th;

		    for (mon = mlist; mon != NULL; mon = next(mon)) {
			th = (struct thing *) ldata(mon);
			turn_off(*th, ISRUN);
			turn_on(*th, ISHELD);
		    }
		    msg("A sudden peace comes over the dungeon.");
		}
	    }
	    else {
		/*
		 * Hold monster scroll.  Stop all monsters within two spaces
		 * from chasing after the hero.
		 */
		    int x,y;
		    struct linked_list *mon;
		    bool gotone=FALSE;

		    for (x = hero.x-2; x <= hero.x+2; x++) {
			for (y = hero.y-2; y <= hero.y+2; y++) {
			    if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x))) {
				if ((mon = find_mons(y, x)) != NULL) {
				    struct thing *th;

				    gotone = TRUE;
				    th = (struct thing *) ldata(mon);
				    turn_off(*th, ISRUN);
				    turn_on(*th, ISHELD);
				}
			    }
			}
		    }
		    if (gotone) msg("A sudden peace surrounds you.");
		    else msg("Nothing happens.");
	    }
	when S_SLEEP:
	    /*
	     * if cursed, you fall asleep
	     */
	    if (cursed) {
		if (ISWEARING(R_ALERT))
		    msg("You feel drowsy for a moment.");
		else {
		    msg("You fall asleep.");
		    no_command += 4 + rnd(SLEEPTIME);
		}
	    }
	    else {
		/*
		 * sleep monster scroll.  
		 * puts all monsters within 2 spaces asleep
		 */
		    int x,y;
		    struct linked_list *mon;
		    bool gotone=FALSE;

		    for (x = hero.x-2; x <= hero.x+2; x++) {
			for (y = hero.y-2; y <= hero.y+2; y++) {
			    if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x))) {
				if ((mon = find_mons(y, x)) != NULL) {
				    struct thing *th;

				    th = (struct thing *) ldata(mon);
				    if (on(*th, ISUNDEAD))
					continue;
				    th->t_no_move += SLEEPTIME;
				    gotone = TRUE;
				}
			    }
			}
		    }
		    if (gotone) 
			msg("The monster(s) around you seem to have fallen asleep.");
		    else 
			msg("Nothing happens.");
	    }
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around hero, next try the room
	     * otherwise give up
	     */
	    if (cursed) {
		i = rnd(4) + 3;
		for(j=0; j<i; j++)
		    creat_mons(&player, (short) 0, TRUE);
	    }
	    else
		creat_mons(&player, (short) 0, TRUE);
	when S_IDENT:
	    /* 
	     * if its blessed then identify everything in the pack
	     */
	    if (blessed) {
		msg("You feel more Knowledgeable!");
		idenpack();
	    }
	    else {
		/*
		 * Identify, let the rogue figure something out
		 */
		if (is_scroll && s_know[S_IDENT] != TRUE) {
		    msg("This scroll is an identify scroll.");
		}
		whatis(NULL);
	    }
	    if (is_scroll)
	        s_know[S_IDENT] = TRUE;
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    if (cursed) {
		msg("Your mind goes blank for a moment.");
		wclear(cw);
		light(&hero);
		status(TRUE);
		if (is_scroll)
		    s_know[S_MAP] = TRUE;
		break;
	    }
	    if (is_scroll && s_know[S_MAP] != TRUE) {
		msg("Oh! This scroll has a map on it!!");
		s_know[S_MAP] = TRUE;
	    }
	    if (blessed)
		turn_on(player, BLESSMAP);
	    overwrite(stdscr, hw);
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++) {
		    switch (nch = ch = mvwinch(hw, i, j)) {
			case SECRETDOOR:
			    /* mvaddch(i, j, nch = DOOR); */
			    nch = secretdoor(i,j);
			    break;
			case '-':
			case '|':
			case DOOR:
			case PASSAGE:
			case ' ':
			case STAIRS:
			    if (mvwinch(mw, i, j) != ' ') {
				struct thing *it;

				it = (struct thing *) ldata(find_mons(i, j));
				if (it && it->t_oldch == ' ')
				    it->t_oldch = nch;
			    }
			    break;
			default:
			    if (!blessed || !isatrap(ch))
				nch = ' ';
			    else {
				struct trap *tp;
				struct room *rp;

				tp = trap_at(i, j);
				rp = roomin(&hero);
				if (tp->tr_type == FIRETRAP && rp != NULL) {
				    rp->r_flags &= ~ISDARK;
				    light(&hero);
		    		}
		    		tp->tr_flags |= ISFOUND;
			    }
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    /*
	     * Copy in what we've discovered
	     */
	    overlay(cw, hw);
	    /*
	     * And set up for display
	     */
	    overwrite(hw, cw);
	when S_GFIND:
	    /*
	     * Scroll of gold detection
	     */
	    if (cursed) {
		int num = roll(3,6);
		int i;
		struct room *rp;
		coord pos;

		msg("You begin to feel greedy and you sense gold.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    mvwaddch(hw, pos.y, pos.x, GOLD);
		}
		overlay(hw, cw);
		if (is_scroll) 
		    s_know[S_GFIND] = TRUE;
		break;
	    }
	    if (blessed)
		turn_on(player, BLESSGOLD);
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
		    if (is_scroll) 
			s_know[S_GFIND] = TRUE;
		    msg("You begin to feel greedy and you sense gold.");
		    overlay(hw,cw);
		    break;
		}
	    }
	    msg("You begin to feel a pull downward.");
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make player disappear and reappear
	     */
	    if (cursed) {
		level += 5 + rnd(10*difficulty);
		new_level(NORMLEV);
		mpos = 0;
		msg("You are banished to the lower regions.");
	    }
	    else if (blessed) {
		int old_level;
		int much = rnd(5) - 4;
		old_level = level;
		if (much != 0) {
		    level += much;
		    if (level < 1)
			level = 1;
		    mpos = 0;
		    new_level(NORMLEV);		/* change levels */
		    if (level == old_level)
			status(TRUE);
		    msg("You are whisked away to another region.");
		}
	    }
	    else {
		teleport();
	    }
	    if (is_scroll)
	        s_know[S_TELEP] = TRUE;
	    if (off(player, ISCLEAR)) {
	        if (on(player, ISHUH))
		    lengthen_fuse(FUSE_UNCONFUSE, rnd(4)+4);
	        else {
		    light_fuse(FUSE_UNCONFUSE, 0, rnd(4)+4, AFTER);
	            turn_on(player, ISHUH);
		}
	    }
	    else msg("You feel dizzy for a moment, but it quickly passes.");
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
	    msg("You hear maniacal laughter in the distance.");
	when S_REMOVE:
	    if (cursed) { /* curse all player's possessions */
		for (nitem = pack; nitem != NULL; nitem = next(nitem)) {
		    nobj = OBJPTR(nitem);
		    if (nobj->o_flags & ISBLESSED) 
			nobj->o_flags &= ~ISBLESSED;
		    else 
			nobj->o_flags |= ISCURSED;
		}
		msg("The smell of fire and brimstone fills the air.");
	    }
	    else if (blessed) {
		bool fixed = FALSE;
		for (nitem = pack; nitem != NULL; nitem = next(nitem)) {
		    nobj = OBJPTR(nitem);
		    if (nobj->o_flags & ISCURSED)
			fixed = TRUE;
		    nobj->o_flags &= ~ISCURSED;
		}
		if (fixed)
		    msg("Your pack and contents glisten brightly.");
		else
		    msg("Your pack glistens brightly.");
	    }
	    else {
		if ((nitem = get_item("remove the curse on",0)) != NULL) {
		    nobj = OBJPTR(nitem);
		    nobj->o_flags &= ~ISCURSED;
		    msg("Removed the curse from %s.",inv_name(nobj,TRUE));
	    	    s_know[S_REMOVE] = TRUE;
		}
	    }
	when S_PETRIFY:
	    switch (mvinch(hero.y, hero.x)) {
		case TRAPDOOR:
		case DARTTRAP:
		case TELTRAP:
		case ARROWTRAP:
		case SLEEPTRAP:
		case BEARTRAP:
		case FIRETRAP:
		    {
			short i;

			/* Find the right trap */
			for (i=0; i<ntraps && !ce(traps[i].tr_pos, hero); i++);
			ntraps--;

			if (!ce(traps[i].tr_pos, hero))
			    msg("What a strange trap!");
			else {
			    while (i < ntraps) {
				traps[i] = traps[i + 1];
				i++;
			    }
			}
		    }
		    goto pet_message;
		case DOOR:
		case SECRETDOOR:
		    {
			struct room *rp=roomin(&hero);
			short i;

			/* Find the right door */
			for (i=0;
			     i<rp->r_nexits && !ce(rp->r_exit[i], hero); i++);
			rp->r_nexits--;

			if (!ce(rp->r_exit[i], hero))
			    msg("What a strange door!");
			else {
			    while (i < rp->r_nexits) {
				rp->r_exit[i] = rp->r_exit[i + 1];
				i++;
			    }
			}
		    }
		    __attribute__ ((fallthrough));
			    
		case FLOOR:
		case PASSAGE:
pet_message:	    msg("The dungeon begins to rumble and shake!");
		    addch(WALL);

		    /* If the player is phased, unphase them */
		    if (on(player, CANINWALL)) {
			extinguish_fuse(FUSE_UNPHASE);
			turn_off(player, CANINWALL);
			msg("Your dizzy feeling leaves you.");
		    }

		    /* Mark the player as in a wall */
		    turn_on(player, ISINWALL);
		    break;
		default:
		    msg("Nothing happens.");
	    }
	when S_GENOCIDE:
	    msg("You have been granted the boon of genocide!--More--");
	    wait_for(' ');
	    msg("");
	    genocide();
	    if (blessed) {
		msg("You have been granted the boon of genocide!--More--");
		wait_for(' ');
		msg("");
		genocide();  /* and again */
	    }
	    if (is_scroll) s_know[S_GENOCIDE] = TRUE;
	when S_PROTECT: 
	{
	    struct linked_list *ll;
	    struct object *lb;
	    if (is_scroll && s_know[S_PROTECT] == FALSE)
	        msg("You are granted the power of protection.");
	    if ((ll = get_item("protect",0)) != NULL) {
		lb = OBJPTR(ll);
		lb->o_flags |= ISPROT;
		if (blessed) {
		    lb->o_flags |= IS2PROT;
		    if (lb->o_type == ARMOR
			&& armors[lb->o_which].a_class - lb->o_ac >= 6 + difficulty)
			msg("Your armor shines brightly.");
		}
		msg("Protected %s.", inv_name(lb,TRUE));
	    }
	    s_know[S_PROTECT] = TRUE;
	}
	when S_MAKEIT:
	    s_know[S_MAKEIT] = TRUE;
	    msg("You have been endowed with the power of creation.");
	    if (blessed)
		create_obj(0, 0, FALSE);
	    else {
		char item=RING;

		switch (rnd(6)) {
		    case 0: item = RING;
		    when 1: item = POTION;
		    when 2: item = SCROLL;
		    when 3: item = ARMOR;
		    when 4: item = WEAPON;
		    when 5: item = STICK;
		}
		create_obj(item, 0, cursed);
	    }
	when S_ALLENCH: 
	{
	    struct linked_list *ll;
	    struct object *lb;
	    int howmuch, flags;
	    if (is_scroll && s_know[S_ALLENCH] == FALSE) {
		msg("You are granted the power of enchantment.");
		msg("You may enchant anything(weapon,ring,armor,scroll,potion)");
	    }
	    if ((ll = get_item("enchant",0)) != NULL) {
		lb = OBJPTR(ll);
		lb->o_flags &= ~ISCURSED;
		if (blessed) {
		    howmuch = 2;
		    flags = ISBLESSED;
		}
		else if (cursed) {
		    howmuch = -1;
		    flags = ISCURSED;
		}
		else {
		    howmuch = 1;
		    flags = ISBLESSED;
		}
		switch(lb->o_type) {
		    case RING:
			lb->o_ac += howmuch;
			limit = 5;
			if (blessed && (difficulty <= 2 || lb->o_flags & IS2PROT))
			    limit += 2;
			if (flags == ISBLESSED &&
			    (lb->o_which == R_WIZARD ||
			     lb->o_which == R_HEALTH ||
			     lb->o_which == R_SEEINVIS ||
			     lb->o_which == R_SEARCH ||
			     lb->o_which == R_REGEN  ||
			     lb->o_which == R_LEVITATION  ||
			     lb->o_which == R_DIGEST)
			    ) {
			    lb->o_flags |= ISBLESSED;
			    }
			if (lb->o_ac > limit && lb->o_ac < 11
				&& rnd(5) == 0) {
			    int on = is_r_on(lb);
			    msg("Your ring explodes in a cloud of smoke.");
			    lb->o_flags &= ~ISCURSED;
			    dropcheck(lb);
			    if (on) {
				switch(lb->o_which) {
				    case R_ADDSTR:
					chg_str(-2, TRUE, FALSE);
				    when R_ADDHIT:
					chg_dext(-2, TRUE, FALSE);
				    when R_ADDINTEL:
					pstats.s_intel -= 2;
					max_stats.s_intel -= 2;
				    when R_ADDWISDOM:
					pstats.s_wisdom -= 2;
					max_stats.s_wisdom -= 2;
				    default:
					;
				}
			    }
			    inpack--;
			    detach(pack, ll);
			    freeletter(ll);
			    discard(ll);
			    lb = NULL;
			}
			else if (is_r_on(lb)) {
			    switch(lb->o_which) {
				case R_ADDSTR:
				    pstats.s_str += howmuch;
				when R_ADDHIT:
				    pstats.s_dext += howmuch;
				when R_ADDINTEL:
				    pstats.s_intel += howmuch;
				when R_ADDWISDOM:
				    pstats.s_wisdom += howmuch;
				when R_CARRYING:
				    updpack(FALSE);
				default:
				    ;
			     }
			}
		    when ARMOR:
			lb->o_ac -= howmuch;
			limit = 8;
			if (blessed && (difficulty <= 2 || lb->o_flags & IS2PROT))
			    limit += 2;
			if (armors[lb->o_which].a_class - lb->o_ac > limit
				&& rnd(5) == 0) {
			    msg("Your %s explodes in a cloud of dust.",
				inv_name(lb, TRUE));
			    lb->o_flags &= ~ISCURSED;
			    if (lb == cur_armor)
			        pstats.s_hpt /= 2;
			    dropcheck(lb);
			    inpack--;
			    detach(pack, ll);
			    freeletter(ll);
			    discard(ll);
			    lb = NULL;
			} else if (armors[lb->o_which].a_class - lb->o_ac >= 6 + difficulty
				&& lb->o_flags & IS2PROT) {
			    msg("Your armor shines brightly.");
			}
		    when STICK:
			lb->o_charges += howmuch + 10;
			if (blessed)
			    lb->o_flags |= ISBLESSED;
			if (lb->o_charges < 0)
			    lb->o_charges = 0;
			if (lb->o_charges > 50
					&& rnd(5) == 0) {
			    msg("Your %s explodes in a cloud of dust.",
				inv_name(lb, TRUE));
			    lb->o_flags &= ~ISCURSED;
			    dropcheck(lb);
			    inpack--;
			    detach(pack, ll);
			    freeletter(ll);
			    discard(ll);
			    lb = NULL;
			}
		    when WEAPON:
			if (rnd(10) < 5 + lb->o_dplus - lb->o_hplus)
			    lb->o_hplus += howmuch;
			else
			    lb->o_dplus += howmuch;
			limit = 15;
			if (blessed && (difficulty <= 2 || lb->o_flags & IS2PROT))
			    limit += 2;
			if (lb->o_hplus + lb->o_dplus > limit
				&& rnd(5) == 0) {
			    msg("Your %s explodes in a cloud of dust.",
				inv_name(lb, TRUE));
			    lb->o_flags &= ~ISCURSED;
			    if (lb == cur_weapon)
			        chg_dext(-2, FALSE, TRUE);
			    dropcheck(lb);
			    inpack--;
			    detach(pack, ll);
			    freeletter(ll);
			    discard(ll);
			    lb = NULL;
			} else if (lb->o_hplus + lb->o_dplus > 15
				&& (lb->o_flags & ISSILVER)
				&& !(lb->o_flags & ISVORPED)
				&& difficulty <= 3) {
			    msg("Your weapon begins to shine.");
			    lb->o_flags |= ISVORPED;  /* vorpal blade */
			}
		    when POTION:
		    case SCROLL:
		    default:
			lb->o_flags |= flags;
		}
		if (lb != NULL) 
		    msg("Enchanted %s.", inv_name(lb,TRUE));
	    }
	    s_know[S_ALLENCH] = TRUE;
	}
	when S_NOTHING:
	    msg("This scroll is blank.");
	when S_SILVER: 
	{
	    struct linked_list *ll;
	    struct object *lb;
	    if (is_scroll && s_know[S_SILVER] == FALSE)
	        msg("You are granted the power of magic hitting.");
	    if ((ll = get_item("anoint",WEAPON)) != NULL) {
		lb = OBJPTR(ll);
		if (blessed && !(lb->o_flags & ISSILVER)) {
		    lb->o_hplus += rnd(2) + 2;
		    lb->o_flags |= ISSILVER;
		    if (lb->o_hplus + lb->o_dplus > 15 && !(lb->o_flags & ISVORPED)
			&& difficulty <= 3) {
			msg("Your weapon has turned to silver and begins to shine!");
			lb->o_flags |= ISVORPED;  /* vorpal blade */
		    } else {
			msg("Your weapon has turned to silver!");
		    }
		}
		else if (cursed && (lb->o_flags & ISSILVER)) {
		    lb->o_hplus -= (rnd(3) + 1);
		    lb->o_flags &= ~ISSILVER;
		    msg("Your weapon looks ordinary again.");
		}
		else if (lb->o_flags & ISSILVER) {
		    msg("Nothing happens.");
		}
		else {
		    lb->o_hplus += rnd(2) + 1;
		    lb->o_flags |= ISSILVER;
		    msg("Your weapon has turned to silver.");
		}
	    }
	    s_know[S_SILVER] = TRUE;
	}
	when S_OWNERSHIP: {
	    struct linked_list *ll;
	    struct object *lb;
	    if (is_scroll && s_know[S_OWNERSHIP] == FALSE)
	        msg("You are granted the power of ownership.");
	    if ((ll = get_item("claim",0)) != NULL) {
		lb = OBJPTR(ll);
		if (cursed && lb->o_flags & (ISOWNED | CANRETURN)) {
		    lb->o_flags &= ~(ISOWNED | CANRETURN);
		    msg("The gods seem to have forgotten you.");
		}
		else if (cursed && !(lb->o_flags & ISLOST)) {
		    lb->o_flags |= ISLOST;
		    msg("The gods look the other way.");
		}
		else if (blessed && lb->o_flags & ISLOST) {
		    lb->o_flags |= CANRETURN;
		    msg("The gods seem to have remembered you.");
		}
		else if (blessed && !(lb->o_flags & ISOWNED)) {
		    lb->o_flags |= (ISOWNED | CANRETURN);
		    msg("The gods smile upon you.");
		}
		else if (blessed | cursed) {
		    msg("Nothing happens.");
		}
		else {
		    lb->o_flags |= CANRETURN;
		    msg("The gods look upon you.");
		}
	    }
	    s_know[S_OWNERSHIP] = TRUE;
	}
	when S_FOODFIND:
	    /*
	     * Scroll of food detection
	     */
	    if (cursed) {
		int num = roll(3,6);
		int i;
		struct room *rp;
		coord pos;

		msg("You begin to feel hungry and you smell food.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    mvwaddch(hw, pos.y, pos.x, FOOD);
		}
		overlay(hw, cw);
		if (is_scroll) 
		    s_know[S_FOODFIND] = TRUE;
		break;
	    }
	    if (blessed)
		turn_on(player, BLESSFOOD);
	    if (off(player, ISUNSMELL) && lvl_obj != NULL) {
		struct linked_list *item;
		struct object *cur;
		struct thing *th;
		int fcount = 0;
		bool same_room = FALSE;
		struct room *rp = roomin(&hero);

		wclear(hw);
		for (item = lvl_obj; item != NULL; item = next(item)) {
		    cur = (struct object *) ldata(item);
		    if (cur->o_type == FOOD) {
			fcount += cur->o_count;
			mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, FOOD);
			if (roomin(&cur->o_pos) == rp)
			    same_room = TRUE;
		    }
		}
		for (item = mlist; item != NULL; item = next(item)) {
		    struct linked_list *pitem;

		    th = (struct thing *) ldata(item);
		    for(pitem=th->t_pack; pitem!=NULL; pitem=next(pitem)) {
			cur = (struct object *) ldata(pitem);
			if (cur->o_type == FOOD) {
			    fcount += cur->o_count;
			    mvwaddch(hw, th->t_pos.y, th->t_pos.x, FOOD);
			    if (roomin(&th->t_pos) == rp)
				same_room = TRUE;
			}
		    }
		}
		if (fcount) {
		    if (is_scroll) 
			s_know[S_FOODFIND] = TRUE;
		    if (same_room)
			msg("FOOOOD!!");
		    else
			msg("You begin to feel hungry and you smell food.");
		    overlay(hw,cw);
		    break;
		}
	    }
	    if (off(player, ISUNSMELL))
	        msg("You can't smell anything.");
	    else
		msg("Nothing happens.");
	when S_ELECTRIFY:
		if (on(player, ISELECTRIC)) {
		    msg("Your violet glow brightens for an instant.");
		    lengthen_fuse(FUSE_UNELECTRIFY, 4 + rnd(8));
		}
		else {
		    msg("Your body begins to glow violet and shoot sparks.");
		    turn_on(player, ISELECTRIC);
		    light_fuse(FUSE_UNELECTRIFY, 0, (blessed ? 3 : 1)*WANDERTIME, AFTER);
		    light(&hero);
		}
		if (is_scroll) 
		    s_know[S_ELECTRIFY] = TRUE;
	when S_PEACE:
	    if (cursed) {
		msg("Nothing happens.");
	    }
	    else { /* Calm all monsters in the room */
		if (mlist == NULL) msg("Nothing happens.");
		else {
		    calm(blessed);
		    if (blessed) 
			msg("A deep peace surrounds you.");
		    else
			msg("A sudden peace surrounds you.");
		}
	    }
	otherwise:
	    msg("What a puzzling scroll!");
	    return;
    }
    look(TRUE);	/* put the result of the scroll on the screen */
    status(FALSE);
    if (is_scroll && s_know[which] && s_guess[which])
    {
	FREE(s_guess[which]);
	s_guess[which] = NULL;
    }
    else if (is_scroll && 
	     !s_know[which] && 
	     askme &&
	     s_guess[which] == NULL) {
	msg("What do you want to call it? ");
	buf[0] = '\0';
	if (get_string(buf, cw) == NORM && strlen(buf) > 0)
	{
	    s_guess[which] = my_malloc(strlen(buf) + 1);
	    strcpy(s_guess[which], buf);
	}
    }
    if (is_scroll) discard(item);
    updpack(TRUE);
}

/*
 * Creat_mons creates the specified monster -- any if 0 
 */

struct thing *
creat_mons (struct thing *person, int monster, bool report)
{
    int x, y;
    bool appear = 0;
    struct thing play_copy;	/* A copy with no phasing */
    struct thing *tp;
    coord mp;

    /* Make sure phasing is off in the copy */
    play_copy = *person;
    turn_off(play_copy, CANINWALL);

    /*
     * Search for an open place
     */
    for (y = person->t_pos.y-1; y <= person->t_pos.y+1; y++)
	for (x = person->t_pos.x-1; x <= person->t_pos.x+1; x++)
	{
	    /*
	     * Don't put a monster in top of the creator or player.
	     */
	    if (y == person->t_pos.y && x == person->t_pos.x)
		continue;
	    if (y == hero.y && x == hero.x) continue;

	    /*
	     * Or anything else nasty 
	     */
	    if (step_ok(y, x, NOMONST, &play_copy)) {
		appear = TRUE;
		if (rnd(1) == 0)
		{
		    mp.y = y;
		    mp.x = x;
		}
	    }
	}
    if (appear) {
	struct linked_list *nitem;

	nitem = new_item(sizeof (struct thing));
	new_monster(nitem, monster == 0 ? randmonster(FALSE, FALSE)
					: monster, &mp, TRUE);
	runto(&mp, &hero);
	tp = THINGPTR(nitem);
	(void) wake_monster(tp->t_pos.y, tp->t_pos.x);

	/* Previously not seen -- now can see it */
	if (on(*tp, HASFIRE) && roomin(&tp->t_pos) == roomin(&hero)) {
	    light(&hero);
	} else if (cansee(tp->t_pos.y, tp->t_pos.x)) {
	    char ch = show(tp->t_pos.y, tp->t_pos.x);
	    mvwaddch(cw, tp->t_pos.y, tp->t_pos.x, ch);
	}

	return(tp);
    }
    if (report) msg("You hear a faint cry of anguish in the distance.");
    return((struct thing *) NULL);
}

/*
 *	This subroutine determines if an object that is a ring is
 *      being worn by the hero  by Bruce Dautrich 4/3/84
 */
int 
is_r_on (struct object *obj)
{
	if(obj==cur_ring[LEFT_1]||obj==cur_ring[LEFT_2]||
	   obj==cur_ring[LEFT_3]||obj==cur_ring[LEFT_4]||
	   obj==cur_ring[RIGHT_1]||obj==cur_ring[RIGHT_2]||
	   obj==cur_ring[RIGHT_3]||obj==cur_ring[RIGHT_4]) {
	   return TRUE;
	}
	return FALSE;
}
