/*
 * Procedures for saving and retrieving a characters
 * starting attributes, armour, and weapon.
 * New addition to version 1.3 by S. A. Hester  11/8/83
 */

#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/* player attributes */
#define	I_APPEAL	1
#define	I_ARM		2
#define	I_CHAR		3
#define	I_DEX		4
#define	I_HITS		5
#define	I_INTEL		6
#define	I_STR		7
#define	I_WEAP		8
#define	I_WEAPENCH	9
#define	I_WELL		10
#define	I_WIS		11
#define MAXPATT		100		/* max player attributes */
#define MAXPDEF		100		/* max saved characters */

int def_array[MAXPDEF][MAXPATT];	/* Pre-defined chars */

int 
geta_player ()
{

	struct linked_list *item;
	struct object *obj;
	char char_file[LINELEN];	/* Where the file should be! */
	FILE	*fp;
	int	arm,wpt,hpadd,dmadd;
	char	pbuf[LINELEN];
	short    cnt,i;

	strcpy(char_file, home);
	strcat(char_file, ROGDEFS);

	if ((fp = fopen(char_file, "rb")) == NULL) {
	    return(FALSE);
	}

	/*
	 * It's OK.  Get the info!
	 */
	if (fread(def_array, sizeof def_array, 1, fp) != 1)
	    return(FALSE);
	fclose(fp);
 
	wclear(hw);
	touchwin(hw);

	cnt = 0;
	for(i=0; i<MAXPDEF; i++)
	    if(def_array[i][I_STR])
		cnt++;

	/* you start with weaker armor in harder games */
	if (difficulty == 2) {
	    for(i=0; i<MAXPDEF; i++)
		if(def_array[i][I_ARM] >= MITHRIL)
		    def_array[i][I_ARM] -= 2;
	} else if (difficulty == 3) {
	    for(i=0; i<MAXPDEF; i++)
		if(def_array[i][I_ARM] >= PLATE_ARMOR)
		    def_array[i][I_ARM] -= 3;
	} else if (difficulty > 3) {
	    for(i=0; i<MAXPDEF; i++)
		if(def_array[i][I_ARM] >= BANDED_MAIL)
		    def_array[i][I_ARM] -= 5;
	}

	if(!cnt){
		mvwaddstr(hw,5,5,"You have no pre-rolled characters!");
		mvwaddstr(hw, LINES - 1, 0, spacemsg);
		draw(hw);
		(void) wgetch(hw);
		return(FALSE);
	}

	sprintf(pbuf, "You have %d character%c:",cnt,(cnt==1)? 0 : 's');
	mvwaddstr(hw,5,5,pbuf);
	for(i=0;i<MAXPDEF;i++)
	    if(def_array[i][I_STR]) {
		char	*class;
		switch(def_array[i][I_CHAR]) {
		    case C_FIGHTER:
			class = "fighter";
		    when C_MAGICIAN:
			class = "magician";
		    when C_CLERIC:
			class = "cleric";
		    when C_THIEF:
			class = "thief";
		    otherwise:
			class = "unknown";
		}
		sprintf(pbuf, "%d. (%s): Int: %d  Str: %d  Wis: %d  Dex: %d  Const: %d  Charisma: %d",
				    i+1,
				  class,
		    def_array[i][I_INTEL],
		    def_array[i][I_STR],
		    def_array[i][I_WIS],
		    def_array[i][I_DEX],
		    def_array[i][I_WELL],
		    def_array[i][I_APPEAL] );
		mvwaddstr(hw,7+(i*3),0,pbuf);
		sprintf(pbuf, "Armor: %s   Weapon: (+%d,+%d)%s    Hit Points: %d",
		armors[(def_array[i][I_ARM])].a_name,
		def_array[i][I_WEAPENCH]/10,
		def_array[i][I_WEAPENCH]%10,
		weaps[(def_array[i][I_WEAP])].w_name,
		def_array[i][I_HITS]);
		mvwaddstr(hw,8+(i*3),0,pbuf);
	    }
	mvwaddstr(hw,0,0,"Do you wish to select a character? ");
	draw(hw);
	i = wgetch(hw);
	if ((i < '1' || i > '9') && i != 'y')
	    return(FALSE);
again:
	wmove(hw, LINES - 1, 0);
	wclrtoeol(hw);
	mvwaddstr(hw,0,0,"Enter the number of a pre-defined character:");
	wclrtoeol(hw);
	draw(hw);
	if (i == 'y') {
	    i = (wgetch(hw) - '0') - 1;
	} else {
	    i = (i - '0') - 1;
	}
	while(i < 0 || i > MAXPDEF-1) {
	    mvwaddstr(hw,0,0,"Use the range 1 to");
	    wprintw(hw," %d!",MAXPDEF);
	    wclrtoeol(hw);
	    draw(hw);
	    i = (wgetch(hw) - '0') - 1;
	}
	if(def_array[i][I_STR] == 0) {
	    mvwaddstr(hw,0,0,"Please enter the number of a known character:");
	    wclrtoeol(hw);
	    wstandout(hw);
	    mvwaddstr(hw, LINES - 1, 0, spacemsg);
	    wstandend(hw);
	    draw(hw);
	    (void) wgetch(hw);
	    goto again;
	}

	/* Fix up some local variables */
	hpadd = def_array[i][I_WEAPENCH]/10;
	dmadd = def_array[i][I_WEAPENCH]%10;
	wpt = def_array[i][I_WEAP];
	arm = def_array[i][I_ARM];

	/* And some global ones too! */
	pstats.s_lvl = 1;
	pstats.s_exp = 0L;
	pstats.s_str = def_array[i][I_STR];
	pstats.s_intel = def_array[i][I_INTEL];
	pstats.s_wisdom = def_array[i][I_WIS];
	pstats.s_dext = def_array[i][I_DEX];
	pstats.s_const = def_array[i][I_WELL];
	pstats.s_charisma = def_array[i][I_APPEAL];
	max_stats.s_hpt = pstats.s_hpt = def_array[i][I_HITS];
	player.t_ctype = def_array[i][I_CHAR];
	player.t_quiet = 0;
	if (player.t_ctype == C_FIGHTER) {
	    pstats.s_dmg = "2d4" ;
	    pstats.s_arm = 8 ;
	}
	else {
	    pstats.s_dmg = "1d4" ;
	    pstats.s_arm = 10 ;
	}
	max_stats = pstats ;
	pack = NULL;
	pstats.s_carry = totalenc();

	/* Now let's give the rogue the necessities! */
	item = spec_item(WEAPON, wpt, hpadd, dmadd);
	obj = (struct object *) ldata(item);
	obj->o_flags |= ISKNOW;
	add_pack(item, TRUE);
	cur_weapon = obj;
	/*
	 * And his suit of armor
	 */
	item = spec_item(ARMOR, arm, 0, 0);
	obj = (struct object *) ldata(item);
	obj->o_flags |= ISKNOW;
	obj->o_weight = armors[arm].a_wght;
	add_pack(item, TRUE);
	cur_armor = obj;
	return(TRUE);
}


