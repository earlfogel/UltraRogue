/*
 * Not much here, any more.
 */

#include <stdio.h>
#include "curses.h"
#include "rogue.h"

#ifdef BSD4
#include <signal.h>

#ifndef _WIN32
void
tstop ()
{
  /*
   * Since we are running with job control,
   * we can assume that the signals remain set after an interupt.
   */

#if defined(__FreeBSD__) || defined(__APPLE__)
  sig_t SigCont;
#else /* __FreeBSD__ */
  __sighandler_t   SigCont;
#endif /* __FreeBSD__ */
  
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
#endif /* !_WIN32 */
#endif /* BSD4 */
