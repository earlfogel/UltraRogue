/*
 * This is a hack,  pure and simple to allow for
 * tty magic in BSD unix (BSD4)
 *
 * I basically need to have all calls that change the tty state
 * to pass though my hands (in sys_dep.c).
 */


#ifdef _WIN32
#define PDC_WIDE 1
#include <pdcurses.h>
#else
#include <curses.h>
#endif

