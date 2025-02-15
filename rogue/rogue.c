#include "curses.h"
#include "rogue.h"
/*
 * Now all the global variables
 */
struct trap traps[MAXTRAPS+MAXTRAPS];	/* twice as much for special effects */
struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room *oldrp;			/* Roomin(&player.t_oldpos) */
struct thing player;			/* The rogue */
struct thing *beast;			/* The last beast that attacked us */
struct thing *foe = NULL;		/* The last beast that we attacked */
struct object *cur_armor;		/* What a well dresssed rogue wears */
struct object *cur_ring[NFINGERS];	/* Which rings are being worn */
struct linked_list *lvl_obj = NULL;
struct linked_list *mlist = NULL;
struct object *cur_weapon = NULL;
struct real_pass rpass;			/* For protection's sake! */
int char_type = -1;			/* what type of character is player */
int foodlev = 1;			/* the more you carry, the more food you need */
int ntraps;				/* Number of traps on this level */
int trader = 0;				/* no. of purchases */
int curprice = -1;			/* current price of item */
int no_move;				/* Number of turns held in place */
int seed;				/* Random number seed */
int dnum;				/* Dungeon number */
int max_level;				/* Deepest player has gone */
int lost_dext;				/* amount of lost dexterity */
int mpos = 0;
int no_command = 0;
int level = 1;
int purse = 0;
int inpack = 0;
int total = 0;
int see_dist = 3;
int no_food = 1;
int count = 0;
int food_left = HUNGERTIME;
int group = 1;
int hungry_state = F_OK;
int infest_dam=0;
int lost_str=0;
int lastscore = -1;
int hold_count = 0;
int trap_tries = 0;
int pray_time = 0;
int spell_power = 0;
int has_artifact = 0;
int picked_artifact = 0;
int active_artifact = 0;
int msg_index = 0;
int luck = 0;
int game_id;
int resurrect = 0;
int difficulty = 2;			/* 1 easy, 2 normal, 3 hard */
int mindifficulty = 2;			/* lowest difficulty level used in a game */
int maxpack = MAXPACK;
int auth_or[MAXAUTH];
int searching_run = 0;
int save_ch = ' ';
char curpurch[15];			/* name of item ready to buy */
char PLAYER = VPLAYER;			/* what the player looks like */
char take;				/* Thing the rogue is taking */
char prbuf[2*LINELEN];			/* Buffer for sprintfs */
char outbuf[BUFSIZ];			/* Output buffer for stdout */
char runch;				/* Direction player is running */
char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char *r_stones[MAXRINGS];		/* Stone settings of the rings */
char *ws_made[MAXSTICKS];		/* What sticks are made of */
char whoami[LINELEN];			/* Name of player */
char fruit[LINELEN];			/* Favorite fruit */
char msgbuf[NMSGS][2*BUFSIZ];		/* message buffer */
char *s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* Players guess at what potion is */
char *r_guess[MAXRINGS];		/* Players guess at what ring is */
char *ws_guess[MAXSTICKS];		/* Players guess at what wand is */
char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */
char file_name[LINELEN];		/* Save file name */
char score_file[LINELEN];		/* Score file name */
char home[LINELEN];			/* User's home directory */
WINDOW *cw;				/* Window that the player sees */
WINDOW *hw;				/* Used for the help command */
WINDOW *mw;				/* Used to store mosnters */
bool pool_teleport = FALSE;		/* just teleported from a pool */
bool inwhgt = FALSE;			/* true if from wghtchk() */
bool after;				/* True if we want after daemons */
bool showcursor = FALSE;		/* Option to show cursor */
bool autopickup = TRUE;			/* Option to pick up things you step on */
bool autosave = FALSE;			/* Option to save game automatically */
bool use_mouse = FALSE;			/* Option to move via mouse clicks */
bool canwizard = FALSE;			/* Will be permitted to do this */
bool s_know[MAXSCROLLS];		/* Do you know what a scroll does */
bool p_know[MAXPOTIONS];		/* Do you know what a potion does */
bool r_know[MAXRINGS];			/* Do you know what a ring does */
bool ws_know[MAXSTICKS];		/* Do you know what a stick does */
bool playing = TRUE;
bool running = FALSE;
bool fighting = FALSE;
bool wizard = FALSE;
bool notify = TRUE;
bool cutcorners = FALSE;
bool doorstop = TRUE;
bool door_stop = FALSE;
bool jump = TRUE;
bool slow_invent = FALSE;
bool firstmove = FALSE;
bool askme = TRUE;			/* ask me about unidentified things */
bool moving = FALSE;
bool in_shell = FALSE;
bool monst_dead = FALSE;
bool game_over = FALSE;
bool serious_fight = FALSE;
#ifdef MOUSE
bool mousemove = FALSE;
#endif
coord delta;				/* Change indicated to get_dir() */
LEVTYPE levtype;			/* type of level i'm on */

