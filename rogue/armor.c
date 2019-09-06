/*
 * This file contains misc functions for dealing with armor
 */

#include "curses.h"
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let them put it on.
 */

void
wear(void) {
    struct linked_list *item;
    struct object *obj;

    if (cur_armor != NULL)
    {
	addmsg("You are already wearing some. ");
	addmsg("You'll have to take it off first");
	addmsg("!");
	endmsg();
	after = FALSE;
	return;
    }

    /* What does player want to wear? */
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;

    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR) {
	 msg("You can't wear that!");
	 return;
    }
    waste_time();
    msg("You are now wearing %s.", armors[obj->o_which].a_name);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the players back
 */

void 
take_off ()
{
    struct object *obj;

    if ((obj = cur_armor) == NULL)
    {
	msg("You aren't wearing armor!");
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    msg("You used to be wearing %c) %s.", pack_char(obj), inv_name(obj, TRUE));
    if (on(player, STUMBLER)) {
	msg("Your foot feels a lot better now.");
	turn_off(player, STUMBLER);
    }
}

