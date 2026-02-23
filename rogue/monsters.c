/*
 * File with various monster functions in it
 *
 */

#include "curses.h"
#include "rogue.h"
#include <ctype.h>
#include <string.h>

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */

short 
randmonster (bool wander, bool no_unique)
{
    int d, cur_level, range, i; 
    float nlevmons = NLEVMONS;

    /* 
     * Do we want a merchant? Merchant is always in place 'nummonst' 
     */
    if (wander && monsters[nummonst].m_wander && rnd(100) < 3) return nummonst;

    /*
     * Calculate the number of new monsters revealed per level,
     * to ensure we have enough to go around, and not too many.
     */
    nlevmons = nummonst/100.0;

    cur_level = level;
    range = 4*NLEVMONS;
    i = 0;
    do
    {
	if (i++ > range*10) { /* just in case all have be genocided */
	    i = 0;
	    if (cur_level > 90)
		cur_level -= 5;  /* so we don't get the same monster each time */
	    if (--cur_level <= 0) {
		if (wander)
		    fatal("Rogue could not make a new wandering monster");
		else
		    fatal("Rogue could not find a monster to make");
	    }
	}
	if (cur_level <= 4) {
	    d = NLEVMONS*(cur_level - 1) + (rnd(range) - (range - 1 - NLEVMONS));
	} else {
	    d = NLEVMONS*4 +
		nlevmons*(cur_level - 1 - 4) + (rnd(range) - (range - 1 - NLEVMONS));
	}
	/* make the mid-dungeon more interesting */
	if (difficulty > 3 && wander && cur_level > 40 && cur_level < 80
	    && d < nummonst-50 && rnd(15) == 0) {
	    d += 50;
	}
	if (d < 1)
	    d = rnd(NLEVMONS) + 1;
	if (d > nummonst - NUMUNIQUE - 1) {
	    if (no_unique)
		d = rnd(range) + (nummonst - NUMUNIQUE - 1) - (range - 1);
	    else if (d > nummonst - 1)
		d = rnd(range+NUMUNIQUE) + (nummonst-1) - (range+NUMUNIQUE-1);
	}
    }
    while  ( wander ? !monsters[d].m_wander || !monsters[d].m_normal 
		   : !monsters[d].m_normal);

    return d;
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

void 
new_monster (struct linked_list *item, int type, coord *cp, bool max_monster)
{
    struct thing *tp;
    struct monster *mp;
    char *ip, *hitp;
    short i, min_intel, max_intel;
    short num_dice, num_sides=8, num_extra=0;

    attach(mlist, item);
    tp = THINGPTR(item);
    tp->t_index = type;
    tp->t_wasshot = FALSE;
    tp->t_type = monsters[type].m_appear;
    tp->t_ctype = C_MONSTER;
    tp->t_no_move = 0;
    tp->t_quiet = 0;
    tp->t_doorgoal = -1;
    tp->t_oldpos = *cp;
    tp->t_pos = tp->t_oldpos;
    tp->t_dest = &(tp->t_pos);
    tp->t_oldch = mvwinch(cw, cp->y, cp->x);
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_index];

    /* Figure out monster's hit points */
    hitp = mp->m_stats.s_hpt;
    num_dice = atoi(hitp);
    if ((hitp = strchr(hitp, 'd')) != NULL) {	/* strchr was index */
	num_sides = atoi(++hitp);
	if ((hitp = strchr(hitp, '+')) != NULL)
	    num_extra = atoi(++hitp);
    }

    if (max_monster)
	tp->t_stats.s_hpt = num_dice * num_sides + num_extra;
    else
	tp->t_stats.s_hpt = roll(num_dice, num_sides) + num_extra;
    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    tp->t_stats.s_dmg = mp->m_stats.s_dmg;
    tp->t_stats.s_exp = mp->m_stats.s_exp + mp->m_add_exp*tp->t_stats.s_hpt;
    tp->t_stats.s_str = mp->m_stats.s_str;

    if (max_level > 80) {
	tp->t_stats.s_hpt += roll(4,(max_level-60)*2);
	tp->t_stats.s_lvl += roll(4,(max_level-60)/8);
	tp->t_stats.s_arm -= roll(2,(max_level-60)/8);
	tp->t_stats.s_str += roll(2,(max_level-60)/12);
	tp->t_stats.s_exp += roll(4, (max_level - 60) * 2) * mp->m_add_exp;
    } else if (type > (level+25)*NLEVMONS && levtype == NORMLEV
	&& type < nummonst && difficulty > 2) {
	tp->t_stats.s_hpt += roll(4,(max_level-40)*2);
	tp->t_stats.s_lvl += roll(4,(max_level-40)/8);
	tp->t_stats.s_arm -= roll(2,(max_level-40)/8);
	tp->t_stats.s_str += roll(2,(max_level-40)/12);
	tp->t_stats.s_exp += roll(4, (max_level - 40) * 2) * mp->m_add_exp;
    }

    /*
     * just initialize other values to something reasonable for now
     * maybe someday will *really* put these in monster table
     */
    tp->t_stats.s_wisdom = 8 + rnd(4);
    tp->t_stats.s_dext = 8 + rnd(4);
    tp->t_stats.s_const = 8 + rnd(4);
    tp->t_stats.s_charisma = 8 + rnd(4);

    if (max_level > 60)
	tp->t_stats.s_dext += roll(2,(max_level-50)/8);

    /* Set the initial flags */
    for (i=0; i<NT_FLAGS; i++) tp->t_flags[i] = 0;
    for (i=0; i<NM_FLAGS; i++)
	turn_on(*tp, mp->m_flags[i]);

    /* suprising monsters don't always surprise you */
    if (!max_monster && on(*tp, CANSURPRISE) && rnd(100) < 20)
	    turn_off(*tp, CANSURPRISE);

    /* If this monster is unique, genocide it */
    if (on(*tp, ISUNIQUE)) mp->m_normal = FALSE;

    /* gods get special abilities */
    if (on(*tp, ISGOD)) {
	turn_on(*tp, CANFRIGHTEN);
	turn_on(*tp, CANCAST);  /* not implemented */
	turn_on(*tp, CANFLY);
	turn_on(*tp, CANBARGAIN);  /* not implemented */
	turn_on(*tp, ISLARGE);
	turn_on(*tp, CANTELEPORT);  /* not implemented */
	turn_on(*tp, CANSPEAK);  /* not implemented */
	turn_on(*tp, CANDARKEN);  /* not implemented */
	turn_on(*tp, CANSEE);
	turn_on(*tp, CANLIGHT);  /* not implemented */
	turn_on(*tp, BMAGICHIT);
    }

    /* make some monsters even nastier in difficult games */
    if (difficulty > 2) {
	if (strcmp(mp->m_name,"vilstrak") == 0) {
	    turn_on(*tp, ISMEAN);
#if 0
	} else if (off(*tp, ISMEAN) && off(*tp, LOWFRIENDLY)
		&& off(*tp, MEDFRIENDLY) && off(*tp, HIGHFRIENDLY)
		&& strcmp(mp->m_name,"quartermaster") != 0
		&& strcmp(mp->m_name,"valkyrie") != 0
		&& rnd(3) == 0) {  /* some neutral monsters turn mean */
	    turn_on(*tp, ISMEAN);
#endif
	} else if (strcmp(mp->m_name,"valkyrie") == 0 && rnd(3) > 0) {
	    turn_on(*tp, CANSUMMON);
	} else if (strcmp(mp->m_name,"time elemental") == 0 && rnd(3) > 0) {
	    turn_on(*tp, CANSUMMON);
	}
    }

    tp->t_turn = TRUE;
    tp->t_pack = NULL;

    /* Normally scared monsters have a chance to not be scared */
    if (on(*tp, ISFLEE) && (rnd(4) == 0)) turn_off(*tp, ISFLEE);

    /* Figure intelligence */
    min_intel = atoi(mp->m_intel);
    if ((ip = (char *) strchr(mp->m_intel, '-')) == NULL)
	tp->t_stats.s_intel = min_intel;
    else {
	max_intel = atoi(++ip);
	if (max_monster)
	    tp->t_stats.s_intel = max_intel;
	else
	    tp->t_stats.s_intel = min_intel + rnd(max_intel - min_intel);
    }
    tp->maxstats = tp->t_stats;

    /* If the monster can shoot, it may have a weapon */
    if (on(*tp, CANSHOOT) && (rnd(100) < 25 || max_monster)) {
	struct linked_list *item, *item1;
	struct object *cur, *cur1;

	item = new_item(sizeof *cur);
	item1 = new_item(sizeof *cur1);
	cur = OBJPTR(item);
	cur1 = OBJPTR(item1);
	cur->o_hplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur->o_dplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur1->o_hplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur1->o_dplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur->o_damage = cur->o_hurldmg =
		cur1->o_damage = cur1->o_hurldmg = "0d0";
	cur->o_ac = cur1->o_ac = 11;
	cur->o_count = cur1->o_count = 1;
	cur->o_group = cur1->o_group = 0;
	if ((cur->o_hplus <= 0) && (cur->o_dplus <= 0)) cur->o_flags = ISCURSED;
	if ((cur1->o_hplus <= 0) && (cur1->o_dplus <= 0))
	    cur1->o_flags = ISCURSED;
	cur->o_flags = cur1->o_flags = 0;
	cur->o_type = cur1->o_type = WEAPON;
	cur->o_mark[0] = cur1->o_mark[0] = '\0';

	/* The monster may use a crossbow, sling, footbow, or an arrow */
	i = rnd(100);
	if (i < 9) {
	    cur->o_which = CROSSBOW;
	    cur1->o_which = BOLT;
	    init_weapon(cur, CROSSBOW);
	    init_weapon(cur1, BOLT);
	}
	else if (i < 54 && off(*tp, ISSMALL)) {
	    cur->o_which = BOW;
	    init_weapon(cur, BOW);
	    if (strcmp(mp->m_name,"elf") == 0 && rnd(10) < 1) {
		cur1->o_which = SILVERARROW;
		init_weapon(cur1, SILVERARROW);
	    }
	    else {
		cur1->o_which = ARROW;
		init_weapon(cur1, ARROW);
            }
	}
	else if (i < 65 || on(*tp, ISLARGE)) {
	    cur->o_which = FOOTBOW;
	    cur1->o_which = FBBOLT;
	    init_weapon(cur, FOOTBOW);
	    init_weapon(cur1, FBBOLT);
	}
	else {
	    cur->o_which = SLING;
	    cur1->o_which = ROCK;
	    init_weapon(cur, SLING);
	    init_weapon(cur1, ROCK);
	}

	attach(tp->t_pack, item);
	attach(tp->t_pack, item1);
    }


    if (ISWEARING(R_AGGR))
	runto(cp, &hero);

    /* see if we can charm it */
    if (type < nummonst) {  /* except friendly fiend and lucifer */
	i = roll(1,100);
#if 0
	if (i == 0 || (on(*tp, LOWFRIENDLY) && i < (pstats.s_charisma - 8)) ||
	    (on(*tp, MEDFRIENDLY) && i < 2 * (pstats.s_charisma - 8)) ||
	    (on(*tp, HIGHFRIENDLY) && i < 3 * (pstats.s_charisma - 8)))
#endif
	if (i == 0 || (on(*tp, LOWFRIENDLY) && i < pstats.s_charisma) ||
	    (on(*tp, MEDFRIENDLY) && i < 2 * pstats.s_charisma) ||
	    (on(*tp, HIGHFRIENDLY) && i < 3 * pstats.s_charisma))
	{
	    if (i > 90)
		turn_on(*tp, ISFRIENDLY);
	    turn_off(*tp, ISMEAN);
	    tp->t_dest = &(tp->t_pos);
	    turn_off(*tp, ISRUN);
	}
    }

    if (on(*tp, ISDISGUISE))
    {
	char mch = '@';

	if (tp->t_pack != NULL)
	    mch = (OBJPTR(tp->t_pack))->o_type;
	else
	    switch (rnd(level > arts[0].ar_level ? 10 : 9))
	    {
		case 0: mch = GOLD;
		when 1: mch = POTION;
		when 2: mch = SCROLL;
		when 3: mch = FOOD;
		when 4: mch = WEAPON;
		when 5: mch = ARMOR;
		when 6: mch = RING;
		when 7: mch = STICK;
		when 8: mch = monsters[randmonster(FALSE, FALSE)].m_appear;
		when 9: mch = ARTIFACT;
	    }
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

void 
wanderer ()
{
    int i, j, cnt=0;
    struct room *hr = roomin(&hero);
    struct linked_list *item;
    struct thing *tp;
    coord cp;
    char *loc;

    /* Find a place for it -- avoid the player's room */
    do {
	do {
	    if (cnt++ > 5000)
		return;
	    i = rnd_room();
	} until(hr != &rooms[i] || levtype == MAZELEV || levtype == THRONE);
	rnd_pos(&rooms[i], &cp);
    } until (step_ok(cp.y, cp.x, NOMONST, NULL));

    /* Create a new wandering monster */
    item = new_item(sizeof *tp);
    new_monster(item, randmonster(TRUE, FALSE), &cp, FALSE);
    tp = THINGPTR(item);
    turn_on(*tp, ISRUN);
    turn_off(*tp, ISDISGUISE);
    if (off(*tp, ISFRIENDLY))
	tp->t_dest = &hero;

    tp->t_pos = cp;	/* Assign the position to the monster */

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

    if (on(*tp, HASFIRE)) {
	rooms[i].r_flags |= HASFIRE;
	rooms[i].r_fires++;
    }

    i = DISTANCE(cp.x, cp.y, hero.x, hero.y);

    if (i < 20)
	loc = "very close to you";
    else if (i < 400)
	loc = "nearby";
    else
	loc = "in the distance";

    if (on(player, CANSCENT) || (player.t_ctype == C_THIEF && rnd(40) == 0)) 
	msg("You smell a new %s %s.", monsters[tp->t_index].m_name, loc);
    if (on(player, CANHEAR) || (player.t_ctype == C_THIEF && rnd(40) == 0)) 
	msg("You hear a new %s moving %s.", monsters[tp->t_index].m_name, loc);

    /* debug("Started a wandering %s.", monsters[tp->t_index].m_name); */
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list *
wake_monster (int y, int x)
{
    struct thing *tp;
    struct linked_list *it;
    struct room *trp;
    char *mname;
    int aplus; /* extent to which armor is blessed/cursed */

    if (cur_armor == NULL)
	aplus = 0;
    else
	aplus = cur_armor->o_ac - armors[cur_armor->o_which].a_class;


    if ((it = find_mons(y, x)) == NULL) {
	debug("Can't find monster in show.");
	return(NULL);
    }
    tp = THINGPTR(it);

    trp = roomin(&tp->t_pos); /* Current room for monster */
    mname = monsters[tp->t_index].m_name;

    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN))
	if ((trp != NULL) && (lvl_obj != NULL)) {
	    struct linked_list *item;
	    struct object *cur;

	    for (item = lvl_obj; item != NULL; item = next(item)) {
		cur = OBJPTR(item);
		if ((cur->o_type == GOLD) &&
		    (roomin(&cur->o_pos) == trp)) {
		    /* Run to the gold */
		    tp->t_dest = &cur->o_pos;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);

		    /* Make it worth protecting */
		    cur->o_count += roll(2,3) * GOLDCALC;
		    break;
		}
	    }
	}

    /*
     * Every time you see a mean monster, it might start chasing you,
     * unique monsters always do
     */
    if (on(*tp, ISUNIQUE) || (rnd(100) > 33 && on(*tp, ISMEAN) && 
	off(*tp, ISHELD) && off(*tp, ISRUN) && !is_stealth(&player) &&
	(off(player, ISINVIS) || on(*tp, CANSEE))))
    {
	if (off(*tp, ISFRIENDLY) || levtype == POSTLEV) {
	    tp->t_dest = &hero;
	    turn_on(*tp, ISRUN);
	    turn_off(*tp, ISDISGUISE);
	}
    }

    /* Handle monsters that can gaze */
    if (on(*tp, ISRUN) && 	/* Monster is not asleep */
	cansee(tp->t_pos.y, tp->t_pos.x)) {	/* Player can see monster */
	/*
	 * Confusion
	 */
	if (on(*tp, CANHUH)) {
	    if (!save(VS_MAGIC)) {
		if (off(player, ISCLEAR) && off(player, ISINVIS)) {
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(20)+HUHDURATION);
		    else {
			light_fuse(FUSE_UNCONFUSE, 0, rnd(20)+HUHDURATION, AFTER);
			msg("The %s's gaze has confused you.",mname);
			turn_on(player, ISHUH);
		    }
		} else {
		    if (!fighting)
			msg("You feel dizzy for a moment, but it quickly passes.");
		}
	    }
	    else if (rnd(100) < 67)
		turn_off(*tp, CANHUH); /* Once you save, maybe that's it */
	}

	/* Sleep */
	if (on(*tp, CANSNORE) && no_command == 0 &&
	    !save(VS_PARALYZATION)) {
 	    if (on(player, ISINVIS)
	     || (cur_armor != NULL && cur_armor->o_flags & IS2PROT
		    && ((aplus < -5 - difficulty) || difficulty < 2)
		    && difficulty <= 3)) {
		msg("The gaze of the %s is deflected by your shiny armor.", mname);
	    } else if (ISWEARING(R_ALERT)) {
		msg("You feel slightly drowsy for a moment.");
	    } else {
		msg("The %s's gaze puts you to sleep.", mname);
		no_command = SLEEPTIME;
		if (rnd(100) < 50) turn_off(*tp, CANSNORE);
	    }
	}

	/* Fear */
	if (on(*tp, CANFRIGHTEN) && off(*tp, CANSURPRISE)) {
	    turn_off(*tp, CANFRIGHTEN);
	    if (!save(VS_WAND) &&
	        !(on(player, ISFLEE) && (player.t_dest == &tp->t_pos))) {
		    if (off(player, SUPERHERO) && off(player, ISINVIS)) {
			turn_on(player, ISFLEE);
			player.t_dest = &tp->t_pos;
			msg("The sight of the %s terrifies you.", mname);
		    }
		    else
			msg("My, the %s looks ugly.", mname);
	    }
	}

	/* blinding creatures */
	if (on(*tp, CANBLIND) && off(player, ISBLIND)
		&& off(player, ISINVIS)
#ifdef EARL
		&& !ISWEARING(R_SEEINVIS)
#endif
		&& !save(VS_WAND)) {
	    if (cur_armor != NULL && cur_armor->o_flags & IS2PROT
		    && ((aplus < -5 - difficulty) || difficulty < 2)
		    && difficulty <= 3) {
		msg("The gaze of the %s is deflected by your shiny armor.", mname);
	    } else {
		msg("The gaze of the %s blinds you.", mname);
		turn_on(player, ISBLIND);
		light_fuse(FUSE_SIGHT, 0, rnd(30)+20, AFTER);
		look(FALSE);
	    }
	}

	/* Turning to stone */
	if (on(*tp, LOOKSTONE)) {
	    turn_off(*tp, LOOKSTONE);

	    if (on(player, CANINWALL) || on(player, ISINVIS)
	     ) {
		msg("The gaze of the %s has no effect.", mname);
	    } else if (cur_armor != NULL && cur_armor->o_flags & IS2PROT
		    && ((aplus < -5 - difficulty) || difficulty < 2)
		    && difficulty <= 3) {
		msg("The gaze of the %s is deflected by your shiny armor.", mname);
	    } else {
		if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
		    msg("The gaze of the %s petrifies you.", mname);
		    if (difficulty >= 2) {
			msg("You are turned to stone !!!");
			mvwaddstr(cw, 0, mpos+1, retstr);
			wait_for('\n');
			death(D_PETRIFY);
			return NULL;  /* prevent NPE in case we die and are reborn */
		    } else {
			no_command = STONETIME;
			fighting = FALSE;
		    }
		}
		else {
		    msg("The gaze of the %s stiffens your limbs.", mname);
		    no_command = STONETIME;
		}
	    }
	}
    }

    /*
     * hero might be able to hear or smell monster if player can't see it
     */
    if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 || on(player, CANHEAR))
		 && !cansee(tp->t_pos.y, tp->t_pos.x))
	msg("You hear a %s nearby.", mname);
    if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 || on(player, CANSCENT))
		 && !cansee(tp->t_pos.y, tp->t_pos.x))
	msg("You smell a %s nearby.", mname);

    return it;
}

