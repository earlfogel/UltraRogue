/*
 * This file has all the code for the option command.
 * I would rather this command were not necessary, but
 * it is the only way to keep the wolves off of my back.
 *
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

typedef union
{
    void *varg;
    char *str;
    int  *iarg;
    bool *barg;
} opt_arg;

struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    opt_arg	o_opt;		/* pointer to thing to set */
    void	(*o_putfunc)(opt_arg *arg,WINDOW *win);	/* function to print value */
    int		(*o_getfunc)(opt_arg *arg,WINDOW *win);	/* function to get value interactively */
};

typedef struct optstruct	OPTION;

void put_bool(opt_arg *o_opt, WINDOW *win);
void put_str(opt_arg *o_opt, WINDOW *win);
void put_abil(opt_arg *o_opt, WINDOW *win);
void put_diff(opt_arg *o_opt, WINDOW *win);
int get_bool(opt_arg *o_opt, WINDOW *win);
int get_str(opt_arg *o_opt, WINDOW *win);
int get_abil(opt_arg *o_opt, WINDOW *win);
int get_mouse(opt_arg *o_opt, WINDOW *win);
int get_diff(opt_arg *o_opt, WINDOW *win);

/*
 * description of an option and what to do with it
 */
OPTION	optlist[] = {
    {"doorstop", "Stop running when adjacent (doorstop): ",
	{&doorstop},	put_bool,	get_bool	},
    {"jump",	 "Show position only at end of run (jump): ",
	{&jump},		put_bool,	get_bool	},
    {"step",	"Do inventories one line at a time (step): ",
	{&slow_invent},	put_bool,	get_bool	},
    {"askme",	"Ask me about unidentified things (askme): ",
	{&askme},		put_bool,	get_bool	},
    {"cutcorners",	"Move sharply around corners (cutcorners): ",
	{&cutcorners},	put_bool,	get_bool	},
    {"showcursor",	"Show cursor while playing (showcursor): ",
	{&showcursor},	put_bool,	get_bool	},
    {"autopickup",	"Pick up things you step on (autopickup): ",
	{&autopickup},	put_bool,	get_bool	},
#ifndef __ANDROID__
    {"autosave",	"Save game automatically (autosave): ",
	{&autosave},	put_bool,	get_bool	},
#endif
#ifdef MOUSE
    {"usemouse",	"Use mouse to move (usemouse): ",
	{&use_mouse},	put_bool,	get_mouse	},
#endif
    {"name",	 "Name (name): ",
	{whoami},		put_str,	get_str		},
    {"fruit",	 "Fruit (fruit): ",
	{fruit},		put_str,	get_str		},
    {"file",	 "Save file (file): ",
	{file_name},	put_str,	get_str		},
    {"score",	 "Score file (score): ",
	{score_file},	put_str,	get_str		},
    {"class",	"Character class (class): ",
	{&char_type},	put_abil,	get_abil	},
    {"difficulty",	"Difficulty: ",
	{&difficulty},	put_diff,	get_diff	},
};

/*
 * print and then set options from the terminal
 */
void 
option ()
{
    OPTION	*op;
    int	retval;

    wclear(hw);
    touchwin(hw);
    /*
     * Display current values of options
     */
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
	waddstr(hw, op->o_prompt);
	(*op->o_putfunc)(&op->o_opt, hw);
	waddch(hw, '\n');
    }
    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
	waddstr(hw, op->o_prompt);
	if ((retval = (*op->o_getfunc)(&op->o_opt, hw))) {
	    if (retval == QUIT)
		break;
	    else if (op > optlist) {	/* MINUS */
		wmove(hw, (op - optlist) - 1, 0);
		op -= 2;
	    }
	    else	/* trying to back up beyond the top */
	    {
		putchar('\007');
		wmove(hw, 0, 0);
		op--;
	    }
	}
    }
    if (showcursor) curs_set(1);
    /*
     * Switch back to original screen
     */
    mvwaddstr(hw, LINES-1, 0, spacemsg);
    draw(hw);
    (void) wgetch(hw);
    clearok(cw, TRUE);
    touchwin(cw);
    after = FALSE;
}

/*
 * put out a boolean
 */
void
put_bool(opt_arg *opt, WINDOW *win)
{
    waddstr(win, *opt->barg ? "True" : "False");
}

/*
 * put out a string
 */
void
put_str(opt_arg *opt, WINDOW *win)
{
    waddstr(win, opt->str);
}

/*
 * print the character type
 */
