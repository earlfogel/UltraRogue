/*
 * machine dependencies
 *
 */

#ifndef _MACH_DEP_
#define _MACH_DEP_

#ifdef	BSD4
#include <stdlib.h>
#endif

#ifdef	FLUTTER
#   include <stdlib.h>
#endif

#ifdef _WIN32
#define random rand
#define srandom srand
#endif

#ifdef	USGV3
#define	_doprnt	_dprnt
#define	setegid	setgid
#define	seteuid	setuid
#endif

#endif
