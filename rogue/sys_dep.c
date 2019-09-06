/*
 * Not much here, any more.
 */

#include <stdio.h>
#include "curses.h"
#include "rogue.h"
#include "mach_dep.h"
#ifdef BSD4
#include <signal.h>
#endif

#ifdef BSD4

#ifndef _WIN32
void 
tstop ()
{
  /*
   * Since we are running with job control,
   * we can assume that the signals remain set after an interupt.
   */


  __sighandler_t   SigCont;

  /*
   * We are basically commited at this point,
   * so ignore return codes,
   */

  (void) mvcur( 0, COLS-1, LINES-1, 0);
  (void) wrefresh( curscr );
  /*
   * Give us a fresh line to work with
   */
  putc( '\n', stdout );
  SigCont = signal( SIGCONT, SIG_DFL );
  (void) kill( 0, SIGSTOP );
  /*
   * This is the point that the process will stop at
   */

  /*
   * Lets restart the world.
   */
  (void) signal( SIGCONT, (sig_t)SigCont );
  wmove(curscr, hero.y, hero.x);
  (void) wrefresh( curscr );
  setup();
  return;
}
#endif /* BSD4 */
#endif

