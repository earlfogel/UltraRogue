/*
 * Rogue definitions and variable declarations
 *
 */
#include "tunable.h"
#include "mach_dep.h"

/*
 * Stuff earl added
 */
#define remove(a,b) my_remove(a,b)
#undef NULL
#define NULL 0

/*
 * Maximum number of different things
 */
#define	MAXROOMS	9
#define	MAXTHINGS	9
#define	MAXOBJ		7
#define	MAXPACK		23
#define	MAXTREAS	30	/* number monsters/treasure in treasure room */
#define	MAXTRAPS	30	/* max traps per level */
#define	MAXTRPTRY	16	/* attempts/level allowed for setting traps */
#define	MAXDOORS	4	/* Maximum doors to a room */
#define NUMUNIQUE	48      /* Number of UNIQUE creatures */
#define NLEVMONS	3       /* Number of new monsters per level */
#define NT_FLAGS	16	/* Number of struct thing flags */
#define NM_FLAGS	16	/* Number of struct monster flags */
#define NMSGS		10	/* Number of messages in msgbuf */
#define NFINGERS	8	/* Number of ring fingers */
#define MAXPURCH	8	/* max purchases per trading post visit */
#define LINELEN		80	/* characters in a buffer */


/* Movement penalties */
#define BACKPENALTY 3
#define SHOTPENALTY 2		/* In line of sight of missile */
#define DOORPENALTY 1		/* Moving out of current room */

/*
 * stuff to do with encumberance
 */
#define NORMENCB	1500	/* normal encumberance */
#define F_OK		 0	/* have plenty of food in stomach */
#define F_HUNGRY	 1	/* player is hungry */
#define F_WEAK		 2	/* weak from lack of food */
#define F_FAINT		 3	/* fainting from lack of food */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/* 
 * The character types
 */
#define	C_FIGHTER	0
#define	C_MAGICIAN	1
#define	C_CLERIC	2
#define	C_THIEF		3
#define	C_MONSTER	4

/*
 * values for games end
 */
#define SCOREIT -1
#define KILLED 	 0
#define CHICKEN  1
#define WINNER   2
#define TOTAL    3

/*
 * definitions for function step_ok:
 *	MONSTOK indicates it is OK to step on a monster -- it
 *	is only OK when stepping diagonally AROUND a monster
 */
#define MONSTOK 1
#define NOMONST 2

/*
 * used for ring stuff
 */
#define LEFT_1	 0
#define LEFT_2	 1
#define LEFT_3	 2
#define LEFT_4	 3
#define RIGHT_1	 4
#define RIGHT_2	 5
#define RIGHT_3	 6
#define RIGHT_4	 7

/*
 * All the fun defines
 */
#define next(ptr) ((ptr)?(ptr->l_next):NULL)
#define prev(ptr) ((ptr)?(ptr->l_prev):NULL)
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define winat(y, x) (mvwinch(mw, y, x)==' '?mvwinch(stdscr, y, x):winch(mw))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)	(struct object *)((*what).l_data)
#define THINGPTR(what)	(struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#ifndef	CTRL
#define CTRL(ch) (ch & 037)
#endif
#define ALLOC(x) malloc((size_t) x)
#define FREE(x) free((void *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT_1, r) || ISRING(LEFT_2, r) ||\
			 ISRING(LEFT_3, r) || ISRING(LEFT_4, r) ||\
			 ISRING(RIGHT_1, r) || ISRING(RIGHT_2, r) ||\
			 ISRING(RIGHT_3, r) || ISRING(RIGHT_4, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == FOOD)
#define isrock(ch) ((ch == WALL) || (ch == '-') || (ch == '|'))
#define is_stealth(tp) \
    (rnd(25) < (tp)->t_stats.s_dext || (tp == &player && ISWEARING(R_STEALTH)))
#ifdef	TCFLSH
#define flushout() ioctl(_tty_ch,TCFLSH,0)
#endif
#define mi_wght mi_worth

/*
 * Ways to die
 */
#define	D_PETRIFY	-1
#define	D_ARROW		-2
#define	D_DART		-3
#define	D_POISON	-4
#define	D_BOLT		-5
#define	D_SUFFOCATION	-6
#define	D_POTION	-7
#define	D_INFESTATION	-8
#define D_DROWN		-9
#define D_FALL		-10
#define D_FIRE		-11
#define D_MISADVENTURE	-12

/*
 * Things that appear on the screens
 */
#define	WALL		' '
#define	PASSAGE		'#'
#define	DOOR		'+'
#define	FLOOR		'.'
#define VPLAYER 	'@'
#define IPLAYER 	'_'
#define	POST		'^'
#define LAIR		'('
#define RUSTTRAP	';'
#define	TRAPDOOR	'>'
#define	ARROWTRAP	'{'
#define	SLEEPTRAP	'$'
#define	BEARTRAP	'}'
#define	TELTRAP		'~'
#define	DARTTRAP	'`'
#define POOL		'"'
#define MAZETRAP	'\\'
#define FIRETRAP	'<'
#define POISONTRAP	'['
#define ARTIFACT	','
#define	SECRETDOOR	'&'
#define	STAIRS		'%'
#define	GOLD		'*'
#define	POTION		'!'
#define	SCROLL		'?'
#define	MAGIC		'$'
#define	BMAGIC		'>'	/*	Blessed	magic	*/
#define	CMAGIC		'<'	/*	Cursed	magic	*/
#define	FOOD		':'
#define	WEAPON		')'
#define	ARMOR		']'
#define	RING		'='
#define	STICK		'/'
#define	CALLABLE	-1
#define	MARKABLE	-2

/*
 * Various constants
 */
#define	MAXAUTH		10		/* Let's be realistic! */
#define	BEARTIME	3
#define	SLEEPTIME	4
#define	FREEZETIME	6
#define	HEALTIME	30
#define	HOLDTIME	2
#define	CHILLTIME	(roll(2, 4))
#define	SMELLTIME	20
#define	STONETIME	8
#define	SICKTIME	10
#define	STPOS		0
#define	WANDERTIME	70
#define HEROTIME	20
#define	BEFORE		1
#define	AFTER		2
#define	HUHDURATION	20
#define	SEEDURATION	850
#define	CLRDURATION	15
#define GONETIME	200
#define	PHASEDURATION	300
#define	HUNGERTIME	1300
#define	MORETIME	150
#define	STINKTIME	6
#define	STOMACHSIZE	2000
#define	ESCAPE		27
#define LINEFEED	10
#define CARRIAGE_RETURN 13
#define	BOLT_LENGTH	10
#define	MARKLEN		20

