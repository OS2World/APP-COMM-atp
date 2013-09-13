/*
   ATP QWK MAIL READER FOR READING AND REPLYING TO QWK MAIL PACKETS.
   Copyright (C) 1992, 1993, 1997  Thomas McWilliams
   Copyright (C) 1990  Rene Cougnenc

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
chosetag.c
*/

#define TLMX 60			/* maximum length for tagline */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "readlib.h"
#include "qlib.h"
#include "ansi.h"
#include "makemail.h"
#ifdef __LCLINT__
#include "chosetag.lh"
#endif

/* 
 * LineCount, count the lines in tagline file.
 */
static unsigned int 
LineCount(FILE * FpFrom)
{
    int Ch, NLn = 0;

    while ((Ch = getc(FpFrom)) != EOF) {
	if (Ch == (int)'\n')
	    NLn++;
    }
    return NLn;
}

/* 
 * InitRandom, seed and initialize random sequence.
 */
static void
InitRandom(void)
{
    srand((unsigned int) time(NULL));
/*  srand ((unsigned int) getpid()   ); <- use this as a last resort */
    return;
}

#ifdef __EMX__
static const char *TagLine = NTAG;  /* PCBoard style tagline  */
static const char *FidoTag = FTAG;  /* Fidonet style tagline  */
/*
 * atp_emx_tag_init, for EMX only: set tagline for DOS or OS/2 at runtime.
 */
static void
atp_emx_tag_init(void)
{
    if (_osmode == DOS_MODE){
        TagLine = NTAX;
        FidoTag = FTAX;
    }
}

#else
static const char CONSPTR TagLine = NTAG;  /* PCBoard style tagline  */
static const char CONSPTR FidoTag = FTAG;  /* Fidonet style tagline  */
#endif

/* current tagline */
static char  CurTag[256] = { '\n', NUL_CHAR };

/* default tag from atprc config file */ 
static char OrigTag[256] = { '\n', NUL_CHAR };

/* run time defined */
static char UserTag[256] = { '\n', NUL_CHAR }; 


/*
 * get_CurTag - provides a copy of the current tagline.
 */
#define TAG_END 84
byte *
get_CurTag(void)
{
    byte *tmpr;
    /* truncate tagline to 80 chars */
    if ( /*@i1 */ CurTag[TAG_END] != NUL_CHAR) {
	CurTag[TAG_END] = '\n';
	CurTag[TAG_END + 1] = NUL_CHAR;
    }
    tmpr = (byte *) strdup(CurTag);
    test_fatal_malloc(tmpr, __FILE__, __LINE__);
    return tmpr;
}


/*
 * tag_set_by_main - called from main() to setup initial tagline. 
 */
void
tag_set_by_main(void)
{
#ifdef __EMX__
    atp_emx_tag_init();
#endif
    /* setup the default TagLines */
    if (fido)
	strcpy(CurTag, FidoTag);
    else
	strcpy(CurTag, TagLine);
    strcat(CurTag, OrigTag);
    if (autotag)
	ChooseTag();
    printf("%s\n", CurTag);
}


/*
 * init_tagstyle - set tagline style to FIDO or PCBoard per atprc.
 */
void
init_tagstyle(const char *value)
{
    if (stricmp(value, "FIDO") == SUCCESS) {
	toglfido();
	strcpy(CurTag, FidoTag);
    } else {
	strcpy(CurTag, TagLine);
    }
}

/*
 * BuildNewTag, used by TagSeek() to copy tagline from taglines.atp file.
 */
static void
BuildNewTag(FILE * Fp)
{
    unsigned int charct = 0;
    char tmptagbuf[TLMX + 50];	/* temporary work buffer */
    char *crtag = tmptagbuf;
    int Ch = 0;

    /* build new tag line */
    do {
	Ch = getc(Fp);
	if (Ch != (int)'\r') {
	    *crtag = (char) Ch;
	    crtag++;
	    charct++;
	}
    } while (Ch != EOF && Ch != (int)'\n' && charct < TLMX);
    if (Ch == EOF) {
	printf("unexpected end of file\n");
    } else {
	crtag--;
	if (/*@i1*/ *crtag != '\n')
	    *crtag = '\n';
	crtag++;
	*crtag = NUL_CHAR;
	if (fido)
	    strcpy(CurTag, FidoTag);	/* setup the default TagLines */
	else
	    strcpy(CurTag, TagLine);
	/*@-usedef */
	strcat(CurTag, tmptagbuf);
	/*@=usedef */
    }
}


/*
 * TagSeek, locate and retrieve a tagline by number.
 */