void
put_abil(opt_arg *opt, WINDOW *win)
{
    char *abil;
#if 0
    if (*opt->iarg < 0)
	*opt->iarg = C_FIGHTER;  /* shouldn't happen, but it did */
#endif
    switch (*opt->iarg) {
	case C_FIGHTER:
	    abil = "Fighter";
	    break;
	case C_MAGICIAN:
	    abil = "Magic User";
	    break;
	case C_CLERIC:
	    abil = "Cleric";
	    break;
	case C_THIEF:
	    abil = "Thief";
	    break;
	default:
	    abil = "??";
	    wprintw(win, "(%d) ", *opt->iarg);
    }
    waddstr(win, abil);
}

/*
 * print the difficulty level
 */
void
put_diff(opt_arg *opt, WINDOW *win)
{
    if (*opt->iarg < 2)
	waddstr(win, "Easy");
    else if (*opt->iarg == 2)
	waddstr(win, "Normal");
    else if (*opt->iarg == 3)
	waddstr(win, "Hard");
    else if (*opt->iarg > 3)
	waddstr(win, "Very Hard");
    wclrtoeol(win);
}

/*
 * allow changing a boolean option and print it out
 */
int
get_bool(opt_arg *opt, WINDOW *win)
{
    int oy, ox;
    bool op_bad;

    curs_set(1);			/* show cursor */
    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *opt->barg ? "True" : "False");
    while(op_bad)	
    {
	wmove(win, oy, ox);
	draw(win);
	switch (wgetch(win))
	{
	    case 't':
	    case 'T':
		*opt->barg = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*opt->barg = FALSE;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
	    case KEY_DOWN:
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		if (!showcursor) curs_set(0);
		return QUIT;
	    case '-':
	    case KEY_UP:
		if (!showcursor) curs_set(0);
		return MINUS;
	    default:
		mvwaddstr(win, oy, ox + 10, "(T or F)");
	}
    }
    wmove(win, oy, ox);
    wclrtoeol(win);
    waddstr(win, *opt->barg ? "True" : "False");
    waddch(win, '\n');
    if (!showcursor) curs_set(0);			/* hide cursor */
    return NORM;
}

/*
 * set a string option
 */
int
get_str(opt_arg *opt, WINDOW *win)
{
    return( get_string(opt->str, win) );
}

int
get_string(char *opt, WINDOW *win)
{
    char *sp;
    int c, oy, ox;
    char buf[LINELEN];

    curs_set(1);			/* show cursor */
    draw(win);
    getyx(win, oy, ox);
    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf;
	(c = wgetch(win)) != '\n'	&& 
	c != '\r'			&& 
	c != '\033'			&& 
	c != '\007'			&&
	c != KEY_DOWN			&&
	sp < &buf[LINELEN-1];
	wclrtoeol(win), draw(win))
    {
	if (c == -1)
	    continue;
	if (c == KEY_UP) {
	    c = '-';
	    break;
	} else if (c == KEY_DOWN) {
	    c = '\n';
	    break;
	} else if (sp == buf) {
	    if (c == '-' && win == hw)	/* To move back a line in hw */
		break;
	    else if (c == '~')
	    {
		strcpy(buf, home);
		waddstr(win, home);
		sp += strlen(home);
		continue;
	    }
	} else if (c == '\010' || c == KEY_BACKSPACE) {
	    *sp-- = '\0';
	    wmove(win, oy, ox+strlen(buf)-1);
	    wclrtoeol(win), draw(win);
	    continue;
	}
	*sp++ = c;
	waddstr(win, unctrl(c));
    }
    *sp = '\0';
    if (sp > buf)	/* only change option if something has been typed */
	strucpy(opt, buf, strlen(buf));
    wmove(win, oy, ox);
    waddstr(win, opt);
    waddch(win, '\n');
    draw(win);
    if (win == cw)
	mpos += sp - buf;
    if (!showcursor) curs_set(0);			/* hide cursor */

    if (c == '-')
	return MINUS;
    else if (c == '\033' || c == '\007')
	return QUIT;
    else
	return NORM;
}

/*
 * The ability field is read-only
 */
int
get_abil(opt_arg *abil, WINDOW *win)
{
    int oy, ox, ny, nx;
    bool op_bad;

    curs_set(1);			/* show cursor */
    op_bad = TRUE;
    getyx(win, oy, ox);
    put_abil(abil, win);
    getyx(win, ny, nx);
    while(op_bad)	
    {
	wmove(win, oy, ox);
	draw(win);
	switch (wgetch(win))
	{
	    case '\n':
	    case '\r':
	    case KEY_DOWN:
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		if (!showcursor) curs_set(0);
		return QUIT;
	    case '-':
	    case KEY_UP:
		if (!showcursor) curs_set(0);
		return MINUS;
	    default:
		mvwaddstr(win, ny, nx + 5, "(no change allowed)");
	}
    }
    wmove(win, ny, nx + 5);
    wclrtoeol(win);
    wmove(win, ny, nx);
    waddch(win, '\n');
    if (!showcursor) curs_set(0);			/* hide cursor */
    return NORM;
}

