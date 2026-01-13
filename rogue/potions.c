/*
 * Function(s) for dealing with potions
 */

#include "curses.h"
#include <string.h>
#include "rogue.h"

void 
quaff (int which, bool blessed)
{
    struct object *obj;
    struct linked_list *item=NULL, *titem;
    struct thing *th;
    bool cursed, is_potion;
    char buf[LINELEN];

    cursed = FALSE;
    is_potion = FALSE;

    if (which < 0) {	/* A regular potion */
	is_potion = TRUE;
	item = get_item("quaff", POTION);
	/*
	 * Make certain that it is somethings that we want to drink
	 */
	if (item == NULL)
	    return;

	obj = (struct object *) ldata(item);
	if (obj->o_type != POTION) {
	    msg("You can't drink that!");
	    return;
	}
	/* remove it from the pack */
	inpack--;
	detach(pack, item);
	freeletter(item);

	/*
	 * Calculate the effect it has on the poor player.
	 */
	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;

	which = obj->o_which;
    }

    switch(which) {
	case P_CLEAR:
	    if (cursed) {
		if (off(player, ISCLEAR)) {
		    msg("Wait, what's going on here. Huh? What? Who?");
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
		    else
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else msg("You feel dizzy for a moment, but it passes.");
	    }
	    else {
		if (blessed) {	/* Make player immune for the whole game */
		    extinguish_fuse(FUSE_UNCLRHEAD);  /* If we have a fuse, put it out */
		    if (off(player, ISCLEAR))
			msg("A strong blue aura surrounds your head.");
		    else
			msg("Your blue aura brightens for a moment.");
		}
		else {	/* Just light a fuse for how long player is safe */
		    if (off(player, ISCLEAR)) {
			light_fuse(FUSE_UNCLRHEAD, 0, CLRDURATION, AFTER);
			msg("A faint blue aura surrounds your head.");
		    }
		    else {  /* If we have a fuse lengthen it, else we
			     * are permanently clear.
			     */
		        if (find_slot(FUSE, FUSE_UNCLRHEAD) == NULL)
			    msg("Your blue aura continues to glow strongly.");
			else {
			    lengthen_fuse(FUSE_UNCLRHEAD, CLRDURATION);
			    msg("Your blue aura brightens for a moment.");
			}
		    }
		}
		turn_on(player, ISCLEAR);
		/* If player is confused, unconfuse them */
		if (on(player, ISHUH)) {
		    extinguish_fuse(FUSE_UNCONFUSE);
		    unconfuse(NULL);
		}
	    }
	when P_HEALING:
	    if (cursed) {
		if (!save(VS_POISON)) {
		    msg("You feel very sick now.");
		    pstats.s_hpt /= 2;
		    if ((pstats.s_hpt -= 1) <= 0) {
			death(D_POISON);
			return;
		    }
		}
		else msg("You feel momentarily sick.");
	    }
	    else {
		if (blessed) {
		    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > 
				max_stats.s_hpt)
			pstats.s_hpt = max_stats.s_hpt += roll(2,4);
		    if (on(player, ISHUH)) {
			extinguish_fuse(FUSE_UNCONFUSE);
			unconfuse(NULL);
		    }
		}
		else {
		    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > 
				max_stats.s_hpt)
		    pstats.s_hpt = max_stats.s_hpt += roll(1,4);
		}
		msg("You begin to feel %sbetter.",
			blessed ? "much " : "");
		if (off(player, PERMBLIND))
		    sight(NULL);
		if (is_potion) 
		    p_know[P_HEALING] = TRUE;
	    }
	when P_ABIL: {
		short ctype;

		/*
		 * if blessed then fix all attributes
		 */
		if (blessed) {
		    add_intelligence(FALSE);
		    add_dexterity(FALSE);
		    add_strength(FALSE);
		    add_wisdom(FALSE);
		    add_const(FALSE);
		    if (rnd(5) > 2)
			pstats.s_arm--;
		}
		/* probably will be own ability */
		else {
		    if (rnd(100) < 70) 
			ctype = player.t_ctype;
		    else do {
			ctype = rnd(4);
			} while (ctype == player.t_ctype);

		    /* Small chance of doing constitution instead */
		    if (rnd(100) < 10) 
			add_const(cursed);
		    else switch (ctype) {
			case C_FIGHTER: add_strength(cursed);
			when C_MAGICIAN: add_intelligence(cursed);
			when C_CLERIC:	add_wisdom(cursed);
			when C_THIEF:	add_dexterity(cursed);
			otherwise:	msg("You're a strange type!");
		    }
		}
		if (is_potion) p_know[P_ABIL] = TRUE;
	    }
	when P_MFIND:
	    /*
	     * Potion of monster detection, if there are monters, detect them
	     */
	    if(cursed) {
		int num = roll(3,6);
		int i;
		char ch;
		struct room *rp;
		coord pos;

		msg("You begin to sense the presence of monsters.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    if (rnd(2))
			ch = 'a' + rnd(26);
		    else
			ch = 'A' + rnd(26);
		    mvwaddch(hw, pos.y, pos.x, ch);
		}
		waddstr(cw, morestr);
		overlay(hw, cw);
		draw(cw);
		wait_for(0);
		msg("");
		if (is_potion) 
		    p_know[P_MFIND] = TRUE;
		break;
	    }
	    if (mlist != NULL) {
		msg("You begin to sense the presence of monsters.");
		waddstr(cw, morestr);
		overlay(mw, cw);
		draw(cw);
		wait_for(0);
		msg("");
		if (is_potion) 
		    p_know[P_MFIND] = TRUE;
		if (blessed)
		    turn_on(player, BLESSMONS);
	    }
	    else
		msg("You have a strange feeling, then it passes.");
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (cursed) {
		int num = roll(3,3);
		int i;
		char ch;
		struct room *rp;
		coord pos;

		msg("You sense the presence of magic on this level.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    if (rnd(9) == 0)
			ch = BMAGIC;
		    else if (rnd(9) == 0)
			ch = CMAGIC;
		    else
			ch = MAGIC;
		    mvwaddch(hw, pos.y, pos.x, ch);
		}
		waddstr(cw, morestr);
		overlay(hw, cw);
		draw(cw);
		wait_for(0);
		msg("");
		if (is_potion) 
		    p_know[P_TFIND] = TRUE;
		break;
	    }
	    if (blessed)
		turn_on(player, BLESSMAGIC);
	    if (lvl_obj != NULL) {
		struct linked_list *mobj;
		struct object *tp;
		bool show;

		show = FALSE;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj)) {
		    tp = (struct object *) ldata(mobj);
		    if (is_magic(tp)) {
			char mag_type=MAGIC;

			if (tp->o_flags & ISCURSED) mag_type = CMAGIC;
			else if (tp->o_flags & ISBLESSED) mag_type = BMAGIC;
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, mag_type);
		    }
		}
		for (titem = mlist; titem != NULL; titem = next(titem)) {
		    struct linked_list *pitem;

		    th = (struct thing *) ldata(titem);
		    for(pitem=th->t_pack; pitem!=NULL; pitem=next(pitem)) {
			if (is_magic((struct object *) ldata(pitem))) {
			    show = TRUE;
			    mvwaddch(hw, th->t_pos.y, th->t_pos.x, MAGIC);
			}
		    }
		}
		if (show) {
		    msg("You sense the presence of magic on this level.");
		    if (is_potion) 
			p_know[P_TFIND] = TRUE;
		    overlay(hw,cw);
		    draw(cw);
		    msg(" ");
		    msg("");
		    break;
		}
	    }
	    msg("You have a strange feeling for a moment, then it passes.");
	when P_SEEINVIS:
	    if (cursed) {
		if (off(player, ISBLIND) && !ISWEARING(R_SEEINVIS))
		{
		    msg("A cloak of darkness falls around you.");
		    turn_on(player, ISBLIND);
		    light_fuse(FUSE_SIGHT, 0, SEEDURATION, AFTER);
		    look(FALSE);
		}
		else
		    msg("Your eyes stop tingling for a moment.");
	    }
	    else if (off(player, PERMBLIND)) {
		if (off(player, CANSEE)) {
		    turn_on(player, CANSEE);
		    msg("Your eyes begin to tingle.");
		    light_fuse(FUSE_UNSEE, 0, blessed ? SEEDURATION*3 :SEEDURATION, AFTER);
		    light(&hero);
		}
		else if (find_slot(FUSE, FUSE_UNSEE) != NULL)
		    lengthen_fuse(FUSE_UNSEE, blessed ? SEEDURATION*3 : SEEDURATION);
		sight(NULL);
	    }
	when P_PHASE:
	    if (cursed) {
		msg("You can't move.");
		no_command = HOLDTIME;
	    }
	    else {
		short duration;

		if (blessed) duration = 3;
		else duration = 1;

		if (on(player, CANINWALL))
		    lengthen_fuse(FUSE_UNPHASE, duration*PHASEDURATION);
		else {
		    light_fuse(FUSE_UNPHASE, 0, duration*PHASEDURATION, AFTER);
		    turn_on(player, CANINWALL);
		}
		msg("You feel %slight-headed!",
		    blessed ? "very " : "");
	    }
	when P_RAISE:
	    if (cursed) 
		lower_level(D_POTION);
	    else {
		msg("You suddenly feel %smore skillful.",
			blessed ? "much " : "");
		p_know[P_RAISE] = TRUE;
		raise_level();
		if (blessed) 
		    raise_level();
	    }
	when P_HASTE:
	    if (cursed) {	/* Slow player down */
		if (on(player, ISHASTE)) { /* Already sped up */
		    extinguish_fuse(FUSE_NOHASTE);
		    nohaste(NULL);
		}
		else {
		    msg("You feel yourself moving %sslower.",
			    on(player, ISSLOW) ? "even " : "");
		    if (on(player, ISSLOW))
			lengthen_fuse(FUSE_NOSLOW, rnd(4) + 4);
		    else if (!ISWEARING(R_FREEDOM)) {
			turn_on(player, ISSLOW);
			player.t_turn = TRUE;
			light_fuse(FUSE_NOSLOW, 0, rnd(4)+4, AFTER);
		    }
		}
	    }
	    else {
		if (off(player, ISSLOW))
		    msg("You feel yourself moving %sfaster.",
			blessed ? "much " : "");
		add_haste(blessed);
	    }
	    if (is_potion) p_know[P_HASTE] = TRUE;
	when P_RESTORE: {
	    int i;

	    msg("Hey, this tastes great.  It make you feel warm all over.");

	    if (lost_str) {
		for (i=0; i<lost_str; i++)
		    extinguish_fuse(FUSE_RES_STRENGTH);
		lost_str = 0;
	    }
	    res_strength(NULL);

	    if (lost_dext) {
		for (i=0; i<lost_dext; i++)
		    extinguish_fuse(FUSE_UNITCH);
		lost_dext = 0;
		turn_off(player, HASITCH);
	    }
	    res_dexterity();

	    res_wisdom();
	    res_intelligence();
	    pstats.s_const = max_stats.s_const;
	}
	when P_INVIS:
	    if (off(player, ISINVIS)) {
		turn_on(player, ISINVIS);
		if (on(player, ISDISGUISE)) {
		    turn_off(player, ISDISGUISE);
		    extinguish_fuse(FUSE_UNDISGUISE);
		    msg("Your skin feels itchy for a moment.");
		}
		msg("You have a tingling feeling all over your body.");
		light_fuse(FUSE_APPEAR, 0, blessed ? WANDERTIME*3 : WANDERTIME, AFTER);
		PLAYER = IPLAYER;
		light(&hero);
	    }
	    else lengthen_fuse(FUSE_APPEAR, blessed ? WANDERTIME*3 : WANDERTIME);
	when P_SMELL:
	    if (cursed) {
		if (on(player, CANSCENT)) {
		    turn_off(player, CANSCENT);
		    extinguish_fuse(FUSE_UNSCENT);
		    msg("You no longer smell monsters around you.");
		}
		else if (on(player, ISUNSMELL)) {
		    lengthen_fuse(FUSE_SCENT, PHASEDURATION);
		    msg("You feel your nose tingle.");
		}
		else {
		    turn_on(player, ISUNSMELL);
		    light_fuse(FUSE_SCENT, 0, PHASEDURATION, AFTER);
		    msg("You can't smell anything now.");
		}
	    }
	    else {
		short duration;

		if (blessed)
		    duration = 3;
		else
		    duration = 1;
		if (on(player, CANSCENT))
		    lengthen_fuse(FUSE_UNSCENT, duration*PHASEDURATION);
		else {
		    light_fuse(FUSE_UNSCENT, 0, duration*PHASEDURATION, AFTER);
		    turn_on(player, CANSCENT);
		}
		msg("You begin to smell monsters all around you.");
	    }
	when P_HEAR:
	    if (cursed) {
		if (on(player, CANHEAR)) {
		    turn_off(player, CANHEAR);
		    extinguish_fuse(FUSE_HEAR);
		    msg("You no longer hear monsters around you.");
		}
		else if (on(player, ISDEAF)) {
		    lengthen_fuse(FUSE_HEAR, PHASEDURATION);
		    msg("You feel your ears burn.");
		}
		else {
		    light_fuse(FUSE_HEAR, 0, PHASEDURATION, AFTER);
		    turn_on(player, ISDEAF);
		    msg("You are surrounded by a sudden silence.");
		}
	    }
	    else {
		short duration;

		if (blessed)
		    duration = 3;
		else
		    duration = 1;
		if (on(player, CANHEAR))
		    lengthen_fuse(FUSE_UNHEAR, duration*PHASEDURATION);
		else {
		    light_fuse(FUSE_UNHEAR, 0, duration*PHASEDURATION, AFTER);
		    turn_on(player, CANHEAR);
		}
		msg("You begin to hear monsters all around you.");
	    }
	when P_SHERO:
	    if (cursed) {
		if (on(player, SUPERHERO)) {
		    msg("You feel ordinary again.");
		    turn_off(player, SUPERHERO);
		    extinguish_fuse(FUSE_UNSHERO);
		    extinguish_fuse(FUSE_UNBHERO);
		}
		else if (on(player, ISUNHERO)) {
		    msg("Your feeling of vulnerability increases.");
		    lengthen_fuse(FUSE_SHERO, 5 + rnd(5));
		}
		else {
		    msg("You feel suddenly vulnerable.");
		    if (pstats.s_hpt == 1) {
			death(D_POTION);
			return;
		    }
		    pstats.s_hpt /= 2;
		    chg_str(-2,FALSE,FALSE);
		    chg_dext(-2,FALSE,FALSE);
		    no_command = 3 + rnd(HEROTIME);
		    turn_on(player, ISUNHERO);
		    light_fuse(FUSE_SHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
		}
	    }
	    else {
		if (on(player, ISFLEE)) {
		    turn_off(player, ISFLEE);
		    msg("You regain your composure.");
		}
		if (on(player, ISUNHERO)) {
		    extinguish_fuse(FUSE_SHERO);
		    shero(NULL);
		}
		else if (on(player, SUPERHERO)) {
		    if (find_slot(FUSE, FUSE_UNBHERO)) 
			lengthen_fuse(FUSE_UNBHERO, HEROTIME + 2*rnd(HEROTIME));
		    else if (find_slot(FUSE, FUSE_UNSHERO) && !blessed) 
			lengthen_fuse(FUSE_UNSHERO, HEROTIME + 2*rnd(HEROTIME));
		    else {
			extinguish_fuse(FUSE_UNSHERO);
			unshero(NULL);
			light_fuse(FUSE_UNBHERO, 0, 2 * (HEROTIME + rnd(HEROTIME)), AFTER);
		    }
		    msg("Your feeling of invulnerablity grows stronger.");
		}
		else {
		    turn_on(player, SUPERHERO);
		    chg_str(10, FALSE, FALSE);
		    chg_dext(5, FALSE, FALSE);
		    quaff(P_HASTE, TRUE);
		    quaff(P_CLEAR, FALSE);
		    if (blessed) {
			light_fuse(FUSE_UNBHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
			msg("You suddenly feel invincible.");
		    }
		    else {
			light_fuse(FUSE_UNSHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
			msg("You suddenly feel invulnerable.");
		    }
		}
	    }
	when P_DISGUISE:
	    if (off(player, ISDISGUISE) && off(player, ISINVIS)) {
		turn_on(player, ISDISGUISE);
		msg("Your body shimmers a moment and then changes.");
		light_fuse(FUSE_UNDISGUISE, 0, blessed ? GONETIME*3 : GONETIME, AFTER);
		if (rnd(2))
		   PLAYER = 'a' + rnd(26);
		else
		   PLAYER = 'A' + rnd(26);
		light(&hero);
	    }
	    else if (off(player, ISINVIS))
		lengthen_fuse(FUSE_UNDISGUISE, blessed ? GONETIME*3 : GONETIME);
	    else
		msg("You feel an itchy feeling under your skin.");
        when P_LEVITATION:
            if (cursed) {
                msg("You can't move.");
                no_command = HOLDTIME;
            }
            else {
                short   duration = (blessed ? 3 : 1);

                if (on(player, CANFLY))
                    lengthen_fuse(FUSE_UNFLY, duration * WANDERTIME);
                else
                {
                    light_fuse(FUSE_UNFLY, 0, duration * WANDERTIME, AFTER);
                    turn_on(player, CANFLY);
                }

                if (!ISWEARING(R_LEVITATION))
                    msg("You %sbegin to float in the air!",
                        blessed ? "quickly " : "");
            }
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }
    status(FALSE);
    if (is_potion && p_know[which] && p_guess[which])
    {
	FREE(p_guess[which]);
	p_guess[which] = NULL;
    }
    else if (is_potion && 
	     !p_know[which] && 
	     askme &&
	     p_guess[which] == NULL)
    {
	msg("What do you want to call it? ");
	buf[0] = '\0';
	if (get_string(buf, cw) == NORM && strlen(buf) > 0)
	{
	    p_guess[which] = my_malloc((unsigned int) strlen(buf) + 1);
	    strcpy(p_guess[which], buf);
	}
    }
    if (is_potion) discard(item);
    updpack(TRUE);
}

/* Lower a level of experience */

void 
lower_level (int who)
{
    int fewer, nsides=0;

    if (--pstats.s_lvl == 0) {
	death(who);		/* All levels gone */
	return;
    }
    msg("You suddenly feel less skillful.");
    pstats.s_exp /= 2;
    switch (player.t_ctype) {
	case C_FIGHTER:		nsides = 12;
	when C_MAGICIAN:	nsides = 4;
	when C_CLERIC:		nsides = 8;
	when C_THIEF:		nsides = 6;
    }
    fewer = max(1, roll(1,nsides) + const_bonus());
    pstats.s_hpt -= fewer;
    max_stats.s_hpt -= fewer;
    if (pstats.s_hpt < 1)
	pstats.s_hpt = 1;
    if (max_stats.s_hpt < 1) {
	death(who);
	return;
    }
}

/*
 * res_dexterity:
 *	Restore player's dexterity
 */

void 
res_dexterity ()
{
    if (lost_dext) {
	chg_dext(lost_dext, FALSE, FALSE);
	lost_dext = 0;
    }
    else {
	pstats.s_dext = max_stats.s_dext + ring_value(R_ADDHIT) +
		(on(player, POWERDEXT) ? 10 : 0) +
		(on(player, SUPERHERO) ? 5 : 0);
    }

}


/*
 * res_wisdom:
 *	Restore player's wisdom
 */

void 
res_wisdom ()
{
    int ring_str;

    /* Discount the ring value */
    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    if (pstats.s_wisdom < max_stats.s_wisdom )
	pstats.s_wisdom = max_stats.s_wisdom;

    /* Redo the rings */
    pstats.s_wisdom += ring_str;
}

/*
 * res_intelligence:
 *	Restore player's intelligence
 */

void 
res_intelligence ()
{
    int ring_str;

    /* Discount the ring value */
    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    if (pstats.s_intel < max_stats.s_intel ) 
	pstats.s_intel = max_stats.s_intel;

    /* Redo the rings */
    pstats.s_intel += ring_str;
}


/*
 * Increase player's strength
 */

void 
add_strength (bool cursed)
{

    if (cursed) {
	msg("You feel slightly weaker now.");
	chg_str(-1, FALSE, FALSE);
    }
    else {
	msg("You feel stronger now.  What bulging muscles!");
	if (lost_str) {
	    lost_str--;
	    chg_str(1, FALSE, FALSE);
	}
	else
	    chg_str(1, TRUE, FALSE);
    }
}


/*
 * Increase player's intelligence
 */
void 
add_intelligence (bool cursed)
{
    int ring_str;	/* Value of ring strengths */

    /* Undo any ring changes */
    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    /* Now do the potion */
    if (cursed) {
	msg("You feel slightly less intelligent now.");
	pstats.s_intel = max(pstats.s_intel - 1, 3);
    }
    else {
	msg("You feel more intelligent now.  What a mind!");
	pstats.s_intel = min(pstats.s_intel + 1, 25);
    }

    /* Adjust the maximum */
    if (max_stats.s_intel < pstats.s_intel)
	max_stats.s_intel = pstats.s_intel;

    /* Now put back the ring changes */
    pstats.s_intel += ring_str;
}


/*
 * Increase player's wisdom
 */

void 
add_wisdom (bool cursed)
{
    int ring_str;	/* Value of ring strengths */

    /* Undo any ring changes */
    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    /* Now do the potion */
    if (cursed) {
	msg("You feel slightly less wise now.");
	pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
    }
    else {
	msg("You feel wiser now.  What a sage!");
	pstats.s_wisdom = min(pstats.s_wisdom + 1, 25);
    }

    /* Adjust the maximum */
    if (max_stats.s_wisdom < pstats.s_wisdom)
	max_stats.s_wisdom = pstats.s_wisdom;

    /* Now put back the ring changes */
    pstats.s_wisdom += ring_str;
}


/*
 * Increase player's dexterity
 */

void 
add_dexterity (bool cursed)
{
    /* Now do the potion */
    if (cursed) {
	msg("You feel less dextrous now.");
	chg_dext(-1, FALSE, TRUE);
    }
    else {
	msg("You feel more dextrous now.  Watch those hands!");
	if (lost_dext) {
	    lost_dext--;
	    chg_dext(1, FALSE, FALSE);
	}
	else
	    chg_dext(1, TRUE, FALSE);
    }
}


/*
 * Increase player's constitution
 */

void 
add_const (bool cursed)
{
    /* Do the potion */
    if (cursed) {
	msg("You feel slightly less healthy now.");
	pstats.s_const = max(pstats.s_const - 1, 3) +
		(on(player, POWERCONST) ? 10 : 0);
    }
    else {
	msg("You feel healthier now.");
	pstats.s_const = min(pstats.s_const + 1, 25) +
		(on(player, POWERCONST) ? 10 : 0);
    }

    /* Adjust the maximum */
    if (max_stats.s_const < pstats.s_const - (on(player, POWERCONST) ? 10 : 0))
	max_stats.s_const = pstats.s_const;
}