static void
TagSeek(const unsigned int tagchoice)
{
    char ttagpath[MAXPATHS];
    /*@+voidabstract */ 
    FILE *Fp = NULL;
    /*@=voidabstract */

    sprintf(ttagpath, "%s%s", HomePath, TAGFILE);
    if ((Fp = fopen(ttagpath, "rb")) == NULL) {		/* open tagline file */
	fprintf(stderr, "Can't open file %s mode %s\n", ttagpath, "read only");
    } else {
	unsigned int NLn = 0;
	int Ch = 0;
	while (NLn != tagchoice && Ch != EOF) {		/* seek to chosen tagline */
	    Ch = getc(Fp);
	    if (Ch == (int)'\n')
		NLn++;
	}
	if (Ch == EOF) {
	    printf("unexpected end of file\n");
	} else {
	    BuildNewTag(Fp);
	}
	fclose(Fp);
    }
}


/*
 * ChooseTag - select a tagline at random from taglines.atp.
 */
void
ChooseTag(void)
{
    FILE *Fp;
    char ttagpath[MAXPATHS];

    sprintf(ttagpath, "%s%s", HomePath, TAGFILE);
    if ((Fp = fopen(ttagpath, "rb")) == NULL) {		/* open tagline file */
	fprintf(stderr, "Can't open file %s mode %s\n", ttagpath, "read only");
    } else {
	const unsigned int NToChoose = 1;
	const unsigned int NSample = LineCount(Fp);
	fclose(Fp);
	/* the following error can occur if the tagline file is empty */
	if (NSample < NToChoose) {
	    fprintf(stderr, "\n%s: file appears empty, line count = %u. ( %s:%d )\n",
		    ttagpath, NSample, __FILE__, __LINE__);
	} else {
	    static atp_BOOL_T flag1st = TRUE;
	    unsigned int toffset;
	    if (flag1st) {
		InitRandom();
		flag1st = FALSE;
	    }
	    toffset = (unsigned int) ((rand()) % NSample);
	    TagSeek(toffset);
	}
    }
}


/* 
 * togltag - toggle tagline styles between FIDO and PCBoard.
 */
void
togltag(void)
{
    Tag("tag fido");
}

/*
 * init_tagpers - get persistent tagline from atprc.
 */
void
init_tagpers(const char *value, const char *sign, const char *aLine)
{
    /* Get persistent tagline */
    if ( /*@i2 */ *value != NUL_CHAR && *sign == '=') {
	const char CONSPTR tptr = strchr(aLine, '=') + 2;
	strcpy(OrigTag, tptr);	/* Copy the Original tagline from config */
	strcat(OrigTag, "\n");
    } else {
	strcpy(OrigTag, " \n");	/* blank tagline */
    }
}


/*
 * in_edit_reply_mode, when true pause display at end of tagline listing.
 */
static atp_BOOL_T in_edit_reply_mode = FALSE ;

/*
 * tag_set_edit_reply_mode - tells tagview() how to format tagline listing. 
 */
void
tag_set_edit_reply_mode(const atp_BOOL_T er_mode)
{
	in_edit_reply_mode = er_mode ; 
}

/* -5 is the initial offset needed for a nice display */
#define LINE_BIAS -5

/* 
 * tagview, view tagline list.
 */
static void
tagview(const char *Path, const char *File)
{
    FILE *fp;
    char tmp[MAXPATHS];

    sprintf(tmp, "%s%c%s", Path, SEP, File);
    if ((fp = fopen(tmp, "rb")) == NULL) {
	/* "No file" */
	red();
	printf("%s %s\n", txt[67], tmp);
    } else {
	/* "line" is the index used by the screen output routine */
	int line = LINE_BIAS;
	/* lines_total provides a reference number for each tagline */
	int lines_total = 0;
	char buf[301];
	CLSCRN();
	clear();
	cyan();
	fflush(stdout);
	/*@-usedef */
	while (fget(buf, 250, fp)) {
	    /*@=usedef */
	    line++;
	    if (line > (get_ScrnRows() - 7)) {
		line = 0;
		if (!more(TRUE)) {
		    break;
		}
		cyan();
	    }
	    printf("%4d: %s\n", lines_total, buf);
	    lines_total++;
	}
	fclose(fp);
	printf("\nDefault Tag : %s", OrigTag);
	printf("User defined: %s", UserTag);
	printf("\n");
	/* if in_edit_reply_mode, pause screen display tgm*/
	if (in_edit_reply_mode)
	    (void)more(TRUE);
    }
}


/*
 *  StealTag, tom glaab's tagline stealer for atp.
 *  tjg october 1993 (tom glaab)
 *  tglaab@nooster.navy.mil
 * StealAsk, prompt user on disposition of stolen tagline.
 */
