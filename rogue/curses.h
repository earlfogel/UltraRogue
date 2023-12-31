/*
 * This is a hack,  pure and simple to allow for
 * tty magic in BSD unix (BSD4)
 *
 * I basically need to have all calls that change the tty state
 * to pass though my hands (in sys_dep.c).
 */


#ifdef _WIN32
#define PDC_WIDE 1
#define PDC_NCMOUSE 1
#include <pdcurses.h>
#else
#include <curses.h>
#ifdef MOUSE
#ifndef PDCURSES
int my_wgetch(WINDOW *);
#define wgetch(win) my_wgetch(win)
#endif
#endif
#ifdef __clang__
#undef mvwinch
#define mvwinch(win,y,x)	(char) (wmove((win),(y),(x)) == ERR ? NCURSES_CAST(chtype, ERR) : winch(win))
#endif
#endif

