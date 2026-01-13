/*
 * This file contains functions for dealing with artifacts
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

#define SIZE(array)	(sizeof (array) / sizeof (*(array)))

int inbag;
char bag_letters[] = "zyxwvutsrqponmlkjihgfedcba";
char *bag_index	= bag_letters + SIZE(bag_letters) - 1;
char *bag_end	= bag_letters + SIZE(bag_letters) - 1;
struct object *get_artifact(int artifact);

/* 
 * apply an artifact
 */

void 
apply ()
{
	struct object *obj;
	int which;
	int chance;
	int i;
	int ch;

	if (has_artifact == 0) {
	    after = FALSE;
	    msg("You have nothing to apply");
	    return;
	}
	msg("apply what? (* for list):");
	ch = readchar();
	msg("");

	if (ch == '*') {
	    ch = 'a';
	    wclear(hw);
	    wprintw(hw, "apply what? ");
	    wmove(hw, 2, 0);
	    for (i=0; i<MAXARTIFACT; i++) {
		if (is_carrying(i)) {
		    wprintw(hw,"%c) %s\n\r",ch,arts[i].ar_name);
		}
		ch++;
	    }
	    draw(hw);
	    ch = wgetch(hw);
	    clearok(cw, TRUE);
	    touchwin(cw);
	}
	if (ch == ESCAPE) {
	    after = FALSE;
            msg("");            /* clear display */
            return;
        }
	i = ch - 'a';
	if (i<0 || i>=MAXARTIFACT || !is_carrying(i)) {
	    after = FALSE;
	    msg("You can't apply that!");
	    return;
	}

	obj = get_artifact(i);
	which = obj->o_which;
	if (!(obj->art_stats.ar_flags & ISACTIVE)) {
	    chance = rnd(100);
	    if (wizard) {
		msg("What roll? ");
		if(get_string(prbuf,cw) == NORM) {
		    chance = atoi(prbuf);
		    if(chance < 0 || chance > 99) {
			msg("Invalid selection.");
			chance = rnd(100);
		    }
		}
	    }
	    debug("Rolled %d.", chance);
	    if (chance > pstats.s_lvl*4) {
		if (rnd(7) == 0 && difficulty >= 2)
		    do_major();
		else
		    do_minor(obj);
	    } else {
		obj->art_stats.ar_flags |= ISACTIVE;
		active_artifact |= (1 << obj->o_which);
	    }
	}
	if (obj->art_stats.ar_flags & ISACTIVE) {
	    switch (which) {
		case TR_PURSE:
		    do_bag(obj);
		when TR_PHIAL:
		    do_phial();
		when TR_AMULET:
		    do_amulet();
		when TR_PALANTIR:
		    do_palantir();
		when TR_CROWN:
		    do_crown();
		when TR_SCEPTRE:
		    do_sceptre();
		when TR_SILMARIL:
		    do_silmaril();
		when TR_WAND:
		    do_wand();
		otherwise:
		    msg("Nothing happens.");
		    return;
	    }
	}
	if (rnd(100) < 1) {
	    obj->art_stats.ar_flags &= ~ISACTIVE;
	    active_artifact &= ~(1 << obj->o_which);
	    msg("You hear a click coming from %s.", inv_name(obj, TRUE));
	}
}

/* 
 * was the hero carrying a particular artifact
 */

bool 
possessed (int artifact)
{
	if ((picked_artifact >> artifact) & 1) 
	   return TRUE;
	else
	   return FALSE;
}

/* 
 * is the hero carrying a particular artifact
 */

bool 
is_carrying (int artifact)
{
	if ((has_artifact >> artifact) & 1) 
	   return TRUE;
	else
	   return FALSE;
}

/* 
 * has the hero activated a particular artifact
 */

bool 
is_active (int artifact)
{
	if ((active_artifact >> artifact) & 1) 
	   return TRUE;
	else
	   return FALSE;
}

/* 
 * is it time to make a new artifact?
 */
bool 
make_artifact ()
{
	int i;
	char had[LINELEN] = "";
	char has[LINELEN] = "";

	if (wizard) {
	    for (i=0; i<MAXARTIFACT; i++) {
		if (possessed(i))
		    strcat(had,"1");
		else
		    strcat(had,"0");
		if (is_carrying(i))
		    strcat(has,"1");
		else
		    strcat(has,"0");
	    }
	}

        mpos = 0;
	for(i = 0; i < MAXARTIFACT; i++) {
	   if ((!possessed(i) && arts[i].ar_level <= level)
	    || (!is_carrying(i) && level > 100 && level%5 == 0)) {
		debug("Artifact possession and picked flags : %s %s.", 
		    has, had);
		return TRUE;
	    }
	}
	return FALSE;
}

/* 
 * make a specified artifact
 */
struct object *
new_artifact (int which, struct object *cur)
{
	if (which >= MAXARTIFACT) {
	    debug("Bad artifact %d.  Random one created.", which);
	    which = rnd(MAXARTIFACT);
	}
	if (which < 0) {
	    for(which = 0; which < MAXARTIFACT; which++)
		if (!is_carrying(which) && arts[which].ar_level <= level)
		    break;
	}
	debug("Artifact number: %d.", which);
	cur->o_hplus = cur->o_dplus = 0;
	cur->o_damage = cur->o_hurldmg = "0d0";
	cur->o_ac = 11;
	cur->o_mark[0] = '\0';
	cur->o_type = ARTIFACT;
	cur->o_which = which;
	cur->o_weight = arts[which].ar_weight;
	cur->o_flags = 0;
	cur->o_group = 0;
	cur->o_count = 1;
	cur->art_stats.ar_flags = 0;
	cur->art_stats.ar_rings = 0;
	cur->art_stats.ar_potions = 0;
	cur->art_stats.ar_scrolls = 0;
	cur->art_stats.ar_wands = 0;
	cur->art_stats.t_art = NULL;
	return NULL;
}

