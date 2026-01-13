/*
 * All the fighting gets done here
 *
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/* 
 * This are the beginning experience levels for all players
 * all further experience levels are computed by muliplying by 2
 */
static unsigned long e_levels[4] = {
	113L,	/* Fighter */
	135L,	/* Magician */
	87L,	/* Cleric */
	72L,	/* Thief */
};

struct matrix att_mat[5] = {
/* Base		Max_lvl,	Factor,		Offset,		Range */
{  10,		17,		2,		1,		2 },
{  9,		21,		2,		1,		5 },
{  10,		19,		2,		1,		3 },
{  10,		21,		2,		1,		4 },
{   7,		25,		1,		0,		2 }
};

static bool keep_fighting;  /* even if we kill something */
static int save_hpt;  /* your hit points at start of fight */

void 
do_fight (int y, int x, bool multiple)
{
    float limit = 0.33;
    if (serious_fight)
	limit *= 0.7;
    if (max_stats.s_hpt > 450)
	limit *= 0.8;

    if (pstats.s_hpt < max_stats.s_hpt*limit
     || (on(player, HASDISEASE) && pstats.s_hpt < max_stats.s_hpt*limit*2)
     || (on(player, HASINFEST) && pstats.s_hpt < max_stats.s_hpt*limit*2)
     || hungry_state == F_FAINT) {
	msg("That's not wise.");
	after = fighting = FALSE;
	return;
    }

    if (multiple) {
	if (!save_hpt || !keep_fighting)
	    save_hpt = pstats.s_hpt;  /* so we can calculate damage later  */
	keep_fighting = TRUE;
    } else {
	keep_fighting = FALSE;
	save_hpt = 0;
    }

    if (isalpha(winat(hero.y+y, hero.x+x))) {
	after = fighting = TRUE;
	do_move(y, x);
    } else {
	if (fighting == FALSE)
	    msg("Nothing there.");
	after = fighting = FALSE;
    }
}

/*
 * fight:
 *	The player attacks the monster.
 */

