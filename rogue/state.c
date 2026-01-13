/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1993, 1995 Nicholas J. Kisseberth

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

/*
    Notes

        Should move all game variables into one place
        Should move save/restore code into save.c or some such
*/

#include <assert.h>
#include <stdlib.h>
#include "curses.h"
#include "rogue.h"
#include "state.h"

/*
     Variables for global game state.

     All variables that need to get saved when saving a game
     are defined in this file. Long term goal is to move many
     of these variables into a "struct level" data type of some
     kind... perhaps not, maybe struct game...

     Other global variables that don't need to get saved are
     kept in main.c.

     Other global variables that don't change during the course
     of a game are kept in urogue.c, monsdata.c, data.c.
*/

#define _X_ { 0,0,0,0,0 }
struct delayed_action
d_list[MAXDAEMONS] = {
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
};

char *prev_format = "UltraRogue Portable Save File Release 101e";
char *save_format = "UltraRogue Portable Save File Release 102e";
char *save_end    = "\nEnd of UltraRogue Game State\n";

/*
 * leave room for future growth
 */
int reserved2 = 0;
int reserved1 = 0;

struct linked_list  *fam_ptr = NULL;        /* A ptr to the familiar        */
struct linked_list  *curr_mons  = NULL;     /* The mons. currently moving   */
struct linked_list  *next_mons = NULL;      /* The mons. after curr_mons    */
struct object       *bag_obj  = NULL;       /* The Magic Bag, if we have it */


/****************************************************************************/
/* Portable Save State Code                                                 */
/*                                                                          */
/*    UltraRogue v1.04                                                      */
/****************************************************************************/

#define URS_STATS        0xABCD0001
#define URS_THING        0xABCD0002
#define URS_OBJECT       0xABCD0003
#define URS_MAGIC        0xABCD0004
#define URS_KNOWS        0xABCD0005
#define URS_GUESSES      0xABCD0006
#define URS_STACKOBJECT  0xABCD0007
#define URS_BAGOBJECT    0xABCD0008
#define URS_MONSTERLIST  0xABCD0009
#define URS_MONSTERSTATS 0xABCD000A
#define URS_MONSTER      0xABCD000B
#define URS_TRAP         0xABCD000C
#define URS_WINDOW       0xABCD000D
#define URS_DAEMONS      0xABCD000E
#define URS_ARTIFACT     0xABCD000F

void
ur_write(FILE *savef, void *ptr, int size)
{
    if (size == 0)
        return;

    fwrite(ptr,size,1,savef);
}

void
ur_read(FILE *savef, void *ptr, int size)
{
    if (size == 0)
        return;

    (void) fread(ptr,size,1,savef);
}

void
ur_write_int(FILE *savef, int c)
{
    ur_write(savef,&c,sizeof(int));
}

int
ur_read_int(FILE *savef)
{
    int i;

    ur_read(savef, &i, sizeof(int));

    return(i);
}

void
ur_write_short(FILE *savef, short c)
{
    ur_write(savef,&c,sizeof(short));
}

short
ur_read_short(FILE *savef)
{
    short s;

    ur_read(savef, &s, sizeof(short));

    return(s);
}

void
ur_write_long(FILE *savef, long c)
{
    ur_write(savef,&c,sizeof(long));
}

long
ur_read_long(FILE *savef)
{
    long l;

    ur_read(savef, &l, sizeof(long));

    return(l);
}

void
ur_write_char(FILE *savef, char c)
{
    ur_write(savef,&c,sizeof(char));
}

char
ur_read_char(FILE *savef)
{
    char c;

    ur_read(savef, &c, sizeof(char));

    return(c);
}

void
ur_write_string(FILE *savef, char *s)
{
    long len;

    len = (s == NULL) ? 0L : strlen(s) + 1 ;

    ur_write_long(savef, len);
    ur_write(savef,s,len);
}


char *
ur_read_string(FILE *savef)
{
    long  len;
    char   *buf;

    len = ur_read_long(savef);

    if (len == 0)
        return(NULL);

    buf = ALLOC(len);

    if (buf == NULL)     /* Should flag a global error condition... */
        return(NULL);

    ur_read(savef,buf,len);

    return(buf);
}

void
ur_write_coord(FILE *savef, coord c)
{
    ur_write_int(savef, c.x);
    ur_write_int(savef, c.y);
}

coord
ur_read_coord(FILE *savef)
{
    coord c;

    c.x = ur_read_int(savef);
    c.y = ur_read_int(savef);

    return(c);
}

void
ur_write_room(FILE *savef, struct room *r)
{
    int i;

    ur_write_coord(savef, r->r_pos);
    ur_write_coord(savef, r->r_max);

    for(i=0; i<MAXDOORS; i++)
        ur_write_coord(savef, r->r_exit[i]);

    ur_write_long(savef, r->r_flags);
    ur_write_int(savef, r->r_nexits);
    ur_write_short(savef, r->r_fires);
}

struct room *
ur_read_room(FILE *savef)
{
    struct room *r;
    int i;

    r = ALLOC( sizeof(struct room) );

    r->r_pos = ur_read_coord(savef);
    r->r_max = ur_read_coord(savef);

    for(i=0; i<MAXDOORS; i++)
        r->r_exit[i] = ur_read_coord(savef);

    r->r_flags = ur_read_long(savef);
    r->r_nexits = ur_read_int(savef);
    r->r_fires = ur_read_short(savef);

    return(r);
}

