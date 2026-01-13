/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	8.0	(Bell Labs)	12/16/82
 */

#include "curses.h"
#include <errno.h>
#include <unistd.h>
#include "rogue.h"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

void 
_detach (struct linked_list **list, struct linked_list *item)
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

void 
_attach (struct linked_list **list, struct linked_list *item)
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

void 
_free_list (struct linked_list **ptr)
{
    struct linked_list *item;

    while (*ptr != NULL)
    {
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
    *ptr = NULL;
}

/*
 * discard:
 *	free up an item
 */

void 
discard (struct linked_list *item)
{
    total -= 2;
    item->l_next = NULL;
    item->l_prev = NULL;
    FREE(item->l_data);
    FREE(item);
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list *
new_item (int size)
{
    struct linked_list *item;

    if ((item = (struct linked_list *) my_malloc(sizeof *item)) == NULL)
	msg("Ran out of memory for header after %d items", total);
    if ((item->l_data = my_malloc(size)) == NULL)
	msg("Ran out of memory for data after %d items", total);
    item->l_next = item->l_prev = NULL;
    item->l_letter = '\0';
    return item;
}

char *
my_malloc (int size)
{
    char *space = (char *) malloc((size_t) size);
    static char errbuf[LINELEN];

    if (space == NULL) {
	sprintf((char *)errbuf,"Rogue ran out of memory (errno = %d, wanted = %d).",
		errno, size);
	fatal((char *)errbuf);
    }
    total++;
    return space;
}