char *spacemsg =	"--Press space to continue--";
char *morestr  =	"--More--";
char *retstr   =	"[Press return to continue]";

/*
 * weapons and their attributes
 */
struct init_weps weaps[MAXWEAPONS] = {
    { "mace",		"2d4",  "1d3", NONE,		ISMETAL, 100, 8 },
    { "long sword",	"1d12", "1d2", NONE,		ISMETAL, 60, 15 },
    { "short bow",	"1d1",  "1d1", NONE,		0, 40, 75 },
    { "arrow",		"1d1",  "1d6", BOW,		ISMANY|ISMISL, 5, 1 },
    { "dagger",		"1d6",  "1d4", NONE,		ISMETAL|ISMANY|ISMISL, 10, 2 },
    { "rock",		"1d2",  "1d4", SLING,		ISMANY|ISMISL, 5, 1 },
    { "two-handed sword","3d6",  "1d2", NONE,		ISMETAL, 250, 30 },
    { "sling",		"0d0",  "0d0", NONE,  		0, 5, 1 },
    { "dart",		"1d1",  "1d3", NONE,  		ISMANY|ISMISL, 5, 1 },
    { "crossbow",	"1d1",  "1d1", NONE,  		0, 100, 15 },
    { "crossbow bolt",	"1d2", "1d12", CROSSBOW,	ISMANY|ISMISL, 7, 1 },
    { "spear",		"1d6",  "1d8", NONE,		ISMETAL|ISMISL, 50, 2 },
    { "trident",	"3d4",  "1d4", NONE,		ISMETAL, 50, 25 },
    { "spetum",		"2d6",  "1d3", NONE,		ISMETAL, 50, 12 },
    { "bardiche",	"3d4",  "1d2", NONE,		ISMETAL, 125, 6 },
    { "short pike",	"1d12", "1d8", NONE,		ISMETAL, 80, 18 },
    { "bastard sword",	"2d8",  "1d2", NONE,		ISMETAL, 100, 20 },
    { "halberd",	"2d6",  "1d3", NONE,		ISMETAL, 175, 10 },
    { "battle axe", 	"1d8",	"1d3", NONE,		ISMETAL, 80, 10 },
    { "arrow", 		"1d2",	"2d8", BOW,		ISSILVER|ISMANY|ISMISL, 10, 3 },
    { "hand axe", 	"1d6",	"1d2", NONE,		ISMETAL, 50, 8 },
    { "club",	 	"1d4",	"1d2", NONE,		0, 30, 2 },
    { "flail",	 	"1d7+1","1d4", NONE,		ISMETAL, 150, 10 },
    { "glaive", 	"1d10",	"1d3", NONE,		ISMETAL, 80, 8 },
    { "guisarme", 	"2d4",	"1d3", NONE,		ISMETAL, 80, 6 },
    { "hammer", 	"1d3",	"1d5", NONE,		ISMETAL|ISMISL, 50, 4 },
    { "javelin", 	"1d4",	"1d6", NONE,		ISMISL, 10, 3 },
    { "morning star", 	"2d4",	"1d3", NONE,		ISMETAL, 125, 8 },
    { "partisan", 	"1d6",	"1d2", NONE,		ISMETAL, 80, 4 },
    { "pick", 		"2d4",	"1d2", NONE,		ISMETAL, 60, 4 },
    { "long pike", 	"2d8",	"2d6", NONE,		ISMETAL, 80, 6 },
    { "scimitar", 	"1d8",	"1d2", NONE,		ISMETAL, 40, 8 },
    { "sling bullet", 	"1d1",	"1d8", SLING,		ISMANY|ISMISL|ISMETAL, 3, 1 },
    { "quarter staff", 	"1d6",	"1d2", NONE,		0, 50, 8 },
    { "broad sword", 	"2d4",	"1d3", NONE,		ISMETAL, 75, 15 },
    { "short sword", 	"1d6",	"1d2", NONE,		ISMETAL, 35, 8 },
    { "shuriken", 	"1d1",	"2d5", NONE,		ISMANY|ISMISL, 4, 1 },
    { "boomerang", 	"1d1",	"1d8", NONE,		CANRETURN|ISMANY|ISMISL, 10, 3 },
    { "burning oil",	"0d0",  "2d10+5",NONE,		CANBURN|ISMANY|ISMISL, 20, 3},
    { "claymore",	"3d7",  "1d2", NONE,		ISMETAL, 300, 27 },
    { "crysknife",	"3d3",  "1d3", NONE,		ISPOISON|ISMANY, 120, 2 },
    { "footbow",	"1d1",  "1d1", NONE,  		0, 90, 10 },
    { "footbow bolt",	"1d2", "1d10", FOOTBOW,		ISMANY|ISMISL, 5, 1 },
    { "machete", 	"1d7",	"1d6", NONE,		ISMETAL, 45, 4 },
    { "leuku", 		"1d6",	"1d5", NONE,		ISMETAL, 40, 3 },
    { "tomahawk", 	"1d6",	"1d6", NONE,		0, 45, 7 },
    { "pertuska",	"2d5",  "1d3", NONE,		ISMETAL, 160, 10 },
    { "sabre",		"2d6",	"1d3", NONE,		ISMETAL, 120, 20},
    { "cutlass",	"1d10", "1d2", NONE,		ISMETAL, 55, 12 },
    { "grenade",	"1d1", "1d2/4d8", NONE,		ISMANY, 10, 3 },
};