/*
 * Save against things
 */
#define	VS_POISON		0
#define	VS_PARALYZATION		0
#define	VS_DEATH		0
#define	VS_PETRIFICATION	1
#define	VS_WAND			2
#define	VS_BREATH		3
#define	VS_MAGIC		4

/*
 * attributes for treasures in dungeon
 */
#define ISCURSED     	       01
#define ISKNOW      	       02
#define ISPOST		       04	/* object is in a trading post */
#define	ISMETAL     	      010
#define ISPROT		      020	/* object is protected */
#define ISBLESSED     	      040
#define ISZAPPED             0100	/* weapon has been charged by dragon */
#define ISVORPED	     0200	/* vorpalized weapon */
#define ISSILVER	     0400	/* silver weapon */
#define ISPOISON	    01000	/* poisoned weapon */
#define CANRETURN	    02000	/* weapon returns if misses */
#define ISOWNED		    04000	/* weapon returns always */
#define ISLOST		   010000	/* weapon always disappears */
#define ISMISL      	   020000
#define ISMANY     	   040000
#define CANBURN           0100000	/* burns monsters */
#define IS2PROT           0200000	/* doubly protected */
/*
 * Various flag bits
 */
#define ISDARK	    	       01
#define ISGONE	    	       02
#define	ISTREAS     	       04
#define ISFOUND     	      010
#define ISTHIEFSET	      020

/* struct thing t_flags (might include player) for monster attributes */
#define ISBLIND     0x00000001UL
#define ISINWALL    0x00000002UL
#define ISRUN       0x00000004UL
#define ISFLEE      0x00000008UL
#define ISINVIS     0x00000010UL
#define ISMEAN      0x00000020UL
#define ISGREED     0x00000040UL
#define CANSHOOT    0x00000080UL
#define ISHELD      0x00000100UL
#define ISHUH       0x00000200UL
#define ISREGEN     0x00000400UL
#define CANHUH      0x00000800UL
#define CANSEE      0x00001000UL
#define HASFIRE     0x00002000UL
#define ISSLOW      0x00004000UL
#define ISHASTE     0x00008000UL
#define ISCLEAR     0x00010000UL
#define CANINWALL   0x00020000UL
#define ISDISGUISE  0x00040000UL
#define CANBLINK    0x00080000UL
#define CANSNORE    0x00100000UL
#define HALFDAMAGE  0x00200000UL
#define CANSUCK     0x00400000UL
#define CANRUST     0x00800000UL
#define CANPOISON   0x01000000UL
#define CANDRAIN    0x02000000UL
#define ISUNIQUE    0x04000000UL
#define STEALGOLD   0x08000000UL

#define STEALMAGIC  0x10000001UL
#define CANDISEASE  0x10000002UL
#define HASDISEASE  0x10000004UL
#define CANSUFFOCATE    0x10000008UL
#define DIDSUFFOCATE    0x10000010UL
#define BOLTDIVIDE  0x10000020UL
#define BLOWDIVIDE  0x10000040UL
#define NOCOLD      0x10000080UL
#define TOUCHFEAR   0x10000100UL
#define BMAGICHIT   0x10000200UL
#define NOFIRE      0x10000400UL
#define NOBOLT      0x10000800UL
#define CARRYGOLD   0x10001000UL
#define CANITCH     0x10002000UL
#define HASITCH     0x10004000UL
#define DIDDRAIN    0x10008000UL
#define WASTURNED   0x10010000UL
#define CANSELL     0x10020000UL
#define CANBLIND    0x10040000UL
#define CANBBURN    0x10080000UL
#define ISCHARMED   0x10100000UL
#define CANSPEAK    0x10200000UL
#define CANFLY      0x10400000UL
#define ISFRIENDLY  0x10800000UL
#define CANHEAR     0x11000000UL
#define ISDEAF      0x12000000UL
#define CANSCENT    0x14000000UL
#define ISUNSMELL   0x18000000UL

#define WILLRUST    0x20000001UL
#define WILLROT     0x20000002UL
#define SUPEREAT    0x20000004UL
#define PERMBLIND   0x20000008UL
#define MAGICHIT    0x20000010UL
#define CANINFEST   0x20000020UL
#define HASINFEST   0x20000040UL
#define NOMOVE      0x20000080UL
#define CANSHRIEK   0x20000100UL
#define CANDRAW     0x20000200UL
#define CANSMELL    0x20000400UL
#define CANPARALYZE 0x20000800UL
#define CANROT      0x20001000UL
#define ISSCAVENGE  0x20002000UL
#define DOROT       0x20004000UL
#define CANSTINK    0x20008000UL
#define HASSTINK    0x20010000UL
#define ISSHADOW    0x20020000UL
#define CANCHILL    0x20040000UL
#define CANHUG      0x20080000UL
#define CANSURPRISE 0x20100000UL
#define CANFRIGHTEN 0x20200000UL
#define CANSUMMON   0x20400000UL
#define TOUCHSTONE  0x20800000UL
#define LOOKSTONE   0x21000000UL
#define CANHOLD     0x22000000UL
#define DIDHOLD     0x24000000UL
#define DOUBLEDRAIN 0x28000000UL

#define ISUNDEAD    0x30000001UL
#define BLESSMAP    0x30000002UL
#define BLESSGOLD   0x30000004UL
#define BLESSMONS   0x30000008UL
#define BLESSMAGIC  0x30000010UL
#define BLESSFOOD   0x30000020UL
#define CANBRANDOM  0x30000040UL /* Types of breath */
#define CANBACID    0x30000080UL
#define CANBFIRE    0x30000100UL
#define CANBBOLT    0x30000200UL
#define CANBGAS     0x30000400UL
#define CANBICE     0x30000800UL
#define CANBPGAS    0x30001000UL /* Paralyze gas */
#define CANBSGAS    0x30002000UL /* Sleeping gas */
#define CANBSLGAS   0x30004000UL /* Slow gas */
#define CANBFGAS    0x30008000UL /* Fear gas */
#define CANBREATHE  0x3000ffc0UL /* Can it breathe at all? */
#define STUMBLER    0x30010000UL
#define POWEREAT    0x30020000UL
#define ISELECTRIC  0x30040000UL
#define HASOXYGEN   0x30080000UL /* Doesn't need to breath air */
#define POWERDEXT   0x30100000UL
#define POWERSTR    0x30200000UL
#define POWERWISDOM 0x30400000UL
#define POWERINTEL  0x30800000UL
#define POWERCONST  0x31000000UL
#define SUPERHERO   0x32000000UL
#define ISUNHERO    0x34000000UL
#define CANCAST     0x38000000UL

