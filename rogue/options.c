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


/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    int		*o_opt;		/* pointer to thing to set */
    void	(*o_putfunc)();	/* function to print value */
    int		(*o_getfunc)();	/* function to get value interactively */
};

typedef struct optstruct	OPTION;

int	get_bool(), get_str(), get_abil(), get_diff();
void put_diff(int *diff, WINDOW *win);

OPTION	optlist[] = {
    {"doorstop", "Stop running when adjacent (doorstop): ",
		 (int *) &doorstop,	put_bool,	get_bool	},
    {"jump",	 "Show position only at end of run (jump): ",
		 (int *) &jump,		put_bool,	get_bool	},
    {"step",	"Do inventories one line at a time (step): ",
		(int *) &slow_invent,	put_bool,	get_bool	},
    {"askme",	"Ask me about unidentified things (askme): ",
		(int *) &askme,		put_bool,	get_bool	},
    {"cutcorners",	"Move sharply around corners (cutcorners): ",
		(int *) &cutcorners,	put_bool,	get_bool	},
    {"showcursor",	"Show cursor while playing (showcursor): ",
		(int *) &showcursor,	put_bool,	get_bool	},
    {"name",	 "Name (name): ",
		(int *) whoami,		put_str,	get_str		},
    {"fruit",	 "Fruit (fruit): ",
		(int *) fruit,		put_str,	get_str		},
    {"file",	 "Save file (file): ",
		(int *) file_name,	put_str,	get_str		},
    {"score",	 "Score file (score): ",
		(int *) score_file,	put_str,	get_str		},
    {"class",	"Character class (class): ",
		(int *) &char_type,	put_abil,	get_abil	},
    {"difficulty",	"Difficulty: ",
		(int *) &difficulty,	put_diff,	get_diff	},
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
	(*op->o_putfunc)(op->o_opt, hw);
	waddch(hw, '\n');
    }
    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
	waddstr(hw, op->o_prompt);
	if ((retval = (*op->o_getfunc)(op->o_opt, hw))) {
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
put_bool(b, win)
bool	*b;
WINDOW *win;
{
    waddstr(win, *b ? "True" : "False");
}

/*
 * put out a string
 */
void
put_str(str, win)
char *str;
WINDOW *win;
{
    waddstr(win, str);
}

/*
 * print the character type
 */
void
put_abil(ability, win)
int *ability;
WINDOW *win;
{
    char *abil;

    switch (*ability) {
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
    }
    waddstr(win, abil);
}

/*
 * print the difficulty level
 */
void
put_diff(diff, win)
int *diff;
WINDOW *win;
{
    if (difficulty < 2)
	waddstr(win, "Easy");
    else if (difficulty == 2)
	waddstr(win, "Normal");
    else if (difficulty > 2)
	waddstr(win, "Hard");
    wclrtoeol(win);
}


/*
 * allow changing a boolean option and print it out
 */
int
get_bool(bp, win)
bool *bp;
WINDOW *win;
{
    int oy, ox;
    bool op_bad;

    curs_set(1);			/* show cursor */
    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    while(op_bad)	
    {
	wmove(win, oy, ox);
	draw(win);
	switch (wgetch(win))
	{
	    case 't':
	    case 'T':
		*bp = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*bp = FALSE;
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
    waddstr(win, *bp ? "True" : "False");
    waddch(win, '\n');
    if (!showcursor) curs_set(0);			/* hide cursor */
    return NORM;
}

/*
 * set a string option
 */
int
get_str(opt, win)
char *opt;
WINDOW *win;
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
get_abil(abil, win)
int *abil;
WINDOW *win;
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
get_diff(diff, win)
int *diff;
WINDOW *win;
{
    int oy, ox, ny, nx;
    bool op_bad;
    int ch;
    int old_diff = difficulty;

    curs_set(1);			/* show cursor */
    op_bad = TRUE;
    getyx(win, oy, ox);
    put_diff(diff, win);
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
		if (ch >= '1' && ch <= '3') {
		    *diff = ch - '0';
		    put_diff(diff, win);
		    getyx(win, ny, nx);
		} else if (tolower(ch) == 'e') {  /* easy */
		    *diff = 1;
		    put_diff(diff, win);
		    getyx(win, ny, nx);
		} else if (tolower(ch) == 'n') {  /* normal */
		    *diff = 2;
		    put_diff(diff, win);
		    getyx(win, ny, nx);
		} else if (tolower(ch) == 'h') {  /* hard */
		    *diff = 3;
		    put_diff(diff, win);
		    getyx(win, ny, nx);
		} else {
		    mvwaddstr(win, ny, nx + 5, "('E'asy, 'N'ormal, 'H'ard)");
		}
		if (*diff != old_diff) {
		    if (*diff < mindifficulty)
			mindifficulty = *diff;
		    tweak_settings(FALSE, old_diff);
		    old_diff = *diff;
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


/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

void 
parse_opts (str)
char *str;
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
		    *(bool *)op->o_opt = TRUE;
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
			    *op->o_opt = 3;
			    mindifficulty = 3;
			} else if (strcasecmp(value,"easy") == 0 || atoi(value) == 1) {
			    *op->o_opt = 1;
			    mindifficulty = 1;
			}
		    } else if (op->o_putfunc != put_abil)
			strcpy((char *)op->o_opt, value);

		    else if (*op->o_opt == -1) { /* Only init ability once */
			int len = strlen(value);

			if (isupper(value[0])) value[0] = tolower(value[0]);
			if (EQSTR(value, "fighter", len))
				*op->o_opt = C_FIGHTER;
			else if (EQSTR(value, "magic", min(len, 5)))
				*op->o_opt = C_MAGICIAN;
			else if (EQSTR(value, "cleric", len))
				*op->o_opt = C_CLERIC;
			else if (EQSTR(value, "thief", len))
				*op->o_opt = C_THIEF;
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
		*(bool *)op->o_opt = FALSE;
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
strucpy (s1, s2, len)
char *s1;
char *s2;
int len;
{
    const char *sp;

    while (len--)
    {
	strcpy(s1, (sp = unctrl(*s2++)));
	s1 += strlen(sp);
    }
    *s1 = '\0';
}