struct init_armor armors[MAXARMORS] = {
	{ "leather armor",		10,  8,   70, 100 },
	{ "ring mail",			20,  7,   50, 250 },
	{ "studded leather armor",	31,  7,   50, 200 },
	{ "scale mail",			43,  6,   70, 250 },
	{ "padded armor",		55,  6,  150, 150 },
	{ "chain mail",			67,  5,  100, 300 },
	{ "splint mail",		77,  4,  150, 350 },
	{ "banded mail",		87,  4,  150, 350 },
	{ "plate mail",		 	93,  3,  400, 400 },
	{ "plate armor",		97,  2,  650, 450 },
	{ "mithril",			99,  2, 10000, 100 },
	{ "crystalline armor",		100, 0, 5500, 300 },
};

struct magic_item things[NUMTHINGS] = {
    { "potion",			250,	5, 0, 0 },	/* potion */
    { "scroll",			260,   30, 0, 0 },	/* scroll */
    { "food",			210,	7, 0, 0 },	/* food */
    { "weapon",			 60,	0, 0, 0 },	/* weapon */
    { "armor",			 90,	0, 0, 0 },	/* armor */
    { "ring",			 70,	5, 0, 0 },	/* ring */
    { "stick",			 60,	0, 0, 0 },	/* stick */
    { "artifact",		  0,    0, 0, 0 },    /* special artifacts */
};

struct magic_item s_magic[MAXSCROLLS] = {
    { "monster confusion",	 55, 125, 0, 0 },
    { "magic mapping",		 50, 150, 20, 20 },
    { "light",			 75, 100, 25, 15 },
    { "hold monster",		 25, 200, 33, 20 },
    { "sleep",			 25,  50, 20, 10 },
    { "enchantment",		135, 130, 14, 9 },
    { "identify",		160, 100, 0, 15 },
    { "scare monster",		 35, 190, 27, 21 },
    { "gold detection",		 15, 110, 20, 20 },
    { "teleportation",		 50, 165, 10, 20 },
    { "create monster",		 25,  75, 30, 0 },
    { "remove curse",		 60, 120, 9, 15 },
    { "petrification",		 10, 185, 0, 0 },
    { "genocide",		  5, 1200, 0, 0 },
    { "cure disease",		 70, 160, 0, 0 },
    { "acquirement",		  5, 2500, 50, 15 },
    { "protection",		 40, 150, 0, 0 },
    { "nothing",		 70, 50, 0, 0 },
    { "magic hitting",		 10, 875, 25, 15 },
    { "ownership",		 20, 550, 25, 15 },
    { "food detection",		 40, 150, 25, 15 },
    { "electrification",	 20, 450, 0, 0 },
    { "peace",			  0, 500, 0, 10 },
};

