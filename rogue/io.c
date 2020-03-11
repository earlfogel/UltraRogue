/*
 * Various input/output functions
 */

#include "curses.h"
#include "mach_dep.h"
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "rogue.h"

static char mbuf[2*BUFSIZ];
static int newpos = 0;

/*
 * msg:
 *	Display a message at the top of the screen.
 */


void 
msg (char * fmt,...)
{
    va_list ap;

    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    wmove(cw, 0, 0);

    va_start(ap,fmt);
    vsprintf(&mbuf[newpos],fmt,ap);
    va_end(ap);
    newpos = strlen(mbuf);
    endmsg();
}

/*
 * add things to the current message
 */
void 
addmsg (char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    vsprintf(&mbuf[newpos],fmt,ap);
    va_end(ap);
    newpos = strlen(mbuf);
}

/*
 * Display a new msg (giving a chance to see the previous one if it
 * is up there with the --More--)
 */
void 
endmsg ()
{
    if (strlen(mbuf) > COLS)
	mbuf[COLS] = '\0';  /* stop overruns */
    strcpy(msgbuf[msg_index], mbuf);
    ++msg_index;
    msg_index = msg_index % 10;
    if (mpos)
    {
	wmove(cw, 0, mpos);
	waddnstr(cw, morestr, COLS - mpos);
	draw(cw);
	wait_for(0);
    }
    mvwaddstr(cw, 0, 0, mbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    draw(cw);
}


/*
 * step_ok:
 *	returns true if it is ok for type to step on ch
 *	flgptr will be NULL if we don't know what the monster is yet!
 */

int 
step_ok (y, x, can_on_monst, flgptr)
int y;
int x;
int can_on_monst;
struct thing *flgptr;
{
    struct linked_list *item;
    char ch;

    /* What is here?  Don't check monster window if MONSTOK is set */
    if (can_on_monst == MONSTOK) ch = mvinch(y, x);
    else ch = winat(y, x);

    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    if (flgptr && on(*flgptr, CANINWALL)) return(TRUE);
	    return FALSE;
	when SCROLL:
	    /*
	     * If it is a scroll, it might be a scare monster scroll
	     * so we need to look it up to see what type it is.
	     */
	    if (flgptr && flgptr->t_ctype == C_MONSTER) {
		item = find_obj(y, x);
		if (item != NULL && (OBJPTR(item))->o_type == SCROLL &&
		    (OBJPTR(item))->o_which == S_SCARE &&
		    rnd(flgptr->t_stats.s_intel) < 12)
			return(FALSE); /* All but smart ones are scared */
	    }
	    return(TRUE);
	otherwise:
	    return (!isalpha(ch));
    }
}
/*
 * shoot_ok:
 *	returns true if it is ok for type to shoot over ch
 */