/* 
 * do_minor: side effects and minor malevolent effects of artifacts
 */
void 
do_minor (struct object *tr)
{
    int which;

    which = rnd(115);
    if (wizard) {
	msg("Which minor effect? (%d)", which);
	if(get_string(prbuf,cw) == NORM) {
	    which = atoi(prbuf);
	    if(which < 0 || which > 104) {
		msg("Invalid selection.");
		which = rnd(105);
	    }
	}
    }
    debug("Rolled %d.", which);
    switch(which) {
	case 0:
		if (off(player, ISBLIND))
		    msg("You develop some acne on your face.");
	when 1:
		if (on(player, CANSCENT)) {
		    msg("A sudden whiff of BO causes you to faint.");
		    no_command = STONETIME;
		}
		else if (off(player, ISUNSMELL))
		    msg("You begin to smell funny.");
	when 2:
		if(off(player, ISBLIND))
		    msg("A wart grows on the end of your nose.");
	when 3:
		if (off(player, ISDEAF))
		    msg("Your hear strange noises in the distance.");
	when 4:
		if (off(player, ISDEAF))
		    msg("You hear shuffling in the distance.");
	when 5:
		if (off(player, ISDEAF))
		    msg("You hear clanking in the distance.");
	when 6:
		if (off(player, ISDEAF))
		    msg("You hear water dripping onto the floor.");
	when 7:
		if (off(player, ISDEAF))
		    msg("The dungeon goes strangely silent.");
	when 8:
		msg("You suddenly feel very warm.");
	when 9:
		msg("You feel very hot.");
	when 10:
		msg("A blast of heat hits you.");
	when 11: {
		struct room *rp;
		static coord fpos;

		if (off(player, ISBLIND))
		    msg("A pillar of flame leaps up beside you.");
		else
		    msg("You feel something very hot nearby.");
		rp = roomin(&hero);
		if (ntraps + 1 < MAXTRAPS + MAXTRAPS && 
				fallpos(&hero, &fpos, FALSE, FALSE)) {
			mvaddch(fpos.y, fpos.x, FIRETRAP);
			traps[ntraps].tr_type = FIRETRAP;
			traps[ntraps].tr_flags = ISFOUND;
			traps[ntraps].tr_show = FIRETRAP;
			traps[ntraps].tr_pos.y = fpos.y;
			traps[ntraps++].tr_pos.x = fpos.x;
			if (rp != NULL) {
			    rp->r_flags &= ~ISDARK;
			    light(&hero);
			    mvwaddch(cw, hero.y, hero.x, PLAYER) ;
			}
		}
	}
	when 12:
		msg("You feel a blast of hot air.");
	when 13:
		msg("You feel very cold.");
	when 14:
		msg("You break out in a cold sweat.");
	when 15:
		if (off(player, ISBLIND) && cur_armor == NULL)
		    msg("You are covered with frost.");
		else if(off(player, ISBLIND))
		    msg("Your armor is covered with frost.");
		else if (cur_armor == NULL)
		    msg("Your body feels very cold and you begin to shiver.");
		else 
		    msg("Your armor feels very cold.  You hear cracking ice.");
	when 16:
		msg("A cold wind whistles through the dungeon.");
	when 17: {
		int change;

		change =  18 - max_stats.s_str;
		chg_str(change, TRUE, FALSE);
		chg_dext(-change, TRUE, FALSE);
		if (change > 0)
		    msg("You feel stronger and clumsier now.");
		else if (change < 0)
		    msg("You feel weaker and more dextrous now.");
		else
		    msg("Nothing happens.");
	}
	when 18:
		msg("You begin to itch all over.");
	when 19:
		msg("You begin to feel hot and itchy.");
	when 20:
		msg("You feel a burning itch.");
		chg_dext(-1, FALSE, TRUE);
		if (off(player, HASITCH)) {
		    turn_on(player, HASITCH);
		    light_fuse(FUSE_UNITCH, 0, roll(4,6), AFTER);
		}
		else 
		    lengthen_fuse(FUSE_UNITCH, rnd(20)+HUHDURATION);
	when 21:
		if (off(player, ISBLIND))
		   msg("Your skin begins to flake and peel.");
		else
		   msg("You feel an urge to scratch an itch.");
	when 22:
		if (off(player, ISBLIND))
		    msg("Your hair begins to turn grey.");
	when 23:
		if (off(player, ISBLIND))
		    msg("Your hair begins to turn white.");
	when 24:
		if (off(player, ISBLIND))
		    msg("Some of your hair instantly turns white.");
	when 25:
		if (off(player, ISBLIND))
		    msg("You are covered with long white hair.");
	when 26:
		if (off(player, ISBLIND))
		    msg("You are covered with long red hair.");
	when 27:
		msg("You grow a beard.");
	when 28:
		msg("Your hair falls out.");
	when 29:
		msg("You feel a burning down below.");
	when 30:
		msg("Your toes fall off.");
	when 31:
		msg("You grow some extra toes.");
	when 32:
		msg("You grow some extra fingers.");
	when 33:
		msg("You grow an extra thumb.");
	when 34:
		msg("Your nose falls off.");
	when 35:
		msg("Your nose gets bigger.");
	when 36:
		msg("Your nose shrinks.");
	when 37:
		msg("An eye grows on your forehead.");
	when 38:
		if(off(player, ISBLIND))
		    msg("You see beady eyes watching from a distance.");
	when 39:
		msg("The dungeon rumbles for a moment.");
	when 40:
		if (off(player, ISBLIND))
		    msg("A flower grows on the floor next to you.");
	when 41:
		msg("You are stunned by a psionic blast.");
		if (on(player, ISHUH))
		    lengthen_fuse(FUSE_UNCONFUSE, rnd(40)+(HUHDURATION*3));
		else {
		    light_fuse(FUSE_UNCONFUSE, 0, rnd(40)+(HUHDURATION*3), AFTER);
		    turn_on(player, ISHUH);
		}
	when 42:
		msg("You are confused by thousands of voices in your head.");
		if (on(player, ISHUH))
		    lengthen_fuse(FUSE_UNCONFUSE, rnd(10)+(HUHDURATION*2));
		else {
		    light_fuse(FUSE_UNCONFUSE, 0, rnd(10)+(HUHDURATION*2), AFTER);
		    turn_on(player, ISHUH);
		}
	when 43:
		if (off(player, ISDEAF))
		    msg("You hear voices in the distance.");
	when 44:
		msg("You feel a strange pull.");
		teleport();
		if (off(player, ISCLEAR)) {
	            if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
	            else {
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
	        	turn_on(player, ISHUH);
		    }
		}
	when 45:
		msg("You feel less healthy now.");
		pstats.s_const = max(pstats.s_const - 1, 3);
		if (difficulty >=2)
		    max_stats.s_const = max(max_stats.s_const - 1, 3);
	when 46:
		msg("You feel weaker now.");
		if (difficulty >= 2)
		    chg_str(-1, TRUE, FALSE);
		else
		    chg_str(-1, FALSE, FALSE);
	when 47:
		msg("You feel less wise now.");
		pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
		if (difficulty >= 2)
		    max_stats.s_wisdom = max(max_stats.s_wisdom - 1, 3);
	when 48:
		msg("You feel less dextrous now.");
		if (difficulty >= 2)
		    chg_dext(-1, TRUE, FALSE);
		else
		    chg_dext(-1, FALSE, FALSE);
	when 49:
		msg("You feel less intelligent now.");
		pstats.s_intel = max(pstats.s_intel - 1, 3);
		if (difficulty >= 2)
		    max_stats.s_intel = max(max_stats.s_intel - 1, 3);
	when 50:
		msg("A trap door opens underneath your feet.");
		mpos = 0;
		level++;
		new_level(NORMLEV);
		if (rnd(4) < 2) {
		    addmsg("You are damaged by the fall");
		    if ((pstats.s_hpt -= roll(1,6)) <= 0) {
			addmsg("!  The fall killed you.");
			endmsg();
			death(D_FALL);
			return;
		    }
		}
		addmsg("!");
		endmsg();
		if (off(player, ISCLEAR) && rnd(4) < 3)
		{
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
		    else
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else msg("You feel dizzy for a moment, but it quickly passes.");
	when 51:
		msg("A maze entrance opens underneath your feet.");
		mpos = 0;
		level++;
		new_level(MAZELEV);
		if (rnd(4) < 2) {
		    addmsg("You are damaged by the fall");
		    if ((pstats.s_hpt -= roll(1,6)) <= 0) {
			addmsg("!  The fall killed you.");
			endmsg();
			death(D_FALL);
			return;
		    }
		}
		addmsg("!");
		endmsg();
		if (off(player, ISCLEAR) && rnd(4) < 3)
		{
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
		    else
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else msg("You feel dizzy for a moment, but it quickly passes.");
	when 52:
		if (off(player, ISDEAF))
		    msg("You hear a wailing sound in the distance.");
		aggravate();
	when 53:
		if (off(player, ISDEAF))
		    msg("You hear a high pitched whining sound.");
		aggravate();
	when 54:
		msg("You can't move.");
		no_command = 3 * HOLDTIME;
	when 55:
		if (off(player, ISDEAF))
		    msg("You hear a buzzing sound.");
		aggravate();
	when 56:
		msg("Your limbs stiffen.");
		no_command = 3 * STONETIME;
	when 57:
		msg("You feel a rock in your shoe hurting your foot.");
		turn_on(player, STUMBLER);
	when 58:
		msg("You get a hollow feeling in your stomach.");
		food_left -= 500;
	when 59:
		msg("Your purse feels lighter.");
		purse = max(purse - 50 - rnd(purse/2), 0);
	when 60:
		msg("A pixie appears and grabs gold from your purse.");
		purse = max(purse - 50 - rnd(50), 0);
	when 61:
		msg("You feel a tingling sensation all over.");
		pstats.s_hpt -= rnd(pstats.s_hpt/3);
	when 62:
		msg("You feel a pull downwards.");
	when 63:
		msg("You feel a strange pull downwards.");
	when 64:
		msg("You feel a peculiar pull downwards.");
	when 65:
		msg("You have a strange urge to go down.");
	when 66:
		msg("You feel a pull upwards.");
	when 67:
		msg("You feel a strange pull upwards.");
	when 68:
		msg("You have a strange feeling for a moment.");
	when 69:
		msg("You float in the air for a moment.");
	when 70:
		msg("You feel very heavy for a moment.");
	when 71:
		msg("You feel a strange sense of loss.");
	when 72:
		msg("You feel the earth spinning underneath your feet.");
	when 73:
		msg("You feel in touch with a Universal Oneness.");
	when 74:
		if (off(player, ISDEAF))
		    msg("You hear voices in the distance.");
	when 75:
		msg("A strange feeling of power comes over you.");
	when 76:
		msg("You feel a strange sense of unease.");
	when 77:
		msg("You feel Lady Luck is looking the other way.");
		luck++;
	when 78:
		msg("You feel your pack vibrate for a moment.");
	when 79:
		msg("You feel someone is watching you.");
	when 80:
		msg("You feel your hair standing on end.");
	when 81:
		msg("Wait!  The walls are moving!");
		new_level(NORMLEV);
	when 82:
		msg("Wait!  Walls are appearing out of nowhere!");
		new_level(MAZELEV);
	when 83:
		blue_light(FALSE, TRUE);
	when 84:
		msg("Your mind goes blank for a moment.");
		wclear(cw);
		light(&hero);
		status(TRUE);
	when 85:
		if (on(player, ISDEAF)) {
		    msg("You feel your ears burn for a moment.");
		    lengthen_fuse(FUSE_HEAR, 2*PHASEDURATION);
		}
		else {
		    msg("You are suddenly surrounded by silence.");
		    turn_on(player, ISDEAF);
		    light_fuse(FUSE_HEAR, 0, 2*PHASEDURATION, AFTER);
		}
	when 86: {
		struct linked_list *item;
		struct object *obj;

		for (item = pack; item != NULL; item = next(item)) {
		    obj = OBJPTR(item);
		    if (obj->o_type != ARTIFACT && rnd(8) == 0) {
			obj->o_flags |= ISCURSED;
			obj->o_flags &= ~ISBLESSED;
		    }
		}
		if (off(player, ISUNSMELL))
		    msg("You smell a faint trace of burning sulfur.");
	}
	when 87:
		msg("You have contracted a parasitic infestation.");
		infest_dam++;
		turn_on(player, HASINFEST);
	when 88: {
		static coord fear;

		msg("You suddenly feel a chill run up and down your spine.");
		turn_on(player, ISFLEE);
		fallpos(&hero, &fear, FALSE, FALSE);
		player.t_dest = &fear;
	}
	when 89:
		if (cur_weapon != NULL) 
		    msg("You feel your %s get very hot.", 
			inv_name(cur_weapon, TRUE));
	when 90:
		if (cur_weapon != NULL) 
		    msg("Your %s glows white for an instant.",
			inv_name(cur_weapon, TRUE));
	when 91:
		if (cur_armor != NULL) 
		    msg("Your %s gets very hot.", inv_name(cur_armor, TRUE));
	when 92:
		if (cur_weapon != NULL) 
		    msg("Your %s suddenly feels very cold.",
			inv_name(cur_weapon, TRUE));
	when 93:
		if (cur_armor != NULL)
		    msg("Your armor is covered by an oily film.");
	when 94:
		read_scroll(S_CREATE, FALSE);
	when 95:
		lower_level(D_POTION);
	when 96: {
		int x, y;

		for (x = -1; x <= 1; x++) {
		    for (y = -1; y <= 1; y++) {
			if (x == 0 && y == 0)
			    continue;
			delta.x = x;
			delta.y = y;
			do_zap(TRUE, WS_POLYMORPH, FALSE);
		    }
		}
	}
	when 97: {
		int x, y;

		for (x = -1; x <= 1; x++) {
		    for (y = -1; y <= 1; y++) {
			if (x == 0 && y == 0)
			    continue;
			delta.x = x;
			delta.y = y;
			do_zap(TRUE, WS_INVIS, FALSE);
		    }
		}
	}
	when 98:
		msg("You feel warm all over.");
		turn_on(player, POWEREAT);
	otherwise:
		tr->art_stats.ar_flags &= ~ISACTIVE;
		msg("You hear a click coming from %s.", inv_name(tr, TRUE));
    }
}


/* 
 * do_major: major malevolent effects
 */
void 
do_major ()
{
    int which;

    which = rnd(12);
    if (wizard) {
	msg("Which major effect? (%d)", which);
	if(get_string(prbuf,cw) == NORM) {
	    which = atoi(prbuf);
	    if(which < 0 || which > 11) {
		msg("Invalid selection.");
		which = rnd(12);
	    }
	}
    }
    debug("Rolled %d.", which);
    switch (which) {
	case 0:
		level += 5 + rnd(10*difficulty);
		new_level(NORMLEV);
		mpos = 0;
		msg("You are banished to the lower regions.");
	when 1:
		if (on(player, ISBLIND)) {
		    msg("The cloak of darkness deepens.");
		    extinguish_fuse(FUSE_SIGHT);
		    light_fuse(FUSE_SIGHT, 0, 4*SEEDURATION, AFTER);
		}
		else {
		    msg("A cloak of darkness falls around you.");
		    light_fuse(FUSE_SIGHT, 0, 2*SEEDURATION, AFTER);
		}
		turn_on(player, ISBLIND);
		look(FALSE);
	when 2:
		new_level(THRONE);
	when 3:
		msg("You feel very warm all over.");
		turn_on(player, SUPEREAT);
	when 4:
		msg("You feel yourself moving %sslower.",
		    on(player, ISSLOW) ? "even " : "");
		if (on(player, ISSLOW))
		    lengthen_fuse(FUSE_NOSLOW, 20 + rnd(20));
		else {
		    turn_on(player, ISSLOW);
		    player.t_turn = TRUE;
		    light_fuse(FUSE_NOSLOW, 0, 20 + rnd(20), AFTER);
		}
	when 5: {
		int num, i;

		num = roll(1,4);

		for (i = 1; i < num; i++)
		    lower_level(D_POTION);
	}
	when 6:
		if (rnd(2))
		    add_intelligence(TRUE);
		if (rnd(2))
		    chg_dext(-1, TRUE, FALSE);
		if (rnd(2))
		    chg_str(-1, TRUE, FALSE);
		if (rnd(2))
		    add_wisdom(TRUE);
		if (rnd(2))
		    add_const(TRUE);
	when 7: {
		static coord fires;
		struct room *rp;

		if (ntraps + 1 >= MAXTRAPS) {
		    msg("You feel a puff of hot air.");
		    return;
		}
		for (; ntraps < MAXTRAPS + MAXTRAPS; ntraps++) {
		    if (!fallpos(&hero, &fires, FALSE, FALSE))
		        break;
		    mvaddch(fires.y, fires.x, FIRETRAP);
		    traps[ntraps].tr_type = FIRETRAP;
		    traps[ntraps].tr_flags |= ISFOUND;
		    traps[ntraps].tr_show = FIRETRAP;
		    traps[ntraps].tr_pos.x = fires.x;
		    traps[ntraps].tr_pos.y = fires.y;
		    if ((rp = roomin(&hero)) != NULL)
			rp->r_flags &= ~ISDARK;
		}
	}
	when 8: {
		struct linked_list *item;
		struct object *obj = NULL;

		if (cur_weapon == NULL) {
		    msg("You feel your hands tingle a moment.");
		    return;
		}
		for (item = pack; item != NULL; item = next(item))
		    if ((obj = OBJPTR(item)) == cur_weapon)
			break;
		if (obj) {
		    if (obj->o_flags & ISMETAL)
			msg("Your %s melts and disappears.", inv_name(obj, TRUE));
		    else
			msg("Your %s crumbles in your hands.",
				    inv_name(obj, TRUE));
		    obj->o_flags &= ~ISCURSED;
		    dropcheck(obj);
		    detach(pack, item);
		    freeletter(item);
		    inpack--;
		    discard(item);
		}
	}
	when 9: {
		struct linked_list *item;
		struct object *obj = NULL;

		if (cur_armor == NULL) {
		    msg("Your body tingles a moment.");
		    return;
		}
		for (item = pack; item != NULL; item = next(item))
		    if ((obj = OBJPTR(item)) == cur_armor)
			break;
		if (obj) {
		    msg("Your %s crumbles into small black powdery dust.",
				inv_name(obj, TRUE));
		    obj->o_flags &= ~ISCURSED;
		    dropcheck(obj);
		    detach(pack, item);
		    freeletter(item);
		    inpack--;
		    discard(item);
		}
	}
	when 10:
		if (cur_weapon == NULL) {
		    msg("Your hand glows yellow for an instant.");
		    return;
		}
		msg("Your %s glows bright red for a moment.", 
		    weaps[cur_weapon->o_which].w_name);
		if (cur_weapon->o_hplus > 0)
		    cur_weapon->o_hplus = -rnd(3);
		else
		    cur_weapon->o_hplus -= rnd(3);
		if (cur_weapon->o_dplus > 0)
		    cur_weapon->o_dplus = -rnd(3);
		else
		    cur_weapon->o_dplus -= rnd(3);
		cur_weapon->o_flags = ISCURSED|ISLOST;
		cur_weapon->o_ac = 0;
	otherwise:
		msg("You feel warm all over.");
		turn_on(player, POWEREAT);
    }
}

/*
 * do_phial: handle powers of the Phial of Galadriel
 */
void 
do_phial ()
{
    int which;

    /* Prompt for action */
    msg("How do you wish to apply the Phial of Galadriel? (* for list): ");

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > 1) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	light");
	mvwaddstr(hw, 3, 0, "[b]	monster confusion");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > 1) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	case 0: read_scroll(S_LIGHT, TRUE);
	when 1:
		if (get_dir()) 
		    do_zap(TRUE, WS_CONFMON, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_palantir: handle powers of the Palantir of Might
 */
void 
do_palantir ()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Palantir of Might? (* for list): ");

    limit = 3;
    if (is_carrying(TR_SCEPTRE))
	limit += 1;
    if (is_carrying(TR_CROWN))
	limit += 1;

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > limit) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	monster detection");
	mvwaddstr(hw, 3, 0, "[b]	gold detection");
	mvwaddstr(hw, 4, 0, "[c]	magic detection");
	mvwaddstr(hw, 5, 0, "[d]	food detection");
	if (limit >= 4)
	    mvwaddstr(hw, 6, 0, "[e]	teleportation");
	if (limit >= 5)
	    mvwaddstr(hw, 7, 0, "[f]	clear thought");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > limit) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	case 0: quaff(P_MFIND, FALSE);
	when 1: read_scroll(S_GFIND, FALSE);
	when 2: quaff(P_TFIND, FALSE);
	when 3: read_scroll(S_FOODFIND, FALSE);
	when 4: read_scroll(S_TELEP, FALSE);
	when 5: quaff(P_CLEAR, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_silmaril: handle powers of the Silmaril of Ea
 */
void 
do_silmaril ()
{
    int which;

    /* Prompt for action */
    msg("How do you wish to apply the Silmaril of Ea? (* for list): ");

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > 2) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	magic mapping");
	mvwaddstr(hw, 3, 0, "[b]	petrification");
	mvwaddstr(hw, 4, 0, "[c]	stairwell downwards");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > 2) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	case 0: read_scroll(S_MAP, FALSE);
	when 1: read_scroll(S_PETRIFY, FALSE);
	when 2:
		msg("A stairwell opens beneath your feet and you go down.");
		level++;
		new_level(NORMLEV);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_amulet: handle powers of the Amulet of Yendor
 */