void
ur_write_object(FILE *savef, struct object *o)
{

    ur_write_long(savef,      URS_OBJECT);
    ur_write_int(savef,       o->o_type);
    ur_write_coord(savef,     o->o_pos);
    ur_write_char(savef,      o->o_launch);
    ur_write_string(savef,    o->o_damage);
    ur_write_string(savef,    o->o_hurldmg);
    ur_write_int(savef,       o->o_count);
    ur_write_int(savef,       o->o_which);
    ur_write_int(savef,       o->o_hplus);
    ur_write_int(savef,       o->o_dplus);
    ur_write_int(savef,       o->o_ac);
    ur_write_long(savef,      o->o_flags);
    ur_write_int(savef,       o->o_group);
    ur_write_int(savef,       o->o_weight);
    ur_write(savef,          &o->o_mark[0], MARKLEN);
    ur_write_long(savef,      o->o_worth);

    if (o->o_type == ARTIFACT)
	ur_write_artifact(savef, &o->art_stats);
}

struct object *
ur_read_object(FILE *savef)
{
    struct object *o;
    long id;

    o = ALLOC(sizeof(struct object));

    if (o == NULL)
        return(NULL);

    memset(o,0,sizeof(struct object));

    id = ur_read_long(savef);

    assert(id == (long) URS_OBJECT);

    o->o_type = ur_read_int(savef);
    o->o_pos  = ur_read_coord(savef);
    o->o_launch = ur_read_char(savef);
    o->o_damage = ur_read_string(savef);
    o->o_hurldmg = ur_read_string(savef);
    o->o_count = ur_read_int(savef);
    o->o_which = ur_read_int(savef);
    o->o_hplus = ur_read_int(savef);
    o->o_dplus = ur_read_int(savef);
    o->o_ac = ur_read_int(savef);
    o->o_flags = ur_read_long(savef);
    o->o_group = ur_read_int(savef);
    o->o_weight = ur_read_int(savef);
    ur_read(savef, &o->o_mark[0], MARKLEN);
    o->o_worth = ur_read_long(savef);

    if (o->o_type == ARTIFACT) {
	struct artifact *a;
	a = ur_read_artifact(savef);
	o->art_stats.ar_flags = a->ar_flags;
	o->art_stats.ar_rings = a->ar_rings;
	o->art_stats.ar_potions = a->ar_potions;
	o->art_stats.ar_scrolls = a->ar_scrolls;
	o->art_stats.ar_wands = a->ar_wands;
	o->art_stats.t_art = a->t_art;
	FREE(a);
    }

    return(o);
}

int
list_size(struct linked_list *l)
{
    int cnt=0;

    if (l == NULL)
        return(0);

    while(l != NULL)
    {
        cnt++;
        l = l->l_next;
    }

    return(cnt);
}

int
find_thing_index(struct linked_list *l, struct thing *item)
{
    int cnt=0;

    while(l != NULL)
    {
        if (item == (struct thing *) l->l_data)
            return(cnt+1);

        cnt++;
        l = l->l_next;
    }

    return(0);
}


int
find_list_index(struct linked_list *l, struct object *item)
{
    int cnt=0;

    if (l == NULL)
        return(-1);

    while(l != NULL)
    {
        if (item == (struct object *) l->l_data)
            return(cnt+1);

        cnt++;
        l = l->l_next;
    }

    return(0);
}

struct object *
find_object(struct linked_list *list, int num)
{
    int cnt = 0;
    struct linked_list *l = list;
	
    if ( (num < 1) || (list == NULL) )
        return(NULL);

    num--;

    for(cnt = 0; cnt < num; cnt++)
    {
        if ( l == NULL )
            return(NULL);

        l = l->l_next;
    }

    return((struct object *) l->l_data);
}

void
dump_list(struct linked_list *list)
{
    int cnt = 0;
    struct linked_list *l = list;
	
    printf("Dumping list...\n");
    while(l != NULL)
    {
        printf("  %d %c\n", cnt, l->l_letter);
        l = l->l_next;
	cnt++;
    }
}

struct thing *
find_thing(struct linked_list *list, int num)
{
    int cnt = 0;
    struct linked_list *l = list;

    if ( (num < 1) || (list == NULL) )
        return(NULL);
    num--;

    for(cnt = 0; cnt < num; cnt++)
    {
        if (l == NULL)
            return(NULL);

        l = l->l_next;
    }

    return((struct thing *) l->l_data);
}


void
ur_write_bag(int isbag, FILE *savef, struct linked_list *l)
{
    int cnt;

    ur_write_long(savef, URS_BAGOBJECT);

    ur_write_int(savef, cnt = list_size(l) );

    if (cnt == 0)
        return;

    while(l != NULL)
    {
        ur_write_object(savef, (struct object *) l->l_data);
	if (isbag)
	    ur_write_char(savef, l->l_letter);
        l = l->l_next;
    }
}

struct linked_list *
ur_read_bag(int isbag, FILE *savef)
{
    long id;
    int i,cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    id = ur_read_long(savef);

    assert( id == (long) URS_BAGOBJECT );

    cnt = ur_read_int(savef);

    for(i = 0; i < cnt; i++)
    {
        l         = (struct linked_list *) my_malloc(sizeof(struct linked_list));
        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        l->l_data =  (char *) ur_read_object(savef);
        if (isbag) {
	    l->l_letter =  ur_read_char(savef);
	    /* printf("%d %c\n", i, l->l_letter); */
	}

        if (previous == NULL)
            head = l;

        previous = l;
    }

    if (l != NULL)
        l->l_next = NULL;

    return(head);
}