/*
 *
 * Change difficulty level on the fly
 */
int
get_diff(opt_arg *opt, WINDOW *win)
{
    int oy, ox, ny, nx;
    bool op_bad;
    int ch;
    int old_diff = *opt->iarg;

    curs_set(1);			/* show cursor */
    op_bad = TRUE;
    getyx(win, oy, ox);
    put_diff(opt, win);
    getyx(win, ny, nx);
    while(op_bad)	
    {
	wmove(win, oy, ox);
	draw(win);
	ch = wgetch(win);
	switch (ch)
	{
	    case '\n':
	    case '\r':
	    case KEY_DOWN:
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		if (!showcursor) curs_set(0);
		return QUIT;
	    case '-':
	    case KEY_UP:
		if (!showcursor) curs_set(0);
		return MINUS;
	    default:
		if (ch >= '1' && ch <= '4') {
		    *opt->iarg = ch - '0';
		} else if (tolower(ch) == 'e') {  /* easy */
		    *opt->iarg = 1;
		} else if (tolower(ch) == 'n') {  /* normal */
		    *opt->iarg = 2;
		} else if (tolower(ch) == 'h') {  /* hard */
		    *opt->iarg = 3;
		} else if (tolower(ch) == 'v') {  /* very hard */
		    *opt->iarg = 4;
		} else {
		    mvwaddstr(win, ny, nx + 5, "(Easy, Normal, Hard, Very hard)");
		}
		if (*opt->iarg != old_diff) {
		    put_diff(opt, win);
		    getyx(win, ny, nx);
		    if (*opt->iarg < mindifficulty)
			mindifficulty = *opt->iarg;
		    tweak_settings(FALSE, old_diff);
		    old_diff = *opt->iarg;
		}
	}
    }
    wmove(win, ny, nx + 5);
    wclrtoeol(win);
    wmove(win, ny, nx);
    waddch(win, '\n');
    if (!showcursor) curs_set(0);			/* hide cursor */
    return NORM;
}

#ifdef MOUSE
/*
 *
 * Use mouse click for movement?
 */
int
get_mouse(opt_arg *opt, WINDOW *win)
{
    int ret;
    bool old_mouse = *opt->barg;

    ret = get_bool(opt, win);

    if (*opt->barg != old_mouse) {
	if (use_mouse) {
	    mousemask(BUTTON1_RELEASED, NULL);	/* enable KEY_MOUSE */
	} else {
	    mousemask(0, NULL);			/* disable KEY_MOUSE */
	}
    }

    return ret;
}
#endif

/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

void 
parse_opts (char *str)
{
    char *sp;
    OPTION *op;
    int len;

    while (*str)
    {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len))
	    {
		if (op->o_putfunc == put_bool)	/* if option is a boolean */
		    *op->o_opt.barg = TRUE;
		else				/* string option */
		{
		    char *start;
		    char value[80];

		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    if (*str == '~')
		    {
			strcpy((char *) value, home);
			start = (char *) value + strlen(home);
			while (*++str == '/')
			    continue;
		    }
		    else
			start = (char *) value;
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    strucpy(start, str, sp - str);

		    /* Put the value into the option field */
		    if (op->o_putfunc == put_diff) {
			if (strcasecmp(value,"hard") == 0 || atoi(value) == 3) {
			    *op->o_opt.iarg = 3;
			    mindifficulty = 3;
			} else if (strcasecmp(value,"easy") == 0 || atoi(value) == 1) {
			    *op->o_opt.iarg = 1;
			    mindifficulty = 1;
			}
		    } else if (op->o_putfunc != put_abil)
			strcpy((char *)op->o_opt.str, value);

		    else if (*op->o_opt.iarg == -1) { /* Only init ability once */
			int len = strlen(value);

			if (isupper(value[0])) value[0] = tolower(value[0]);
			if (EQSTR(value, "fighter", len))
				*op->o_opt.iarg = C_FIGHTER;
			else if (EQSTR(value, "magic", min(len, 5)))
				*op->o_opt.iarg = C_MAGICIAN;
			else if (EQSTR(value, "cleric", len))
				*op->o_opt.iarg = C_CLERIC;
			else if (EQSTR(value, "thief", len))
				*op->o_opt.iarg = C_THIEF;
		    }
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_putfunc == put_bool
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
	    {
		*op->o_opt.barg = FALSE;
		break;
	    } else if (strcmp(str,"debug") == 0) {  /* a hidden option */
		canwizard = TRUE;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * copy string using unctrl for things
 */
void 
strucpy (char *s1, char *s2, int len)
{
    const char *sp;

    while (len--)
    {
	strcpy(s1, (sp = unctrl(*s2++)));
	s1 += strlen(sp);
    }
    *s1 = '\0';
}
