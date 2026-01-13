/*
 * Functions for dealing with problems brought about by weapons
 *
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"



/*
 * missile:
 *	Fire a missile in a given direction
 */

void 
missile (int ydelta, int xdelta, struct linked_list *item, struct thing *tp)
{
	struct object *obj ;
	struct linked_list *nitem ;

	/*
	* Get which thing we are hurling
	*/
	if (item == NULL) {
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (!dropcheck(obj) || is_current(obj)) {
		return ;
	}
	/*
	* Get rid of the thing. If it is a non-multiple item object, or
	* if it is the last thing, just drop it. Otherwise, create a new
	* item with a count of one.
	*/
	if (obj->o_count < 2) {
		detach(tp->t_pack, item) ;
		if (tp->t_pack == pack) {
			inpack-- ;
			freeletter(item);
		}
	} else {
		obj->o_count-- ;
		nitem = (struct linked_list *) new_item(sizeof *obj) ;
		obj = (struct object *) ldata(nitem) ;
		*obj = *((struct object *) ldata(item)) ;
		obj->o_count = 1 ;
		item = nitem ;
	}
	if (obj->o_type == ARTIFACT)
		has_artifact &= ~(1 << obj->o_which);
	if (obj->o_type == SCROLL && obj->o_which == S_SCARE) {
		if (obj->o_flags & ISBLESSED)
		    obj->o_flags &= ~ISBLESSED;
		else 
		    obj->o_flags |= ISCURSED;
	}
	updpack (FALSE);
	do_motion(obj, ydelta, xdelta, tp) ;
	/*
	* AHA! Here it has hit something. If it is a wall or a door,
	* or if it misses (combat) the monster, put it on the floor
	*/
	if (!hit_monster(unc(obj->o_pos), obj, tp)) {
	    if (obj->o_type == WEAPON && obj->o_which == GRENADE) {
		static coord fpos;

		msg("BOOOM!");
		aggravate();
		if (ntraps + 1 < MAXTRAPS + MAXTRAPS && 
				fallpos(&obj->o_pos, &fpos, FALSE, FALSE)) {
		    mvaddch(fpos.y, fpos.x, TRAPDOOR);
		    traps[ntraps].tr_type = TRAPDOOR;
		    traps[ntraps].tr_flags = ISFOUND;
		    traps[ntraps].tr_show = TRAPDOOR;
		    traps[ntraps].tr_pos.y = fpos.y;
		    traps[ntraps++].tr_pos.x = fpos.x;
		    light(&hero);
		}
		discard(item);
	    }
	    else if (obj->o_flags & ISLOST) {
		if (obj->o_type == WEAPON)
		    addmsg("The %s", weaps[obj->o_which]);
		else
		    addmsg(inv_name(obj, TRUE));
		msg(" vanishes in a puff of greasy smoke.");
		discard(item);
	    }
	    else {
		if (obj->o_flags & CANRETURN)
		    msg("You have %s.", inv_name(obj, TRUE));
		fall(item, TRUE) ;
	    }
	}
	else if (obj->o_flags & ISOWNED) {
		add_pack(item, TRUE);
		msg("You have %s.", inv_name(obj, TRUE));
	} else {
	    discard(item);
	}
	mvwaddch(cw, hero.y, hero.x, PLAYER) ;
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
void 
do_motion (struct object *obj, int ydelta, int xdelta, struct thing *tp)
{

	/*
	* Come fly with us ...
	*/
	obj->o_pos = tp->t_pos ;
	for ( ; ;) {
		int ch ;
		/*
		* Erase the old one
		*/
		if (!ce(obj->o_pos, tp->t_pos) &&
		    cansee(unc(obj->o_pos)) &&
		    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
			mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, show(obj->o_pos.y, obj->o_pos.x)) ;
		}
		/*
		* Get the new position
		*/
		obj->o_pos.y += ydelta ;
		obj->o_pos.x += xdelta ;
		if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR && !ce(obj->o_pos, hero)) {
			/*
			* It hasn't hit anything yet, so display it
			* If it alright.
			*/
			if (cansee(unc(obj->o_pos)) &&
			    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
				mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type) ;
				draw(cw) ;
			}
			continue ;
		}
		break ;
	}
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