void
ur_write_monsters(FILE *savef, struct linked_list *l)
{
    int cnt;

    ur_write_long(savef, URS_MONSTERLIST);

    cnt = list_size(l);

    ur_write_int(savef, cnt);

    if (cnt < 1)
        return;

    while(l != NULL)
    {
        ur_write_thing(savef, (struct thing *) l->l_data);
        l = l->l_next;
    }
}

struct linked_list *
ur_read_monsters(FILE *savef)
{
    long id;
    int i,cnt;
    struct linked_list *l=NULL, *previous = NULL, *head = NULL;

    id = ur_read_long(savef);

    assert(id == (long) URS_MONSTERLIST);

    cnt = ur_read_int(savef);

    if (cnt == 0)
        return(NULL);

    for(i = 0; i < cnt; i++)
    {
        l         = (struct linked_list *) my_malloc(sizeof(struct linked_list));

        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        l->l_data= (char *) ur_read_thing(savef);

        if (previous == NULL)
            head = l;

        previous = l;
    }

    if (l != NULL)
        l->l_next = NULL;

    return(head);
}

void
ur_write_monster_stats(FILE *savef, struct mstats *m)
{
    ur_write_long(savef, URS_MONSTERSTATS);
    ur_write_short(savef, m->s_str);
    ur_write_long(savef, m->s_exp);
    ur_write_int(savef, m->s_lvl);
    ur_write_int(savef, m->s_arm);
    ur_write_string(savef, m->s_hpt);
    ur_write_string(savef, m->s_dmg);
}

struct mstats *
ur_read_monster_stats(FILE *savef)
{
    long id;
    struct mstats *m;

    id = ur_read_long(savef);

    assert(id == (long) URS_MONSTERSTATS);

    m = ALLOC( sizeof(struct mstats) );

    m->s_str = ur_read_short(savef);
    m->s_exp = ur_read_long(savef);
    m->s_lvl = ur_read_int(savef);
    m->s_arm = ur_read_int(savef);
    m->s_hpt = ur_read_string(savef);
    m->s_dmg = ur_read_string(savef);

    return(m);
}

void
ur_write_monster(FILE *savef, struct monster *m)
{
    int i;

    ur_write_long(savef, URS_MONSTER);
    ur_write_string(savef, m->m_name);
    ur_write_short(savef, m->m_carry);
    ur_write_int(savef, m->m_normal);
    ur_write_int(savef, m->m_wander);
    ur_write_char(savef, m->m_appear);
    ur_write_string(savef, m->m_intel);

    for(i = 0; i < NM_FLAGS; i++)
        ur_write_long(savef, m->m_flags[i]);

    ur_write_string(savef, m->m_typesum);
    ur_write_short(savef, m->m_numsum);
    ur_write_short(savef, m->m_add_exp);
    ur_write_monster_stats(savef, &m->m_stats);
}

struct monster *
ur_read_monster(FILE *savef)
{
    struct monster *m;
    struct mstats *mstats;

    m = ALLOC( sizeof(struct monster) );

    m->m_name = ur_read_string(savef);
    m->m_carry = ur_read_short(savef);
    m->m_normal = ur_read_int(savef);
    m->m_wander = ur_read_int(savef);
    m->m_appear = ur_read_char(savef);
    m->m_intel = ur_read_string(savef);
    ur_read(savef, &m->m_flags[0], NM_FLAGS*sizeof(long));
    m->m_typesum = ur_read_string(savef);
    m->m_numsum = ur_read_short(savef);
    m->m_add_exp = ur_read_short(savef);

    mstats = ur_read_monster_stats(savef);

    m->m_stats = *mstats;
    FREE(mstats);

    return(m);
}

void
ur_write_trap(FILE *savef, struct trap *t)
{
    ur_write_long(savef, URS_TRAP);
    ur_write_coord(savef, t->tr_pos);
    ur_write_long(savef, t->tr_flags);
    ur_write_char(savef, t->tr_type);
    ur_write_char(savef, t->tr_show);
}

struct trap *
ur_read_trap(FILE *savef)
{
    struct trap *t;
    long id;

    id = ur_read_long(savef);

    assert(id == (long) URS_TRAP);

    t = ALLOC( sizeof(struct trap));

    t->tr_pos = ur_read_coord(savef);
    t->tr_flags = ur_read_long(savef);
    t->tr_type = ur_read_char(savef);
    t->tr_show = ur_read_char(savef);

    return(t);
}

void
ur_write_artifact(FILE *savef, struct artifact *a)
{
    ur_write_long(savef, URS_ARTIFACT);
    ur_write_long(savef, a->ar_flags);
    ur_write_long(savef, a->ar_rings);
    ur_write_long(savef, a->ar_potions);
    ur_write_long(savef, a->ar_scrolls);
    ur_write_long(savef, a->ar_wands);
    ur_write_bag(1,savef, a->t_art);
}

struct artifact *
ur_read_artifact(FILE *savef)
{
    struct artifact *a;
    long id;

    id = ur_read_long(savef);

    assert(id == (long) URS_ARTIFACT);

    a = ALLOC( sizeof(struct artifact));

    a->ar_flags = ur_read_long(savef);
    a->ar_rings = ur_read_long(savef);
    a->ar_potions = ur_read_long(savef);
    a->ar_scrolls = ur_read_long(savef);
    a->ar_wands = ur_read_long(savef);
    a->t_art = ur_read_bag(1,savef);

    return(a);
}

