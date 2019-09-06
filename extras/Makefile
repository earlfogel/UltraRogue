TUNABLE=tunable.o
INSD = /usr/games/lib/urogue/src

CFLAGS = -g -O0 -I../rogue
LIB = rogue/libsrogue.o
DISTRIB = charfile scorefile sizeof
SOURCES = Makefile charfile scorefile.c sizeof.c

all: $(DISTRIB)

clean:
	rm -rf $(DISTRIB) *.o

distribclean:
	@echo "making distrib clean"
	rm -rf $(INSD)
