/*
 * Read and execute the user commands
 *
 */

#ifdef _WIN32
#define _POSIX
#define sig_t __p_sig_fn_t
#endif

#include "curses.h"
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <unistd.h>
#include "rogue.h"

/*
 * command:
 *	Process the user commands
 */

#ifdef MOUSE
    MEVENT event;  /* mouse events */
    static coord dest = {0,0};
    static coord prev = {0,0};
#endif

void 
command ()
{
    static int ch;
    int ntimes = 1;			/* Number of player moves */
    static char countch;
    bool an_after = FALSE;
    static coord dta;
    static int minfight;
    static int waitcount;

    if (on(player, CANFLY) && rnd(2) && running)
	ntimes++;
    if (on(player, ISHASTE)) 
	ntimes++;
    if (fighting && player.t_ctype == C_FIGHTER &&
		(pstats.s_lvl > 12 || 
		(pstats.s_lvl > 6 && pstats.s_lvl < 13 && rnd(2))))
	ntimes *=2;
    if (on(player, ISSLOW)) {
	if (player.t_turn != TRUE) ntimes--;
	player.t_turn ^= TRUE;
    }

    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--)
    {
#if 0
	if (wizard && on(player, BLESSMAGIC)) {
	    overlay(mw, cw);  /* monster awareness */
	    draw(cw);
	}
#endif

	moving = FALSE;
	look(after);
	if (!running) {
	    door_stop = FALSE;
	    searching_run = 0;
	}
	status(FALSE);
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
#ifdef MOUSE
	if (mousemove) {
	    static char pch = ' ';
	    if (!jump) {
		draw(cw);		/* Draw screen */
		if (wizard)
		    usleep(40000);
		else
		    usleep(25000);
	    } else if (pch != ch) {	/* changed direction */
		draw(cw);
		usleep(40000);
		pch = ch;
	    }
	} else
#endif
	if (!((running || count || fighting) && jump)) {
	    draw(cw);			/* Draw screen */
	    if (running)
		usleep(4000);
	    else if (count)
		usleep(8000);
	    else
		usleep(12000);
	}
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (no_command)
	{
	    usleep(50000);
	    if (--no_command == 0) {
		msg("You can move again.");
		if (serious_fight
		    && pstats.s_hpt > max_stats.s_hpt*2/3
		    && hungry_state != F_FAINT) {
		    ch = 'F';
		} else {
		    fighting = moving = FALSE;
		    serious_fight = FALSE;
		}
	    }
	}
	if (!no_command)
	{
	    if (fighting) {
		/* do nothing */
	    } else if (running) {
		/* If in a corridor, if we are at a turn with only one
		 * way to go, turn that way.
		 */
		if ((winat(hero.y, hero.x) == PASSAGE) && off(player, ISHUH) &&
		    (off(player, ISBLIND))) {
		    switch (runch) {
			case 'h' : corr_move(0, -1);
			when 'j' : corr_move(1, 0);
			when 'k' : corr_move(-1, 0);
			when 'l' : corr_move(0, 1);
		    }
		}
		if (searching_run == 1) {
		    ch = runch;
		    searching_run++;
		} else if (searching_run == 2) {
		    if (winat(hero.y, hero.x) == PASSAGE || levtype != NORMLEV
#ifdef MOUSE
			|| isalpha(show(prev.y, prev.x)) /* being chased */
#endif
			|| pstats.s_hpt < max_stats.s_hpt) {
			ch = runch;
		    } else {
			ch = 's';
		    }
		    searching_run--;
		} else {
		    ch = runch;
		}
	    }
#ifdef MOUSE
	    else if (mousemove) {
		/*
		 * choose direction (h,j,k,l,...)
		 */
		ch = countch = do_mousemove(dest, prev);
		if (ch == ' ') {
		    prev.x = prev.y = 0;
		} else if (ch == 's') {
		    draw(cw);
		    usleep(100000);
		} else if (strchr("hjklyubn", ch)) {
		    prev.x = hero.x;
		    prev.y = hero.y;
		}
	    }
#endif
	    else if (count) ch = countch;
	    else
	    {
		if (save_ch != ' ') {
		    ch = save_ch;
		    save_ch = ' ';
		} else {
		    ch = readchar();
		}
#if 0
if (ch >= KEY_MIN)
fprintf(stderr, "ch: '%s' [0%o]\n", unctrl(ch), ch);
#endif
		ch = unarrow(ch);  /* translate arrow keys */
#ifdef EARL
		if (ch == 'x')
		    ch = '.'; /* rest - left handed */
#endif
		if (ch == CTRL('F')) {
		    ch = 'F';
		    serious_fight = TRUE;
		} else {
		    serious_fight = FALSE;
		}
		if (mpos != 0 && !running && ch != ' ')
		    msg("");	/* Erase message if its there */
	    }
	}
	else {
		ch = '.';
	}

#ifdef MOUSE
	    /*
	     * convert mouse click into a command
	     */
	    if (ch == KEY_MOUSE) {
		if (getmouse(&event) == OK
		  && event.bstate & BUTTON1_RELEASED) {
		    dest.x = event.x;
		    dest.y = event.y;
		    ch = do_mouseclick(dest);
		    /*
		     * if destination is unreachable, pick another
		     */
		    if (mousemove)
			dest = fix_mousedest(dest);
		} else {
		    ch = ' ';
		}
	    }
#endif

	if (!no_command)
	{
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch))
	    {
		count = 0;
		while (isdigit(ch))
		{
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;
		/*
		 * Preserve count for commands which can be repeated.
		 */
		switch (ch) {
		    case 'h': case 'j': case 'k': case 'l':
		    case 'y': case 'u': case 'b': case 'n':
		    case 'H': case 'J': case 'K': case 'L':
		    case 'Y': case 'U': case 'B': case 'N':
		    case 'q': case 'r': case 's': case 'm':
		    case 't': case 'c': case 'I': case '.':
		    case 'z': case 'p': case 'd':
		    case CTRL('K'): case CTRL('L'): case CTRL('H'): case CTRL('J'): 
		    case CTRL('Y'): case CTRL('U'): case CTRL('B'): case CTRL('N'):
			break;
		    default:
			count = 0;
		}
	    }

	    /* Save current direction */
	    if (!running) /* If running, it is already saved */
	    switch (ch) {
		case 'h': case 'j': case 'k': case 'l':
		case 'y': case 'u': case 'b': case 'n':
		    runch = ch;
		    break;
		case 'H': case 'J': case 'K': case 'L':
		case 'Y': case 'U': case 'B': case 'N':
		    runch = tolower(ch);
		    if (doorstop && !on(player, ISBLIND)) {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    break;
		case CTRL('H'): case CTRL('J'): case CTRL('K'): case CTRL('L'):
	        case CTRL('Y'): case CTRL('U'): case CTRL('B'): case CTRL('N'):
#define UNCTRL(x)	((x) + 'A' - 1)
		    ch = UNCTRL(ch);
#undef	UNCTRL
		    runch = tolower(ch);
		    if (doorstop && !on(player, ISBLIND)) {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    searching_run = 1;	/* alternately search and move */
		    break;
	    }

	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch)
	    {
		when 'h' : do_move(0, -1);
		when 'j' : do_move(1, 0);
		when 'k' : do_move(-1, 0);
		when 'l' : do_move(0, 1);
		when 'y' : do_move(-1, -1);
		when 'u' : do_move(-1, 1);
		when 'b' : do_move(1, -1);
		when 'n' : do_move(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 'm':
		    moving = TRUE;
		    if (!get_dir()) {
			after = FALSE;
			break;
		    }
		    do_move(delta.y, delta.x);
		when 'F' : case 'f':
		    if (!fighting) {	/* begin fighting */
			minfight = 10;
			if (ch == 'F') minfight += 30;
			if (serious_fight) minfight += 30;
			if (max_level > 80) minfight += 20;
			foe = NULL;
		    } else {		/* continue fighting */
			minfight--;
		    }
		    /*
		     * Either we're starting a new fight, or the monster
		     * we were 'F'ighting is gone (moved or died).
		     */
		    if (!fighting ||
		        (ch == 'F'
			    && !can_fight(hero.x+dta.x,hero.y+dta.y))) {
			/*
			 * Look for a monster to fight.
			 * If we can't find one, ask the player.
			 */
			if (pick_monster(ch) || (!fighting && get_dir())) {
			    dta.y = delta.y;
			    dta.x = delta.x;
			    beast = NULL;
			    if (ch == 'F') {
				waitcount = 2;
				if (serious_fight)
				    waitcount++;
			    } else {
				waitcount = 0;
			    }
			} else {
			    if (ch == 'F' &&
				pstats.s_hpt > max_stats.s_hpt/3 &&
				hungry_state != F_FAINT &&
				waitcount > 0) {
				/*
				 * wait a bit,
				 * in case a monster moves into range
				 */
				if (!pick_monster(ch))
				    waitcount--;
				if (waitcount == 0)
				    after = FALSE;
			    } else {
				fighting = FALSE;
				after = FALSE;
#if 0
				if (ch == 'F' && waitcount == 0)
				    debug("You've run out of monsters to fight.");
#endif
			    }
			    break;
			}
		    }
		    do_fight(dta.y, dta.x,
			(ch == 'F') ? TRUE : FALSE);
		when 't':
		    if (!get_dir())
			after = FALSE;
		    else
			missile(delta.y, delta.x,
				get_item("throw", NULL), &player);
		when 'Q' : after = FALSE; quit();
		when 'i' : after = FALSE; inventory(pack, 0);
		when 'I' : after = FALSE; game_over=TRUE; picky_inven(); game_over=FALSE;
		when 'd' : drop(NULL);
		when 'q' : quaff(-1, FALSE);
		when 'r' : read_scroll(-1, FALSE);
		when 'e' : eat();
		when '=' : listens();
		when 'A' : apply();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'P' : ring_on();
		when 'R' : ring_off();
		when 'o' : after = FALSE; option();
		    if (fd_data[1].mi_name != fruit)
			strcpy(fd_data[1].mi_name, fruit);
		when 'C' : call(FALSE);
		when 'M' : call(TRUE);
		when '~' : after = FALSE; next_level();
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when CTRL('T') :
		    if (get_dir()) steal();
		    else after = FALSE;
		when 'D' : dip_it();
		when 'G' : gsense();
		when '^' : set_trap(&player, hero.y, hero.x);
		when 's' : search(FALSE);
		when 'z' :
		    if (get_dir())
			do_zap(TRUE, NULL, FALSE);
		    else
			after = FALSE;
		when 'p' : pray();
		when 'c' : cast();
		when 'a' :
		    if (get_dir())
			affect();
		    else after = FALSE;
		when 'v' : after = FALSE;
			   msg("UltraRogue version %s.",
				release);
		when CTRL('R') : after = FALSE;
#ifdef EARL
		    char fname[200];
		    strcpy(fname, home);
		    strcat(fname, "rogue.asave");
		    if (autosave && access(fname, F_OK) == -0) {
			msg("Do you want to restart this level? (y/N)");
			wrefresh(cw);
			if (readchar() == 'y')
			    death(D_MISADVENTURE);	/* restart level? */
			msg("");
		    }
#endif
				WINDOW *tmpwin = newwin(LINES, COLS, 0, 0);
				wclear(tmpwin);
				wrefresh(tmpwin);
				(void) delwin(tmpwin);
				usleep(50000);
				wrefresh(cw);
				touchwin(cw); /* MMMMMMMMMM */
		when CTRL('P') : {
			    bool decrement = FALSE;

			    after = FALSE; 
			    if (mpos == 0)
				decrement = TRUE;
			    msg_index = (msg_index + 9) % 10;
			    msg(msgbuf[msg_index]);
			    if (decrement)
				msg_index = (msg_index + 9) % 10;
		}
		when 'S' : 
		    after = FALSE;
		    if (save_game())
		    {
			wclear(cw);
			draw(cw);
			endwin();
			exit(0);
		    }
		when '.' : if (rnd(2) == 0) player.t_quiet++;	/* Rest command */
		when ',' :
		    if (levtype == POSTLEV)
			buy_it();
		    else
			add_pack(NULL, FALSE);
		when ' ' : case '\0' : case '\r' :
		    after = FALSE;	/* Do Nothing */
		when CTRL('W') :
		    after = FALSE;
		    if (wizard)
		    {
			wizard = FALSE;
			trader = 0;
			msg("Not wizard any more.");
		    }
		    else
		    {
		       if(canwizard) {
				msg("Welcome, oh mighty wizard.");
				wizard = TRUE;
				(void) signal(SIGQUIT, SIG_DFL);
#if 0
				pstats.s_hpt = max_stats.s_hpt;
				quaff(P_RESTORE, FALSE);
				spell_power = 0;
				pray_time = 0;
#endif
		       }
		       else
			    msg("Sorry.");
		    }
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
		when '#':
		    if (levtype == POSTLEV)		/* buy something */
			buy_it();
		    after = FALSE;
		when '$':
		    if (levtype == POSTLEV)		/* price something */
			price_it();
		    after = FALSE;
		when '%':
		    if (levtype == POSTLEV)		/* sell something */
			sell_it();
		    after = FALSE;
		otherwise :
		    after = FALSE;
		    if (wizard) switch (ch)
		    {
			case CTRL('V') : create_obj(0, 0, FALSE);
			when CTRL('I') : inventory(lvl_obj, 0);
			when CTRL('Z') : whatis(NULL);
			when CTRL('D') : msg("rnd(4)%d, rnd(40)%d, rnd(100)%d",
						rnd(4),rnd(40),rnd(100));
			when CTRL('A') : overlay(stdscr,cw);
			when CTRL('M') : overlay(mw,cw);
			when CTRL('X') : teleport();
			when CTRL('E') : msg("food left: %d\tfood level: %d", 
						food_left, foodlev);
			when '@' : activity();
			when CTRL('G') : 
			{
			    int tlev;
			    prbuf[0] = NULL;
			    msg("Which level? ");
			    if(get_string(prbuf,cw) == NORM) {
				msg("");
				tlev = atoi(prbuf);
				if(tlev < 1) {
				    msg("Illegal level.");
				}
				else if (tlev > 3000) {
					levtype = THRONE;
					level = tlev - 3000;
				}
				else if (tlev > 2000) {
					levtype = MAZELEV;
					level = tlev - 2000;
				}
				else if (tlev > 1000) {
					levtype = POSTLEV;
					level = tlev - 1000;
				} 
				else {
					levtype = NORMLEV;
					level = tlev;
				}
				new_level(levtype);
			    }
			}
			when CTRL('C') :
			{
			    struct linked_list *item;

			    if ((item = get_item("charge", STICK)) != NULL)
				((struct object *)ldata(item))->o_charges=10000;
			}
			when 'V' :
			{
			    struct linked_list *item;

			    if ((item = get_item("price", 0)) != NULL)
				msg("Worth %d.", 
				    get_worth(((struct object *)ldata(item))));
			}
			when CTRL('O') :
			{
			    int i;
			    struct linked_list *item;
			    struct object *obj;

			    for (i = 0; i < 20; i++)
				raise_level();

			    /*
			     * make decent statistics
			     */
			    pstats.s_hpt += 1000;
			    max_stats.s_hpt += 1000;
			    pstats.s_str = 25;
			    max_stats.s_str = 25;
			    pstats.s_intel = 25;
			    max_stats.s_intel = 25;
			    pstats.s_wisdom = 25;
			    max_stats.s_wisdom = 25;
			    pstats.s_dext = 25;
			    max_stats.s_dext = 25;
			    pstats.s_const = 25;
			    max_stats.s_const = 25;
			    /*
			     * Give the rogue a sword 
			     */
			    item = spec_item(WEAPON, CLAYMORE, 10, 10);
			    add_pack(item, TRUE);
			    cur_weapon = (struct object *) ldata(item);
			    cur_weapon->o_flags |= ISKNOW;

			    /*
			     * and a stick
			     */
			    item = spec_item(STICK, WS_ANNIH, 10000, 0);
			    obj = (struct object *) ldata(item);
			    obj->o_flags |= ISKNOW;
	    		    ws_know[WS_ANNIH] = TRUE;
	    		    if (ws_guess[WS_ANNIH])
	    		    {
				FREE(ws_guess[WS_ANNIH]);
				ws_guess[WS_ANNIH] = NULL;
			    }
			    add_pack(item, TRUE);
			    /*
			     * and his suit of armor
			     */
			    item = spec_item(ARMOR, CRYSTAL_ARMOR, 15, 0);
			    obj = (struct object *) ldata(item);
			    obj->o_flags |= ISKNOW;
			    obj->o_weight = armors[CRYSTAL_ARMOR].a_wght * 0.2;
			    cur_armor = obj;
			    add_pack(item, TRUE);
			    purse += 50000;
			    updpack(TRUE);
			}
			otherwise :
			    if (ch < ' ' || ch > '~')
				msg("Illegal wizard command '%s' [0%o]", unctrl(ch), ch);
			    else
				msg("Illegal wizard command '%s'", unctrl(ch));
			    count = 0;
		    }
		    else
		    {
			if (ch < ' ' || ch > '~')
			    msg("Illegal command '%s' [0%o].", unctrl(ch), ch);
			else
			    msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
			after = FALSE;
		    }
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If player ran into something to take, let them pick it up.
	 * unless its a trading post
	 */
	if (take != 0 && levtype != POSTLEV) {
	    if (autopickup && !moving && (!searching_run || take == GOLD))
	        pick_up(take);
#ifdef MOUSE
	    else if (autopickup && mousemove && take == GOLD)
	        pick_up(take);
#endif
	    else
		show_floor();
	}
	if (!running)
	    door_stop = FALSE;

	/* If after is true, mark an_after as true so that if
	 * we are hasted, the first "after" will be noted.
	 */
	if (after) an_after = TRUE;
    }

    /*
     * Kick off the rest of the daemons and fuses
     */
    if (an_after)
    {
	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);

	/* Special abilities */
	if ((player.t_ctype == C_THIEF) &&
	    (rnd(100) < (2*pstats.s_dext + 5*pstats.s_lvl))) 
	    search(TRUE);
	if (ISWEARING(R_SEARCH)) {
	    if (!ring_cursed(R_SEARCH)) 
		search(FALSE);
	    if (ring_blessed(R_SEARCH)) 
		search(FALSE);
	}
	if (ISWEARING(R_TELEPORT) && rnd(100) < 2) {
	    teleport();
	    if (off(player, ISCLEAR)) {
		if (on(player, ISHUH))
		    lengthen_fuse(FUSE_UNCONFUSE, rnd(8)+HUHDURATION);
		else 
		    light_fuse(FUSE_UNCONFUSE, 0, rnd(8)+HUHDURATION, AFTER);
		turn_on(player, ISHUH);
	    }
	    else msg("You feel dizzy for a moment, but it quickly passes.");
	}

	/* suffocating */
	if (find_slot(FUSE, FUSE_SUFFOCATE) != NULL) {
	    msg("You are still suffocating.");
	    fighting = running = FALSE;
	}
	/* accidents and general clumsiness */
	if (fighting && rnd(50) == 0 && minfight < 1) {
	    msg("You become tired of this nonsense.");
	    fighting = FALSE;
	}
	/* If player is infested, take off a hit point */
	if (on(player, HASINFEST) && !ISWEARING(R_HEALTH)) {
	    if ((pstats.s_hpt -= infest_dam) <= 0) {
		death(D_INFESTATION);
		return;
	    } else if (pstats.s_hpt < max_stats.s_hpt/3) {
		running = FALSE;
	    }
	}
	if (on(player, ISELECTRIC)) {
	    int lvl;
	    struct linked_list *item;
	    struct linked_list tmp_item;
	    struct thing *tp;

	    for (item = mlist; item != NULL; item = next(item)) {
		tp = (struct thing *) ldata(item);
		if (tp && DISTANCE(tp->t_pos.y, tp->t_pos.x, hero.y, hero.x) < 5) {
		    if (on(*tp, NOBOLT))
			continue;
		    if ((tp->t_stats.s_hpt -= roll(2,4)) <= 0) {
			msg("The %s is killed by an electric shock.",
				monsters[tp->t_index].m_name);
			tmp_item.l_next = item->l_next;
			killed(item, TRUE, TRUE);
			item = &tmp_item;
			continue;
		    }
		    lvl = tp->t_stats.s_intel - 5;
		    if (lvl < 0)
			lvl = 10 + tp->t_stats.s_intel;
		    if (rnd(lvl + 1)/5 == 0) {
			turn_on(*tp, ISFLEE);
			if (!fighting)
			    msg("The %s is shocked by electricity.",
				    monsters[tp->t_index].m_name);
		    }
		    else
			if (!fighting)
			    msg("The %s is zapped by your electricity.",
				    monsters[tp->t_index].m_name);
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);
		    running = FALSE;
		}
	    }
	}

	if (difficulty >= 2 && !fighting && (no_command == 0) && cur_weapon != NULL
		&& rnd(on(player, STUMBLER) ? 399 : 9999) == 0
		&& rnd(pstats.s_dext) < 
		2 - hitweight() + (on(player, STUMBLER) ? 4 : 0)) {
	    msg("You trip and stumble over your weapon.");
	    running = FALSE;
	    if (rnd(8) == 0 && (pstats.s_hpt -= roll(1,10)) <= 0) {
		msg("You break your neck and die.");
		death(D_FALL);
		return;
	    }
	    else if (cur_weapon->o_flags & ISPOISON && rnd(4) == 0) {
		msg("You are cut by your %s!", inv_name(cur_weapon, TRUE));
		if (!save(VS_POISON)) {
		    if (pstats.s_hpt == 1) {
			msg("You die from the poison in the cut.");
			death(D_POISON);
			return;
		    }
		    else {
			msg("You feel very sick now.");
			pstats.s_hpt /= 2;
			chg_str(-2, FALSE, FALSE);
		    }
		}
	    }
	}
	if (difficulty > 0) {
	    /* may get summoned to a lower level */
	    int rare = 200000;
	    if (difficulty < 2)
		rare = 400000;
	    else if (difficulty > 2)
		rare = 150000;

	    if (rnd(rare) == 0) {
		new_level(THRONE);
		fighting = running = FALSE;
	    }
	}
    }
#ifdef EARL
    if (find_slot(DAEMON, DAEMON_DOCTOR) == NULL) {
	static bool doctor_just_died = TRUE;
	if (doctor_just_died) {
	    msg("Oh no, the doctor is gone!");
	    wait_for(0);
	}
	doctor_just_died = FALSE;
	fighting = running = FALSE;
    }
#endif
}