void
ur_write_stats(FILE *savef, struct stats *s)
{
    ur_write_long(savef, URS_STATS);
    ur_write_string(savef, s->s_dmg);
    ur_write_long(savef, s->s_exp);
    ur_write_int(savef, s->s_hpt);
    ur_write_int(savef, s->s_pack);
    ur_write_int(savef, s->s_carry);
    ur_write_int(savef, s->s_lvl);
    ur_write_int(savef, s->s_arm);
    ur_write_short(savef, s->s_str);
    ur_write_short(savef, s->s_intel);
    ur_write_short(savef, s->s_wisdom);
    ur_write_short(savef, s->s_dext);
    ur_write_short(savef, s->s_const);
    ur_write_short(savef, s->s_charisma);
}

struct stats *
ur_read_stats(FILE *savef)
{
    struct stats *s;
    long id;

    id = ur_read_long(savef);

    assert(id == (long) URS_STATS);

    s = ALLOC(sizeof(struct stats));

    s->s_dmg = ur_read_string(savef);
    s->s_exp = ur_read_long(savef);
    s->s_hpt = ur_read_int(savef);
    s->s_pack = ur_read_int(savef);
    s->s_carry = ur_read_int(savef);
    s->s_lvl = ur_read_int(savef);
    s->s_arm = ur_read_int(savef);
    s->s_str = ur_read_short(savef);
    s->s_intel = ur_read_short(savef);
    s->s_wisdom = ur_read_short(savef);
    s->s_dext = ur_read_short(savef);
    s->s_const = ur_read_short(savef);
    s->s_charisma = ur_read_short(savef);

    return(s);
}

void
ur_write_thing(FILE *savef, struct thing *t)
{
    int i;

    ur_write_long(savef, URS_THING);
    ur_write_bag(1, savef, t->t_pack);
    ur_write_stats(savef, &t->t_stats);
    ur_write_stats(savef, &t->maxstats);

    ur_write_coord(savef, t->t_pos);
    ur_write_coord(savef, t->t_oldpos);

    /* save destination, i.e. monster chasing player,
       but not player fleeing from monster */

    if (t->t_dest != NULL || t->t_type == 0) {
#if 0
     && t->t_dest->x == hero.x
     && t->t_dest->y == hero.y) {
#endif
	ur_write_int(savef, 1);
    } else {
	ur_write_int(savef, -1);
    }

    for(i = 0; i < NT_FLAGS; i++)
        ur_write_long(savef, t->t_flags[i]);

    ur_write_int(savef, t->t_turn);
    ur_write_int(savef, t->t_wasshot);
    ur_write_short(savef, t->t_ctype);
    ur_write_short(savef, t->t_index);
    ur_write_short(savef, t->t_no_move);
    ur_write_short(savef, t->t_quiet);
    ur_write_short(savef, t->t_doorgoal);
    ur_write_char(savef, t->t_type);
    ur_write_char(savef, t->t_disguise);
    ur_write_char(savef, t->t_oldch);
}

struct thing *
ur_read_thing(FILE *savef)
{
    long id;
    int i;
    struct thing *t;
    struct stats *s;
    int dest;

    id = ur_read_long(savef);

    assert(id == (long) URS_THING);

    t = ALLOC( sizeof(struct thing) );

    t->t_pack = ur_read_bag(1, savef);

    s = ur_read_stats(savef);
    t->t_stats = *s;
    FREE(s);

    s = ur_read_stats(savef);
    t->maxstats = *s;
    FREE(s);

    t->t_pos = ur_read_coord(savef);
    t->t_oldpos = ur_read_coord(savef);

    /* restore destination, e.g. monster chasing player */
    dest = ur_read_int(savef);
    if (dest==-1) {
	t->t_dest = &(t->t_pos);
    } else {
	t->t_dest = &hero;
    }

    for(i = 0; i < NT_FLAGS; i++)
        t->t_flags[i] = ur_read_long(savef);

    t->t_turn = ur_read_int(savef);
    t->t_wasshot = ur_read_int(savef);
    t->t_ctype = ur_read_short(savef);
    t->t_index = ur_read_short(savef);
    t->t_no_move = ur_read_short(savef);
    t->t_quiet = ur_read_short(savef);
    t->t_doorgoal = ur_read_short(savef);
    t->t_type = ur_read_char(savef);
    t->t_disguise = ur_read_char(savef);
    t->t_oldch = ur_read_char(savef);

    return(t);
}

/*
 * Note that there's an off-by-one error in these two routines on Linux,
 * so we don't read/write the last row and column of the window.
 *
 * Fixing this would break backwards compatibility with old save files.
 *
 * Also note that win->_maxx and win->_maxy differ in ncurses and pdcurses,
 * so we shouldn't use them, but that also breaks backwards compatibility.
 */

void
ur_write_window(FILE *savef, WINDOW *win)
{
    int i,j;
#ifdef PDCURSES
    int y = getmaxy(win);
    int x = getmaxx(win);
#else
    int y = getmaxy(win) - 1;
    int x = getmaxx(win) - 1;
#endif

    ur_write_long(savef, URS_WINDOW);

    ur_write_short(savef, y);
    ur_write_short(savef, x);

    for(i=0; i < y; i++)
        for(j = 0; j < x; j++)
            ur_write_short(savef, mvwinch(win,i,j));
}