#define CANTRAMPLE  0x40000001UL
#define CANSWIM     0x40000002UL
#define LOOKSLOW    0x40000004UL
#define CANWIELD    0x40000008UL
#define CANDARKEN   0x40000010UL
#define ISFAST      0x40000020UL
#define CANBARGAIN  0x40000040UL
#define NOMETAL     0x40000080UL
#define CANSPORE    0x40000100UL
#define NOSHARP     0x40000200UL
#define DRAINWISDOM 0x40000400UL
#define DRAINBRAIN  0x40000800UL
#define ISLARGE     0x40001000UL
#define ISSMALL     0x40002000UL
#define CANSTAB     0x40004000UL
#define ISFLOCK     0x40008000UL
#define ISSWARM     0x40010000UL
#define CANSTICK    0x40020000UL
#define CANTANGLE   0x40040000UL
#define DRAINMAGIC  0x40080000UL
#define SHOOTNEEDLE 0x40100000UL
#define CANZAP      0x40200000UL
#define HASARMOR    0x40400000UL
#define CANTELEPORT 0x40800000UL
#define ISBERSERK   0x41000000UL
#define ISFAMILIAR  0x42000000UL
#define HASFAMILIAR 0x44000000UL
#define SUMMONING   0x48000000UL

#define CANREFLECT  0x50000001UL
#define LOWFRIENDLY 0x50000002UL
#define MEDFRIENDLY 0x50000004UL
#define HIGHFRIENDLY    0x50000008UL
#define MAGICATTRACT    0x50000010UL
#define ISGOD       0x50000020UL
#define CANLIGHT    0x50000040UL
#define HASSHIELD   0x50000080UL
#define HASMSHIELD  0x50000100UL
#define LOWCAST     0x50000200UL
#define MEDCAST     0x50000400UL
#define HIGHCAST    0x50000800UL
#define WASSUMMONED 0x50001000UL
#define HASSUMMONED 0x50002000UL
#define CANTRUESEE  0x50004000UL
#define CLASSIC     0x50008000UL  /* monster from urogue 1.0.2 */

#define FLAGSHIFT       28UL
#define FLAGINDEX       0x0000000fL
#define FLAGMASK        0x0fffffffL

/* on - check if a monster flag is on */
#define on(th, flag) \
        (((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag & FLAGMASK)) != 0)

/* off - check if a monster flag is off */
#define off(th, flag) \
        (((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag & FLAGMASK)) == 0)

/* turn_on - turn on a monster flag */
#define turn_on(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag & FLAGMASK))

/* turn_off - turn off a monster flag */
#define turn_off(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~(flag & FLAGMASK))


/* types of things */
#define TYP_POTION	0
#define TYP_SCROLL	1
#define TYP_FOOD	2
#define TYP_WEAPON	3
#define TYP_ARMOR	4
#define TYP_RING	5
#define TYP_STICK	6
#define TYP_ARTIFACT	7
#define	NUMTHINGS	8

/*
 * Potion types
 */
#define	P_CLEAR		0
#define	P_ABIL		1
#define	P_SEEINVIS	2
#define	P_HEALING	3
#define	P_MFIND		4
#define	P_TFIND		5
#define	P_RAISE		6
#define	P_HASTE		7
#define	P_RESTORE	8
#define	P_PHASE		9
#define P_INVIS		10
#define P_SMELL		11
#define P_HEAR		12
#define P_SHERO		13
#define P_DISGUISE	14
#define P_LEVITATION	15
#define	MAXPOTIONS	16

/*
 * Scroll types
 */
#define	S_CONFUSE	0
#define	S_MAP		1
#define	S_LIGHT		2
#define	S_HOLD		3
#define	S_SLEEP		4
#define	S_ALLENCH	5
#define	S_IDENT		6
#define	S_SCARE		7
#define	S_GFIND		8
#define	S_TELEP		9
#define	S_CREATE	10
#define	S_REMOVE	11
#define	S_PETRIFY	12
#define	S_GENOCIDE	13
#define	S_CURING	14
#define S_MAKEIT	15
#define S_PROTECT	16
#define S_NOTHING	17
#define S_SILVER	18
#define S_OWNERSHIP	19
#define S_FOODFIND	20
#define S_ELECTRIFY     21
#define	S_PEACE		22
#define	MAXSCROLLS	23

/*
 * Weapon types
 */