/*
 * quit:
 *	Have player make certain, then exit.
 */

void 
quit ()
{
    int ch;

    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, (sig_t)quit) != (sig_t) quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    ch = readchar();
    if (ch == 'y')
    {
	clear();
	move(LINES-1, 0);
	draw(stdscr);
	score(pstats.s_exp, CHICKEN, 0);
	byebye();
    }
    else
    {
	signal(SIGINT, (sig_t)quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status(FALSE);
	draw(cw);
	mpos = 0;
	count = 0;
	fighting = running = 0;
	/* this is a hack, because ^C is both quit and wizard mode charge item */
	if (wizard && after) {
	    struct linked_list *item;
	    if ((item = get_item("charge", STICK)) != NULL)
		((struct object *)ldata(item))->o_charges=1000;
	}
    }
}

/*
 * search:
 *	Player gropes about to find hidden things.
 */

void 
search (bool is_thief)
{
    int x, y;
    char ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him/her a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++)
	{
	    ch = winat(y, x);
	    if (isatrap(ch)) {
		    struct trap *tp;
		    struct room *rp ;

		    if (isatrap(mvwinch(cw, y, x)))
			continue;
		    tp = trap_at(y, x);
		    if ((tp->tr_flags & ISTHIEFSET) ||
			(rnd(100) > 50 && !is_thief)) break;
		    rp = roomin(&hero);
		    if (tp->tr_type == FIRETRAP && rp != NULL) {
		        rp->r_flags &= ~ISDARK;
			light(&hero);
		    }
		    tp->tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, ch);
#ifdef MOUSE
		    if (!mousemove)
#endif
			count = 0;
		    if (x != hero.x && y != hero.y) {
			running = FALSE;
			msg(tr_name(tp->tr_type));
		    }
	    }
	    else if (ch == SECRETDOOR) {
		    if (rnd(100) < 30 && !is_thief) {
			mvaddch(y, x, DOOR);
#ifdef MOUSE
			if (!mousemove)
#endif
			    count = 0;
		    }
	    }
	}
}