void
ur_read_window(FILE *savef, WINDOW *win)
{
    int i,j;
#ifdef PDCURSES
    int y = getmaxy(win);
    int x = getmaxx(win);
#else
    int y = getmaxy(win) - 1;
    int x = getmaxx(win) - 1;
#endif
    int maxy, maxx;
    long id;

    id = ur_read_long(savef);

    assert(id == (long) URS_WINDOW);

    maxy = ur_read_short(savef);
    maxx = ur_read_short(savef);

    if (y != maxy || x != maxx) {
	char oops[200];
	endwin();
#ifdef PDCURSES
	sprintf(oops, "Terminal dimensions (%dx%d) do not match saved game (%dx%d).",
		x,y,maxx,maxy);
	printf("%s\nPlease set window size and try again.\n", oops);
	printf("I.e.:\n");
	printf("   set PDC_LINES=%d\n", maxy);
	printf("   set PDC_COLS=%d\n", maxx);
#else
	sprintf(oops, "Terminal dimensions (%dx%d) do not match saved game (%dx%d).",
		x+1,y+1,maxx+1,maxy+1);
	printf("%s\nPlease resize window and try again.\n", oops);
#endif
	exit(1);
    }


    for(i=0; i < maxy; i++)
        for(j = 0; j < maxx; j++)
            mvwaddch(win,i,j,ur_read_short(savef));
}

void
ur_write_daemons(FILE *savef)
{
    int i;
    int thing;

    ur_write_long(savef, URS_DAEMONS);

    for(i = 0; i < MAXDAEMONS; i++)
    {
        ur_write_int(savef, d_list[i].d_type );
        ur_write_int(savef, d_list[i].d_when );
        ur_write_int(savef, d_list[i].d_id);

        if (d_list[i].d_id == DAEMON_DOCTOR &&
	    d_list[i].d_type == DAEMON)
            thing = find_thing_index(mlist, d_list[i].d_arg);
	else
	    thing = 0;
        ur_write_int(savef, thing);

        ur_write_int(savef, d_list[i].d_time );
    }
}

void
ur_read_daemons(FILE *savef)
{
    long id;
    int i, thing;
    demoncnt = 0;
	
    id = ur_read_long(savef);

    assert(id == (long) URS_DAEMONS);

    for(i = 0; i < MAXDAEMONS; i++)
    {
        d_list[i].d_type = ur_read_int(savef);
        d_list[i].d_when = ur_read_int(savef);
        d_list[i].d_id = ur_read_int(savef);
        thing = ur_read_int(savef);
        d_list[i].d_time = ur_read_int(savef);

        if (d_list[i].d_type != EMPTY &&
	    d_list[i].d_id == DAEMON_DOCTOR &&
	    d_list[i].d_type == DAEMON )
        {
            if (thing == 0)
                d_list[i].d_arg = &player;
            else
                d_list[i].d_arg = find_thing(mlist, thing);

            if (d_list[i].d_arg == NULL)
                d_list[i].d_type = EMPTY;
        }

        if (d_list[i].d_type != EMPTY)
            demoncnt++;
    }
}