#define MACE		0		/* mace */
#define SWORD		1		/* long sword */
#define BOW		2		/* short bow */
#define ARROW		3		/* arrow */
#define DAGGER		4		/* dagger */
#define ROCK		5		/* rocks */
#define TWOSWORD	6		/* two-handed sword */
#define SLING		7		/* sling */
#define DART		8		/* darts */
#define CROSSBOW	9		/* crossbow */
#define BOLT		10		/* crossbow bolt */
#define SPEAR		11		/* spear */
#define TRIDENT		12		/* trident */
#define SPETUM		13		/* spetum */
#define BARDICHE	14 		/* bardiche */
#define SPIKE		15		/* short pike */
#define BASWORD		16		/* bastard sword */
#define HALBERD		17		/* halberd */
#define BATTLEAXE	18		/* battle axe */
#define SILVERARROW     19              /* silver arrows */
#define HANDAXE		20		/* hand axe */
#define CLUB		21		/* club */
#define FLAIL		22		/* flail */
#define GLAIVE		23		/* glaive */
#define GUISARME	24		/* guisarme */
#define HAMMER		25		/* hammer */
#define JAVELIN		26		/* javelin */
#define MSTAR		27		/* morning star */
#define PARTISAN	28		/* partisan */
#define PICK		29		/* pick */
#define LPIKE		30		/* long pike */
#define SCIMITAR	31		/* scimitar */
#define BULLET		32		/* sling bullet */
#define QSTAFF		33		/* quarter staff */
#define BRSWORD		34		/* broad sword */
#define SHSWORD		35		/* short sword */
#define SHIRIKEN	36		/* shurikens */
#define BOOMERANG	37		/* boomerangs */
#define MOLOTOV		38		/* molotov cocktails */
#define CLAYMORE	39		/* claymore sword */
#define CRYSKNIFE	40		/* crysknife */
#define FOOTBOW		41		/* footbow */
#define FBBOLT		42		/* footbow bolt */
#define MACHETE		43		/* machete */
#define LEUKU		44		/* leuku */
#define TOMAHAWK	45		/* tomahawk */
#define PERTUSKA	46		/* pertuska */
#define SABRE		47		/* sabre */
#define CUTLASS		48		/* cutlass sword */
#define GRENADE		49		/* grenade for explosions */
#define MAXWEAPONS	50		/* types of weapons */
#define NONE		100		/* no weapon */

/*
 * Armor types
 */
#define	LEATHER		0
#define	RING_MAIL	1
#define	STUDDED_LEATHER	2
#define	SCALE_MAIL	3
#define	PADDED_ARMOR	4
#define	CHAIN_MAIL	5
#define	SPLINT_MAIL	6
#define	BANDED_MAIL	7
#define	PLATE_MAIL	8
#define	PLATE_ARMOR	9
#define	MITHRIL		10
#define	CRYSTAL_ARMOR   11
#define	MAXARMORS	12

/*
 * Ring types
 */
#define	R_PROTECT	0
#define	R_ADDSTR	1
#define	R_SUSABILITY	2
#define	R_SEARCH	3
#define	R_SEEINVIS	4
#define	R_ALERT		5
#define	R_AGGR		6
#define	R_ADDHIT	7
#define	R_ADDDAM	8
#define	R_REGEN		9
#define	R_DIGEST	10
#define	R_TELEPORT	11
#define	R_STEALTH	12
#define	R_ADDINTEL	13
#define	R_ADDWISDOM	14
#define	R_HEALTH	15
#define R_VREGEN	16
#define R_LIGHT		17
#define R_DELUSION	18
#define R_CARRYING	19
#define R_ADORNMENT	20
#define R_LEVITATION	21
#define R_FIRERESIST	22
#define R_COLDRESIST	23
#define R_ELECTRESIST	24
#define R_RESURRECT	25
#define R_BREATHE	26
#define R_FREEDOM	27
#define R_WIZARD	28
#define R_TELCONTROL	29
#define	MAXRINGS	30

/*
 * Rod/Wand/Staff types
 */

#define	WS_LIGHT	0
#define	WS_HIT		1
#define	WS_ELECT	2
#define	WS_FIRE		3
#define	WS_COLD		4
#define	WS_POLYMORPH	5
#define	WS_MISSILE	6
#define	WS_SLOW_M	7
#define	WS_DRAIN	8
#define	WS_CHARGE	9
#define	WS_TELMON	10
#define	WS_CANCEL	11
#define WS_CONFMON	12
#define WS_ANNIH	13
#define WS_ANTIM	14
#define WS_PARALYZE	15
#define WS_MDEG		16
#define WS_NOTHING	17
#define WS_INVIS	18
#define WS_BLAST	19
#define	MAXSTICKS	20

/*
 * Food types
 */

#define FD_RATION	0
#define FD_FRUIT	1
#define FD_CRAM		2
#define FD_CAKES	3
#define FD_LEMBA	4
#define FD_MIRUVOR	5
#define	MAXFOODS	6

/*
 * Artifact types
 */

#define TR_PURSE	0
#define TR_PHIAL	1
#define TR_AMULET	2
#define TR_PALANTIR	3
#define TR_CROWN	4
#define TR_SCEPTRE	5
#define TR_SILMARIL	6
#define TR_WAND		7
#define MAXARTIFACT	8

/*
 * Artifact flags
 */

#define ISUSED	       01
#define ISACTIVE       02

/*
 * Now we define the structures and types
 */

/*
 * level types
 */
typedef enum {
	NORMLEV,	/* normal level */
	POSTLEV,	/* trading post level */
	MAZELEV,	/* maze level */
	THRONE		/* unique monster's throne room */
} LEVTYPE;

/*
 * Help list
 */

struct h_list {
    char h_ch;
    char *h_desc;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
    char l_letter;			/* Letter for inventory */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
    int mi_worth;
    int mi_curse;
    int mi_bless;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    long r_flags;			/* Info about the room */
    short r_fires;			/* Number of fires in room */
    int r_nexits;			/* Number of exits */
    coord r_exit[MAXDOORS];		/* Where the exits are */
};

/*
 * Initial artifact stats
 */
struct init_artifact {
	char *ar_name;		/* name of the artifact */
	int ar_level;		/* first level where it appears */
	int ar_rings;		/* number of ring effects */
	int ar_potions;		/* number of potion effects */
	int ar_scrolls;		/* number of scroll effects */
	int ar_wands;		/* number of wand effects */
	int ar_worth;		/* gold pieces */
	int ar_weight;		/* weight of object */
};

/*
 * Artifact attributes
 */
struct artifact {
	long ar_flags;		/* general flags */
	long ar_rings;		/* ring effects flags */
	long ar_potions;	/* potion effects flags */
	long ar_scrolls;	/* scroll effects flags */
	long ar_wands;		/* wand effects flags */
	struct linked_list *t_art; /* linked list pointer */
};

/*
 * Array of all traps on this level
 */
