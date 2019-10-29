/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	3.13 (Berkeley) 6/16/81
 */

#ifdef _WIN32
#define _POSIX
#define sig_t __p_sig_fn_t
#endif

#include "curses.h"
#include <time.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "mach_dep.h"
#include "rogue.h"

static char *rip[] = {
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |    killed by     |",
"                  |                  |",
"                  |       1980       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

char	*killname();

/*
 * death:
 *	Do something really fun when hero dies
 */

void 
death (monst)
int monst;
{
    char **dp = rip, *killer;
    struct tm *lt;
    time_t date;
    char buf[80];
    struct tm *localtime();
    int i;

    if (ISWEARING(R_RESURRECT)) {
	int die = TRUE;

	if (resurrect-- == 0) 
	    msg("You've run out of lives.");
	else if (!save_resurrect(ring_value(R_RESURRECT)))
	    msg("Your attempt to return from the grave fails.");
	else {
	    struct linked_list *item;
	    struct object *obj;
	    int rm, flags;
	    coord pos;

	    die = FALSE;
	    msg("You feel sudden warmth and then nothingness.");
	    teleport();
	    if (ring_value(R_RESURRECT) > 1 && rnd(10)) {
		pstats.s_hpt = 2 * pstats.s_const;
		pstats.s_const = max(pstats.s_const - 1, 3);
	    }
	    else {
		item = pack;
		while (item != NULL) {
		    obj = (struct object *) ldata(item);
		    flags = obj->o_flags;
		    obj->o_flags &= ~ISCURSED;
		    dropcheck(obj);
		    obj->o_flags = flags;
		    detach(pack, item);
		    freeletter(item);
		    attach(lvl_obj, item);
		    inpack--;
		    if (obj->o_type == ARTIFACT)
			has_artifact &= ~(1 << obj->o_which);
		    do {
			rm = rnd_room();
		        rnd_pos(&rooms[rm], &pos);
		    } until (winat(pos.y, pos.x) == FLOOR);
		    mvaddch(pos.y, pos.x, obj->o_type);
		    obj->o_pos = pos;
		    item = pack;
		}
		pstats.s_hpt = pstats.s_const;
		pstats.s_const = max(pstats.s_const - roll(2,2), 3);
	    }
	    chg_str(roll(1,4), TRUE, FALSE);
	    pstats.s_lvl = max(pstats.s_lvl, 1);
	    no_command += 2 + rnd(4);
	    if (on(player, ISHUH))
		lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
	    else
		light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
	    turn_on(player, ISHUH);
	    light(&hero);
	}
	if (die) {
	    wmove(cw, mpos, 0);
	    waddstr(cw, morestr);
	    draw(cw);
	    wait_for(0);
	}
	else
	    return;
    }
    status(FALSE);
    msg("Oh no, you died.");
    mvwaddstr(cw, 0, 18, retstr);
    draw(cw);
    wait_for('\n');

    time(&date);
    lt = localtime(&date);
    clear();
    if (LINES > 24) {
	i = (LINES - 24)/ 2;
    } else {
	i = (LINES - 14)/ 2;
    }
    move(i, 0);
    while (*dp)
	printw("%s\n", *dp++);
    mvaddstr(i+6, 28-((strlen(whoami)+1)/2), whoami);
    sprintf(buf, "%ld Points", pstats.s_exp );
    mvaddstr(i+7, 28-((strlen(buf)+1)/2), buf);
    killer = killname(monst);
    mvaddstr(i+9, 28-((strlen(killer)+1)/2), killer);
    sprintf(prbuf, "%4d", lt->tm_year + 1900);
    mvaddstr(i+10, 26, prbuf);

    /*
     * show recent messages, in case player missed them
     */
    if (LINES > 24) {
	move(i+14, 0);
	printw("To recap your last moments, ...\n\n");
	for (i=2;i<10;i++) {
	    printw("    %s\n", msgbuf[(msg_index+i)%10]);
	}
    }

    move(LINES-1, 0);
    idenpack();
    refresh();
    score(pstats.s_exp, KILLED, monst);
    exit(0);
}

/*
 * score -- figure score and post it.
 */

/* VARARGS2 */
void 
score (amount, flags, monst)
long amount;
int flags;
int monst;
{
    static struct sc_ent {
	long sc_score;
	char sc_name[76];
	long sc_gold;
	int sc_flags;
	int sc_level;
	short sc_artifacts;
	short sc_monster;
	int sc_game_id;
    } top_ten[10];
    struct sc_ent *scp;
    struct sc_ent *regame = NULL;
    struct sc_ent *sc2;
    FILE *outf;
    char *killer;
    static char *reason[] = {
	"killed",
	"quit",
	"a winner",
	"a total winner"
    };
    char *packend;
    extern int fd_score;


    signal(SIGINT, (sig_t)byebye);
#ifdef USGV3
    noraw();
    nl();
#endif
    if (flags != WINNER && flags != TOTAL && flags != SCOREIT) {
	game_over = TRUE;
	if (flags == CHICKEN)
	    packend = "when you quit";
	else
	    packend = "at your untimely demise";
	noecho();
	nl();
	mvaddstr(LINES - 1, 0, spacemsg);
	refresh();
	wait_for(' ');
	showpack(packend);
    }

    /*
     * no score at end of game for wizard/developer
     */
    if (canwizard && flags != SCOREIT)
	return;
    if (amount == 0 && flags != SCOREIT)
	return;

    /*
     * Open file and read list
     */

    if (fd_score < 0) {
	if (flags != SCOREIT)
	    endwin();
	else
	    printf("Unable to open %s\n", score_file);
	return;
    }

    outf = fdopen(fd_score, "w");

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
	scp->sc_score = 0L;
	scp->sc_name[0] = '\0';
	scp->sc_gold = 0L;
	scp->sc_flags = 0;
	scp->sc_level = 0;
	scp->sc_monster = 0;
	scp->sc_artifacts = 0;
	scp->sc_game_id = 0;
    }

    signal(SIGINT, SIG_DFL);
    if (flags != SCOREIT)
    {
	mvaddstr(LINES - 1, 0, morestr);
	refresh();
	fflush(stdout);
	wait_for(0);
    }
    read(fd_score, (char *) top_ten, sizeof top_ten);

    /*
     * Insert player in list if need be
     */
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	if (scp->sc_game_id == game_id)
	    regame = scp;	/* we've seen this game before */
    }
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	if (amount > scp->sc_score)
	    break;
    }

    if (scp < &top_ten[10] && amount > 0) {
	/*
	 * Congrats, made it into top 10
	 */
	if (regame) {  /* reuse the same slot */
	    scp = regame;
	} else {
	    sc2 = &top_ten[9];
	    while (sc2 > scp) {	/* make room for new entry */
		*sc2 = sc2[-1];
		sc2--;
	    }
	}
	scp->sc_score = amount;
	scp->sc_gold = purse;
	strcpy(scp->sc_name, whoami);
	sprintf(prbuf, ", Level %d %s", pstats.s_lvl, 
		    cnames[player.t_ctype][min(pstats.s_lvl,11) - 1]);
	strcat(scp->sc_name, prbuf);
	scp->sc_flags = flags;
	if (flags == WINNER || flags == TOTAL)
	    scp->sc_level = max_level;
	else
	    scp->sc_level = level;
	scp->sc_monster = monst;
	scp->sc_artifacts = picked_artifact;
	scp->sc_game_id = game_id;
    }
    if (flags != SCOREIT) {
	refresh();
	endwin();
    }
    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank  Score    Gold\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++) {

	if (scp->sc_score) {
	    printf("%-2d    %-8ld %-8ld\t%s:\n", (int) (scp - top_ten + 1),
		scp->sc_score, scp->sc_gold, scp->sc_name);
	    if (scp->sc_artifacts) {
		char things[60];
		int  i;
		bool first = TRUE;

		things[0] = '\0';
		for (i = 0; i <= MAXARTIFACT; i++) {
		    if (scp->sc_artifacts & (1 << i)) {
			if (strlen(things))
			    strcat(things, ", ");
			if (first) {
			    strcat(things, "retrieved ");
			    first = FALSE;
			}
			if (55 - strlen(things) < strlen(arts[i].ar_name)) {
			    printf("\t\t\t%s\n", things);
			    things[0] = '\0';
		        }
			strcat(things, arts[i].ar_name);
		    }
		}
		if (strlen(things)) 
		    printf("\t\t\t%s,", things);
		putchar('\n');
	    }
	    printf("\t\t\t%s on level %d", 
		reason[scp->sc_flags], scp->sc_level);
	    if (scp->sc_flags == 0) {
		printf(" by");
		killer = killname(scp->sc_monster);
		printf(" %s", killer);
	    }

	    if (canwizard)
		printf(" (game #%d)", scp->sc_game_id);

	    if (scp->sc_game_id == game_id) { /* the game we just played */
		printf(" (just now).\n");
	    } else {
		printf(".\n");
	    }
	}
    }
    fseek(outf, 0L, 0);
    /*
     * Update the list file
     */
    write(fileno(outf), (char *) top_ten, sizeof top_ten);
    fclose(outf);
}

