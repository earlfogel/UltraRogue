#
# Makefile for srogue
#

HDRS=	rogue.h mach_dep.h tunable.h curses.h 
OBJS= 	vers.o armor.o artifact.o chase.o command.o daemon.o daemons.o fight.o \
	encumb.o get_play.o init.o io.o list.o main.o maze.o misc.o \
	monsdata.o monsters.o move.o new_level.o options.o pack.o passages.o \
	player.o potions.o rings.o rip.o rooms.o save.o state.o scrolls.o \
	sticks.o things.o trader.o tunable.o sys_dep.o weapons.o wizard.o rogue.o \
	mouse.o

CFILES= vers.c armor.c artifact.c chase.c command.c daemon.c daemons.c fight.c \
	encumb.c get_play.c init.c io.c list.c main.c maze.c misc.c \
	monsdata.c monsters.c move.c new_level.c options.c pack.c passages.c \
	player.c potions.c rings.c rip.c rooms.c save.c state.c scrolls.c \
	sticks.c things.c trader.c tunable.c sys_dep.c weapons.c wizard.c rogue.c \
	mouse.c

SOURCES = $(CFILES) rogue.h mach_dep.h Makefile

CRLIB=	 -lncurses

# Installation variables.
# install srogue here
BIN=	/home/earl/bin
UMASK=	0755
CHMOD=	chmod
CP=	cp
RM=	rm -f
LD = 	ld
CTAGS=	ctags
TOUCH=	touch
PROGRAM=	urogue
TEST=		tidy
OPROGRAM=	urogue.bak

#LDFLAGS= -fsanitize=address -fsanitize=undefined
#CFLAGS= -O0 -g -DBSD4=1 -Wall -fsanitize=address -fsanitize=undefined

# VERY IMPORTANT -----
# BSD4 is defined if the system you are on is a 4.x BSD system
#CFLAGS= -O -DBSD4=1
#CFLAGS= -O0 -g -DBSD4=1 -Wall # -fwritable-strings # -traditional
# USGV3 is defined if the system you are on is a System 3 system
# CFLAGS= -O -DUSGV3=1
#CFLAGS= -O0 -g -DBSD4=1 -Wall
CFLAGS= -O -DBSD4=1 -DMOUSE -Wall -Wextra -Wformat=2 -Wno-old-style-definition

# for clang/LLVM
#CC=clang
#CFLAGS= -O -DBSD4=1 -DMOUSE -Wall -Wextra -Wformat=2 \
#	-Wno-deprecated-non-prototype \
#	-Wno-knr-promoted-parameter \
#	-Wno-unused-but-set-variable \
#	-Wno-sometimes-uninitialized


$(PROGRAM):	$(OBJS) $(MAKEFILE)
		$(CC) $(LDFLAGS) $(OBJS) $(CRLIB) -o $@
		@echo $@ made.

test:		$(PROGRAM)
		$(CP) $(PROGRAM) $(BIN)/$(TEST)
		$(RM) $(PROGRAM)
		$(CHMOD) $(UMASK) $(BIN)/$(TEST)

install:	
		/bin/mv $(BIN)/$(PROGRAM) $(BIN)/$(OPROGRAM)
		$(CP) $(TEST) $(BIN)/$(PROGRAM)
		$(CHMOD) $(UMASK) $(BIN)/$(PROGRAM)

tags:
		$(CTAGS) $(HDRS) $(CFILES)

clean:
		rm -f $(OBJS) $(PROGRAM) core 

count:
		wc -l $(HDRS) $(CFILES)

depend:
	grep '^#[ 	]*include[ 	]*"' ${CFILES} | \
	sed -e 's/:[^"]*"\([^"]*\)"/:	\1/' -e 's/\.c/.o/' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '' >> Makefile
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it


armor.o:	curses.h rogue.h
artifact.o:	curses.h rogue.h
chase.o:	curses.h rogue.h
command.o:	curses.h rogue.h
daemon.o:	curses.h rogue.h
daemons.o:	curses.h rogue.h
fight.o:	curses.h rogue.h mach_dep.h
encumb.o:	curses.h rogue.h
get_play.o:	curses.h rogue.h
init.o:		curses.h rogue.h
io.o:		curses.h rogue.h mach_dep.h
list.o:		curses.h rogue.h
main.o:		curses.h mach_dep.h rogue.h tunable.h
maze.o:		curses.h rogue.h
misc.o:		curses.h rogue.h
monsdata.o:	curses.h rogue.h
monsters.o:	curses.h rogue.h mach_dep.h
mouse.o:	curses.h rogue.h
move.o:		curses.h rogue.h
new_level.o:	curses.h rogue.h
options.o:	curses.h rogue.h mach_dep.h 
pack.o:		curses.h rogue.h
passages.o:	curses.h rogue.h
player.o:	curses.h rogue.h
potions.o:	curses.h rogue.h
rings.o:	curses.h rogue.h
rip.o:		curses.h rogue.h mach_dep.h 
rooms.o:	curses.h rogue.h
save.o:		curses.h rogue.h mach_dep.h
scrolls.o:	curses.h rogue.h
save.o:		curses.h rogue.h state.h
state.o:	curses.h rogue.h state.h
sticks.o:	curses.h rogue.h
things.o:	curses.h rogue.h
trader.o:	curses.h rogue.h
tunable.o:      tunable.h
sys_dep.o:	curses.h rogue.h mach_dep.h
weapons.o:	curses.h rogue.h
wizard.o:	curses.h rogue.h mach_dep.h
rogue.o:	curses.h rogue.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