static void
StealAsk(char *tagtemp)
{
    char tprompt[80];
    /* begin common code */
    clear();
    yellow();
    printf("\nyou've selected: ");
    high();
    cyan();
    puts(tagtemp);
    clear();
    printf("\n");
    yellow();
    sprintf(tprompt, "%s", "do you want to save it to disk? ");
    high();
    if (YesNo(YES, tprompt)) {
	char ttagpath[MAXPATHS];
	FILE *tagfile;
	clear();
	sprintf(ttagpath, "%s%s", HomePath, TAGFILE);
	if ((tagfile = fopen(ttagpath, "a")) != NULL) {
	    fputs(tagtemp, tagfile);
	    fputc('\n', tagfile);
	} else {
	    red();
	    high();
	    fprintf(stderr, "Can't open file %s mode %s\n", ttagpath, "append");
	}
	clear();
	fclose(tagfile);
    }
}


/*
 * StealTag, capture a tagline from the current message.
 */
static void
StealTag(void)
{
    const byte *ptr = (byte *) rbuf + header_SIZE;
    const byte pcboard_tag_mark = (byte) 254;
    const byte CONSPTR plimit = ptr + (size_t) get_MsgSize();

    /* seek to first "square" mark */
    while (ptr < plimit && /*@i1 */ *ptr != pcboard_tag_mark)
	ptr++;
    /* seek to next "square" mark */
    while (++ptr < plimit && /*@i2 */ *ptr != (byte) '\n' && *ptr != pcboard_tag_mark)	/* empty loop */
	;			/* empty loop */
    /* found a tagline */
    if (ptr < plimit && /*@i1 */ *ptr != (byte) '\n') {
	char tagtemp[TLMX + 2];
	int i = 0;
	ptr += 2;
	/* copy it to our buffer */
	while (ptr < plimit && /*@i1 */ *ptr != (byte) '\n' && i < TLMX) {
	    tagtemp[i] = (char) *ptr;
	    ptr++;
	    i++;
	}
	/* guarantee that the string is terminated */
	tagtemp[i] = NUL_CHAR;
	/* prompt user for disposition */
	StealAsk(tagtemp);
    } else {
	/* can not find a tagline delimited by the "square" mark */
	red();
	high();
	printf("\nno tag found\n");
	clear();
    }
}


/*
 * AddTag, enter a tagline interactively. 
 */
static void
AddTag(void)
{
    char *tagtemp, tprompt[80];
    sprintf(tprompt, "%s", "enter tagline: ");
    do {
	tagtemp = readline(tprompt, do_scroll);
    } while (tagtemp == NULL);
    if (tagtemp != NULL) {
	if (strlen(tagtemp) > TLMX) {
	    printf("too long... \n\n");
	} else {
	    StealAsk(tagtemp);
	}
	free_string(tagtemp);
    }
}				/* end addtag */


/*
 * TagHelp, display tagline manager help screen.
 */
static void
TagHelp(void)
{
    int j = 0;
    white();
    high();
    printf("\n%s", taghlp[1]);
    clear();
    printf("\n");
    for (; j < (int) strlen(taghlp[1]); j++)
	(void)putchar('-');
    printf("\n");
    high();
    printf("\t tag list\t%s\n", taghlp[2]);
    printf("\t tag ?   \t%s\n", taghlp[3]);
    printf("\t tag `n' \t%s\n", taghlp[4]);
    printf("\t tag random\t%s\n", taghlp[5]);
    printf("\t tag swap\t%s\n", taghlp[6]);
    printf("\t tag auto\t%s\n", taghlp[7]);
    printf("\t tag     \t%s\n", taghlp[8]);
    printf("\t tag help\t%s\n", taghlp[9]);
    printf("\t tag steal\t%s\n", taghlp[11]);
    printf("\t tag add\t%s\n", taghlp[12]);
    printf("\t fido    \t%s\n", taghlp[10]);
    printf("\n");
    printf("Current TagLine is:");
    printf("%s\n", CurTag);
}


/*
 * TagSwap, called from Tag() to swap taglines.
 */
static atp_BOOL_T
TagSwap(atp_BOOL_T dflag)
{
    dflag = (dflag ? FALSE : TRUE);
    if (fido)
	strcpy(CurTag, FidoTag);
    else
	strcpy(CurTag, TagLine);
    if (dflag)
	strcat(CurTag, OrigTag);
    else
	strcat(CurTag, UserTag);
    printf("Current Tag now set to:");
    printf("%s\n", CurTag);
    return dflag ;
}


/*
 * TagAuto, called from Tag() to toggle automatic taglines.
 */
static void
TagAuto(void)
{
    toglauto();
    if (autotag) {
	printf("\nautotag is ON\n");
	ChooseTag();
	printf("Current Tag now set to:");
	printf("%s\n", CurTag);
    } else {
	printf("\nautotag is OFF\n\n");
    }
}