void 
do_amulet ()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Amulet of Yendor? (* for list): ");

    limit = 0;
    if (is_carrying(TR_PURSE))
	limit += 1;
    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > limit) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	level evaluation");
	if (limit >= 1)
	    mvwaddstr(hw, 3, 0, "[b]	invisibility");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > limit) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	case 0: {
		int count = 0;
		int max_nasty = 0;
		struct linked_list *item;
		struct thing *tp;
		char *colour, *temp;

		for (item = mlist; item != NULL; item = next(item)) {
		    tp = (struct thing *) ldata(item);
		    count++;
		    max_nasty = max(max_nasty, 
			(10 - tp->t_stats.s_arm) * tp->t_stats.s_hpt);
		}

		if (count < 3) 
		    colour = "black";
		else if (count < 6)
		    colour = "red";
		else if (count < 9)
		    colour = "orange";
		else if (count < 12)
		    colour = "yellow";
		else if (count < 15)
		    colour = "green";
		else if (count < 18)
		    colour = "blue";
		else if (count < 25)
		    colour = "violet";
		else
		    colour = "pink with purple polka dots";

		if (max_nasty < 10)
		    temp = "feels cold in your hand";
		else if (max_nasty < 30)
		    temp = "feels cool";
		else if (max_nasty < 200)
		    temp = "feels warm and soft";
		else if (max_nasty < 1000)
		    temp = "feels warm and slippery";
	        else if (max_nasty < 5000)
		    temp = "feels hot and dry";
		else if (max_nasty < 10000)
		    temp = "feels too hot to hold";
		else if (max_nasty < 20000) {
		    temp = "burns your hand badly";
		    if ((pstats.s_hpt -= roll(1,6)) <= 0) {
			msg("You die from the burning oil that spews forth.");
			death(D_FIRE);
			return;
		    }
		}
		else
		    temp = "tingles your hand";

		msg("The amulet glows %s and %s.", colour, temp);
	}
	when 1: quaff(P_INVIS, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_bag: handle powers of the Magic Purse of Yendor
 * as a bag of holding
 */
void 
do_bag (struct object *obj)
{
    int ch, which, limit;

    limit = 2;
    if (is_carrying(TR_AMULET))
	limit += 1;

    /* Prompt for action */
    msg("How do you wish to apply the Magic Purse of Yendor? (* for list): ");
    ch = readchar();
    msg("");

    if (ch == '*') {
	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	inventory");
	mvwaddstr(hw, 3, 0, "[b]	add to bag");
	mvwaddstr(hw, 4, 0, "[c]	remove from bag");
	if (limit >= 3)
	    mvwaddstr(hw, 5, 0, "[d]	see invisible");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	ch = wgetch(hw);
	clearok(cw, TRUE);
	touchwin(cw);
    }
    if (ch == ESCAPE) {
	after = FALSE;
	return;
    }

    which = ch - 'a';
    if (which < 0 || which > limit) {
	msg("You can't do that!");
	return;
    }

    msg("Your attempt is successful.");
    switch (which) {
	case 0:
		bag_inventory(obj->art_stats.t_art);
	when 1:
		add_bag(&obj->art_stats.t_art);
	when 2: {
		struct linked_list *item;
		struct object *op;

		if ((item = get_bag(&obj->art_stats.t_art)) != NULL) {
		    detach(obj->art_stats.t_art,item);
		    delbagletter(item);
		    inbag--;
		    if (add_pack(item, FALSE) == FALSE) {
			op = (struct object *) ldata(item);
			op->o_pos = hero;
			fall(item, TRUE);
		    }
		}
	}
	when 3: 
		quaff(P_SEEINVIS, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_sceptre: handle powers of the Sceptre of Might
 */
void 
do_sceptre ()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Sceptre of Might? (* for list): ");

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }

    limit = 5;
    if (is_carrying(TR_CROWN))
	limit += 1;
    if (is_carrying(TR_PALANTIR))
	limit += 1;

    if (which < 0 || which > limit) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	cancellation");
	mvwaddstr(hw, 3, 0, "[b]	polymorph monster");
	mvwaddstr(hw, 4, 0, "[c]	slow monster");
	mvwaddstr(hw, 5, 0, "[d]	teleport monster");
	mvwaddstr(hw, 6, 0, "[e]	monster confusion");
	mvwaddstr(hw, 7, 0, "[f]	paralyze monster");
	if (limit >= 6)
	    mvwaddstr(hw, 8, 0, "[g]	drain life");
	if (limit >= 7)
	    mvwaddstr(hw, 9, 0, "[h]	smell monster");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > limit) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 7) {
	msg("Your finger slips.");
	which = rnd(6);
	if (wizard) {
	    msg("What wand? (%d)", which);
	    if(get_string(prbuf,cw) == NORM) {
		which = atoi(prbuf);
		if(which < 0 || which > 5) {
		msg("Invalid selection.");
		which = rnd(6);
		msg("Rolled %d.", which);
		}
	    }
	}
    }

    switch (which) {
	case 0: if (get_dir()) 
		    do_zap(TRUE, WS_CANCEL, TRUE);
	when 1: if (get_dir()) 
		    do_zap(TRUE, WS_POLYMORPH, FALSE);
	when 2: if (get_dir()) 
		    do_zap(TRUE, WS_SLOW_M, TRUE);
	when 3: if (get_dir()) 
		    do_zap(TRUE, WS_TELMON, TRUE);
	when 4: if (get_dir()) 
		    do_zap(TRUE, WS_CONFMON, FALSE);
	when 5: if (get_dir()) 
		    do_zap(TRUE, WS_PARALYZE, FALSE);
	when 6: if (get_dir()) 
		    do_zap(TRUE, WS_DRAIN, FALSE);
	when 7: quaff(P_SMELL, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * do_wand: handle powers of the Wand of Orcus
 */
void 
do_wand ()
{
    int which, i;

    /* Prompt for action */
    msg("How do you wish to apply the Wand of Orcus? (* for list): ");

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }

    if (which < 0 || which >= MAXSTICKS) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	for (i = 0; i < MAXSTICKS; i++) {
	    sprintf(prbuf, "[%c]	%s", i + 'a', ws_magic[i].mi_name);
	    mvwprintw(hw, i+2, 0, "%s", prbuf);
	}
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which >= MAXSTICKS) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 12) {
	msg("Your finger slips.");
	which = rnd(MAXSTICKS);
	if (wizard) {
	    msg("What wand? (%d)", which);
	    if(get_string(prbuf,cw) == NORM) {
		which = atoi(prbuf);
		if(which < 0 || which >= MAXSTICKS) {
		    msg("Invalid selection.");
		    which = rnd(MAXSTICKS);
		    msg("Rolled %d.", which);
		}
	    }
	}
    }

    if (get_dir()) 
	do_zap(TRUE, which, FALSE);
}