int 
shoot_ok (ch)
int ch;
{
    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	read one character
 */

int 
readchar ()
{
    return(wgetch(cw));
}


/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

void 
status (display)
bool display;
{
    struct stats *stat_ptr, *max_ptr;
    int oy, ox, temp;
    char *pb;
    static char buf[LINELEN];
    static int hpwidth = 0, s_hungry = -1;
    static int s_lvl = -1, s_pur, s_hp = -1, s_str, maxs_str, 
		s_ac = 0;
    static short s_intel, s_dext, s_wisdom, s_const, s_charisma;
    static short maxs_intel, maxs_dext, maxs_wisdom, maxs_const, maxs_charisma;
    static unsigned long s_exp = 0;
    static int s_carry, s_pack;
    char *health_state = NULL;
    static char *s_health_state;
    bool first_line=FALSE;

    /* Use a mini status version if we have a small window */
    if (COLS < 80) {
	ministat();
	return;
    }

    stat_ptr = &pstats;
    max_ptr  = &max_stats;

    /*
     * If nothing has changed in the first line, then skip it
     */
    if (!display				&&
	s_lvl == level				&& 
	s_intel == stat_ptr->s_intel		&&
	s_wisdom == stat_ptr->s_wisdom		&&
	s_dext == stat_ptr->s_dext		&& 
	s_const == stat_ptr->s_const		&&
	s_charisma == stat_ptr->s_charisma	&&
	s_str == stat_ptr->s_str		&& 
	s_pack == stat_ptr->s_pack		&&
	s_carry == stat_ptr->s_carry		&&
	maxs_intel == max_ptr->s_intel		&& 
	maxs_wisdom == max_ptr->s_wisdom	&&
	maxs_dext == max_ptr->s_dext		&& 
	maxs_const == max_ptr->s_const		&&
	maxs_charisma == max_ptr->s_charisma	&&
	maxs_str == max_ptr->s_str		) goto line_two;

    /* Display the first line */
    first_line = TRUE;
    getyx(cw, oy, ox);
    sprintf(buf, "Int:%d(%d)  Str:%d", stat_ptr->s_intel,
    	max_ptr->s_intel, stat_ptr->s_str);

    /* Maximum strength */
    pb = &buf[strlen(buf)];
    sprintf(pb, "(%d)", max_ptr->s_str);

    pb = &buf[strlen(buf)];
    sprintf(pb, "  Wis:%d(%d)  Dxt:%d(%d)  Const:%d(%d)  Carry:%d(%d)",
	stat_ptr->s_wisdom,max_ptr->s_wisdom,stat_ptr->s_dext,max_ptr->s_dext,
	stat_ptr->s_const,max_ptr->s_const,stat_ptr->s_pack/10,
	stat_ptr->s_carry/10);

    /* Update first line status */
    s_intel = stat_ptr->s_intel;
    s_wisdom = stat_ptr->s_wisdom;
    s_dext = stat_ptr->s_dext;
    s_const = stat_ptr->s_const;
    s_charisma = stat_ptr->s_charisma;
    s_str = stat_ptr->s_str;
    s_pack = stat_ptr->s_pack;
    s_carry = stat_ptr->s_carry;
    maxs_intel = max_ptr->s_intel;
    maxs_wisdom = max_ptr->s_wisdom;
    maxs_dext = max_ptr->s_dext;
    maxs_const = max_ptr->s_const;
    maxs_charisma = max_ptr->s_charisma;
    maxs_str = max_ptr->s_str;

    /* Print the line */
    mvwaddstr(cw, LINES - 2, 0, buf);
    wclrtoeol(cw);

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
line_two: 

    /*
     * work out current health
     */
    if (find_slot(FUSE, FUSE_SUFFOCATE) != NULL) {
	health_state = "  Suffocating";
    } else if (on(player, HASINFEST)) {
	health_state = "  Ill";
    } else if (on(player, HASDISEASE)) {
	health_state = "  Sick";
    } else if (on(player, HASITCH)) {
	health_state = "  Itchy";
    } else if (on(player, HASSTINK)) {
	health_state = "  Sickened";
    } else if (on(player, ISBLIND)) {
	health_state = "  Blind";
    } else if (on(player, ISDEAF)) {
	health_state = "  Deaf";
    } else if (on(player, ISSLOW)) {
	health_state = "  Slow";
    } else if (on(player, ISFLEE)) {
	health_state = "  Terrified";
    } else if (stat_ptr->s_intel < 8) {
	health_state = "  Dim-witted";
    } else if (stat_ptr->s_str < 8) {
	health_state = "  Feeble";
    } else if (stat_ptr->s_wisdom < 8) {
	health_state = "  Clueless";
    } else if (stat_ptr->s_dext < 8) {
	health_state = "  Bumbling";
    } else if (stat_ptr->s_const < 8) {
	health_state = "  Frail";
#if 0
    } else if (on(player, ISHUH) && fighting) {
	health_state = "  Confused fight";
    } else if (fighting && serious_fight) {
	health_state = "  Fighting";
    } else if (on(player, CANINWALL) && p_know[P_PHASE]) {
	health_state = "  Phasing";
#endif
    } else if (on(player, ISHUH)) {
	health_state = "  Confused";
    } else if (on(player, ISUNSMELL)) {
	health_state = "  Unscented";
    } else if (on(player, STUMBLER)) {
	health_state = "  Limping";
    } else if (on(player, SUPEREAT) || on(player, POWEREAT)) {
	health_state = "  Getting Hungry";
    } else if (cur_armor == NULL && cur_weapon == NULL) {
	health_state = "  No Armor, No Weapon";
    } else if (cur_armor == NULL) {
	health_state = "  No Armor";
    } else if (cur_weapon == NULL) {
	health_state = "  No Weapon";
    } else if (wizard) {
	health_state = "  Debug Mode";
    } else {
	health_state = NULL;
    }

    if (!display				&&
	s_hp == stat_ptr->s_hpt			&& 
	s_exp == stat_ptr->s_exp		&& 
        s_pur == purse && 
	s_ac == (cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
		: stat_ptr->s_arm) - ring_value(R_PROTECT) && 
	s_health_state == health_state		&& 
	s_lvl == level 				&& 
	s_hungry == hungry_state		) return;
	
    if (!first_line) getyx(cw, oy, ox);
    if (s_hp != max_ptr->s_hpt)
    {
	temp = s_hp = max_ptr->s_hpt;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    sprintf(buf, "Lvl:%d  Au:%d  Hp:%*d(%*d)  Ac:%d  Exp:%d/%ld  %s",
	level, purse, hpwidth, stat_ptr->s_hpt, hpwidth, max_ptr->s_hpt,
	(cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
		: stat_ptr->s_arm) - ring_value(R_PROTECT),
	stat_ptr->s_lvl, stat_ptr->s_exp,
	cnames[player.t_ctype][min(stat_ptr->s_lvl-1, 10)]);

    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = stat_ptr->s_hpt;
    s_exp = stat_ptr->s_exp; 
    s_ac = (cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
	: stat_ptr->s_arm) - ring_value(R_PROTECT);
    s_health_state = health_state;
    mvwaddstr(cw, LINES - 1, 0, buf);
    switch (hungry_state)
    {
	case F_OK:
	    if (health_state != NULL) {
		waddstr(cw, health_state);
	    } else {
		wclrtoeol(cw);
	    }
	when F_HUNGRY:
	    waddstr(cw, "  Hungry");
	when F_WEAK:
	    waddstr(cw, "  Weak");
	when F_FAINT:
	    waddstr(cw, "  Fainting");
    }
    wclrtoeol(cw);
    s_hungry = hungry_state;
    wmove(cw, oy, ox);
}

void 
ministat ()
{
    int oy, ox, temp;
    static char buf[LINELEN];
    static int hpwidth = 0;
    static int s_lvl = -1, s_pur, s_hp = -1;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_pur == purse && s_lvl == level)
	    return;
	
    getyx(cw, oy, ox);
    if (s_hp != max_stats.s_hpt)
    {
	temp = s_hp = max_stats.s_hpt;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    sprintf(buf, "Lv: %d  Au: %-5d  Hp: %*d(%*d)",
	level, purse, hpwidth, pstats.s_hpt, hpwidth, max_stats.s_hpt);

    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    mvwaddstr(cw, LINES - 1, 0, buf);
    wclrtoeol(cw);
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the player types the right key
 */

void 
wait_for (ch)
int ch;
{
    int c;

    save_ch = '\0';

    if (ch == '\n') {
        while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    } else if (ch == 0) {
	while ((c = readchar()) >= KEY_MIN)  /* ignore arrow keys */
	    continue;
	if (c == ESCAPE) {
	    fighting = FALSE;
	    count = 0;
	} else if (c != '\n' && c != ' ' && c != '\0') {
	    save_ch = c;  /* save to use as next command */
	}
    } else {
        while (readchar() != ch)
	    continue;
    }
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

void
show_win(scr, message)
WINDOW *scr;
char *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    draw(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * dbotline:
 *	Displays message on bottom line and waits for a space to return
 */
void
dbotline(scr,message)
WINDOW *scr;
char *message;
{
	mvwaddstr(scr,LINES-1,0,message);
	draw(scr);
	wait_for(' ');	
}


/*
 * restscr:
 *	Restores the screen to the terminal
 */
void
restscr(scr)
WINDOW *scr;
{
	clearok(scr,TRUE);
	touchwin(scr);
}
