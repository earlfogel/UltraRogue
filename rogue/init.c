/*
 * global variable initializaton
 *
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "rogue.h"


static char *rainbow[] = {
	"Red",		"Blue",		"Green",	"Yellow",
	"Black",	"Brown",	"Orange",	"Pink",
	"Purple",	"Grey",		"White",	"Silver",
	"Gold",		"Violet",	"Clear",	"Vermilion",
	"Ecru",		"Turquoise",	"Magenta",	"Amber",
	"Topaz",	"Plaid",	"Tan",		"Tangerine",
	"Aquamarine",	"Scarlet",	"Khaki",	"Crimson",
	"Indigo",	"Beige",	"Lavender",	"Saffron",
} ;
#define NCOLORS (sizeof rainbow / sizeof (char *))

static char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum",
} ;

static char *stones[] = {
	"Agate",		"Alexandrite",		"Amethyst",
	"Azurite",		"Carnelian",		"Chrysoberyl",
	"Chrysoprase",		"Citrine",		"Diamond",
	"Emerald",		"Garnet",		"Hematite",
	"Jacinth",		"Jade",			"Kryptonite",
	"Lapus lazuli",		"Malachite",		"Moonstone",
	"Obsidian",		"Olivine",		"Onyx",
	"Opal",			"Pearl",		"Peridot",
	"Quartz",		"Rhodochrosite",	"Ruby",
	"Sapphire",		"Sardonyx",		"Serpentine",
	"Spinel",		"Tiger eye",		"Topaz",
	"Tourmaline",		"Turquoise",
} ;
#define NSTONES (sizeof stones / sizeof (char *))

static char *wood[] = {
	"Avocado wood",	"Balsa",	"Banyan",	"Birch",
	"Cedar",	"Cherry",	"Cinnibar",	"Dogwood",
	"Driftwood",	"Ebony",	"Eucalyptus",	"Hemlock",
	"Ironwood",	"Mahogany",	"Manzanita",	"Maple",
	"Oak",		"Pine",		"Redwood",	"Rosewood",
	"Teak",		"Walnut",	"Zebra wood", 	"Persimmon wood",
} ;
#define NWOOD (sizeof wood / sizeof (char *))

static char *metal[] = {
	"Aluminium",	"Bone",		"Brass",	"Bronze",
	"Copper",	"Chromium",	"Iron",		"Lead",
	"Magnesium",	"Pewter",	"Platinum",	"Steel",
	"Tin",		"Titanium",	"Zinc",
} ;
#define NMETAL (sizeof metal / sizeof (char *))



/*
 * init_player:
 *	roll up the rogue
 */

void 
init_player ()
{
    int special = rnd(10) ;

    pstats.s_lvl = 1 ;
    pstats.s_exp = 0L ;

    if (char_type == -1) {
	/* See what type character will be */
	wclear(hw) ;
	touchwin(hw) ;
	mvwaddstr(hw,2,0,"[1] Fighter\n[2] Magician\n[3] Cleric\n[4] Thief") ;
	mvwaddstr(hw, 0, 0, "What character class do you desire? ") ;
	draw(hw) ;
	char_type = getchar() - '0' ;
	while (char_type < 1 || char_type > 4) {
	    mvwaddstr(hw,0,0,"Please enter a character type between 1 and 4: ");
	    draw(hw) ;
	    char_type = getchar() - '0';
	}
	char_type-- ;
    }
    player.t_ctype = char_type ;
    player.t_quiet = 0;
    max_stats.s_hpt = pstats.s_hpt = 15 + rnd(5);
    pstats.s_str = 8 + rnd(5) ;
    pstats.s_intel = 8 + rnd(5) ;
    pstats.s_wisdom = 8 + rnd(5) ;
    pstats.s_dext = 8 + rnd(5) ;
    pstats.s_const = 10 + rnd(8) ;
    pstats.s_charisma = 8 + rnd(4) ;
    pstats.s_dmg = "1d4" ;
    pstats.s_arm = 10 ;

    /* Now for the special ability */
    switch (char_type) {
	case C_FIGHTER:
	    pstats.s_str = (special == 4) ? 18 : 16 ;
	    pstats.s_const += rnd(4);
	    pstats.s_dmg = "2d4" ;
	    pstats.s_arm = 8;
	when C_MAGICIAN:
	    pstats.s_intel = (special == 4) ? 18 : 16 ;
	when C_CLERIC:
	    pstats.s_wisdom = (special == 4) ? 18 : 16 ;
	    pstats.s_str += rnd(4);
	when C_THIEF:
	    pstats.s_dext = (special == 4) ? 18 : 16 ;
    }
    if (pstats.s_const > 15)
	pstats.s_hpt += pstats.s_const - 15;
    max_stats = pstats ;
    pack = NULL ;
    pstats.s_carry = totalenc();
}