struct trap {
    char tr_type;			/* What kind of trap */
    char tr_show;			/* Where disguised trap looks like */
    coord tr_pos;			/* Where trap is */
    long tr_flags;			/* Info about trap (i.e. ISFOUND) */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    short s_str;			/* Strength */
    short s_intel;			/* Intelligence */
    short s_wisdom;			/* Wisdom */
    short s_dext;			/* Dexterity */
    short s_const;			/* Constitution */
    short s_charisma;			/* Charisma */
    unsigned long s_exp;		/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    int s_pack;				/* current weight of his pack */
    int s_carry;			/* max weight he can carry */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure describing a fighting being (monster at initialization)
 */
struct mstats {
    short s_str;			/* Strength */
    long s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    char *s_hpt;			/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    bool t_turn;			/* If slowed, is it a turn to move */
    bool t_wasshot;			/* Was character shot last round? */
    char t_type;			/* What it is */
    char t_disguise;			/* What mimic looks like */
    char t_oldch;			/* Character that was where it was */
    short t_ctype;			/* Character type */
    short t_index;			/* Index into monster table */
    short t_no_move;			/* How long the thing can't move */
    short t_quiet;			/* used in healing */
    short t_doorgoal;			/* What door are we heading to? */
    coord t_pos;			/* Position */
    coord t_oldpos;			/* Last position */
    coord *t_dest;			/* Where it is running to */
    unsigned long t_flags[16];		/* State word */
    struct linked_list *t_pack;		/* What the thing is carrying */
    struct stats t_stats;		/* Physical description */
    struct stats maxstats;		/* maximum(or initial) stats */
};

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    short m_carry;			/* Probability of carrying something */
    bool m_normal;			/* Does monster exist? */
    bool m_wander;			/* Does monster wander? */
    char m_appear;			/* What does monster look like? */
    char *m_intel;			/* Intelligence range */
    unsigned long m_flags[16];		/* Things about the monster */
    char *m_typesum;			/* type of creature can he summon */
    short m_numsum;			/* how many creatures can he summon */
    short m_add_exp;			/* Added experience per hit point */
    struct mstats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char o_launch;			/* What you need to launch it */
    char *o_damage;			/* Damage if used like sword */
    char *o_hurldmg;			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    long o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
    int o_weight;			/* weight of this object */
    char o_mark[MARKLEN];		/* Mark the specific object */
    unsigned long o_worth;		/* value in trading post */
    struct artifact art_stats;		/* substructure for artifacts */
};
/*
 * weapon structure
 */
struct init_weps {
    char *w_name;		/* name of weapon */
    char *w_dam;		/* hit damage */
    char *w_hrl;		/* hurl damage */
    char w_launch;		/* need to launch it */
    int  w_flags;		/* flags */
    int  w_wght;		/* weight of weapon */
    int  w_worth;		/* worth of this weapon */
};

/*
 * armor structure 
 */
struct init_armor {
	char *a_name;		/* name of armor */
	int  a_prob;		/* chance of getting armor */
	int  a_class;		/* normal armor class */
	int  a_worth;		/* worth of armor */
	int  a_wght;		/* weight of armor */
};

struct matrix {
    int base;			/* Base to-hit value (AC 10) */
    int max_lvl;		/* Maximum level for changing value */
    int factor;			/* Amount base changes each time */
    int offset;			/* What to offset level */
    int range;			/* Range of levels for each offset */
};

struct spells {
    short s_which;		/* which scroll or potion */
    short s_cost;		/* cost of casting spell */
    short s_type;		/* scroll or potion */
    bool  s_blessed;		/* is the spell blessed? */
};

struct  real_pass {
	char	rp_pass[20];
	char	rp_pkey[2];
};

extern struct h_list helpstr[];
/*
 * Now all the global variables
 */