void
save_file(FILE *savef)
{
    int i,weapon,armor,ring,room= -1,monster;

    ur_write_string(savef, save_format);

    ur_write_string(savef,"\nScrolls\n");
    for(i = 0; i < MAXSCROLLS; i++) {
        ur_write_string(savef,s_names[i]);
        ur_write_short(savef,(short) s_know[i]);
	if (s_guess[i] == NULL)
	    ur_write_string(savef,"");
	else
	    ur_write_string(savef,s_guess[i]);
    }

    ur_write_string(savef,"\nPotions\n");
    for(i = 0; i < MAXPOTIONS; i++) {
        ur_write_string(savef,p_colors[i]);
        ur_write_short(savef,(short) p_know[i]);
	if (p_guess[i] == NULL)
	    ur_write_string(savef,"");
	else
	    ur_write_string(savef,p_guess[i]);
    }

    ur_write_string(savef,"\nRings\n");
    for(i = 0; i < MAXRINGS; i++) {
        ur_write_string(savef,r_stones[i]);
        ur_write_short(savef,(short) r_know[i]);
	if (r_guess[i] == NULL)
	    ur_write_string(savef,"");
	else
	    ur_write_string(savef,r_guess[i]);
    }

    ur_write_string(savef,"\nSticks\n");
    for(i = 0; i < MAXSTICKS; i++) {
        ur_write_string(savef,ws_made[i]);
        ur_write_short(savef,(short) ws_know[i]);
	if (ws_guess[i] == NULL)
	    ur_write_string(savef,"");
	else
	    ur_write_string(savef,ws_guess[i]);
    }

    ur_write_string(savef,"\nStick types\n");
    for(i = 0; i < MAXSTICKS; i++)
        ur_write_string(savef,ws_type[i]);

    ur_write_string(savef, "\nTraps on this level\n");
    ur_write_int(savef, MAXTRAPS);
    for(i = 0; i < MAXTRAPS; i++)
        ur_write_trap(savef, &traps[i]);

    ur_write_string(savef,"\nRooms on this level\n");
    ur_write_int(savef, MAXROOMS);
    for(i = 0; i < MAXROOMS; i++)
    {
        ur_write_room(savef, &rooms[i]);

        if (&rooms[i] == oldrp)
            room = i;
    }
    ur_write_int(savef,room); /* save for recovery of oldrp */

    ur_write_string(savef,"\nThe Rogue\n");
    ur_write_thing(savef, &player);

    ur_write_string(savef,"\nObjects on this level\n");
    ur_write_bag(0, savef, lvl_obj);

    ur_write_string(savef,"\nRogue's Familiar, if any \n");
    ur_write_monsters(savef, fam_ptr);

    ur_write_string(savef,"\nMonsters on this level\n");
    ur_write_monsters(savef, mlist);

    monster = find_thing_index(mlist, beast);
    ur_write_int(savef, monster);

    /* which monsters still exist, eg. not genocided */
    ur_write_int(savef, nummonst);
    for(i = 0; i <= nummonst+2; i++) {
        ur_write_short(savef, (short) monsters[i].m_normal);
        ur_write_short(savef, (short) monsters[i].m_wander);
    }

    ur_write_string(savef,"\nItems in use by rogue\n");
    weapon = find_list_index(player.t_pack, cur_weapon);
    armor  = find_list_index(player.t_pack, cur_armor);
    ur_write_int(savef, weapon);
    ur_write_int(savef, armor);

    for(i=0; i < NFINGERS; i++)
    {
        if (cur_ring[i] == NULL)
	    ring = -1;
	else
            ring = find_list_index(player.t_pack, cur_ring[i]);

        ur_write_int(savef, ring);
    }

    ur_write_string(savef,"\nActive Daemons and Fuses\n");
    ur_write_daemons(savef);

    ur_write_string(savef, "\nMisc\n");

    for(i = 0; i < NMSGS; i++)
        ur_write_string(savef, msgbuf[i]);

    ur_write_int(savef, msg_index);
    ur_write_int(savef, foodlev);
    ur_write_int(savef, ntraps);
    ur_write_int(savef, dnum);
    ur_write_int(savef, max_level);
    ur_write_int(savef, lost_dext);
    ur_write_int(savef, no_command);
    ur_write_int(savef, level);
    ur_write_int(savef, see_dist);
    ur_write_int(savef, no_food);
    ur_write_int(savef, count);
    ur_write_int(savef, food_left);
    ur_write_int(savef, group);
    ur_write_int(savef, hungry_state);
    ur_write_int(savef, infest_dam);
    ur_write_int(savef, lost_str);
    ur_write_int(savef, hold_count);
    ur_write_int(savef, trap_tries);
    ur_write_int(savef, has_artifact);
    ur_write_int(savef, picked_artifact);
    ur_write_int(savef, active_artifact);
    ur_write_int(savef, luck);
    ur_write_int(savef, resurrect);
    ur_write_int(savef, inpack);
    ur_write_int(savef, inbag);
    ur_write_char(savef, PLAYER);
    ur_write_char(savef, take);
    ur_write_char(savef, runch);
    ur_write_int(savef, char_type);
    ur_write_int(savef,  pool_teleport);
    ur_write_int(savef, inwhgt);
    ur_write_int(savef, after);
    ur_write_int(savef, showcursor);
    ur_write_int(savef, canwizard);
    ur_write_int(savef, playing);
    ur_write_int(savef, running);
    ur_write_int(savef, fighting);
    ur_write_int(savef, wizard);
    ur_write_int(savef, moving);
    ur_write_coord(savef, delta);
    ur_write_int(savef, levtype);
    ur_write_int(savef, purse);
    ur_write_int(savef, total);
    ur_write_int(savef, trader);
    ur_write_int(savef, spell_power);
    ur_write_int(savef, pray_time);
    ur_write_int(savef, pack_index-pack_letters);
    ur_write_int(savef, bag_index-bag_letters);
    ur_write_string(savef,pack_letters);
    ur_write_string(savef,bag_letters);

    ur_write_string(savef,"\nWindows\n");
    ur_write_window(savef, cw);
    ur_write_window(savef, hw);
    ur_write_window(savef, mw);
    ur_write_window(savef, stdscr);

    ur_write_string(savef,"\nGame Options\n");
    ur_write_int(savef, difficulty);
    ur_write_int(savef, doorstop);
    ur_write_int(savef, jump);
    ur_write_int(savef, firstmove);
    ur_write_int(savef, askme);
    ur_write_int(savef, cutcorners);

    ur_write_int(savef, autopickup);
    ur_write_int(savef, autosave);
    ur_write_int(savef, use_mouse);
    ur_write_int(savef, reserved2);
    ur_write_int(savef, reserved1);  /* for future use */

    ur_write_int(savef, game_id);
    ur_write_string(savef,whoami);
    ur_write_string(savef,fruit);
    ur_write_string(savef,""); 	/* was file_name */
    ur_write_string(savef,"");  /* was score_file */

    ur_write_string(savef,save_end);	/* for DUMPSTRING */
    ur_write_string(savef,save_end);	/* to verify end of file */
}

#define DUMPSTRING { str = ur_read_string(savef); /*if (!isendwin()) endwin(); fprintf(stderr,"%s",str); refresh();*/ FREE(str); }