/*
 * do_crown: handle powers of the Crown of Might
 */
void 
do_crown ()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Crown of Might? (* for list): ");

    which = readchar() - 'a';
    if (which == ESCAPE - 'a') {
	after = FALSE;
	return;
    }

    limit = 9;
    if (is_carrying(TR_PALANTIR))
	limit += 1;
    if (is_carrying(TR_SCEPTRE))
	limit += 1;

    if (which < 0 || which > limit) {
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0, "[a]	add strength");
	mvwaddstr(hw, 3, 0, "[b]	add intelligence");
	mvwaddstr(hw, 4, 0, "[c]	add wisdom");
	mvwaddstr(hw, 5, 0, "[d]	add dexterity");
	mvwaddstr(hw, 6, 0, "[e]	add constitution");
	mvwaddstr(hw, 7, 0, "[f]	normal strength");
	mvwaddstr(hw, 8, 0, "[g]	normal intelligence");
	mvwaddstr(hw, 9, 0, "[h]	normal wisdom");
	mvwaddstr(hw, 10, 0, "[i]	normal dexterity");
	mvwaddstr(hw, 11, 0, "[j]	normal constitution");
	if (limit >= 10)
	    mvwaddstr(hw, 12, 0, "[k]	disguise");
	if (limit >= 11)
	    mvwaddstr(hw, 13, 0, "[l]	super heroism");
	wmove(hw, 0, 0);
	wprintw(hw, "Which power do you wish to use?");
	draw(hw);
	which = wgetch(hw) - 'a';
	while (which < 0 || which > limit) {
	    if (which == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed powers. ");
	    draw(hw);
	    which = wgetch(hw) - 'a';
	}
	mvwaddstr(hw, 0, 0, "Your attempt is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	(void) wgetch(hw);
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	case 0: 
		if (off(player, POWERSTR)) {
		    turn_on(player, POWERSTR);
		    chg_str(10, FALSE, FALSE);
		    msg("You feel much stronger now.");
		}
		else
		    msg("Nothing happens.");
	when 1: 
		if (off(player, POWERINTEL)) {
		    pstats.s_intel += 10;
		    turn_on(player, POWERINTEL);
		    msg("You feel much more intelligent now.");
		}
		else
		    msg("Nothing happens.");
	when 2: 
		if (off(player, POWERWISDOM)) {
		    pstats.s_wisdom += 10;
		    turn_on(player, POWERWISDOM);
		    msg("Your feel much wiser know.");
		}
		else
		    msg("Nothing happens.");
	when 3: 
		if (off(player, POWERDEXT)) {
		    turn_on(player, POWERDEXT);
		    chg_dext(10, FALSE, FALSE);
		    msg("You feel much more dextrous now.");
		}
		else
		    msg("Nothing happens.");
	when 4: 
		if (off(player, POWERCONST)) {
		    pstats.s_const += 10;
		    turn_on(player, POWERCONST);
		    msg("You feel much healthier now.");
		}
		else
		    msg("Nothing happens.");
	when 5: 
		if (on(player, POWERSTR)) {
		    turn_off(player, POWERSTR);
		    chg_str(-10, FALSE, FALSE);
		    msg("Your muscles bulge less now.");
		}
		else
		    msg("Nothing happens.");
	when 6: 
		if (on(player, POWERINTEL)) {
		    pstats.s_intel = max(pstats.s_intel - 10, 
				3 + ring_value(R_ADDINTEL));
		    turn_off(player, POWERINTEL);
		    msg("You feel less intelligent now.");
		}
		else
		    msg("Nothing happens.");
	when 7: 
		if (on(player, POWERWISDOM)) {
		    pstats.s_wisdom = max(pstats.s_wisdom - 10,
				3 + ring_value(R_ADDWISDOM));
		    turn_off(player, POWERWISDOM);
		    msg("You feel less wise now.");
		}
		else
		    msg("Nothing happens.");
	when 8: 
		if (on(player, POWERDEXT)) {
		    turn_off(player, POWERDEXT);
		    chg_dext(-10, FALSE, FALSE);
		    msg("You feel less dextrous now.");
		}
		else
		    msg("Nothing happens.");
	when 9: 
		if (on(player, POWERCONST)) {
		    pstats.s_const -= 10;
		    turn_off(player, POWERCONST);
		    msg("You feel less healthy now.");
		}
		else
		    msg("Nothing happens.");
	when 10:
		quaff(P_DISGUISE, FALSE);
	when 11:
		quaff(P_SHERO, FALSE);
	otherwise:
		msg("What a strange thing to do!!");
    }
}