extern struct trap traps[];
extern struct room rooms[];		/* One for each room -- A level */
extern struct room *oldrp;		/* Roomin(&oldpos) */
extern struct linked_list *mlist;	/* List of monsters on the level */
extern struct thing player;		/* The rogue */
extern struct thing *beast;		/* The last monster attacking */
extern struct thing *foe;		/* The last monster we attacked */
extern struct monster monsters[];	/* The initial monster states */
extern struct linked_list *lvl_obj;	/* List of objects on this level */
extern struct object *cur_weapon;	/* Which weapon he is weilding */
extern struct object *cur_armor;	/* What a well dresssed rogue wears */
extern struct object *cur_ring[];	/* Which rings are being worn */
extern struct magic_item things[];	/* Chances for each type of item */
extern struct magic_item s_magic[];	/* Names and chances for scrolls */
extern struct magic_item p_magic[];	/* Names and chances for potions */
extern struct magic_item r_magic[];	/* Names and chances for rings */
extern struct magic_item ws_magic[];	/* Names and chances for sticks */
extern struct magic_item fd_data[];	/* Names and chances for food */
extern struct spells magic_spells[];	/* spells for magic users */
extern struct spells cleric_spells[];	/* spells for magic users */
extern struct real_pass rpass;		/* For protection's sake! */
extern char *cnames[][11];		/* Character level names */
extern char curpurch[];			/* name of item ready to buy */
extern char PLAYER;			/* what the player looks like */
extern int resurrect;			/* resurrection counter */
extern int char_type;			/* what type of character is player */
extern int foodlev;			/* how fast he eats food */
extern int see_dist;			/* how far he can see^2 */
extern int level;			/* What level rogue is on */
extern int trader;			/* number of purchases */
extern int curprice;			/* price of an item */
extern int purse;			/* How much gold the rogue has */
extern int mpos;			/* Where cursor is on top line */
extern int ntraps;			/* Number of traps on this level */
extern int no_move;			/* Number of turns held in place */
extern int no_command;			/* Number of turns asleep */
extern int inpack;			/* Number of things in pack */
extern int inbag;			/* Number of things in magic bag */
extern int total;			/* Total dynamic memory bytes */
extern int lastscore;			/* Score before this turn */
extern int no_food;			/* Number of levels without food */
extern int seed;			/* Random number seed */
extern int count;			/* Number of times to repeat command */
extern int dnum;			/* Dungeon number */
extern int max_level;			/* Deepest player has gone */
extern int food_left;			/* Amount of food in hero's stomach */
extern int group;			/* Current group number */
extern int hungry_state;		/* How hungry is he */
extern int infest_dam;			/* Damage from parasites */
extern int lost_str;			/* Amount of strength lost */
extern int lost_dext;			/* amount of dexterity lost */
extern int hold_count;			/* Number of monsters holding player */
extern int trap_tries;			/* Number of attempts to set traps */
extern int pray_time;			/* Number of prayer points/exp level */
extern int spell_power;			/* Spell power left at this level */
extern int auth_or[MAXAUTH];		/* MAXAUTH priviledged players */
extern int has_artifact;		/* set for possesion of artifacts */
extern int picked_artifact;		/* set for any artifacts picked up */
extern int active_artifact;		/* set for activated artifacts */
extern int msg_index;			/* pointer to current message buffer */
extern int luck;			/* how expensive things to buy thing */
extern int game_id;			/* unique identifier for each game */
extern int difficulty;			/* 1 easy, 2 normal, 3 hard */
extern int mindifficulty;		/* lowest difficulty level used in a game */
extern int maxpack;			/* max items that fit in pack */
extern int nummonst;			/* number of types of monsters */
extern int searching_run;		/* alternately search and step */
extern int save_ch;			/* saved command for next turn */
extern char take;			/* Thing the rogue is taking */
extern char prbuf[];			/* Buffer for sprintfs */
extern char outbuf[];			/* Output buffer for stdout */
extern char runch;			/* Direction player is running */
extern char *s_names[];			/* Names of the scrolls */
extern char *p_colors[];		/* Colors of the potions */
extern char *r_stones[];		/* Stone settings of the rings */
extern struct init_weps weaps[];	/* weapons and attributes */
extern struct init_armor armors[];	/* armors and attributes */
extern struct init_artifact arts[];	/* artifacts and attributes */
extern char *ws_made[];			/* What sticks are made of */
extern char *release;			/* Release number of rogue */
extern char whoami[];			/* Name of player */
extern char fruit[];			/* Favorite fruit */
extern char msgbuf[NMSGS][2*BUFSIZ];	/* message buffer */
extern char *s_guess[];			/* Players guess at what scroll is */
extern char *p_guess[];			/* Players guess at what potion is */
extern char *r_guess[];			/* Players guess at what ring is */
extern char *ws_guess[];		/* Players guess at what wand is */
extern char *ws_type[];			/* Is it a wand or a staff */
extern char file_name[];		/* Save file name */
extern char score_file[];		/* Score file name */
extern char home[];			/* User's home directory */
extern WINDOW *cw;			/* Window that the player sees */
extern WINDOW *hw;			/* Used for the help command */
extern WINDOW *mw;			/* Used to store mosnters */
extern bool pool_teleport;		/* just teleported from a pool */
extern bool inwhgt;			/* true if from wghtchk() */
extern bool running;			/* True if player is running */
extern bool fighting;			/* True if player is fighting */
extern bool playing;			/* True until he quits */
extern bool wizard;			/* True if allows wizard commands */
extern bool after;			/* True if we want after daemons */
extern bool notify;			/* True if player wants to know */
extern bool cutcorners;			/* Option for moving around corners */
extern bool doorstop;			/* Option whether runs stop early */
extern bool door_stop;			/* Stop running when we pass a door */
extern bool jump;			/* Show running as series of jumps */
extern bool slow_invent;		/* Inventory one line at a time */
extern bool firstmove;			/* First move after setting door_stop */
extern bool showcursor;			/* Option to show cursor */
extern bool autopickup;			/* Option to pick up stuff you step on */
extern bool autosave;			/* Option to save game automatically */
extern bool use_mouse;			/* Option to allow mouse click to move */
extern bool canwizard;			/* Will be permitted to do this */
extern bool askme;			/* Ask about unidentified things */
extern bool moving;			/* move using 'm' command */
extern bool s_know[];			/* Does he know what a scroll does */
extern bool p_know[];			/* Does he know what a potion does */
extern bool r_know[];			/* Does he know what a ring does */
extern bool ws_know[];			/* Does he know what a stick does */
extern bool in_shell;			/* True if executing a shell */
extern bool monst_dead;			/* Indicates if monster got killed */
extern bool game_over;			/* Is this the end? */
extern bool serious_fight;		/* fight longer and harder */
#ifdef MOUSE
extern bool mousemove;
#endif
extern coord oldpos;			/* Position before last look() call */
extern coord delta;			/* Change indicated to get_dir() */
extern char *spacemsg;
extern char *morestr;
extern char *retstr;
extern LEVTYPE levtype;

/*
 * Function Prototypes
 */

/* armor.c */
void wear(void);
void take_off(void);
void waste_time(void);
/* artifact.c */
void apply(void);
bool possessed(int artifact);
bool is_carrying(int artifact);
bool is_active(int artifact);
bool make_artifact(void);
struct object *new_artifact(int which, struct object *cur);
void do_minor(struct object *tr);
void do_major(void);
void do_phial(void);
void do_palantir(void);
void do_silmaril(void);
void do_amulet(void);
void do_bag(struct object *obj);
void do_sceptre(void);
void do_wand(void);
void do_crown(void);
void add_bag(struct linked_list **bag);
struct linked_list *get_bag(struct linked_list **bag);
void bag_inventory(struct linked_list *list);
int bag_char(struct object *obj, struct linked_list *bag);
void bagletter(struct linked_list *item);
void delbagletter(struct linked_list *item);
/* chase.c */
void do_chase(struct thing *th, bool flee);
void runto(coord *runner, coord *spot);
int chase(struct thing *tp, coord *ee, bool flee);
struct room *roomin(coord *cp);
struct linked_list *find_mons(int y, int x);
int diag_ok(coord *sp, coord *ep, struct thing *flgptr);
int cansee(int y, int x);
coord *can_shoot(coord *er, coord *ee);
bool straight_shot(int ery, int erx, int eey, int eex, coord *shooting);
struct linked_list *get_hurl(struct thing *tp);
bool can_blink(struct thing *tp);
/* command.c */
void command(void);
void quit(void);
void search(bool is_thief);
void help(void);
void identify(void);
void d_level(void);
void u_level(void);
void shell(void);
void call(bool mark);
bool pick_monster (char ch);
bool can_fight (int x, int y);

/* daemon.c */

extern int demoncnt;

struct delayed_action
{
    int d_type;
    int d_when;
    int d_id;
    void *d_arg;
    int d_time;
};

#define EMPTY  0
#define DAEMON 1
#define FUSE   2

typedef void fuse_type;
typedef void my_daemon;

typedef union
{
    void *varg;
    int  *iarg;
    struct linked_list *ll;
} fuse_arg;