struct magic_item p_magic[MAXPOTIONS] = {
    { "clear thought",		 90, 380, 27, 15 },
    { "gain ability",		 90, 250, 15, 15 },
    { "see invisible",		 74, 170, 25, 15 },
    { "healing",		150, 130, 27, 27 },
    { "monster detection",	 60, 120, 15, 20 },
    { "magic detection",	 70, 105, 20, 15 },
    { "raise level",		  1, 900, 11, 10 },
    { "haste self",		110, 200, 30, 5 },
    { "restore abilities",	160, 140, 0, 0 },
    { "phasing",		 45, 240, 21, 20 },
    { "invisibility",		 40, 200, 0, 15 },
    { "acute scent",		 30, 200, 20, 15 },
    { "acute hearing",		 30, 200, 20, 15 },
    { "super heroism",		 10, 800, 20, 15 },
    { "disguise",		 40, 500, 0, 15 },
    { "flying",			  0, 500, 0, 15 },
};

struct magic_item r_magic[MAXRINGS] = {
    { "protection",		 70, 200, 33, 25 },
    { "add strength",		 65, 200, 33, 25 },
    { "sustain ability",	 40, 480, 0, 0 },
    { "searching",		 65, 250, 0, 0 },
    { "extra sight",		 40, 175, 0, 0 },
    { "alertness",		 40, 190, 0, 0 },
    { "aggravate monster",	 35, 100, 100, 0 },
    { "dexterity",		 65, 220, 33, 25 },
    { "increase damage",	 65, 220, 33, 25 },
    { "regeneration",		 35, 260, 0, 0 },
    { "slow digestion",		 40, 340, 15, 10 },
    { "teleportation",		 35, 100, 100, 0 },
    { "stealth",		 50, 100, 0, 0 },
    { "add intelligence",	 60, 540, 33, 25 },
    { "increase wisdom",	 60, 220, 33, 25 },
    { "sustain health",		 60, 250, 0,  0 },
    { "vampiric regeneration",	 20, 800, 25, 10 },
    { "illumination",		 20, 300, 0, 0 },
    { "delusion",		 20, 100, 75, 0 },
    { "carrying",		 20, 200, 30, 0 },
    { "adornment",		 15, 10000, 0, 0 },
    { "levitation",		 20, 450, 0, 0 },
    { "fire resistance",	 10, 250, 0, 0 },
    { "cold resistance",	 10, 250, 0, 0 },
    { "lightning resistance",	 10, 250, 0, 0 },
    { "resurrection",		  1, 8000, 0, 0 },
    { "breathing",		 10, 250, 0, 0 },
    { "free action",		 10, 225, 0, 0 },
    { "wizardry",		  4, 950, 0, 0 },
    { "teleport control",	  5, 850, 0, 0 },
};

struct magic_item ws_magic[MAXSTICKS] = {
    { "light",			 90, 120, 20, 20 },
    { "striking",		 58, 115, 0, 0 },
    { "lightning",		 25, 200, 0, 0 },
    { "fire",			 25, 200, 0, 0 },
    { "cold",			 30, 200, 0, 0 },
    { "polymorph",		 90, 210, 0, 0 },
    { "magic missile",		 90, 170, 0, 0 },
    { "slow monster",		 76, 320, 25, 20 },
    { "drain life",		 90, 210, 20, 0 },
    { "charging",		 70, 100, 0, 0 },
    { "teleport monster",	 90, 140, 25, 20 },
    { "cancellation",		 38, 130, 0, 0 },
    { "confuse monster",   	 50, 100, 0, 0},
    { "disintegration",	  	 10, 550, 33, 0},
    { "anti-matter",		 30, 180, 0, 0},
    { "paralyze monster",	 38, 300, 0, 0},
    { "degenerate monster",	 30, 200, 30, 0},
    { "nothing",		 30, 100, 0, 0},
    { "invisibility",		 30, 150, 30, 5},
    { "blasting",		 10, 220, 0, 0},
};