/*
 * add_bag:
 *	move an object from the bag to the bag of holding
 */
void 
add_bag (struct linked_list **bag)
{
    struct linked_list *ip, *lp=NULL;
    struct object *obj, *op=NULL;
    struct linked_list *item;
    bool exact;

    if ((item = get_item("add", 0)) == NULL) 
	return;

    obj = (struct object *) ldata(item);
    if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE) {
	msg("Even wizards can't do that!");
	return;
    }

    if (!dropcheck(obj))
	return;

    if (obj->o_group) {
	for (ip = *bag; ip != NULL; ip = next(ip)) {
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group) {
		op->o_count++;
		discard(item);
		item = ip;
		goto added;
	    }
	}
    }

    exact = FALSE;
    for (ip = *bag; ip != NULL; ip = next(ip)) {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }

    if (ip == NULL) {
	for (ip = *bag; ip != NULL; ip = next(ip)) {
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else {
	while (ip != NULL && op->o_type == obj->o_type) {
	    if (op->o_which == obj->o_which) {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }

    if ((ip == NULL || !exact || !ISMULT(obj->o_type))
	&& inbag >= maxpack - 1) {
	    msg("Your bag has no more room.");
	    return;
    }

    detach(pack, item);
    inpack--;
    freeletter(item);
    updpack(FALSE);
    inbag++;
    bagletter(item);

    if (ip == NULL) {
	if (*bag == NULL)
	    *bag = item;
	else {
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else {
	if (exact && ISMULT(obj->o_type)) {
	    op->o_count += obj->o_count;
	    inbag--;			/* adjust for previous addition */
	    discard(item);
	    delbagletter(item);
	    item = ip;
	    goto added;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    *bag = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
added:
    obj = (struct object *) ldata(item);
    msg("Added %s.", inv_name(obj, 1));
    return;
}

/*
 * get_bag:
 *	pick something out of bag
 */
struct linked_list *
get_bag (struct linked_list **bag)
{
    struct linked_list *obj, *pit;
    struct object *pob;
    char ch, och, anr;
    int cnt;

    if (*bag == NULL) {
	msg("You don't have anything in your bag.");
	return NULL;
    }
    for(;;) {
	msg("remove what? (* for list): ");
	ch = readchar();
	mpos = 0;
	if (ch == ESCAPE) {		/* abort if escape hit */
	    after = FALSE;
	    msg("");		/* clear display */
	    return NULL;
	}
	if (ch == '*') {
	    wclear(hw);
	    pit = *bag;		/* point to bag */
	    cnt = 0;
	    for (ch = 'a'; pit != NULL ; pit = next(pit), ch++) {
		pob = OBJPTR(pit);
		wprintw(hw,"%c) %s\n\r",pit->l_letter,inv_name(pob,FALSE));
		if (++cnt >= LINES - 2 && next(pit) != NULL) {
		    cnt = 0;
		    dbotline(hw, spacemsg);
		    wclear(hw);
		}
	    }
	    wmove(hw, LINES - 1,0);
	    wprintw(hw,"remove what? ");
	    draw(hw);		/* write screen */
	    anr = FALSE;
	    do {
		ch = readchar();
		if (isupper(ch))
		    ch = tolower(ch);
		if (isalpha(ch) || ch == ESCAPE)
		    anr = TRUE; 
	    } while(!anr);		/* do till we got it right */
	    restscr(cw);		/* redraw orig screen */
	    if(ch == ESCAPE) {
		after = FALSE;
		msg("");		/* clear top line */
		return NULL;	/* all done if abort */
	    }
	}
	for(obj = *bag,och = 'a'; obj != NULL;obj = next(obj),och++)
	    if (ch == obj->l_letter)
		break;
	if (obj == NULL) {
	    msg("Not in bag.");
	    continue;
	}
	else
	    return (obj);
    }
}

/*
 * bag_inventory:
 *	list what is in the bag
 */
void 
bag_inventory (struct linked_list *list)
{
    struct object *obj;
    char ch;
    int n_objs;
    char inv_temp[LINELEN];

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list)) {
	obj = (struct object *) ldata(list);
	switch (n_objs++) {
	    case 0:
		sprintf(inv_temp, "%c) %s", list->l_letter, 
			inv_name(obj, FALSE));
		break;
	    case 1:
		wclear(hw);
		waddstr(hw, inv_temp);
		waddch(hw, '\n');
		wprintw(hw, "%c) %s\n", list->l_letter, inv_name(obj, FALSE));
		break;
	    default:
		wprintw(hw, "%c) %s\n", list->l_letter, inv_name(obj, FALSE));
	}
    }
    if (n_objs == 0) {
	msg("Your bag is empty." );
	return;
    }
    if (n_objs == 1) {
	msg(inv_temp);
	return;
    }
    mvwaddstr(hw, LINES-1, 0, spacemsg);
    draw(hw);
    (void) wgetch(hw);
    clearok(cw, TRUE);
    touchwin(cw);
}

int 
bag_char (struct object *obj, struct linked_list *bag)
{
    struct linked_list *item;
    char c;

    c = 'a';
    for (item = bag; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return item->l_letter;
	else
	    c++;
    msg("Help! Item not in bag!");
    return '?';
}

void 
bagletter (struct linked_list *item)
{
	if (item != NULL) {
		if (bag_index > bag_letters && islower(bag_index[-1]))
			item->l_letter = *--bag_index;
		else
			item->l_letter = '?';
	}
}

void 
delbagletter (struct linked_list *item)
{
	if (item != NULL && islower(item->l_letter))
		if (bag_index < bag_end)
			*bag_index++ = item->l_letter;
}


/*
 * get_artifact:
 *      find an artifact in pack or bag
 */
struct object *
get_artifact (int artifact)
{
    struct linked_list *bag = NULL;
    struct linked_list *item;
    struct object *obj;

    /* first check the player's pack */
    for (item = pack; item != NULL; item = next(item)) {
	obj = OBJPTR(item);
	if (obj->o_type == ARTIFACT && obj->o_which == artifact) {
	    return(obj);
	} else if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE) {
	    bag = obj->art_stats.t_art;
	}
    }

    /* next check the magic purse */
    if (bag != NULL) {
	for (item = bag; item != NULL; item = next(item)) {
	    obj = OBJPTR(item);
	    if (obj->o_type == ARTIFACT && obj->o_which == artifact) {
		return(obj);
	    }
	}
    }
    return(NULL);
}