typedef union
{
    void *varg;
    int  *iarg;
    struct thing *thingptr;
} daemon_arg;

struct fuse
{
    int index;
    fuse_type (*func)(fuse_arg *arg);
};

struct daemon
{
    int index;
    my_daemon (*func)(daemon_arg *arg);
};

#define MAXDAEMONS 60

#define FUSE_NULL          0
#define FUSE_SWANDER       1
#define FUSE_UNCONFUSE     2
#define FUSE_UNSCENT       3
#define FUSE_SCENT         4
#define FUSE_UNHEAR        5
#define FUSE_HEAR          6
#define FUSE_UNSEE         7
#define FUSE_UNSTINK       8
#define FUSE_UNCLRHEAD     9
#define FUSE_UNPHASE      10
#define FUSE_SIGHT        11
#define FUSE_RES_STRENGTH 12
#define FUSE_NOHASTE      13
#define FUSE_NOSLOW       14
#define FUSE_SUFFOCATE    15
#define FUSE_CURE_DISEASE 16
#define FUSE_UNITCH       17
#define FUSE_APPEAR       18
#define FUSE_UNELECTRIFY  19
#define FUSE_UNBHERO      20
#define FUSE_UNSHERO      21
#define FUSE_UNXRAY       22
#define FUSE_UNDISGUISE   23
#define FUSE_SHERO        24
#define FUSE_WGHTCHK      25
#define FUSE_CURE_INFEST  26
#define FUSE_UNFLY        27
#define FUSE_MAX          28

#define DAEMON_NULL       0
#define DAEMON_DOCTOR     1
#define DAEMON_ROLLWAND   2
#define DAEMON_STOMACH    3
#define DAEMON_RUNNERS    4
#define DAEMON_MAX        5

extern struct delayed_action d_list[MAXDAEMONS];
extern struct daemon daemons[DAEMON_MAX];
extern struct fuse fuses[FUSE_MAX];

extern struct delayed_action *d_slot(void);
extern struct delayed_action *find_slot(int id, int type);
extern void start_daemon(int id, void *arg, int whendo);
extern void kill_daemon(int id);
extern void do_daemons(int now);
extern void light_fuse(int id, void *arg, int time, int whendo);
extern void lengthen_fuse(int id, int xtime);
extern void extinguish_fuse(int id);
extern void do_fuses(int flag);
extern void activity(void);