void 
total_winner ()
{
    struct linked_list *item;
    struct object *obj;
    int worth, oldpurse;
    char c;
    struct linked_list *bag = NULL;

    game_over = TRUE;
    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have \n");
    addstr("escaped the Dungeons of Doom alive.  You journey home \n");
    addstr("and sell all your loot at a great profit.\n");
    addstr("The White Council approves the recommendation of\n");
    if (player.t_ctype == C_FIGHTER)
	addstr("the fighters guild and appoints you Lord Protector\n");
    else if (player.t_ctype == C_MAGICIAN)
	addstr("the magicians guild and appoints you Master Wizard\n");
    else if (player.t_ctype == C_CLERIC)
	addstr("the temple priests and appoints you Master of the Flowers\n");
    else if (player.t_ctype == C_THIEF) {
	addstr("the thieves guild under protest and appoints you\n");
	addstr("Master of the Highways\n");
    }
    addstr("of the Land Between the Mountains.\n");
    mvaddstr(LINES - 1, 0, spacemsg);
    refresh();
    wait_for(' ');
    clear();
    idenpack();
    oldpurse = purse;
    mvaddstr(0, 0, "   Worth  Item");
    for (c = 'a', item = pack; item != NULL; c++, item = next(item)) {
	obj = (struct object *) ldata(item);
	worth = get_worth(obj);
	purse += worth;
	if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE) 
	    bag = obj->art_stats.t_art;
	mvprintw(c - 'a' + 1, 0, "%c) %8d  %s", c, 
		worth, inv_name(obj, FALSE));
    }
    if (bag != NULL) {
	mvaddstr(LINES - 1, 0, morestr);
	refresh();
	wait_for(0);
	clear();
	mvprintw(0, 0, "Contents of the Magic Purse of Yendor:\n");
	for (c = 'a', item = bag; item != NULL; c++, item = next(item)) {
	    obj = OBJPTR(item);
	    worth = get_worth(obj);
	    purse += worth;
	    whatis(item);
	    mvprintw(c - 'a' + 1, 0, "%c) %8d %s\n", c, 
			worth, inv_name(obj, FALSE));
	}
    }
    mvprintw(c - 'a' + 1, 0,"   %6d  Gold Pieces          ", oldpurse);
    refresh();
    if (has_artifact == 255) 
	score(pstats.s_exp, TOTAL, 0);
    else
	score(pstats.s_exp, WINNER, 0);
    exit(0);
}