void 
genocide ()
{
    struct linked_list *ip;
    struct thing *mp;
    int i, ch;
    struct linked_list *nip;
    int num_monst = nummonst-1, pres_monst=1, num_lines=2*(LINES-3);
    short which_monst;
    char monst_name[60];
    char monst_num[13];	/* blank if monster is already gone */

    /* Print out the monsters */
    while (num_monst > 0) {
	int left_limit;

	if (num_monst < num_lines) left_limit = (num_monst+1)/2;
	else left_limit = num_lines/2;

	wclear(hw);
	touchwin(hw);

	/* Print left column */
	wmove(hw, 2, 0);
	for (i=0; i<left_limit; i++) {
	    snprintf(monst_num, 4, "%d", pres_monst);
	    if (monsters[pres_monst].m_normal == FALSE &&
		monsters[pres_monst].m_wander == FALSE) {
		sprintf(monst_num, "%*c", (int) strlen(monst_num), ' ');
	    }
	    sprintf(monst_name, "[%s] %s\n",
				monst_num, monsters[pres_monst].m_name);
	    if ((int)strlen(monst_name) > COLS/2)
		monst_name[COLS/2] = '\0';	/* truncate long names */
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	/* Print right column */
	for (i=0; i<left_limit && pres_monst<=nummonst-1; i++) {
	    snprintf(monst_num, 12, "%d", pres_monst);
	    if (monsters[pres_monst].m_normal == FALSE &&
		monsters[pres_monst].m_wander == FALSE) {
		sprintf(monst_num, "%*c", (int) strlen(monst_num), ' ');
	    }
	    sprintf(monst_name, "[%s] %s\n",
				monst_num, monsters[pres_monst].m_name);
	    if ((int)strlen(monst_name) > COLS/2)
		monst_name[COLS/2] = '\0';	/* truncate long names */
	    wmove(hw, i+2, COLS/2);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	if ((num_monst -= num_lines) > 0) {
	    mvwaddstr(hw, LINES-1, 0, morestr);
	    draw(hw);
	    ch = wgetch(hw);
	    if (ch >= '1' && ch <= '9') {
		ungetch(ch);
		mvwaddstr(hw, 0, 0, "Which monster do you wish to wipe out? ");
		draw(hw);
		goto get_monst;
	    }
	}

	else {
	    mvwaddstr(hw, 0, 0, "Which monster do you wish to wipe out? ");
	    draw(hw);
	}
    }

get_monst:
    monst_name[0] = '\0';
    get_string(monst_name, hw);
    if (strcmp(monst_name, "q") == 0 || strlen(monst_name) < 1) {
	clearok(cw, TRUE);
	touchwin(cw);
	return;
    }
    if (strcmp(monst_name, "b") == 0) {	/* back up and try again */
	genocide();
	return;
    }
    if (strcmp(monst_name, "0") == 0 && difficulty >= 2) {	/* suicide */
	death(D_MISADVENTURE);
	return;
    }
    which_monst = atoi(monst_name);
    /* nummonst==quartermaster, nummonst+2=shopkeeper */
    if (which_monst < 1 ||
        (which_monst > nummonst && which_monst != nummonst+2)) {
	mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
	draw(hw);
	goto get_monst;
    }

    /* Set up for redraw */
    clearok(cw, TRUE);
    touchwin(cw);

    /* Remove this monster from the present level */
    for (ip = mlist; ip; ip = nip) {
	mp = THINGPTR(ip);
	nip = next(ip);
	if (mp->t_index == which_monst) {
	    killed(ip, FALSE, TRUE);
	}
    }

    /* Remove from available monsters */
    monsters[which_monst].m_normal = FALSE;
    monsters[which_monst].m_wander = FALSE;
    mpos = 0;
    msg("You have wiped out the %s.", monsters[which_monst].m_name);
}


/*
 * id_monst returns the name(s) of a monster given its letter
 */

char * 
id_monst (int monster)
{
    int i;
    static char buf[200];
    struct linked_list *item;
    struct thing *tp;

    /*
     * check monsters on current level
     */
    strcpy(buf, "");
    for (item = mlist; item != NULL; item = next(item)) {
        tp = THINGPTR(item);
        if (tp->t_type == monster && strstr(buf,monsters[tp->t_index].m_name) == NULL) {
	    strcat(buf, monsters[tp->t_index].m_name);
	    strcat(buf, ",");
	}
    }
    /*
     * if none match, check all monsters
     */
    if (buf[0] == '\0') {
	for (i=1; i<=nummonst+2; i++) {
	    if (monsters[i].m_appear == monster) {
		strcat(buf, monsters[i].m_name);
		strcat(buf, ",");
	    }
	}
    }
    if ((int)strlen(buf) >= COLS - 5)
	buf[COLS-5] = '\0';
    buf[strlen(buf)-1] = '\0';
    return(buf);
}


/*
 * Check_residue takes care of any effect of the monster 
 */
void 
check_residue (struct thing *tp)
{
    /*
     * Take care of special abilities
     */
    if (on(*tp, DIDHOLD) && (--hold_count == 0)) turn_off(player, ISHELD);

    /* If it has lowered player, give back a level, maybe */
    if (on(*tp, DIDDRAIN) && rnd(2) == 0) raise_level();

    /* If frightened of this monster, stop */
    if (on(player, ISFLEE) &&
	player.t_dest == &tp->t_pos) turn_off(player, ISFLEE);

    /* If monster was suffocating player, stop it */
    if (on(*tp, DIDSUFFOCATE)) extinguish_fuse(FUSE_SUFFOCATE);

    /* If something with fire, may darken */
    if (on(*tp, HASFIRE)) {
	struct room *rp=roomin(&tp->t_pos);

	if (rp && (--(rp->r_fires) <= 0)) {
	    rp->r_flags &= ~HASFIRE;
	    light(&tp->t_pos);
	}
    }
}

/* Sell displays a menu of goods from which the player may choose
 * to purchase something.
 */

void 
sell (struct thing *tp)
{
    struct linked_list *item;
    int i, j, min_worth, nitems, goods=0, chance, which_item, w;
    struct object *obj;
    char buffer[LINELEN];
    struct {
	int which;
	int plus1, plus2;
	int count;
	int worth;
	int flags;
	char *name;
    } selection[10];

    min_worth = -1; 		/* hope item is never worth less than this */
    item = find_mons(tp->t_pos.y, tp->t_pos.x); /* Get pointer to monster */

    /* Select the items */
    nitems = rnd(6) + 5;
    switch (rnd(6)) {
	/* Armor */
	case 0:
	case 1:
	    goods = ARMOR;
	    for (i=0; i<nitems; i++) {
		chance = rnd(100);
		for (j = 0; j < MAXARMORS; j++)
		    if (chance < armors[j].a_prob)
			break;
		if (j == MAXARMORS) {
		    debug("Picked a bad armor %d", chance);
		    j = 0;
		}
		selection[i].which = j;
		selection[i].count = 1;
		selection[i].flags = 0;
		if (rnd(100) < 40) selection[i].plus1 = rnd(5) + 1;
		else selection[i].plus1 = 0;
		selection[i].name = armors[j].a_name;
		switch(luck) {
		    case 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(5);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(5);
			}
	        }

		/* Calculate price */
		w = armors[j].a_worth;
		w *= (1 + luck + (10 * selection[i].plus1));
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Weapon */
	case 2:
	case 3:
	    goods = WEAPON;
	    for (i=0; i<nitems; i++) {
		selection[i].which = rnd(MAXWEAPONS);
		selection[i].count = 1;
		selection[i].flags = 0;
		if (rnd(100) < 35) {
		    selection[i].plus1 = rnd(3);
		    selection[i].plus2 = rnd(3);
		}
		else {
		    selection[i].plus1 = 0;
		    selection[i].plus2 = 0;
		}
		if (weaps[selection[i].which].w_flags & ISMANY)
		    selection[i].count = rnd(15) + 8;
		selection[i].name = weaps[selection[i].which].w_name;
		switch(luck) {
		    case 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -rnd(3);
			    selection[i].plus2 = -rnd(3);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -rnd(3);
			    selection[i].plus2 = -rnd(3);
			}
	        }
		w = weaps[selection[i].which].w_worth * selection[i].count;
		w *= (1 + luck + (10 * selection[i].plus1 + 
			10 * selection[i].plus2));
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Staff or wand */
	case 4:
	    goods = STICK;
	    for (i=0; i<nitems; i++) {
		selection[i].which = pick_one(ws_magic, MAXSTICKS);
		selection[i].plus1 = rnd(11) + 5;	/* Charges */
		selection[i].count = 1;
		selection[i].flags = 0;
		selection[i].name = ws_magic[selection[i].which].mi_name;
		switch(luck) {
		    case 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = 1;
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = 1;
			}
	        }
		w = ws_magic[selection[i].which].mi_worth;
		w += (luck + 1) * 20 * selection[i].plus1;
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Ring */
	case 5:
	    goods = RING;
	    for (i=0; i<nitems; i++) {
		selection[i].which = pick_one(r_magic, MAXRINGS);
		selection[i].plus1 = rnd(2) + 1;  /* Armor class */
		selection[i].count = 1;
		selection[i].flags = 0;
		if (rnd(100) < r_magic[selection[i].which].mi_bless + 10)
		    selection[i].plus1 += rnd(2) + 1;
		selection[i].name = r_magic[selection[i].which].mi_name;
		switch(luck) {
		    case 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(2);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(2);
			}
	        }
		w = r_magic[selection[i].which].mi_worth;
		switch (selection[i].which) {
		case R_DIGEST:
		    if (selection[i].plus1 > 2) selection[i].plus1 = 2;
		    else if (selection[i].plus1 < 1) selection[i].plus1 = 1;
		    __attribute__ ((fallthrough));
		/* fall thru here to other cases */
		case R_ADDSTR:
		case R_ADDDAM:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDINTEL:
		case R_ADDWISDOM:
		    if (selection[i].plus1 > 0)
			w += selection[i].plus1 * 50;
		}
		w *= (1 + luck);
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if(min_worth > selection[i].worth * selection[i].count)
		    min_worth = selection[i].worth;
	    }
    }

    /* See if player can afford an item */
    if (min_worth > purse) {
	msg("The %s eyes your small purse and departs.",
			monsters[nummonst].m_name);
	/* Get rid of the monster */
	killed(item, FALSE, FALSE);
	monst_dead = TRUE;
	return;
    }

    /* Display the goods */
    msg("The %s shows you %s wares.--More--", monsters[nummonst].m_name,
	(rnd(2)? "his": "her"));
    wait_for(0);
    msg("");
    clearok(cw, TRUE);
    touchwin(cw);

    wclear(hw);
    touchwin(hw);
    for (i=0; i < nitems; i++) {
	mvwaddch(hw, i+2, 0, '[');
	waddch(hw, (char) ((short) 'a' + i));
	waddstr(hw, "] ");
	switch (goods) {
	    case ARMOR:
		waddstr(hw, "Some ");
	    when WEAPON:
		if (selection[i].count == 1)
		    waddstr(hw, " A ");
		else {
		    sprintf(buffer, "%2d ", selection[i].count);
		    waddstr(hw, buffer);
		}
	    when STICK:
		waddstr(hw, "A ");
		waddstr(hw, ws_type[selection[i].which]);
		waddstr(hw, " of ");
	    when RING:
		waddstr(hw, "A ring of ");
	}
	waddstr(hw, selection[i].name);
	if (selection[i].count > 1)
	    waddstr(hw, "s");
	sprintf(buffer, "    Price:  %d", selection[i].worth);
	waddstr(hw, buffer);
    }
    sprintf(buffer, "Purse:  %d", purse);
    mvwaddstr(hw, nitems+3, 0, buffer);
    mvwaddstr(hw, 0, 0, "How about one of the following goods? ");
    draw(hw);
    /* Get rid of the monster */
    killed(item, FALSE, FALSE);
    monst_dead = TRUE;

    which_item = wgetch(hw) - 'a';
    while (which_item < 0 || which_item >= nitems) {
	if (which_item == ESCAPE - 'a') {
	    return;
	}
	mvwaddstr(hw, 0, 0, "Please enter one of the listed items. ");
	draw(hw);
	which_item = wgetch(hw) - 'a';
    }

    if (selection[which_item].worth > purse) {
	msg("You cannot afford it.");
	return;
    }

    purse -= selection[which_item].worth;

    item = spec_item(goods, selection[which_item].which,
		     selection[which_item].plus1, selection[which_item].plus2);

    obj = OBJPTR(item);
    if (selection[which_item].count > 1) {
	obj->o_count = selection[which_item].count;
	obj->o_group = newgrp();
    }

    if (selection[which_item].flags) {
	obj->o_flags |= ISCURSED;
	obj->o_flags &= ~ISBLESSED;
    }

    /* If a stick or ring, let player know the type */
    switch (goods) {
	case STICK: ws_know[selection[which_item].which] = TRUE;
	when RING:  r_know[selection[which_item].which] = TRUE;
    }

    if (add_pack(item, FALSE) == FALSE) {

	obj->o_pos = hero;
	fall(item, TRUE);
    }
}