/* daemons.c */
void runners(daemon_arg *arg);
void doctor(daemon_arg *tp);
void rollwand(daemon_arg *arg);
void stomach(daemon_arg *arg);
void swander(fuse_arg *arg);
void unconfuse(fuse_arg *arg);
void unscent(fuse_arg *arg);
void scent(fuse_arg *arg);
void unhear(fuse_arg *arg);
void hear(fuse_arg *arg);
void unsee(fuse_arg *arg);
void unstink(fuse_arg *arg);
void unclrhead(fuse_arg *arg);
void unphase(fuse_arg *arg);
void unfly(fuse_arg *arg);
void sight(fuse_arg *arg);
void res_strength(fuse_arg *arg);
void nohaste(fuse_arg *arg);
void noslow(fuse_arg *arg);
void suffocate(fuse_arg *arg);
void cure_disease(fuse_arg *arg);
void cure_infest(fuse_arg *arg);
void un_itch(fuse_arg *arg);
void appear(fuse_arg *arg);
void unelectrify(fuse_arg *arg);
void unshero(fuse_arg *arg);
void unbhero(fuse_arg *arg);
void unxray(fuse_arg *arg);
void undisguise(fuse_arg *arg);
void shero(fuse_arg *arg);
/* encumb.c */
void updpack(int getmax);
int packweight(void);
int itemweight(struct object *wh);
int playenc(void);
int totalenc(void);
void wghtchk(fuse_arg *arg);
int hitweight(void);
/* fight.c */
void do_fight(int y, int x, bool multiple);
int fight(coord *mp, struct object *weap, bool thrown);
int attack(struct thing *mp, struct object *weapon, bool thrown);
int swing(int charclass, int at_lvl, int op_arm, int wplus);
void next_level(void);
void check_level(void);
int roll_em(struct thing *att_er, struct thing *def_er, struct object *weap, bool hurl, struct object *cur_weapon);
char *prname(char *who, bool upper);
void hit(char *er, char *ee);
void miss(char *er, char *ee);
int save_throw(int which, struct thing *tp);
int save(int which);
int dext_plus(int dexterity);
int dext_prot(int dexterity);
int str_plus(int str);
int add_dam(int str);
int hung_dam(void);
void raise_level(void);
void thunk(struct object *weap, char *mname);
void m_thunk(struct object *weap, char *mname);
void bounce(struct object *weap, char *mname);
void m_bounce(struct object *weap, char *mname);
void my_remove(coord *mp, struct linked_list *item);
int is_magic(struct object *obj);
void killed(struct linked_list *item, bool pr, bool points);
struct object *wield_weap(struct object *weapon, struct thing *mp);
/* get_play.c */
int geta_player(void);
int puta_player(int arm, int wpt, int hpadd, int dmadd);
/* init.c */
void init_player(void);
void init_things(void);
void init_fd(void);
void init_colors(void);
void init_names(void);
void init_stones(void);
void init_materials(void);
void init_monsters(char flag);
void badcheck(char *name, struct magic_item *magic, int bound);
/* io.c */
void msg(char *fmt, ...);
void addmsg(char *fmt, ...);
void endmsg(void);
int step_ok(int y, int x, int can_on_monst, struct thing *flgptr);
int shoot_ok(int ch);
int readchar(void);
void status(bool display);
void ministat(void);
void wait_for(int ch);
void show_win(WINDOW *scr, char *message);
void dbotline(WINDOW *scr, char *message);
void restscr(WINDOW *scr);
/* list.c */
void _detach(struct linked_list **list, struct linked_list *item);
void _attach(struct linked_list **list, struct linked_list *item);
void _free_list(struct linked_list **ptr);
void discard(struct linked_list *item);
struct linked_list *new_item(int size);
char *my_malloc(int size);
/* main.c */
int main(int argc, char **argv);
void endit(void);
void fatal(char *s);
int rnd(int range);
int roll(int number, int sides);
void setup(void);
void playit(void);
int too_much(void);
int author(void);
int makesure(void);
void areuok(char *file);
void checkout(void);
void chmsg(char *fmt, int arg);
void loadav(double *avg);
int holiday(void);
int ucount(void);
void tweak_settings(bool first_time, int old_difficulty);
void usage(void);
/* maze.c */
void do_maze(void);
void draw_maze(void);
char *moffset(int y, int x);
char *foffset(int y, int x);
int findcells(int y, int x);
void rmwall(int newy, int newx, int oldy, int oldx);
void crankout(void);
/* misc.c */
char *tr_name(int ch);
void look(bool wakeup);
int secretdoor(int y, int x);
struct linked_list *find_obj(int y, int x);
void eat(void);
void chg_str(int amt, bool both, bool lost);
void chg_dext(int amt, bool both, bool lost);
void add_haste(bool blessed);
void aggravate(void);
void calm(bool blessed);
char *vowelstr(char *str);
int is_current(struct object *obj);
int get_dir(void);
int unarrow(int ch);
bool maze_view(int y, int x);
void listens(void);
/* monsdata.c */
/* monsters.c */
short randmonster(bool wander, bool no_unique);
void new_monster(struct linked_list *item, int type, coord *cp, bool max_monster);
void wanderer(void);
struct linked_list *wake_monster(int y, int x);
void genocide(void);
char *id_monst(int monster);
void check_residue(struct thing *tp);
void sell(struct thing *tp);
/* mouse.c */
#ifdef MOUSE
char do_mousemove(coord dest, coord prev);
char do_mouseclick(coord dest);
coord fix_mousedest(coord dest);
#endif
/* move.c */
void do_run(int ch);
void corr_move(int dy, int dx);
void do_move(int dy, int dx);
void light(coord *cp);
bool blue_light(bool blessed, bool cursed);
int show(int y, int x);
int be_trapped(struct thing *th, coord *tc);
void dip_it(void);
struct trap *trap_at(int y, int x);
void set_trap(struct thing *tp, int y, int x);
coord *rndmove(struct thing *who);
int isatrap(int ch);
/* new_level.c */
void new_level(LEVTYPE ltype);
int rnd_room(void);
void put_things(LEVTYPE ltype);
void do_throne(void);
void cleanup_old_level(void);
/* options.c */
void option(void);
void parse_opts(char *str);
void strucpy(char *s1, char *s2, int len);
int get_string(char *opt, WINDOW *win);
/* pack.c */
bool add_pack(struct linked_list *item, bool silent);
int inventory(struct linked_list *list, int type);
void pick_up(int ch);
void picky_inven(void);
struct linked_list *get_item(char *purpose, int type);
int pack_char(struct object *obj);
void del_pack(struct linked_list *what);
void cur_null(struct object *op);
void idenpack(void);
void getletter(struct linked_list *item);
void freeletter(struct linked_list *item);
void show_floor(void);
/* passages.c */
void do_passages(void);
void conn(int r1, int r2);
void door(struct room *rm, coord *cp);
/* player.c */
int const_bonus(void);
void gsense(void);
void steal(void);
void pray(void);
void affect(void);
void cast(void);
/* potions.c */
void quaff(int which, bool blessed);
void lower_level(int who);
void res_dexterity(void);
void res_wisdom(void);
void res_intelligence(void);
void add_strength(bool cursed);
void add_intelligence(bool cursed);
void add_wisdom(bool cursed);
void add_dexterity(bool cursed);
void add_const(bool cursed);
/* rings.c */
void ring_on(void);
void ring_off(void);
int ring_eat(int hand);
char *ring_num(struct object *obj);
int ring_value(int type);
int ring_blessed(int type);
int ring_cursed(int type);
/* rip.c */
void death(int monst);
void score(long amount, int flags, int monst);
void total_winner(void);
char *killname(int monst);
void showpack(char *howso);
void byebye(void);
int save_resurrect(int bonus);
/* rogue.c */
extern int maxprayers;
extern int maxspells;
/* rooms.c */
void do_rooms(void);
void draw_room(struct room *rp);
void horiz(int cnt);
void vert(int cnt);
void rnd_pos(struct room *rp, coord *cp);
/* save.c */
int save_game(void);
void auto_save(int sig);
extern int restore(char *file);
void encwrite(char *start, int size, FILE *outf);
int encread(char *start, int size, int infd);
int putword(int word, FILE *file);
int getword(int fd);
/* scrolls.c */
void read_scroll(int which, bool blessed);
struct thing *creat_mons(struct thing *person, int monster, bool report);
int is_r_on(struct object *obj);
/* sticks.c */
void fix_stick(struct object *cur);
void do_zap(bool gotdir, int which, bool blessed);
void drain(int ymin, int ymax, int xmin, int xmax);
char *charge_str(struct object *obj);
bool shoot_bolt(struct thing *shooter, coord start, coord dir, bool get_points, int reason, char *name, int damage);
/* sys_dep.c */
int _dprnt(char *fmt, int *argp, FILE *file);
void tstop(void);
/* things.c */
char *inv_name(struct object *obj, bool drop);
int drop(struct linked_list *item);
int dropcheck(struct object *op);
struct linked_list *new_thing(void);
struct linked_list *spec_item(int type, int which, int hit, int damage);
int pick_one(struct magic_item *magic, int nitems);
char *blesscurse(int flags);
int extras(void);
/* trader.c */
void do_post(void);
int price_it(void);
void buy_it(void);
void sell_it(void);
int open_market(void);
char *typ_name(struct object *obj);
int get_worth(struct object *obj);
void trans_line(void);
/* vers.c */
/* weapons.c */
void missile(int ydelta, int xdelta, struct linked_list *item, struct thing *tp);
void do_motion(struct object *obj, int ydelta, int xdelta, struct thing *tp);
void fall(struct linked_list *item, bool pr);
void init_weapon(struct object *weap, int type);
int hit_monster(int y, int x, struct object *obj, struct thing *tp);
char *num(int n1, int n2);
void wield(void);
int fallpos(coord *pos, coord *newpos, bool scatter, bool under);
/* wizard.c */
void whatis(struct linked_list *what);
void create_obj(int which_item, int which_type, bool cursed);
int getbless(void);
void makemon(void);
int teleport(void);
