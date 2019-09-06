/*
 * machine dependencies
 *
 */

#ifndef _MACH_DEP_
#define _MACH_DEP_

#ifdef	BSD4
#include <stdlib.h>

#ifdef _WIN32
#define	srand48	srand
#define	lrand48	rand
#else
#define	strchr	index
#define srand48	srandom
#define lrand48	random
#endif
#endif

#ifdef	USGV3
#define	srand48	srand
#define	lrand48	rand
#define	_doprnt	_dprnt
#define	setegid	setgid
#define	seteuid	setuid
#endif

#endif