int
restore_file(FILE *savef)
{
    int i,j;
    char *str;
    struct trap *t;
    struct room *r;
    struct thing *p;
    bool compatibility_mode = FALSE;

    str = ur_read_string(savef);

    if (strcmp(str, save_format) == 0) {
	/* good */
    } else if (strcmp(str, prev_format) == 0) {
	compatibility_mode = TRUE;
    } else {
	endwin();
        printf("Save Game Version: %s\n", str);
        printf("Real Game Version: %s\n", save_format);
        printf("Sorry, versions don't match.\n");
        return(FALSE);
    }
    FREE(str);

    DUMPSTRING
    for(i=0; i < MAXSCROLLS; i++) {
	if (s_names[i] != NULL) FREE(s_names[i]);
        s_names[i] = ur_read_string(savef);
        s_know[i] = (bool) ur_read_short(savef);
	if (!compatibility_mode) {
	    str = ur_read_string(savef);
	    if (*str == '\0')
		FREE(str);
	    else
		s_guess[i] = str;
	}
    }

    DUMPSTRING
    for(i=0; i < MAXPOTIONS; i++) {
	if (p_colors[i] != NULL) FREE(p_colors[i]);
        p_colors[i] = ur_read_string(savef);
        p_know[i] = (bool) ur_read_short(savef);
	if (!compatibility_mode) {
	    str = ur_read_string(savef);
	    if (*str == '\0')
		FREE(str);
	    else
		p_guess[i] = str;
	}
    }

    DUMPSTRING
    for(i=0; i < MAXRINGS; i++) {
        r_stones[i] = ur_read_string(savef);
        r_know[i] = (bool) ur_read_short(savef);
	if (!compatibility_mode) {
	    str = ur_read_string(savef);
	    if (*str == '\0')
		FREE(str);
	    else
		r_guess[i] = str;
	}
    }

    DUMPSTRING
    for(i=0; i < MAXSTICKS; i++) {
        ws_made[i] = ur_read_string(savef);
        ws_know[i] = (bool) ur_read_short(savef);
	if (!compatibility_mode) {
	    str = ur_read_string(savef);
	    if (*str == '\0')
		FREE(str);
	    else
		ws_guess[i] = str;
	}
    }

    DUMPSTRING
    for(i=0; i < MAXSTICKS; i++)
        ws_type[i] = ur_read_string(savef);

    DUMPSTRING
    i = ur_read_int(savef);
    assert(i == MAXTRAPS);

    for(i=0;i<MAXTRAPS;i++)
    {
        t = ur_read_trap(savef);
        traps[i] = *t;
        FREE(t);
    }

    DUMPSTRING
    i = ur_read_int(savef);
    assert(i == MAXROOMS);

    for(i=0;i<MAXROOMS;i++)
    {
        r = ur_read_room(savef);
        rooms[i] = *r;
        FREE(r);
    }
    i = ur_read_int(savef);
    oldrp = &rooms[i];

    DUMPSTRING
    p = ur_read_thing(savef);
    player = *p;
    FREE(p);

    DUMPSTRING
    lvl_obj = ur_read_bag(0, savef);
/* dump_list(player.t_pack); */

    DUMPSTRING
    fam_ptr = ur_read_monsters(savef);

    DUMPSTRING
    mlist = ur_read_monsters(savef);
    i = ur_read_int(savef);
    beast = find_thing(mlist, i);

    /* which monsters still exist, eg. not genocided */
    i = ur_read_int(savef);
    if (i != nummonst) {
	endwin();
	printf("Saved game has %d monsters, expected %d.\n", i+2, nummonst+2);
	return(FALSE);
    }
    for(i = 0; i <= nummonst+2; i++) {
	monsters[i].m_normal = (bool) ur_read_short(savef);
	monsters[i].m_wander = (bool) ur_read_short(savef);
    }

    DUMPSTRING
    i = ur_read_int(savef);
    cur_weapon = find_object(player.t_pack, i);

    i = ur_read_int(savef);
    cur_armor = find_object(player.t_pack, i);

    for(j=0; j < NFINGERS; j++)
    {
        i = ur_read_int(savef);
        if (i == -1)
            cur_ring[j] = NULL;
        else
            cur_ring[j] = find_object(player.t_pack, i);
    }

    DUMPSTRING
    ur_read_daemons(savef);

    DUMPSTRING
    for(i = 0; i < NMSGS; i++)
    {
        str = ur_read_string(savef);
	if (str != NULL)
	    strcpy(&msgbuf[i][0],str);
        FREE(str);
    }

    msg_index = ur_read_int(savef);

    foodlev  = ur_read_int(savef);
    ntraps = ur_read_int(savef);
    dnum = ur_read_int(savef);
    max_level = ur_read_int(savef);
    lost_dext = ur_read_int(savef);
    no_command = ur_read_int(savef);
    level = ur_read_int(savef);
    see_dist  = ur_read_int(savef);
    no_food = ur_read_int(savef);
    count = ur_read_int(savef);
    food_left = ur_read_int(savef);
    group = ur_read_int(savef);
    hungry_state = ur_read_int(savef);
    infest_dam = ur_read_int(savef);
    lost_str = ur_read_int(savef);
    hold_count = ur_read_int(savef);
    trap_tries = ur_read_int(savef);
    has_artifact  = ur_read_int(savef);
    picked_artifact  = ur_read_int(savef);
    active_artifact = ur_read_int(savef);
    luck = ur_read_int(savef);
    resurrect = ur_read_int(savef);
    inpack = ur_read_int(savef);
    inbag = ur_read_int(savef);
    PLAYER = ur_read_char(savef);
    take = ur_read_char(savef);
    runch = ur_read_char(savef);
    char_type = ur_read_int(savef);
    pool_teleport = (bool) ur_read_int(savef);
    inwhgt = (bool) ur_read_int(savef);
    after = (bool) ur_read_int(savef);
    showcursor = (bool) ur_read_int(savef);
    if (showcursor)
	curs_set(1);	/* show cursor */
    else
	curs_set(0);	/* hide cursor */
    if (!canwizard) {
	canwizard = (bool) ur_read_int(savef);
    } else {
	(void) ur_read_int(savef);
    }
    playing = (bool) ur_read_int(savef);
    running = (bool) ur_read_int(savef);
    fighting = (bool) ur_read_int(savef);
    if (!wizard) {
	wizard = (bool) ur_read_int(savef);
    } else {
	(void) ur_read_int(savef);
    }
    moving = (bool) ur_read_int(savef);
    delta = ur_read_coord(savef);
    levtype = ur_read_int(savef);
    purse = ur_read_int(savef);
    total = ur_read_int(savef);
    trader = ur_read_int(savef);
    spell_power = ur_read_int(savef);
    pray_time = ur_read_int(savef);
    pack_index = ur_read_int(savef) + pack_letters;
    bag_index = ur_read_int(savef) + bag_letters;
    str = ur_read_string(savef);
    strcpy(pack_letters,str);
    FREE(str);
    str = ur_read_string(savef);
    strcpy(bag_letters,str);
    FREE(str);

    DUMPSTRING
    ur_read_window(savef, cw);
    ur_read_window(savef, hw);
    ur_read_window(savef, mw);
    ur_read_window(savef, stdscr);

    DUMPSTRING
    difficulty = ur_read_int(savef);
    doorstop = (bool) ur_read_int(savef);
    jump = (bool) ur_read_int(savef);
    firstmove = (bool) ur_read_int(savef);
    askme = (bool) ur_read_int(savef);
    cutcorners = (bool) ur_read_int(savef);

    autopickup = (bool) ur_read_int(savef);
    autosave = (bool) ur_read_int(savef);
    use_mouse = (bool) ur_read_int(savef);
#ifdef MOUSE
    if (use_mouse)
	mousemask(BUTTON1_RELEASED, NULL);  /* for mouse buttons */
#endif
    reserved2 = (bool) ur_read_int(savef);
    reserved1 = (bool) ur_read_int(savef);  /* for future use */

    game_id = ur_read_int(savef);
    str = ur_read_string(savef);
    strcpy(whoami,str);
    FREE(str);
    str = ur_read_string(savef);
    strcpy(fruit,str);
    FREE(str);
    fd_data[1].mi_name = fruit; /* put fruit in the right place */

    str = ur_read_string(savef);
    if (strcmp(str, file_name) != 0
	&& strstr(str, "rogue.asave") != NULL)
	msg("Restore file: %s", file_name);  /* hmm, it moved */
    /* strcpy(unused_str1,str); */  /* was file_name */
    FREE(str);
    str = ur_read_string(savef);
    /* strcpy(unused_str2,str); */  /* was score_file */
    FREE(str);

    DUMPSTRING
    str = ur_read_string(savef);
    if (strcmp(str, save_end) != 0)
    {
	endwin();
        printf("Sorry, save file incomplete.\n");
        printf("Found: '%s' at position %ld\n", str, ftell(savef));
        printf("Expecting: '%s'\n", save_end);
        return(FALSE);
    }
    FREE(str);

    tweak_settings(FALSE, 2);  /* put things back the way they were */

    /*
     * Shouldn't happen
     */
    if (char_type < 0) {
	if (pstats.s_intel > pstats.s_str) {
	    msg("I guess you are a magician.");
	    char_type = 1;
	} else if (pstats.s_wisdom > pstats.s_str) {
	    msg("Ok wise guy, now you are a cleric.");
	    char_type = 2;
	} else if (pstats.s_dext > pstats.s_str) {
	    msg("Hmm, I guess you are a thief.");
	    char_type = 3;
	} else {
	    msg("Suffering from a loss of focus, you become a fighter.");
	    char_type = 0;
	}
    }
    if (find_slot(DAEMON, DAEMON_DOCTOR) == NULL) {
	msg("Paging the doctor.");
	start_daemon(DAEMON_DOCTOR, &player, AFTER);
	if (find_slot(DAEMON, DAEMON_DOCTOR) == NULL) {
	    endwin();
	    printf("\nSorry, the doctor is out.\n");
	    exit(1);
	}
    }
    if (find_slot(DAEMON, DAEMON_STOMACH) == NULL) {
	msg("Waking up your stomach.");
	start_daemon(DAEMON_STOMACH, 0, AFTER);
	if (find_slot(DAEMON, DAEMON_STOMACH) == NULL) {
	    endwin();
	    printf("\nSorry, your stomach has gone away.\n");
	    exit(1);
	}
    }
    if (find_slot(DAEMON, DAEMON_RUNNERS) == NULL) {
	msg("Waking up monsters");
	start_daemon(DAEMON_RUNNERS, 0, AFTER);
	if (find_slot(DAEMON, DAEMON_RUNNERS) == NULL) {
	    endwin();
	    printf("\nSorry, the monsters are too tired to play\n");
	    exit(1);
	}
    }

#ifdef EARL
    if (hungry_state > F_HUNGRY)
	no_food++;  /* cheat to avoid starvation */
#endif

    return(TRUE);
}