int 
fight (coord *mp, struct object *weap, bool thrown)
{
    struct thing *tp;
    struct linked_list *item;
    bool did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL) {
	debug("Fight what @ %d,%d", mp->y, mp->x);
	return 0;
    }
    tp = (struct thing *) ldata(item);
    foe = tp;

    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    player.t_quiet = 0;
    tp->t_quiet = 0;
    runto(mp, &hero);
    /*
     * Let player know it was really a mimic (if it was one).
     */
    if (on(*tp, ISDISGUISE) && (tp->t_type != tp->t_disguise) &&
	off(player, ISBLIND)) {
	msg("Wait! That's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, ISDISGUISE);
	did_hit = thrown;
    }
    if (on(*tp, CANSURPRISE) && off(player, ISBLIND)) {
	msg("Wait! There's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, CANSURPRISE);
	did_hit = thrown;
    }

    if (did_hit) {
	char *mname;

	did_hit = FALSE;
	mname = (on(player, ISBLIND)) ? "monster" : monsters[tp->t_index].m_name;
	if (!can_blink(tp) &&
	    (off(*tp, MAGICHIT) || (weap != NULL && (weap->o_hplus > 0 || weap->o_dplus > 0))) &&
	    (off(*tp, BMAGICHIT) || (weap != NULL && (weap->o_hplus > 1 || weap->o_dplus > 1))) &&
	    roll_em(&player, tp, weap, thrown, cur_weapon)) {
	    did_hit = TRUE;
	    tp->t_wasshot = TRUE;

	    if (thrown) {
		if (weap != NULL && weap->o_type == WEAPON
			&& weap->o_which == GRENADE) {
		    msg("BOOOM!");
		    aggravate();
		}
		thunk(weap, mname);
	    }
	    else
		hit(NULL, mname);

            /* hitting a friendly monster is curtains */
            if (on(*tp, ISFRIENDLY)) {
                turn_off(*tp, ISFRIENDLY);
                turn_on(*tp, ISMEAN);
            }

	    /* If the player hit a rust monster */
	    if (on(*tp, CANRUST)) {
		if (!thrown && (weap != NULL) &&
		    (weap->o_flags & ISMETAL) &&
		    !(weap->o_flags & ISPROT) &&
		    !(weap->o_flags & ISSILVER) &&
		    (weap->o_hplus < 1) && (weap->o_dplus < 1)) {
		    if (rnd(100) < 50) weap->o_hplus--;
		    else weap->o_dplus--;
		    msg("Your %s appears to be weaker now!",
		        weaps[weap->o_which].w_name);   
		}
		else if (!thrown && weap != NULL && (weap->o_flags & ISMETAL) && !fighting)
		    msg("The rust vanishes from your %s!",
			weaps[weap->o_which].w_name);   
	    }
		
	    /* flammable monsters die from burning weapons */
	    if ( thrown && on(*tp, CANBBURN) && 
			(weap->o_flags & CANBURN) &&
			!save_throw(VS_WAND, tp)) {
		msg("The %s vanishes in a ball of flame.", 
			monsters[tp->t_index].m_name);
		killed(item, TRUE, TRUE);
	    }

	    /* spores explode and may infest hero */
	    if (on(*tp, CANSPORE)) {
		msg("The %s explodes in a cloud of dust.",
		    monsters[tp->t_index].m_name);
		if (ISWEARING(R_HEALTH) || rnd(20) > 0) {
		    if (!fighting)
			msg("The dust makes it hard to breath.");
		} else {
		    msg("You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		    light_fuse(FUSE_CURE_INFEST, 0, roll(8,4) * SICKTIME, AFTER);
		    fighting = FALSE;
		}
	    }

	    /* fireproof monsters laugh at you when burning weapon hits */
	    if ( thrown && on(*tp, NOFIRE) && (weap->o_flags & CANBURN)) 
		msg("The %s laughs as the %s bounces.", 
			monsters[tp->t_index].m_name,
			weaps[weap->o_which].w_name);

            /* metal weapons pass through NOMETAL monsters */
            if (on(*tp, NOMETAL) && (weap != NULL) &&
                (weap->o_flags & ISMETAL) &&
		!(weap->o_flags & ISVORPED && difficulty <= 3)) {
		msg("The %s passes through the %s!",
		    weaps[weap->o_which].w_name,
		    monsters[tp->t_index].m_name);
		fighting = FALSE;
            }

	    /* If the player hit something that shrieks, wake the dungeon */
	    if (on(*tp, CANSHRIEK)) {
		turn_off(*tp, CANSHRIEK);
		if (on(player, CANHEAR)) {
		    msg("You are stunned by the %s's shriek.", mname);
		    no_command += 4 + rnd(8);
		}
		else if (off(player, ISDEAF))
		    msg("The %s emits a piercing shriek.", mname);
		else
		    msg("The %s seems to be trying to make some noise.", mname);
		aggravate();
		if (rnd(wizard ? 3 : 50) == 0 && cur_armor != NULL 
		    && cur_armor->o_which == CRYSTAL_ARMOR
		    && (difficulty > 1)) {

		    struct linked_list *item;
		    struct object *obj;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = (struct object *) ldata(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    } else if (!(cur_armor->o_flags & IS2PROT) || difficulty > 3) {
			msg("Your armor shatters from the shriek.");
			cur_armor = NULL;
			del_pack(item);
			fighting = FALSE;
		    }
		}
	    }

	    /* If the player hit something that can surprise, it can't now */
	    if (on(*tp, CANSURPRISE)) turn_off(*tp, CANSURPRISE);

	    /* vorpal blades are awesome */
	    if (cur_weapon != NULL && cur_weapon->o_flags & ISVORPED
		    && on(*tp, CANSUMMON) && rnd(difficulty) == 0
		    && difficulty <= 3) {
		turn_off(*tp, CANSUMMON);
	    }

	    /* If the player hit something that can summon, it will try to */
	    if (on(*tp, CANSUMMON) && rnd(40) < tp->t_stats.s_lvl
		    && tp->t_stats.s_hpt > 0 ) {
	        char *helpname;
	        int which, i;

		turn_off(*tp, CANSUMMON);
	        msg("The %s summons help!", mname);
	        helpname = monsters[tp->t_index].m_typesum;
	        if (helpname) {
		    for (which=1; which<nummonst; which++) {
			 if (strcmp(helpname, monsters[which].m_name) == NULL)
			     break;
		    }
		    if (which >= nummonst)
			debug("Couldn't find summoned one.");
		} else {
		    debug("Couldn't find monster to summon.");
		    which = nummonst+1;
		}

		/* summoned monster was genocided */
		if (which <= nummonst && 
		     !monsters[which].m_normal &&
		     !monsters[which].m_wander) {
		     msg("The %s becomes very annoyed at you!", mname);
		     if (on(*tp, ISSLOW))
			turn_off(*tp, ISSLOW);
		     else
			turn_on(*tp, ISHASTE);
		} else {
		    bool summon_ok = TRUE;

		    /* can't summon a unique that already exists */
		    for (i=0; i<NM_FLAGS; i++) {
			if (monsters[which].m_flags[i] == ISUNIQUE) {
			    struct linked_list *nitem;
			    struct thing *ntp;
			    for (nitem = mlist; nitem != NULL; nitem = next(nitem)) {
				ntp = THINGPTR(nitem);
				if (ntp->t_index == which) {
				    msg("The %s becomes rather annoyed at you!", mname);
				    summon_ok = FALSE;
				}
			    }
			}
		    }

		    if (summon_ok) {
			for (i=0; i<monsters[tp->t_index].m_numsum; i++)
			    creat_mons(&player, which, FALSE);
		    }
		}
	    }

	    /* Can the player confuse? */
	    if (on(player, CANHUH) && !thrown) {
		msg("Your hands stop glowing red!");
		msg("The %s appears confused.", mname);
		turn_on(*tp, ISHUH);
		turn_off(player, CANHUH);
	    }
	    /* Merchants just disappear if hit */
	    /* increases prices and curses objects from now on though */
	    if (on(*tp, CANSELL)) {
		msg("The %s disappears with %s wares in a flash.",mname,
		    (rnd(2)? "his": "her"));
		killed(item, FALSE, FALSE);
		aggravate();
		luck++;
	    }

	    else if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE, TRUE);

	    /* If the monster is fairly intelligent and about to die, it
	     * may turn tail and run.
	     */
	    else if ((tp->t_stats.s_hpt < max(10,tp->maxstats.s_hpt/10)) &&
		     (rnd(25) < tp->t_stats.s_intel)) {
		turn_on(*tp, ISFLEE);

		/* If monster was suffocating us, stop it */
		if (on(*tp, DIDSUFFOCATE)) {
		    turn_off(*tp, DIDSUFFOCATE);
		    extinguish_fuse(FUSE_SUFFOCATE);
		}

		/* If monster held us, stop it */
		if (on(*tp, DIDHOLD) && (--hold_count == 0))
			turn_off(player, ISHELD);
		turn_off(*tp, DIDHOLD);
	    }
	}
	else {
	    if (thrown) 
		bounce(weap, mname);
	    else
		miss(NULL, mname);
	}
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

int 
attack (struct thing *mp, struct object *weapon, bool thrown)
{
    char *mname;
    bool did_hit = FALSE;
    bool was_running = FALSE;
    int s_hpt, damage;

    /* If the monster is in a wall, it cannot attack */
    if (on(*mp, ISINWALL)) 
	return(FALSE);

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    if (running)
	was_running = TRUE;
    running = FALSE;
    player.t_quiet = 0;
    mp->t_quiet = 0;
    s_hpt = pstats.s_hpt;  /* save this so we can calculate damage later  */

    if (on(*mp, ISDISGUISE) && off(player, ISBLIND))
	turn_off(*mp, ISDISGUISE);
    mname = on(player, ISBLIND) ? "the monster" : monsters[mp->t_index].m_name;
    if (roll_em(mp, &player, weapon, thrown, wield_weap(weapon, mp))) {
	did_hit = TRUE;

	/*
	 * If two monsters gang up on our hero, stop the fight
	 */
	if (fighting && !keep_fighting) {
	    if (beast == NULL) {
		beast = mp;
	    } else if (beast != mp && !keep_fighting) {
		msg("Another monster enters the fray.");
		fighting = FALSE;
	    }
	}

	if (thrown) m_thunk(weapon, mname);
	else hit(mname, NULL);

	if (pstats.s_hpt <= 0) {
	    death(mp->t_index);	/* Bye bye life ... */
	    return TRUE;
	}

	/*
	 * surprising monsters appear after they shoot at you 
	 */
	if (thrown) {
	    if (on(*mp, CANSURPRISE)) 
		turn_off(*mp, CANSURPRISE);
	}
	if (!thrown) {
	    /*
	     * If a vampire hits, it may take half your hit points
	     */
	    if (on(*mp, CANSUCK) && !save(VS_MAGIC)) {
		if (pstats.s_hpt == 1) {
		    death(mp->t_index);
		    return TRUE;
		}
		else {
		    pstats.s_hpt /= 2;
		    msg("You feel your life force being drawn from you.");
		}
	    }

	    /*
	     * monsters hitting hard can shatter crystalline armor
	     * or cause it to begin ringing if they are strong enough
	     */
	    if (cur_armor != NULL && cur_armor->o_which == CRYSTAL_ARMOR) {
		if (rnd(mp->t_stats.s_str + (cur_armor->o_ac/2)) > 20
		    && (difficulty > 1)) {

		    struct linked_list *item;
		    struct object *obj;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = (struct object *) ldata(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    } else if (!(cur_armor->o_flags & IS2PROT) || difficulty > 3) {
			msg("Your armor is shattered by the blow.");
			cur_armor = NULL;
			del_pack(item);
			fighting = FALSE;
		    }
		}
		else if (rnd(mp->t_stats.s_str) > 15) {
		    msg("Your armor rings from the blow.");
		    if (difficulty >= 2)
			aggravate();
		}
	    }

	    /*
	     * Stinking monsters make player weaker (to hit)
	     */
	    if (on(*mp, CANSTINK)) {
		turn_off(*mp, CANSTINK);
		if (!save(VS_POISON)) {
		    if (on(player, CANSCENT)) {
			msg("You pass out from the stench of the %s.", mname);
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("The stench of the %s sickens you.", mname);
		    if (on(player, HASSTINK)) 
			lengthen_fuse(FUSE_UNSTINK, STINKTIME);
		    else {
			turn_on(player, HASSTINK);
			light_fuse(FUSE_UNSTINK, 0, STINKTIME, AFTER);
		    }
		}
	    }

	    /*
	     * Chilling monster reduces strength each time unless you are
	     * wearing crystal armor
	     */
	    if (on(*mp, CANCHILL) && 
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {

		msg("You cringe at the %s's chilling touch.", mname);
		if (!ISWEARING(R_SUSABILITY)) {
		    chg_str(-1, FALSE, TRUE);
		    if (lost_str == 0)
			light_fuse(FUSE_RES_STRENGTH, 0, CHILLTIME, AFTER);
		    else lengthen_fuse(FUSE_RES_STRENGTH, CHILLTIME);
		}
	    }

	    /*
	     * itching monsters reduce dexterity (temporarily)
	     */
	    if (on(*mp, CANITCH) && !save(VS_POISON)) {
		msg("The claws of the %s scratch you!", mname);
		if(ISWEARING(R_SUSABILITY)) {
		    msg("The scratch has no effect.");
		}
		else {
		    msg("You feel a burning itch.");
		    turn_on(player, HASITCH);
		    chg_dext(-1, FALSE, TRUE);
		    light_fuse(FUSE_UNITCH, 0, roll(4,6), AFTER);
		}
	    }


	    /*
	     * If a hugging monster hits, it may SQUEEEEEEEZE
	     * unless you are wearing crystal armor
	     */
	    if (on(*mp, CANHUG) && 
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {
		if (roll(1,20) >= 18 || roll(1,20) >= 18) {
		    msg("The %s squeezes you against itself.", mname);
		    if ((pstats.s_hpt -= roll(2,8)) <= 0) {
			death(mp->t_index);
			return TRUE;
		    }
		}
	    }

            /* a trampling monster may step on the player */
            if (on(*mp, CANTRAMPLE)) {
                if (roll(1, 20) >= 16) {
		    if (!fighting)
			msg("The %s stomps on your foot.", mname);

                    if ((pstats.s_hpt -= roll(1, mp->t_stats.s_lvl)) <= 0) {
                        death(mp->t_index);
                        return TRUE;
                    }
                }
            }

	    /*
	     * If a disease-carrying monster hits, there is a chance the
	     * player will catch the disease
	     */
	    if (on(*mp, CANDISEASE) &&
		(rnd(pstats.s_const) < mp->t_stats.s_lvl) &&
		off(player, HASDISEASE)) {

		if (ISWEARING(R_HEALTH)) 
		    msg("The wound heals quickly.");
		else {
		    turn_on(player, HASDISEASE);
		    light_fuse(FUSE_CURE_DISEASE, 0, roll(4,4) * SICKTIME, AFTER);
		    msg("You have contracted a disease!");
		}
	    }

	    /*
	     * If a rust monster hits, you lose armor
	     */
	    if (on(*mp, CANRUST)) {
		if (cur_armor != NULL &&
		    cur_armor->o_which != LEATHER &&
		    cur_armor->o_which != PADDED_ARMOR &&
		    cur_armor->o_which != CRYSTAL_ARMOR &&
		    cur_armor->o_which != MITHRIL &&
		    !(cur_armor->o_flags & ISPROT) &&
		    cur_armor->o_ac < pstats.s_arm+1) {
		    msg("Your armor appears to be weaker now. Oh my!");
		    cur_armor->o_ac++;
		}
		else if (cur_armor != NULL && 
		    cur_armor->o_which != LEATHER &&
		    cur_armor->o_which != PADDED_ARMOR &&
		    cur_armor->o_which != CRYSTAL_ARMOR &&
		    cur_armor->o_which != MITHRIL &&
		    (cur_armor->o_flags & ISPROT) && !fighting)
		    msg("The rust vanishes instantly!");
	    }

	    /* If a surprising monster hit you, you can see it now */
	    if (on(*mp, CANSURPRISE)) 
		turn_off(*mp, CANSURPRISE);

	    /*
	     * If an infesting monster hits you, you get a parasite or rot
	     */
	    if (on(*mp, CANINFEST) && rnd(pstats.s_const) < mp->t_stats.s_lvl) {
		if (ISWEARING(R_HEALTH)) 
		    msg("The wound quickly heals.");
		else {
		    turn_off(*mp, CANINFEST);
		    msg("You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		    light_fuse(FUSE_CURE_INFEST, 0, roll(8,4) * SICKTIME, AFTER);
		    fighting = FALSE;
		}
	    }

	    /*
	     * Ants have poisonous bites
	     */
	    if (on(*mp, CANPOISON) && !save(VS_POISON)) {
		if (ISWEARING(R_SUSABILITY))
		    msg("A sting momentarily weakens you.");
		else {
		    chg_str(-1, FALSE, FALSE);
		    msg("You feel a sting in your arm and now feel weaker.");
		}
	    }

	    /*
	     * Cause fear by touching
	     */
	    if (on(*mp, TOUCHFEAR)) {
		turn_off(*mp, TOUCHFEAR);
		if (!save(VS_WAND)
		    && !(on(player, ISFLEE) && (player.t_dest == &mp->t_pos))) {
			if (off(player, SUPERHERO)) {
			    turn_on(player, ISFLEE);
			    player.t_dest = &mp->t_pos;
			    msg("The %s's touch terrifies you.", mname);
			}
			else
			    msg("The %s's touch feels cold and clammy.",
					mname);
		}
	    }

	    /*
	     * Suffocating our hero
	     */
	    if (on(*mp, CANSUFFOCATE) && (rnd(100) < 15) &&
		(find_slot(FUSE, FUSE_SUFFOCATE) == NULL)) {
		turn_on(*mp, DIDSUFFOCATE);
		msg("The %s is beginning to suffocate you.", mname);
		light_fuse(FUSE_SUFFOCATE, 0, roll(4,2), AFTER);
		fighting = FALSE;
	    }

	    /*
	     * Turning to stone
	     */
	    if (on(*mp, TOUCHSTONE)) {
		turn_off(*mp, TOUCHSTONE);
		if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		else {
		    if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
			msg("The touch of the %s petrifies you.", mname);
			if (difficulty >= 2) {
			    msg("You are turned to stone !!! --More--");
			    wait_for(' ');
			    death(D_PETRIFY);
			    return TRUE;
			} else {
			    no_command = rnd(STONETIME) + 2;
			    fighting = FALSE;
			}
		    }
		    else {
			msg("The %s's touch stiffens your limbs.", mname);
			no_command = rnd(STONETIME) + 2;
		    }
		}
	    }

	    /*
	     * Wraiths might drain energy levels
	     */
	    if ((on(*mp, CANDRAIN) || on(*mp, DOUBLEDRAIN)) && rnd(100) < 15) {
		lower_level(mp->t_index);
		if (on(*mp, DOUBLEDRAIN)) 
		    lower_level(mp->t_index);
		turn_on(*mp, DIDDRAIN);  
		fighting = FALSE;
	    }

            /* drain a wisdom point */
            if (on(*mp, DRAINWISDOM) && rnd(100) < 15
		&& !ISWEARING(R_SUSABILITY)) {
                /* Undo any ring changes */
		int ring_str;
                ring_str = ring_value(R_ADDWISDOM) +
                    (on(player, POWERWISDOM) ? 10 : 0);
                pstats.s_wisdom -= ring_str;
                                
                msg("You feel slightly less wise now.");
                pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
#ifndef EARL
		if (rnd(difficulty) != 0)
		    max_stats.s_wisdom = pstats.s_wisdom;
#endif
                /* Now put back the ring changes */
                pstats.s_wisdom += ring_str;

            }

            /* drain a intelligence point */
            if (on(*mp, DRAINBRAIN) && rnd(100) < 15
		&& !ISWEARING(R_SUSABILITY)) {
                /* Undo any ring changes */
		int ring_str;
                ring_str = ring_value(R_ADDINTEL) +
                    (on(player, POWERINTEL) ? 10 : 0);
                pstats.s_intel -= ring_str;

                msg("You feel slightly less intelligent now.");
                pstats.s_intel = max(pstats.s_intel - 1, 3);
#ifndef EARL
		if (rnd(difficulty) != 0)
		    max_stats.s_intel = pstats.s_intel;
#endif
                /* Now put back the ring changes */
                pstats.s_intel += ring_str;
            }

	    /*
	     * Violet fungi and others stop the poor player from moving
	     */
	    if (on(*mp, CANHOLD) && off(*mp, DIDHOLD) 
			&& !ISWEARING(R_FREEDOM)) {
		turn_on(player, ISHELD);
		turn_on(*mp, DIDHOLD);
		hold_count++;
	    }

	    /*
	     * Sucker will suck blood and run
	     */
	    if (on(*mp, CANDRAW)) {
		turn_off(*mp, CANDRAW);
		turn_on(*mp, ISFLEE);
		msg("The %s sates itself with your blood.", mname);
		if ((pstats.s_hpt -= 12) <= 0) {
		    death(mp->t_index);
		    return TRUE;
		}
	    }

	    /*
	     * Bad smell will force a reduction in strength
	     */
	    if (on(*mp, CANSMELL)) {
		turn_off(*mp, CANSMELL);
		if (save(VS_MAGIC) || ISWEARING(R_SUSABILITY))
		    msg("You smell an unpleasant odor.");
		else {
		    short odor_str = -(rnd(6)+1);

		    if (on(player, CANSCENT)) {
			msg("You pass out from a foul odor.");
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("You are overcome by a foul odor.");
		    if (lost_str == 0) {
			chg_str(odor_str, FALSE, TRUE);
			light_fuse(FUSE_RES_STRENGTH, 0, SMELLTIME, AFTER);
		    }
		    else 
			lengthen_fuse(FUSE_RES_STRENGTH, SMELLTIME);
		}
	    }

	    /*
	     * Paralyzation
	     */
	    if (on(*mp, CANPARALYZE)) {
		turn_off(*mp, CANPARALYZE);
		if (!save(VS_PARALYZATION) && no_command == 0) {
		    if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		    else {
			msg("The %s's touch paralyzes you.", mname);
			no_command = FREEZETIME;
		    }
		}
	    }

	    /*
	     * Rotting
	     */
	    if (on(*mp, CANROT)) {
		turn_off(*mp, CANROT);
		turn_on(*mp, DOROT);
	    }

	    if (on(*mp, STEALGOLD)) {
		/*
		 * Leperachaun steals some gold
		 */
		long lastpurse;
		struct linked_list *item;
		struct object *obj;

		lastpurse = purse;
		purse -= GOLDCALC;
		if (!save(VS_MAGIC))
		    purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		if (purse < 0)
		    purse = 0;
		if (purse != lastpurse) {
		    msg("Your purse feels lighter.");

		    /* Give the gold to the thief */
		    for (item=mp->t_pack; item != NULL; item=next(item)) {
			obj = (struct object *) ldata(item);
			if (obj->o_type == GOLD) {
			    obj->o_count += lastpurse - purse;
			    break;
			}
		    }

		    /* Did we do it? */
		    if (item == NULL) {	/* Then make some */
			item = new_item(sizeof *obj);
			obj = (struct object *) ldata(item);
			obj->o_type = GOLD;
			obj->o_count = lastpurse - purse;
			obj->o_hplus = obj->o_dplus = 0;
			obj->o_damage = obj->o_hurldmg = "0d0";
			obj->o_ac = 11;
			obj->o_group = 0;
			obj->o_flags = 0;
			obj->o_mark[0] = '\0';
			obj->o_pos = mp->t_pos;

			attach(mp->t_pack, item);
		    }
		}

		if (rnd(2))
		    turn_on(*mp, ISFLEE);
		turn_on(*mp, ISINVIS);
	    }

	    if (on(*mp, STEALMAGIC)) {
		struct linked_list *item, *steal;
		struct object *obj;
		int worth = 0;

		/*
		 * Nymph's steal a magic item, look through the pack
		 * and pick out one we like.
		 */
		steal = NULL;
		for (item = pack; item != NULL; item = next(item)) {
		    obj = (struct object *) ldata(item);
		    if (rnd(33) == 0 && !(obj->o_flags & ISPROT)) {
			if (obj->o_flags & ISBLESSED)
			    obj->o_flags &= ~ISBLESSED;
			else
			    obj->o_flags |= ISCURSED;
			msg("You feel nimble fingers reach into you pack.");
		    }
		    if (((obj != cur_armor && obj != cur_weapon &&
			obj != cur_ring[LEFT_1] && obj != cur_ring[LEFT_2] &&
			obj != cur_ring[LEFT_3] && obj != cur_ring[LEFT_4] &&
			obj != cur_ring[RIGHT_1] && obj != cur_ring[RIGHT_2] &&
			obj != cur_ring[RIGHT_3] && obj != cur_ring[RIGHT_4] &&
			!(obj->o_flags & ISPROT) && is_magic(obj) )
			    || (level > 95 && difficulty >= 2))
			&& get_worth(obj) > worth) {
			steal = item;
			worth = get_worth(obj);
		    }
		}
		if (steal != NULL) {
		    struct object *obj;
		    obj = (struct object *) ldata(steal);

		    if (obj->o_flags & ISOWNED) {
			turn_on(*mp, NOMOVE);
			msg("The %s is transfixed by your ownership spell.",
				mname);
		    } else {
			if (obj->o_count > 1 && obj->o_group == 0) {
			    int oc;
			    struct linked_list *nitem;
			    struct object *op;

			    oc = --(obj->o_count);
			    obj->o_count = 1;
			    nitem = new_item(sizeof *obj);
			    op = (struct object *) ldata(nitem);
			    *op = *obj;
			    msg("The %s stole %s!", mname, inv_name(obj, TRUE));
			    obj->o_count = oc;
			    attach(mp->t_pack, nitem);
			}
			else {
			    msg("The %s stole %s!", mname, inv_name(obj, TRUE));
			    obj->o_flags &= ~ISCURSED;
			    dropcheck(obj);
			    detach(pack, steal);
			    freeletter(steal);
			    attach(mp->t_pack, steal);
			    inpack--;
			    if (obj->o_type == ARTIFACT)
				has_artifact &= ~(1 << obj->o_which);
			}
			if (rnd(2))
			    turn_on(*mp, ISFLEE);
			turn_on(*mp, ISINVIS);
			updpack(FALSE);
			fighting = FALSE;
		    }
		}
	    }
	}
	if (off(player, ISBLIND) && on(player, CANSEE) && on(*mp, CANSURPRISE))
	      turn_off(*mp, CANSURPRISE);
    }
    else {
	/* If the thing was trying to surprise, no good */
	if (on(*mp, CANSURPRISE)) turn_off(*mp, CANSURPRISE);

	if (on(*mp, DOROT)) {
	    msg("Your skin crawls and you feel weaker.");

	    pstats.s_hpt -= 2;
	    if (pstats.s_hpt <= 0) {
		death(mp->t_index);	/* Bye bye life ... */
		return TRUE;
	    }
	}
	else if (thrown) { 
	    if (was_running)
		running = TRUE;
	    m_bounce(weapon, mname);
	} else {
	    miss(mname, NULL);
	}
    }

    damage = s_hpt - pstats.s_hpt;
    if (damage && (fighting || keep_fighting)) {

	/* too much damage in one round */
	if (damage > s_hpt/5) {
	    msg("The %s scored an excellent hit on you!", mname);
	    fighting = keep_fighting = FALSE;
	/* too much damage in one fight */
	} else if (save_hpt-pstats.s_hpt > pstats.s_hpt/2
		) {
	    msg("Ouch, that hurt.");
	    fighting = keep_fighting = FALSE;
	}
    }

    count = 0;
    status(FALSE);
    return(did_hit);
}

/*
 * swing:
 *	returns true if the swing hits
 */

int 
swing (int charclass, int at_lvl, int op_arm, int wplus)
{
    int res = rnd(20)+1;
    int need;

    need = att_mat[charclass].base -
	   att_mat[charclass].factor *
	   ((min(at_lvl, att_mat[charclass].max_lvl) -
	    att_mat[charclass].offset)/att_mat[charclass].range) +
	   (10 - op_arm);
    if (need > 20 && need <= 25) need = 20;

    /* give monsters a chance to hit well armored player */
    if (difficulty >= 2) {
	if (charclass == C_MONSTER && need > 15) {
	    if (difficulty > 3 || (difficulty > 2 && level < 90))
		need = 15 + ((need - 15) * 0.5);
	    else
		need = 15 + ((need - 15) * 0.67);

	    /* the deeper you go, the harder it gets */
	    if (level > 35 && difficulty > 2)
		need -= level / 20;
	}

	/*
	 * If monster is too weak to hurt us (or vice versa)
	 * but it's close, give them a chance.
	 * This makes the mid-dungeon more interesting.
	 */
	if (level > 35 && level < 85
	    && need > 20 + wplus
	    && (need < 22 + wplus + (difficulty*2))
	    && res == 20 && rnd(5-difficulty) == 0
	    ) {
#if 0
		if (charclass == C_MONSTER)
		    msg("Lucky hit (%d+%d < %d)", res, wplus, need);
		else
		    msg("Lucky hit (you have %d, normally need: %d)", res+wplus, need);
#endif
		return (1);
	}
    }

    return (res+wplus >= need);
}

void
next_level()
{
    int i = 1;
    unsigned long exp;

    exp = e_levels[player.t_ctype];
    while (exp <= pstats.s_exp) {
	i++;
	exp *= 2L;
    }
    msg("You need %d more points to reach level %d.", exp - pstats.s_exp, i+1);
}

/*
 * check_level:
 *	Check to see if the player has gone up a level.
 */

void 
check_level ()
{
    int i, j, add = 0;
    unsigned long exp;
    int nsides = 10;

    i = 0;
    exp = e_levels[player.t_ctype];
    while (exp <= pstats.s_exp) {
	i++;
	exp *= 2L;
    }
    if (++i > pstats.s_lvl) {
	switch (player.t_ctype) {
	    case C_FIGHTER:	nsides = 12;
	    when C_MAGICIAN:	nsides = 4;
	    when C_CLERIC:	nsides = 8;
	    when C_THIEF:	nsides = 6;
	}

	/* Take care of multi-level jumps */
	for (j=0; j < (i-pstats.s_lvl); j++)
	    add += max(1, roll(1,nsides) + const_bonus());
	max_stats.s_hpt += add;
	if ((pstats.s_hpt += add) > max_stats.s_hpt)
	    pstats.s_hpt = max_stats.s_hpt;
	msg("Welcome, %s, to level %d.",
	    cnames[player.t_ctype][min(i-1, 10)], i);
	pray_time = 0;	/* A new round of prayers */
	spell_power = 0; /* A new round of spells */
	fighting = FALSE;
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */

int 
roll_em (struct thing *att_er, struct thing *def_er, struct object *weap, bool hurl, struct object *cur_weapon)
{
    struct stats *att, *def;
    char *cp;
    int ndice, nsides, nplus, def_arm;
    bool did_hit = FALSE;
    int prop_hplus, prop_dplus;

    /* Get statistics */
    att = &att_er->t_stats;
    def = &def_er->t_stats;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch) {
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0) {
		    weap->o_damage = "0d0";
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	/* Is attacker weak? */
	if (on(*att_er, HASSTINK)) 
	    hplus -= 2;

	/* Code changed by Bruce Dautrich 4/4/84 to fix bug */
	if (att_er == &player)	{	/* Is it the player? */
	    hplus += hitweight();	/* adjust for encumberence */
	    dplus += hung_dam();	/* adjust damage for hungry player */
	    dplus += ring_value(R_ADDDAM);
	}
	ndice = atoi(cp);
	if (cp == NULL || (cp = strchr(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (cp != NULL && (cp = strchr(cp, '+')) != NULL) 
	    nplus = atoi(++cp);
	else nplus = 0;

	if (def == &pstats) {
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac - 10 + pstats.s_arm;
	    else
		def_arm = def->s_arm;
	    def_arm -= ring_value(R_PROTECT);
	}
	else
	    def_arm = def->s_arm;
	if ((weap != NULL && weap->o_type == WEAPON &&
		(weap->o_flags & ISSILVER) && 
		!save_throw(VS_MAGIC,def_er)) ||
	    swing(att_er->t_ctype, att->s_lvl, def_arm-dext_prot(def->s_dext),
		  hplus+str_plus(att->s_str)+dext_plus(att->s_dext))) {
	    int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug("Damage for %dd%d came out %d.", ndice, nsides, proll);
	    damage = dplus + proll + nplus + add_dam(att->s_str);

	    /* Check for half damage monsters */
	    if (on(*def_er, HALFDAMAGE) && (weap != NULL) &&
		!((weap->o_flags & CANBURN) && on(*def_er, CANBBURN)))
		damage /= 2;

	    /* undead get twice damage from silver weapons */
	    if (on(*def_er, ISUNDEAD) && 
			(weap != NULL) && (weap->o_flags & ISSILVER))
		damage *= 2;

	    /* Check for fireproof monsters */
	    if (on(*def_er, NOFIRE) && (weap != NULL) && 
			(weap->o_flags & CANBURN))
		damage = 0;

            /* Check for metal proof monsters */
            if (on(*def_er, NOMETAL) && (weap != NULL) &&
                (weap->o_flags & ISMETAL) &&
		!(weap->o_flags & ISVORPED))
                damage = 0;

	    /* Check for poisoned weapons */
	    if ((weap != NULL) && (weap->o_flags & ISPOISON) 
			&& off(*def_er, ISUNDEAD)
			&& !save_throw(VS_POISON, def_er)) {
		damage = (def->s_hpt / 2) + 5;
		/* debug("Defender was hit by poison."); */
	    }

	    /* vorpal blades are awesome */
	    if (cur_weapon != NULL && cur_weapon->o_flags & ISVORPED
		    && on(*def_er, BLOWDIVIDE) && rnd(difficulty) == 0
		    && difficulty <= 3) {
                turn_off(*def_er, BLOWDIVIDE);
            }

	    /* Check for no-damage and division */
	    if (on(*def_er, BLOWDIVIDE) &&
			!((weap != NULL) && (weap->o_flags & CANBURN))) {
		struct thing *mcopy;
		mcopy = creat_mons(def_er, def_er->t_index, FALSE);
		if (def_er->t_stats.s_lvl > 1) {
		    /* 
		     * the number of times a monster can divide
		     * is based on it's experience level
		     */
		    if (rnd(difficulty-1) == 0)
			def_er->t_stats.s_lvl--;
		} else {
		    turn_off(*def_er, BLOWDIVIDE);
		    if (mcopy)
			turn_off(*mcopy, BLOWDIVIDE);
		}
		if (mcopy) {
		    if (def_er->t_stats.s_exp > 9)
			def_er->t_stats.s_exp /= 2;  /* share the points */
		    mcopy->t_stats.s_lvl = def_er->t_stats.s_lvl;
		    mcopy->t_stats.s_exp = def_er->t_stats.s_exp;
		    mcopy->t_stats.s_hpt = def_er->t_stats.s_hpt;
		    debug("The %s divided!", monsters[def_er->t_index].m_name);
		    if (!serious_fight)
			fighting = FALSE;
		}
		damage = 0;
		did_hit = TRUE;
		break;
	    }

	    /* thick skin reduces damage */
	    if (difficulty < 2) {
		if (def_er == &player && pstats.s_arm < 10 && damage > 20) {
		    damage -= 10 - pstats.s_arm;
		}
	    }

	    damage = max(0, damage);
	    def->s_hpt -= damage;	/* Do the damage */

	    if (att_er == &player && ISWEARING(R_VREGEN)) {
		damage = (ring_value(R_VREGEN) * damage) / 3;
		pstats.s_hpt = min(max_stats.s_hpt, pstats.s_hpt + damage);
	    }

	    did_hit = TRUE;
	}
	if (cp == NULL || (cp = strchr(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char *
prname (char *who, bool upper)
{
    static char tbuf[LINELEN];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you"); 
    else if (on(player, ISBLIND))
	strcpy(tbuf, "the monster");
    else
    {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

void 
hit (char *er, char *ee)
{
    char *s = "";

    if (fighting)
	return;
    addmsg(prname(er, TRUE));
    switch (rnd(4)) {
	case 0: s = " scored an excellent hit on ";
	when 1: s = " hit ";
	when 2: s = (er == 0 ? " have injured " : " has injured ");
	when 3: s = (er == 0 ? " swing and hit " : " swings and hits ");
    }
    addmsg(s);
    addmsg(prname(ee, FALSE));
    addmsg(".");
    endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */

void 
miss (char *er, char *ee)
{
    char *s = "";

    if (fighting)
	return;
    addmsg(prname(er, TRUE));
    switch (rnd(4))
    {
	case 0: s = (er == 0 ? " miss" : " misses");
	when 1: s = (er == 0 ? " swing and miss" : " swings and misses");
	when 2: s = (er == 0 ? " barely miss" : " barely misses");
	when 3: s = (er == 0 ? " just miss" : " just misses");
    }
    addmsg(s);
    addmsg(" %s", prname(ee, FALSE));
    addmsg(".");
    endmsg();
}

/*
 * save_throw:
 *	See if a creature save against something
 */
int 
save_throw(int which, struct thing *tp)
{
    int need;
    int ring_bonus = 0;
    int armor_bonus = 0;

    if (tp == &player)
    {
        ring_bonus = ring_value(R_PROTECT);

        if (cur_armor != NULL && (which == VS_WAND ||
            which == VS_MAGIC || which == VS_PETRIFICATION))
        {
            if (cur_armor->o_which == MITHRIL)
                armor_bonus += 5;
            armor_bonus += (armors[cur_armor->o_which].a_class
                    - cur_armor->o_ac);
        }

	if (difficulty < 2) {
	    /* do nothing */
	} else if (difficulty == 2 || level > 90) {
	    if (ring_bonus > 1) ring_bonus /= 2;
	    if (armor_bonus > 1) armor_bonus /= 2;
	} else if (difficulty > 2) {
	    if (ring_bonus > 1) 
		ring_bonus = max(1, ring_bonus / 3);
	    if (armor_bonus > 1)
		armor_bonus = max(1, armor_bonus / 3);
	}
    }

    need = 14 + which - tp->t_stats.s_lvl / 2 - ring_bonus - armor_bonus;

    /* the deeper you go, the harder it gets */
    if (tp == &player && difficulty > 2) {
	if (level > 20)
	    need += level / 20;
	else
	    need += rnd(2);
    }

    /* Roll of 20 always saves */

    if (need < 1)
        need = 1;
    else if (need > 20)
        need = 20;

    return(roll(1, 20) >= need);
}

/*
 * save:
 *	See if we save against various nasty things
 */

int 
save (int which)
{
    return save_throw(which, &player);
}

/*
 * dext_plus:
 *	compute to-hit bonus for dexterity
 */

int 
dext_plus (int dexterity)
{
    return ((dexterity-10)/3);
}


/*
 * dext_prot:
 *	compute armor class bonus for dexterity
 */

int 
dext_prot (int dexterity)
{
    return ((dexterity-9)/2);
}
/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */

int 
str_plus (int str)
{
    return((str-10)/3);
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */

int 
add_dam (int str)
{
    return((str-9)/2);
}

/*
 * hung_dam:
 *	Calculate damage depending on players hungry state
 */
int 
hung_dam ()
{
	int howmuch = 0;

	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	howmuch = 0;
		when F_WEAK:	howmuch = -1;
		when F_FAINT:	howmuch = -2;
	}
	return howmuch;
}

/*
 * raise_level:
 *	The player just magically went up a level.
 */

void 
raise_level ()
{
    if (pstats.s_exp < e_levels[player.t_ctype]/2)
	pstats.s_exp = e_levels[player.t_ctype];
    else pstats.s_exp *= 2;
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */

void 
thunk (struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap->o_type == WEAPON)
	msg("The %s hits %s.",weaps[weap->o_which].w_name,prname(mname, FALSE));
    else
	msg("You hit %s.", prname(mname, FALSE));
}

/*
 * mthunk:
 *	 A missile from a monster hits the player
 */

void 
m_thunk (struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap->o_type == WEAPON)
	msg("%s's %s hits you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s hits you.", prname(mname, TRUE));
}

/*
 * bounce:
 *	A missile misses a monster
 */

void 
bounce (struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap->o_type == WEAPON)
	msg("The %s misses %s.",weaps[weap->o_which].w_name,prname(mname,FALSE));
    else
	msg("You missed %s.", prname(mname, FALSE));
}

/*
 * m_bounce:
	  A missile from a monster misses the player
 */

void 
m_bounce (struct object *weap, char *mname)
{
    if (fighting || running)
	return;
    if (weap->o_type == WEAPON)
	msg("%s's %s misses you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s misses you.", prname(mname, TRUE));
}

/*
 * remove a monster from the screen
 */
void
remove(coord *mp, struct linked_list *item)
{
    struct linked_list *pitem, *nexti;
    struct thing *tp;

    if (item == NULL)
	return;

    tp = (struct thing *) ldata(item);

    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    while (pitem != NULL)
    {
	struct object *obj;

	nexti = next(tp->t_pack);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);

	/* it's baack! */
	if ((obj->o_type != WEAPON && rnd(20) == 0) ||
	    (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE))
	    fall(pitem, FALSE);
	else discard(pitem);

	pitem = nexti;
    }

    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, ((struct thing *) ldata(item))->t_oldch);
    detach(mlist, item);
    discard(item);
    monst_dead = TRUE;
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */

int 
is_magic (struct object *obj)
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return obj->o_ac != armors[obj->o_which].a_class;
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case STICK:
	case RING:
	case ARTIFACT:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */

void 
killed (struct linked_list *item, bool pr, bool points)
{
    struct thing *tp;
    struct linked_list *pitem, *nexti;

    if (!keep_fighting)
	fighting = FALSE;
    tp = (struct thing *) ldata(item);
    if (pr)
    {
	addmsg("You have defeated ");
	if (on(player, ISBLIND))
	    msg("it.");
	else
	{
	    msg("the %s.", monsters[tp->t_index].m_name);
	}
    }

    if (tp->t_index == nummonst+2) {
                        /* killed Friendly Fiend */
        struct linked_list *item;
        struct object *obj;

        levtype = NORMLEV;              /* hero can take objects now */
	if (difficulty >= 2) {
	    msg("The gods become very angry at you.");
	    levtype = THRONE;  /* wandering monsters may appear */
	    luck += 4;
	    for (item = pack; item != NULL; item = next(item)) {
		obj = OBJPTR(item);
		if (rnd(3) == 0 && !(obj->o_flags & ISPROT)) {
		    if (obj->o_flags & ISBLESSED)
			obj->o_flags &= ~ISBLESSED;
		    else
			obj->o_flags |= ISCURSED;
		    obj->o_flags &= ~(ISSILVER & ISZAPPED & ISVORPED
			& ISPOISON & CANRETURN & ISOWNED);
		    obj->o_flags |= ISLOST;
		    obj->o_hplus = 0;
		    obj->o_dplus = 0;
		    if (obj->o_type == RING && obj->o_ac != 11)
			obj->o_ac = 0;
		    else if (obj->o_type == ARMOR)
			obj->o_ac = armors[obj->o_which].a_class + 1;
		    else if (obj->o_type == STICK)
			obj->o_charges = 0;
		}
	    }
        }
    }

    /* Take care of any residual effects of the monster */
    check_residue(tp);

    if (points) {
	pstats.s_exp += tp->t_stats.s_exp;

	/*
	 * Do adjustments if we went up a level
	 */
	check_level();
    }

    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    while (pitem != NULL)
    {
	struct object *obj;

	nexti = next(tp->t_pack);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);

	if (points)
	    fall(pitem, FALSE);
	else discard(pitem);

	pitem = nexti;
    }

    /*
     * Get rid of the monster.
     */
    remove(&tp->t_pos, item);
}


/* Returns a pointer to the weapon the monster is wielding corresponding to
 * the given thrown weapon
 */

struct object *
wield_weap (struct object *weapon, struct thing *mp)
{
    int look_for;
    struct linked_list *pitem;

    if (weapon == NULL)
	return (NULL);
    switch (weapon->o_which) {
	case BOLT:	/* Find the crossbow */
	    look_for = CROSSBOW;
	    break;
	case ARROW:	/* Find the bow */
	    look_for = BOW;
	    break;
	case SILVERARROW:	/* Find the bow */
	    look_for = BOW;
	    break;
	case ROCK:	/* find the sling */
	    look_for = SLING;
	    break;
	default:
	    return(NULL);
    }

    for (pitem=mp->t_pack; pitem; pitem=next(pitem))
	if (((struct object *) ldata(pitem))->o_which == look_for)
	    return((struct object *) ldata(pitem));

    return(NULL);
}