/*
 * TagList, called from Tag() to list taglines in taglines.atp file.
 */
static void
TagList(char *tmp, char *tmp2)
{
    sprintf(tmp2, "%s%s", HomePath, TAGFILE);
    printf("%s\n", tmp2);
    if (access(tmp2, R_OK) == SUCCESS){
	tmp[0] = NUL_CHAR ;
	sprintf(tmp, "%s", HomePath);
	tagview(tmp, TAGFILE);
    } else {
	printf("\nDefault Tag : %s", OrigTag);
	printf("User defined: %s", UserTag);
	printf("\n");
    }
}


/*
 * TagNumb, called from Tag() to select a specific numbered tagline from file.
 */
static void
TagNumb(char *tmp)
{
    int i = atoi((char *) tmp);
    i = i < 0 ? -i : i;
    TagSeek((unsigned) i);
    printf("Current Tag now set to:");
    printf("%s\n", CurTag);
}

/*
 * TagFido,  toggle tagline styles between fido and pcb.
 */
static void
TagFido(char *tmp)
{
    strcpy(tmp, CurTag);
    toglfido();
    if (fido) {
	strcpy(CurTag, FidoTag);
	strcat(CurTag, (tmp + strlen(TagLine)));
    } else {
	strcpy(CurTag, TagLine);
	strcat(CurTag, (tmp + strlen(FidoTag)));
    }
    printf("Current Tag now set to:");
    printf("%s\n", CurTag);
}


/*      
 * TagUser, change current tag to user defined tagline.
 */
static atp_BOOL_T
TagUser(const char *line, atp_BOOL_T dflag)
{
    line += 4;
    strcpy(UserTag, line);
    strcat(UserTag, "\n");
    if (fido)
	strcpy(CurTag, FidoTag);
    else
	strcpy(CurTag, TagLine);
    strcat(CurTag, UserTag);
    printf("Tag is now set to :");
    printf("%s\n", CurTag);
    dflag = FALSE;
    return dflag ;
}


static atp_BOOL_T tag_flag = TRUE; 
/* 
 * get_tag_flag - returns flag indicating if taglines are on or off.
 */
atp_BOOL_T
get_tag_flag(void)
{
	return tag_flag;
}


/*
 * Tag - wrapper to invoke various tagline manager functions.
 */
void
Tag(const char *line)
{
    static atp_BOOL_T dflag = FALSE;	/* toggle for default and user defined tagline */
    char dummy[50], tmp[256], tmp2[256];
    tmp2[0] = tmp[0] = NUL_CHAR;
    sscanf(line, "%s %s %s", dummy, tmp, tmp2);
    if ( /*@i1 */ tmp2[0] == NUL_CHAR) {
	if (strnicmp((char *) tmp, "list", 4) == SUCCESS) {	/* list taglines */
	    TagList(tmp, tmp2);
	} else if (Numeric(tmp)) {	/* try to get a tagline from the list */
	    TagNumb(tmp);
	} else if ( /*@i1 */ tmp[0] == '?') {	/* Print current tag */
	    printf("Current TagLine is:");
	    printf("%s\n", CurTag);
	} else if (stricmp((char *) tmp, "off") == SUCCESS) {	/* select tag at random  */
	    tag_flag = FALSE;
	    printf("taglines are now disabled\n");
	} else if (stricmp((char *) tmp, "on") == SUCCESS) {	/* select tag at random  */
	    tag_flag = TRUE;
	    printf("taglines are now enabled\n");
	} else if (strnicmp((char *) tmp, "rand", 4) == SUCCESS) {	/* select tag at random  */
	    ChooseTag();
	    printf("Current Tag now set to:");
	    printf("%s\n", CurTag);
	} else if ((strnicmp((char *) tmp, "help", 4) == SUCCESS) || /*@i1 */ line[3] == NUL_CHAR) {	/* select tag help menu */
	    TagHelp();
	} else if (strnicmp((char *) tmp, "auto", 4) == SUCCESS) {	/* toggle autotag line selection */
	    TagAuto();
	} else if (strnicmp((char *) tmp, "steal", 5) == SUCCESS) {
	    StealTag();		/* invoke stealtag (tg) */
	} else if (strnicmp((char *) tmp, "add", 3) == SUCCESS) {
	    AddTag();		/* add a tagline to your collection (tg) */
	} else if (strnicmp((char *) tmp, "fido", 4) == SUCCESS) {
	    TagFido(tmp);
	} else if (strnicmp((char *) tmp, "swap", 4) == SUCCESS) {	/* restore standard tag */
	    dflag = TagSwap(dflag);
	}
    } else if ( /*@i1 */ line[3] == SPC_CHAR) {
	dflag = TagUser(line, dflag);
    }
}