void 
fall (struct linked_list *item, bool pr)
{
	struct object *obj ;
	struct room *rp ;
	static coord fpos ;

	obj = (struct object *) ldata(item) ;
	rp = roomin(&hero);
	if (obj->o_flags & CANRETURN) {
	    add_pack(item, TRUE);
	    return;
	}
	else if (fallpos(&obj->o_pos, &fpos,
			obj->o_type != WEAPON && !(obj->o_flags&ISMISL), FALSE)) {

		/*
		 * did it land on/under a monster?
		 */
		if (isalpha(winat(fpos.y, fpos.x))) {
		    struct linked_list *item;
		    struct thing *tp;
		    if ((item = find_mons(fpos.y, fpos.x))) {
			tp = (struct thing *) ldata(item);
			tp->t_oldch = obj->o_type;
		    }
		}

		if (obj->o_flags & CANBURN 
			&& ntraps + 1 < MAXTRAPS + MAXTRAPS) {
		    mvaddch(fpos.y, fpos.x, FIRETRAP);
		    traps[ntraps].tr_type = FIRETRAP;
		    traps[ntraps].tr_flags = ISFOUND;
		    traps[ntraps].tr_show = FIRETRAP;
		    traps[ntraps].tr_pos.y = fpos.y;
		    traps[ntraps++].tr_pos.x = fpos.x;
		    if (rp != NULL)
			rp->r_flags &= ~ISDARK;
		}
		else {
		    mvaddch(fpos.y, fpos.x, obj->o_type) ;
		    obj->o_pos = fpos ;
		    attach(lvl_obj, item) ;
		}
		if (rp != NULL &&
		    (!(rp->r_flags & ISDARK) ||
		    (rp->r_flags & HASFIRE))) {
			light(&hero) ;
			mvwaddch(cw, hero.y, hero.x, PLAYER) ;
		}
		return ;
	}
	if (pr && !fighting && !running) {
		if (obj->o_type == WEAPON)
			addmsg("The %s", weaps[obj->o_which].w_name);
		else
			addmsg(inv_name(obj, TRUE));
		msg(" vanishes as it hits the ground.");
	}
	discard(item) ;
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

void 
init_weapon (struct object *weap, int type)
{
	struct init_weps *iwp ;

	iwp = &weaps[type] ;
	weap->o_type = WEAPON;
	weap->o_damage = iwp->w_dam ;
	weap->o_hurldmg = iwp->w_hrl ;
	weap->o_launch = iwp->w_launch ;
	weap->o_flags = iwp->w_flags ;
	weap->o_weight = iwp->w_wght;
	if (weap->o_flags & ISMANY) {
		weap->o_count = rnd(8) + 8 ;
		weap->o_group = newgrp() ;
	} else {
		weap->o_count = 1 ;
	}
}

/*
 * Does the missile hit the monster
 */

int 
hit_monster (int y, int x, struct object *obj, struct thing *tp)
{
	static coord mp ;

	mp.y = y ;
	mp.x = x ;
	if (tp == &player) {
		/* Make sure there is a monster where it landed */
		if (!isalpha(mvwinch(mw, y, x))) {
			return(FALSE) ;
		}
		return(fight(&mp, obj, TRUE)) ;
	} else {
		if (!ce(mp, hero)) {
			return(FALSE) ;
		}
		return(attack(tp, obj, TRUE)) ;
	}
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *
num (int n1, int n2)
{
	static char numbuf[LINELEN] ;

	if (n1 == 0 && n2 == 0) {
		return "+0" ;
	}
	if (n2 == 0) {
		sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1) ;
	} else {
		sprintf(numbuf, "%s%d, %s%d", n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2) ;
	}
	return(numbuf) ;
}

/*
 * wield:
 *	Pull out a certain weapon
 */

void 
wield ()
{
	struct linked_list *item ;
	struct object *obj, *oweapon ;

	oweapon = cur_weapon ;
	if (!dropcheck(cur_weapon)) {
		cur_weapon = oweapon ;
		return ;
	}
	cur_weapon = oweapon ;
	if ((item = get_item("wield", WEAPON)) == NULL) {
		after = FALSE ;
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (obj->o_type != WEAPON) {
	    msg ("You can't wield that!");
	    return;
	}
	if (obj != cur_weapon && is_current(obj)) {
		after = FALSE ;
		return ;
	}
	msg("You are now wielding %s.", inv_name(obj, TRUE)) ;
	cur_weapon = obj ;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
int 
fallpos (
coord *pos,
coord *newpos,
bool scatter,  /* stuff may scatter further */
bool under)     /* even under the player */
{
    int y, x, cnt, ch ;

    cnt = 0 ;
    newpos->x = newpos->y = -1;
    for (y = pos->y - 1 ; y <= pos->y + 1 ; y++) {
	for (x = pos->x - 1 ; x <= pos->x + 1 ; x++) {
	/*
	 * look for an empty spot near the given position
	 */
	    if (y == hero.y && x == hero.x && !under) {
		continue;
	    }
	    if (((ch = winat(y, x)) == FLOOR
	      || (scatter && ch == PASSAGE)
	      || (scatter && isalpha(ch) &&
		  (mvwinch(stdscr, y, x) == FLOOR || mvwinch(stdscr, y, x) == PASSAGE)
		 )
	        ) && rnd(++cnt) == 0) {
		    newpos->y = y ;
		    newpos->x = x ;
	    }
	}
    }
    if (scatter && newpos->x == -1) {
	/*
	 * If we haven't found an empty spot, take one
	 * (virtual) step in each direction and try again
	 */
	for (y = pos->y - 1 ; y <= pos->y + 1 ; y++) {
		for (x = pos->x - 1 ; x <= pos->x + 1 ; x++) {
			ch = winat(y, x);
			if (ch != ' ' && ch != '|' && ch != '-' && ch != '&') {
			    coord trypos;
			    trypos.x = x;
			    trypos.y = y;
			    if (fallpos(&trypos, newpos, FALSE, TRUE)) {
				return(TRUE);
			    }
			}
		}
	}
    }
    return (newpos->x != -1) ;
}