/*
 * help:
 *	Give single character help, or the whole mess if they want
 */

void 
help ()
{
    struct h_list *strp = helpstr;
    int cnt;
    int lines = LINES - 2;

    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
	if (strp->h_desc == 0) {
	    if (!wizard) {
		break;
	    } else {
		strp++;
		continue;
	    }
	}

	mvwaddstr(hw, cnt % lines, cnt > lines-1 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	strp++;

	if (++cnt >= lines*2 && strp->h_ch && (strp->h_desc != NULL || wizard)) {
	    wmove(hw, LINES-1, 0);
	    wprintw(hw, "%s", morestr);
	    draw(hw);
	    (void) wgetch(hw);
	    wclear(hw);
	    cnt = 0;
	}
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw, "%s", spacemsg);
    draw(hw);
    (void) wgetch(hw);
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status(FALSE);
    touchwin(cw);
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */

void 
identify ()
{
    int ch;
    char *str;

    msg("What do you want identified? ");
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE)
    {
	msg("");
	return;
    }
#ifdef MOUSE
	    /*
	     * identify the thing they clicked on
	     */
	    if (ch == KEY_MOUSE
		  && getmouse(&event) == OK
		  && event.bstate & BUTTON1_RELEASED
		   ) {
		    if (mvwinch(cw, event.y, event.x) == VPLAYER)
			ch = VPLAYER;
		    else if (mvwinch(cw, event.y, event.x) == IPLAYER)
			ch = IPLAYER;
		    else
			ch = winat(event.y, event.x);
	    }
#endif
    if (isalpha(ch)) {
	str = id_monst(ch);
    } else switch(ch)
    {
	case '|':
	case '-':
	    str = "wall of a room";
	when GOLD:	str = "gold";
	when STAIRS :	str = "passage leading down";
	when DOOR:	str = "door";
	when FLOOR:	str = "room floor";
	when VPLAYER:	str = "The hero of the game ---> you";
	when IPLAYER:	str = "you (but invisible)";
	when PASSAGE:	str = "passage";
	when POST:	str = "trading post";
	when POOL:	str = "a shimmering pool";
	when TRAPDOOR:	str = "trapdoor";
	when ARROWTRAP:	str = "arrow trap";
	when SLEEPTRAP:	str = "sleeping gas trap";
	when BEARTRAP:	str = "bear trap";
	when TELTRAP:	str = "teleport trap";
	when DARTTRAP:	str = "dart trap";
	when MAZETRAP:	str = "entrance to a maze";
	when FIRETRAP:  str = "fire trap";
	when POISONTRAP:str = "poison pool trap";
	when LAIR:	str = "monster lair entrance";
	when RUSTTRAP:  str = "rust trap";
	when POTION:	str = "potion";
	when SCROLL:	str = "scroll";
	when FOOD:	str = "food";
	when WEAPON:	str = "weapon";
	when ' ' :	str = "solid rock";
	when ARMOR:	str = "armor";
	when ARTIFACT:	str = "an artifact from bygone ages";
	when RING:	str = "ring";
	when STICK:	str = "wand or staff";
	otherwise:	str = "unknown character";
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *	Player wants to go down a level
 */

void 
d_level ()
{
    bool no_phase=FALSE;

    if (isatrap(winat(hero.y, hero.x))) {
	be_trapped(&player, &hero);
	return;
    }
    if (winat(hero.y, hero.x) != STAIRS) {
	if (off(player, CANINWALL)) {	/* Must use stairs if can't phase */
	    msg("I see no way down.");
	    return;
	}
	extinguish_fuse(FUSE_UNPHASE);	/* Using phase to go down gets rid of it */
	no_phase = TRUE;
    }
    if (ISWEARING(R_LEVITATION) || on(player, CANFLY)) {
	msg("You can't!  You're floating in the air.");
	return;
    }
    if (rnd(pstats.s_dext) < 2-hitweight()+(on(player, STUMBLER) ? 4 : 0)) {
	msg("You trip and fall down the stairs.");
	if ((pstats.s_hpt -= roll(1,10)) <= 0) {
	    msg("You break your neck and die.");
	    death(D_FALL);
	    return;
	}
    }
    level++;
    new_level(NORMLEV);
    if (no_phase) unphase(NULL);
}

/*
 * u_level:
 *	Player wants to go up a level
 */

void 
u_level ()
{
    char ch;

    ch = winat(hero.y, hero.x);
    if (has_artifact && (ch == STAIRS ||
		(on(player, CANINWALL)
		&& (ISWEARING(R_LEVITATION) || on(player, CANFLY))))) {
	if (--level == 0)
	    total_winner();
	else if (rnd(wizard ? 4 : 20) == 0)
	    new_level(THRONE);
	else {
	    new_level(NORMLEV);
	    if (!is_carrying(TR_SILMARIL) || !is_active(TR_SILMARIL))
		msg("You feel a wrenching sensation in your gut.");
	}
	if (on(player, CANINWALL) && ch != STAIRS) {
	    extinguish_fuse(FUSE_UNPHASE);
	    unphase(NULL);
	}
	return;
    }
    else if (ch != STAIRS && 
		!(on(player, CANINWALL)
		&& (ISWEARING(R_LEVITATION)|| on(player, CANFLY))))
	msg("I see no way up.");
    else 
	msg("Your way is magically blocked.");
}


/*
 * allow a user to call a potion, scroll, or ring something
 */
void 
call (bool mark)
{
    struct object *obj;
    struct linked_list *item;
    char **guess = r_guess, *elsewise = "";
    bool *know;

    if (mark) item = get_item("mark", MARKABLE);
    else item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type)
    {
	case RING:
	    guess = r_guess;
	    know = r_know;
	    elsewise = (r_guess[obj->o_which] != NULL ?
			r_guess[obj->o_which] : r_stones[obj->o_which]);
	when POTION:
	    guess = p_guess;
	    know = p_know;
	    elsewise = (p_guess[obj->o_which] != NULL ?
			p_guess[obj->o_which] : p_colors[obj->o_which]);
	when SCROLL:
	    guess = s_guess;
	    know = s_know;
	    elsewise = (s_guess[obj->o_which] != NULL ?
			s_guess[obj->o_which] : s_names[obj->o_which]);
	when STICK:
	    guess = ws_guess;
	    know = ws_know;
	    elsewise = (ws_guess[obj->o_which] != NULL ?
			ws_guess[obj->o_which] : ws_made[obj->o_which]);
	otherwise:
	    if (!mark) {
		msg("You can't call that anything.");
		return;
	    }
	    else know = (bool *) 0;
    }
    if (know && know[obj->o_which] && !mark) {
	msg("That has already been identified.");
	return;
    }
    if (mark) {
	if (obj->o_mark[0]) {
	    msg("Was marked \"%s\".", obj->o_mark);
	}
	msg("What do you want to mark it? ");
    }
    else {
	msg("Was called \"%s\".", elsewise);
	msg("What do you want to call it? ");
    }
    prbuf[0] = '\0';
    if (get_string(prbuf, cw) == NORM) {
	if (mark) {
	    strncpy(obj->o_mark, prbuf, MARKLEN-1);
	    obj->o_mark[MARKLEN-1] = '\0';
	}
	else if (strlen(prbuf) > 0) {
	    if (guess[obj->o_which] != NULL)
		FREE(guess[obj->o_which]);
	    guess[obj->o_which] = my_malloc((unsigned int) strlen(prbuf) + 1);
	    strcpy(guess[obj->o_which], prbuf);
	} else {
	    if (guess[obj->o_which] != NULL)
		FREE(guess[obj->o_which]);
	    guess[obj->o_which] = NULL;
	}
    }
}


/*
 * Pick a monster to fight, so the rogue doesn't have to.
 *
 * If we kill a monster while 'F'ighting, pick another
 * and keep going as long as there's a monster in reach.
 */
bool
pick_monster (char ch)
{
    int x, y, found_monster=0;
    coord found;

    /* initialize to stop gcc uninitialized warnings */
    found.x = hero.x;
    found.y = hero.y;

    if (off(player, ISBLIND)) {
	for (x = hero.x - 1; x <= hero.x + 1; x++) {
	    if (rnd(2)) {
		for (y = hero.y - 1; y <= hero.y + 1; y++) {
		    if (can_fight(x,y)) {
			found_monster++;
			found.x = x - hero.x;
			found.y = y - hero.y;
		    }
		}
	    } else {
		for (y = hero.y + 1; y >= hero.y - 1; y--) {
		    if (can_fight(x,y)) {
			found_monster++;
			found.x = x - hero.x;
			found.y = y - hero.y;
		    }
		}
	    }
	}
    }
    if (found_monster == 1) {
	delta.y = found.y;
	delta.x = found.x;
	return(TRUE);
    } else if (found_monster > 1 && ch == 'F') {
	delta.y = found.y;
	delta.x = found.x;
	return(TRUE);
    } else
	return(FALSE);
}

/*
 * see if there's a monster we'd like to fight at the given position
 */
bool
can_fight (int x, int y)
{
    int mch;
    coord tryp;
    struct linked_list *item;
    struct thing *tp;

    mch = show(y, x);
    tryp.x = x;
    tryp.y = y;
    if (isalpha(mch)
     && diag_ok(&tryp, &hero, &player)
     && (item = find_mons(y, x))
     ) {
	tp = THINGPTR(item);
	if (tp == foe)  /* same monster as last time */
	    return(TRUE);
	if (!serious_fight &&
	    (tp->t_index == nummonst /* quartermaster */
	    || on(*tp, BLOWDIVIDE) || on(*tp, ISFRIENDLY)))
		return(FALSE);
	return(TRUE);
    }
    return(FALSE);
}