int 
puta_player (int arm, int wpt, int hpadd, int dmadd)
{

	char char_file[LINELEN];	/* Where the file should be! */
	FILE	*fp;
	char	pbuf[200];
	char	*class;
	int   i;
	bool	no_write;

	strcpy(char_file, home);
	strcat(char_file, ROGDEFS);

	no_write = FALSE;
	wclear(hw);
	touchwin(hw);


	    switch(player.t_ctype) {
		case C_FIGHTER:
		    class = "fighter";
		when C_MAGICIAN:
		    class = "magician";
		when C_CLERIC:
		    class = "cleric";
		when C_THIEF:
		    class = "thief";
		otherwise:
		    class = "unknown";
	    }

	wclear(hw);
	sprintf(pbuf, "You have rolled a %s with the following attributes:",class);
	mvwaddstr(hw,2,0,pbuf);
	sprintf(pbuf, "Int: %d    Str: %d    Wis: %d   Dex: %d  Const: %d  Charisma: %d",
	    pstats.s_intel,
	    pstats.s_str,
	    pstats.s_wisdom,
	    pstats.s_dext,
	    pstats.s_const,
	    pstats.s_charisma );
	mvwaddstr(hw,4,5,pbuf);

	sprintf(pbuf, "%s    (+%d,+%d)%s    Hit Points: %d",
	    armors[arm].a_name,
	    hpadd, dmadd,
	    weaps[wpt].w_name,
	    pstats.s_hpt );
	mvwaddstr(hw,5,5,pbuf);

	if (difficulty > 3) {
	    mvwaddstr(hw,7,5,"-- press any key to continue --");
	    draw(hw);
	    wgetch(hw);
	    return(TRUE);
	}

	mvwaddstr(hw,0,0,"Would you like to re-roll the character?");
	draw(hw);
	if(wgetch(hw) == 'y')
	    return(FALSE);
	if(no_write)
	    return(TRUE);
	mvwaddstr(hw,0,0,"Would you like to save this character?");
	wclrtoeol(hw);
	draw(hw);
	if(wgetch(hw) != 'y')
	    return(TRUE);

	wstandout(hw);
	mvwaddstr(hw,8,0,"YOUR CURRENT CHARACTERS:");
	wstandend(hw);
	wclrtoeol(hw);
	for(i=0;i<MAXPDEF;i++) {
	    if(def_array[i][I_STR]) {
		switch(def_array[i][I_CHAR]) {
		    case C_FIGHTER:
			class = "fighter";
		    when C_MAGICIAN:
			class = "magician";
		    when C_CLERIC:
			class = "cleric";
		    when C_THIEF:
			class = "thief";
		    otherwise:
			class = "unknown";
		}
		sprintf(pbuf, "%d. (%s): Int: %d   Str: %d   Wis: %d  Dex: %d  Const: %d  Charisma: %d",
				    i+1,
				  class,
		    def_array[i][I_INTEL],
		    def_array[i][I_STR],
		    def_array[i][I_WIS],
		    def_array[i][I_DEX],
		    def_array[i][I_WELL],
		    def_array[i][I_APPEAL] );
		mvwaddstr(hw,12+(i*3),0,pbuf);
		sprintf(pbuf, "%s    (+%d,+%d)%s    Hit Points: %d",
		armors[(def_array[i][I_ARM])].a_name,
		def_array[i][I_WEAPENCH]/10,
		def_array[i][I_WEAPENCH]%10,
		weaps[(def_array[i][I_WEAP])].w_name,
		def_array[i][I_HITS] );
		mvwaddstr(hw,13+(i*3),0,pbuf);
	    }
	    else {
		sprintf(pbuf, "%d.  NO CHARACTER DEFINED!", i+1);
		mvwaddstr(hw,12+(i*3),0,pbuf);
	    }
	}
	mvwaddstr(hw,0,0,"Enter a number which this character is to be saved as:");
	draw(hw);
	i = (wgetch(hw) - '0') - 1;
	while(i < 0 || i > MAXPDEF-1) {
	    mvwaddstr(hw,0,0,"Use the range 1 to");
	    wprintw(hw," %d!",MAXPDEF);
	    wclrtoeol(hw);
	    draw(hw);
	    i = (wgetch(hw) - '0') - 1;
	}
	/* Preserve some local variables */
	def_array[i][I_WEAPENCH] = (hpadd *10) + dmadd;
	def_array[i][I_WEAP] = wpt;
	def_array[i][I_ARM] = arm;

	/* And some global ones too! */
	def_array[i][I_STR] = pstats.s_str;
	def_array[i][I_INTEL] = pstats.s_intel;
	def_array[i][I_WIS] = pstats.s_wisdom;
	def_array[i][I_DEX] = pstats.s_dext;
	def_array[i][I_WELL] = pstats.s_const;
	def_array[i][I_APPEAL] = pstats.s_charisma;
	def_array[i][I_HITS] = pstats.s_hpt;
	def_array[i][I_CHAR] = player.t_ctype;

	/* OK. Now let's write this stuff out! */
	if ((fp = fopen(char_file, "wb")) == NULL)
	{
	    no_write = TRUE;
	    sprintf(pbuf,"I can't seem to open/create %s", char_file);
	    mvwaddstr(hw,5,5,pbuf);
	    mvwaddstr(hw,6,5,"However I'll let you play it anyway!");
	    mvwaddstr(hw, LINES - 1, 0, spacemsg);
	    draw(hw);
	    (void) wgetch(hw);
	    return(TRUE);
	}
	fwrite(def_array, sizeof def_array, 1, fp);
	fclose(fp);
	return(TRUE);
}