char *
killname (monst)
int monst;
{
    static char mons_name[80];

    if (monst >= 0) {
	switch (monsters[monst].m_name[0]) {
	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
		sprintf(mons_name, "an %s", monsters[monst].m_name);
		break;
	    default:
		sprintf(mons_name, "a %s", monsters[monst].m_name);
	}
	return(mons_name);
    }
    else
	switch (monst) {
	    case D_ARROW:
		return "an arrow";
	    case D_DART:
		return "a dart";
	    case D_BOLT:
		return "a bolt";
	    case D_POISON:
		return "poison";	/* Cursed healing potion */
	    case D_POTION:
		return "a cursed potion";
	    case D_PETRIFY:
		return "petrification";
	    case D_SUFFOCATION:
		return "suffocation";
	    case D_INFESTATION:
		return "a parasite";
	    case D_DROWN:
		return "drowning";
	    case D_FALL:
		return "falling";
	    case D_FIRE:
		return "slow boiling in oil";
	}
    return "";
}

/*
 * showpack:
 *	Display the contents of the hero's pack
 */
void 
showpack (howso)
char *howso;
{
	char *iname;
	int cnt, worth, ch, oldpurse;
	struct linked_list *item;
	struct object *obj;
	struct linked_list *bag = NULL;

	cnt = 1;
	clear();
	mvprintw(0, 0, "Contents of your pack %s:\n",howso);
	ch = 'a';
	oldpurse = purse;
	purse = 0;
	for (item = pack; item != NULL; item = next(item)) {
		obj = OBJPTR(item);
		worth = get_worth(obj);
		whatis(item);
		purse += worth;
		if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
			bag = obj->art_stats.t_art;
		iname = inv_name(obj, FALSE);
		mvprintw(cnt, 0, "%c) %s\n",ch,iname);
		ch += 1;
		if (++cnt > LINES - 5 && next(item) != NULL) {
			cnt = 1;
			mvaddstr(LINES - 1, 0, morestr);
			refresh();
			wait_for(0);
			clear();
		}
	}
	if (bag != NULL) {
		mvaddstr(LINES - 1, 0, morestr);
		refresh();
		wait_for(0);
		clear();
		cnt = 1;
		ch = 'a';
		mvprintw(0, 0, "Contents of the Magic Purse of Yendor %s:\n",
			howso);
		for (item = bag; item != NULL; item = next(item)) {
			obj = OBJPTR(item);
			worth = get_worth(obj);
			whatis(item);
			purse += worth;
			mvprintw(cnt, 0, "%c) %s\n", ch, inv_name(obj, FALSE));
			ch += 1;
			if (++cnt > LINES - 5 && next(item) != NULL) {
				cnt = 1;
				mvaddstr(LINES - 1, 0, morestr);
				refresh();
				wait_for(0);
				clear();
			}
		}
	}
	mvprintw(cnt + 1,0,"Carrying %d gold pieces", oldpurse);
	mvprintw(cnt + 2,0,"Carrying objects worth %d gold pieces", purse);
	purse += oldpurse;
	refresh();
}

void 
byebye ()
{
    clear();
    move(LINES-1, 0);
    draw(stdscr);
    endwin();
    printf("\n");
    exit(0);
}

/*
 * save_resurrect: 
 *	chance of resurrection according to modifed D&D probabilities
 */
int 
save_resurrect (bonus)
int bonus;
{
    int need, adjust;

    adjust = pstats.s_const + bonus - luck;
    if (adjust > 17)
	return TRUE;
    else if (adjust < 14)
	need = 5 * (adjust + 5);
    else
	need = 90 + 2 * (adjust - 13);
    return (roll(1, 100) < need);
}