/*
 * Contains definitions and functions for dealing with things like
 * potions and scrolls
 */

/*
 * init_things
 *	Initialize the probabilities for types of things
 */
void 
init_things ()
{
    struct magic_item *mp ;

    for (mp = &things[1] ; mp < &things[NUMTHINGS] ; mp++)
	mp->mi_prob += (mp-1)->mi_prob ;
    badcheck("things", things, NUMTHINGS) ;
}

/*
 * init_fd
 *	Initialize the probabilities for types of food
 */
void 
init_fd ()
{
    struct magic_item *mp ;

    for (mp = &fd_data[1] ; mp < &fd_data[MAXFOODS] ; mp++)
	mp->mi_prob += (mp-1)->mi_prob ;
    badcheck("food", fd_data, MAXFOODS) ;
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */

void 
init_colors ()
{
    int i ;
    char *str, *str2 ;

    for (i = 0 ; i < MAXPOTIONS ; i++)
    {
	do
	    str = rainbow[rnd(NCOLORS)] ;
	until (isupper(*str)) ;
	str2 = ALLOC(LINELEN);
	strcpy(str2, str);
	*str2 = tolower(*str2) ;
	p_colors[i] = str2 ;
	p_know[i] = FALSE ;
	p_guess[i] = NULL ;
	if (i > 0)
		p_magic[i].mi_prob += p_magic[i-1].mi_prob ;
    }
    badcheck("potions", p_magic, MAXPOTIONS) ;
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */

void 
init_names ()
{
    int nsyl ;
    char *cp, *sp ;
    int i, nwords ;

    for (i = 0 ; i < MAXSCROLLS ; i++)
    {
	cp = prbuf ;
	nwords = rnd(COLS/20) + 1 + (COLS > 40 ? 1 : 0) ;
	while(nwords--)
	{
	    nsyl = rnd(3)+1 ;
	    while(nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))] ;
		while(*sp)
		    *cp++ = *sp++ ;
	    }
	    *cp++ = ' ' ;
	}
	*--cp = '\0' ;
	s_names[i] = (char *) new(strlen(prbuf)+1) ;
	s_know[i] = FALSE ;
	s_guess[i] = NULL ;
	strcpy(s_names[i], prbuf) ;
	if (i > 0)
		s_magic[i].mi_prob += s_magic[i-1].mi_prob ;
    }
    badcheck("scrolls", s_magic, MAXSCROLLS) ;
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */

void 
init_stones ()
{
    int i ;
    char *str;

    for (i = 0 ; i < MAXRINGS ; i++)
    {
	do
	    str = stones[rnd(NSTONES)] ;
	until (isupper(*str)) ;
	r_stones[i] = str ;
	r_know[i] = FALSE ;
	r_guess[i] = NULL ;
	if (i > 0)
		r_magic[i].mi_prob += r_magic[i-1].mi_prob ;
    }
    badcheck("rings", r_magic, MAXRINGS) ;
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */

void 
init_materials ()
{
    int i ;
    char *str;

    for (i = 0 ; i < MAXSTICKS ; i++)
    {
	do
	    if (rnd(100) > 50)
	    {
		str = metal[rnd(NMETAL)] ;
		if (isupper(*str))
			ws_type[i] = "wand" ;
	    }
	    else
	    {
		str = wood[rnd(NWOOD)] ;
		if (isupper(*str))
			ws_type[i] = "staff" ;
	    }
	until (isupper(*str)) ;
	ws_made[i] = str ;
	ws_know[i] = FALSE ;
	ws_guess[i] = NULL ;
	if (i > 0)
		ws_magic[i].mi_prob += ws_magic[i-1].mi_prob ;
    }
    badcheck("sticks", ws_magic, MAXSTICKS) ;
}

/*
 * init_monsters:
 *	Choose which monsters to use:
 *	c - classic monsters from v 1.0.2
 *	a - all 300+ monsters
 *	r - a random assortment
 *	default - classic monsters plus a random assortment
 */

void 
init_monsters (char flag)
{
    int i, j, k;
    int nmonst = 0, nsummoned = 0;
    char *summoned[nummonst+1];
    bool keep;

    for (i=1; i<nummonst-1; i++) {
	keep = TRUE;
	if (flag == 'r') {  /* Select monsters at random */
	    /* keep at least one monster for the first level */
	    if (i==3 && monsters[1].m_normal == FALSE
	      && monsters[2].m_normal == FALSE) {
	        /* do nothing */
	    } else if (rnd(2) == 0) {  /* remove half the rest */
		monsters[i].m_normal = FALSE;
		monsters[i].m_wander = FALSE;
		keep = FALSE;
	    }

	} else if (flag == 'c') {  /* Keep the classic monsters, remove the rest */
	    bool classic = FALSE;
	    for (j=0; j<NM_FLAGS; j++) {
		if (monsters[i].m_flags[j] == CLASSIC) {
		    classic = TRUE;
		}
	    }
	    if (!classic) {
		monsters[i].m_normal = FALSE;
		monsters[i].m_wander = FALSE;
		keep = FALSE;
	    }

	} else if (flag == 'a') {  /* Keep all monsters */
	    /* do nothing */

	} else {  /* The classic monsters plus a few extras */
	    bool classic = FALSE;
	    for (j=0; j<NM_FLAGS; j++) {
		if (monsters[i].m_flags[j] == CLASSIC) {
		    classic = TRUE;
		}
	    }
	    if (!classic && rnd(6) > 0) {
		monsters[i].m_normal = FALSE;
		monsters[i].m_wander = FALSE;
		keep = FALSE;
	    } else {
		nmonst++;
	    }
	}
	/* keep track of summoned monsters */
	if (keep && monsters[i].m_numsum > 0) {
	    for (k=0; k<nsummoned; k++) {
		if (strcmp(monsters[i].m_typesum, summoned[k]) == 0)
		    break;  /* it's already in the list */
	    }
	    if (k == nsummoned)  /* we didn't find it */
		summoned[nsummoned++] = monsters[i].m_typesum;
	}
    }

    /* put back any summoned monsters we still need */
    for (i=nummonst-1; i>1; i--) {
	if (monsters[i].m_normal==FALSE && monsters[i].m_wander==FALSE) {
	    for (j=0; j<nsummoned; j++) {
		if (strcmp(monsters[i].m_name, summoned[j]) == 0) {
		    monsters[i].m_normal = TRUE;
		    monsters[i].m_wander = TRUE;
		    monsters[i].m_flags[NM_FLAGS-1] = 0L;
		    nmonst++;
		    /* in case it summons something too */
		    if (monsters[i].m_numsum > 0) {
			for (k=0; k<nsummoned; k++) {
			    if (strcmp(monsters[i].m_typesum, summoned[k]) == 0)
				break;  /* it's already in the list */
			}
			if (k == nsummoned)  /* we didn't find it */
			    summoned[nsummoned++] = monsters[i].m_typesum;
		    }
		    if (nsummoned > nummonst) {
			endwin();
			printf("Too many (%d) summoned monsters, aborting", nsummoned);
			exit(1);
		    }
		}
	    }
	}
    }
    if (wizard) {
	printf("\r\nSelecting %d monsters...\r\n", nmonst);
	fflush(stdout);
	usleep(750000);
    }
}


void 
badcheck (name, magic, bound)
char *name;
struct magic_item *magic;
int bound;
{
    struct magic_item *end ;

    if (magic[bound - 1].mi_prob == 1000)
	return ;
    printf("\nBad percentages for %s:\n", name) ;
    for (end = &magic[bound] ; magic < end ; magic++)
	printf("%3d%% %s\n", magic->mi_prob, magic->mi_name) ;
    printf("[hit RETURN to continue]") ;
    fflush(stdout) ;
    while (getchar() != '\n')
	continue ;
}