struct magic_item fd_data[MAXFOODS] = {
    { "food ration",					400, 2, 20, 20 },
    { "fruit-with-very-long-name",			300, 1, 0, 0 },
    { "cram",						120, 3, 0, 0 },
    { "honey cake",					 80, 10, 0, 0 },
    { "lemba",						 50, 50, 0, 0 },
    { "miruvor",					 50, 200, 0, 0 },
};

struct init_artifact arts[MAXARTIFACT] = {
    { "Magic Purse of Yendor",	25, 1, 1, 1, 1, 460000, 50 },
    { "Phial of Galadriel",	35, 2, 2, 2, 1, 1250000, 10 },
    { "Amulet of Yendor",	45, 4, 1, 1, 2, 1600000, 10 },
    { "Palantir of Might",	60, 1, 4, 1, 2, 1850000, 70 },
    { "Crown of Might",		70, 6, 2, 1, 1, 2350000, 50 },
    { "Sceptre of Might",	80, 2, 2, 1, 6, 3800000, 50 },
    { "Silmaril of Ea",		90, 4, 2, 5, 1, 5000000, 50 },
    { "Wand of Orcus",		100, 4, 2, 3, 10, 8000000, 50 },
};

/*
 * these are the spells that a magic user can cast
 */
struct spells magic_spells[] = {
	{ P_TFIND,		3,	TYP_POTION,	FALSE },
	{ S_IDENT,		5,	TYP_SCROLL,	FALSE },
	{ P_MFIND,		7,	TYP_POTION,	FALSE },
	{ S_LIGHT,		15,	TYP_SCROLL,	TRUE  },
	{ S_REMOVE,		15,	TYP_SCROLL,	FALSE },
	{ S_TELEP,		15,	TYP_SCROLL,	FALSE },
	{ S_CONFUSE,		15,	TYP_SCROLL,	FALSE },
	{ S_MAP,		15,	TYP_SCROLL,	FALSE },
	{ P_CLEAR,		20,	TYP_POTION,	FALSE },
	{ S_SLEEP,		25,	TYP_SCROLL,	FALSE },
	{ WS_MISSILE,		25,	TYP_STICK,	FALSE },
	{ P_SEEINVIS,		25,	TYP_POTION,	TRUE  },
	{ WS_COLD,		30,	TYP_STICK,	FALSE },
	{ WS_ELECT,		30,	TYP_STICK,	FALSE },
	{ WS_CANCEL,		30,	TYP_STICK,	FALSE },
	{ S_PROTECT,		30,	TYP_SCROLL,	FALSE },
	{ WS_FIRE,		30,	TYP_STICK,	FALSE },
	{ P_PHASE,		40,	TYP_POTION,	FALSE },
	{ S_HOLD,		50,	TYP_SCROLL,	FALSE },
	{ S_PETRIFY,		50,	TYP_SCROLL,	FALSE },
	{ S_ALLENCH,		75,	TYP_SCROLL,	FALSE },
};
int maxspells = sizeof magic_spells / sizeof (struct spells);

/*
 * these are the spells that a cleric can cast
 */
struct spells cleric_spells[] = {
	{ P_MFIND,		3,	TYP_POTION,	FALSE },
	{ P_TFIND,		7,	TYP_POTION,	FALSE },
	{ S_IDENT,		15,	TYP_SCROLL,	FALSE },
	{ S_REMOVE,		20,	TYP_SCROLL,	FALSE },
	{ P_HEALING,		30,	TYP_POTION,	FALSE },
#if 0
	{ S_LIGHT,		15,	TYP_SCROLL,	TRUE  },
#endif
	{ S_CURING,		35,	TYP_SCROLL,	FALSE },
	{ P_HASTE,		35,	TYP_POTION,	FALSE },
	{ P_CLEAR,		35,	TYP_POTION,	FALSE },
	{ P_SEEINVIS,		35,	TYP_POTION,	TRUE  },
	{ P_RESTORE,		35,	TYP_POTION,	FALSE },
	{ P_PHASE,		40,	TYP_POTION,	FALSE },
	{ S_TELEP,		40,	TYP_SCROLL,	FALSE },
	{ S_PEACE,		60,	TYP_SCROLL,	FALSE },
	{ P_LEVITATION,		80,	TYP_POTION,	TRUE },
};
int maxprayers = sizeof cleric_spells / sizeof (struct spells);

char *cnames[4][11] = {
{	"Veteran",		"Warrior",	
	"Swordsman",		"Hero",
	"Swashbuckler",		"Myrmidon",	
	"Champion",		"Superhero",
	"Lord",			"Lord",		
	"Lord"
},
{	"Prestidigitator",	"Evoker",	
	"Conjurer",		"Theurgist",
	"Thaumaturgist",	"Magician",	
	"Enchanter",		"Warlock",
	"Sorcerer",		"Necromancer",
	"Wizard"
},
{	"Acolyte",		"Adept",
	"Priest",		"Curate",
	"Prefect",		"Canon",
	"Lama",			"Patriarch",
	"High Priest",		"High Priest",
	"High Priest"
},
{	"Rogue",		"Footpad",
	"Cutpurse",		"Robber",
	"Burglar",		"Filcher",
	"Sharper",		"Magsman",
	"Thief",		"Master Thief",
	"Master Thief"
}
} ;

struct h_list helpstr[] = {
	{'?',	"	prints help"},
	{'/',	"	identify object or creature"},
	{'h',	"	left"},
	{'j',	"	down"},
	{'k',	"	up"},
	{'l',	"	right"},
	{'y',	"	up & left"},
	{'u',	"	up & right"},
	{'b',	"	down & left"},
	{'n',	"	down & right"},
	{'<',	"SHIFT><dir> run til adjacent"},
	{'<',	"CTRL><dir> run and search"},
	{'m',	"	move onto without picking up"},
	{'t',	"	throw something"},
	{'z',	"	zap a wand or staff"},
	{'>',	"	go down a staircase"},
	{'s',	"	search for trap/secret door"},
	{'.',	"	rest for a while"},
	{',',	"	pick up an object"},
	{'i',	"	inventory"},
	{'I',	"	inventory single item"},
	{'q',	"	quaff potion"},
	{'r',	"	read paper"},
	{'e',	"	eat food"},
	{'w',	"	wield a weapon (w- to unwield)"},
	{'W',	"	wear armor"},
	{'T',	"	take armor off"},
	{'P',	"	put on ring"},
	{'R',	"	remove ring"},
	{'A',	"       activate/apply an artifact"},
	{'d',	"	drop object"},
	{'C',	"	call object (generic)"},
	{'M',	"	mark object (specific)"},
	{'o',	"	examine/set options"},
	{'c',	"	cast a spell"},
	{'p',	"	pray"},
	{'a',	"	affect the undead"},
	{'^',	"	set a trap"},
	{'G',	"	sense gold"},
	{'D',	"	dip something (into a pool)"},
	{CTRL('T'),	"	take (steal) from (direction)"},
	{CTRL('R'),	"	redraw screen"},
	{CTRL('P'),	"	back up to 10 previous messages"},
	{ESCAPE,	"	cancel command"},
	{'v',	"	print program version number"},
	{'S',	"	save game and exit"},
	{'Q',	"	quit"},
	{'=',	"	listen for monsters"},
	{'~',	"	points needed to raise levels"},
	{'f',	"	fight monster"},
	{'F',	"	fight monsters"},
	{CTRL('F'),	"	fight all monsters"},
/*
 * Wizard commands.  Identified by (h_ch != 0 && h_desc == 0).
 */
	{'-',		0},
	{'V',		"	print worth of object"},
	{'@',		"	system statistics"},
	{CTRL('A'),	"	floor map"},
	{CTRL('C'),	"	charge item"},
	{CTRL('D'),	"	random number check"},
	{CTRL('E'),	"	food statistics"},
	{CTRL('G'),	"	goto level"},
	{CTRL('I'),	"	inventory level"},
	{CTRL('M'),	"	see monster"},
	{CTRL('O'),	"	outfit pack"},
	{CTRL('V'),	"	create item"},
	{CTRL('W'),	"	wizard mode"},
	{CTRL('X'),	"	random teleport"},
	{CTRL('Z'),	"	identify item"},
	{0,		0}
};
