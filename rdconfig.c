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
rdconfig.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define _ATP_RDCONFIG_C
#include "reader.h"
#include "readlib.h"
#include "makemail.h"
#include "qlib.h"
#include "ansi.h"

/************************ BEGIN GLOBAL VARIABLES ****************************/

/* global pointers */                                                          /*@+voidabstract */
FILE *fidx  = NULL;             /* current open conference index stream     */
FILE *fmsg  = NULL;             /* current open conference message stream   */ /*@=voidabstract */
char *rbuf  = NULL;             /* Pointer to work buffer for messages      */

/* global constants */
const size_t mybuf  = MYBUF ;
const size_t maxbuf = MAXBUF ; 
const size_t one_record = (size_t)1;
const size_t header_SIZE = (size_t)HDRSIZE ;
const size_t block_SIZE = (size_t)BLKSIZE;
const size_t index_SIZE = (size_t)IDXSIZE;

 /* passed text to readline editor */
const char *luxptr;

/* local path data */
static char rc_HomePath[MAXPATHS]; /* Home directory root of msg dirs       */
static char rc_MailPath[MAXPATHS]; /* Where to look for new mail            */
static char rc_ReplyPath[MAXPATHS];/* Where to put replies                  */
static char rc_WorkPath[MAXPATHS]; /* Where to archive/unarchive mail       */
static char rc_CurBoard[50];       /* Name of the current board...          */
static char rc_termtype[80];       /* TERM environ variable                 */

/* public access to path data */
const char CONSPTR HomePath  = rc_HomePath;
const char CONSPTR MailPath  = rc_MailPath;
const char CONSPTR ReplyPath = rc_ReplyPath;
const char CONSPTR WorkPath  = rc_WorkPath;
const char CONSPTR CurBoard  = rc_CurBoard;
const char CONSPTR atp_termtype  = rc_termtype;

/* local strings set by atprc */
static char User1Name[50];         /* user name taken from atprc            */
static char Editor[50];            /* name of the standard text editor      */
static char Archiver[50];          /* program to pack qwk message replies   */
static char UnArchiver[50];        /* program to unpack qwk message packets */
static char qwklist[100];          /* command line for listing QWK packet   */
static char bltlist[100];          /* command line for listing bulletins    */
static char speller[100];          /* command line for invoking speller     */

/* pseudo const boolean flag variables set from atprc */
atp_BOOL_T autotag  = TRUE;        /* flag for random taglines              */
atp_BOOL_T graphics = FALSE;       /* flag for VT102 line graphics mode     */
atp_BOOL_T ansi     = TRUE;        /* flag true if ansi output is on        */
atp_BOOL_T silent   = FALSE;       /* flag for no tty bell                  */
atp_BOOL_T fido     = FALSE;       /* flag for tagline style                */
atp_BOOL_T color    = TRUE;        /* flag true if ansi output is on        */
atp_BOOL_T pcbext   = TRUE;        /* flag true to display long subjects    */

/*  local state flags */
static atp_BOOL_T pmail = FALSE;   /* flag true if personal mail waiting    */
static atp_BOOL_T caps  = FALSE;   /* true if bbs prefers caps for to/from  */
static atp_BOOL_T HeadLetter = TRUE; /* true if automatic header is active  */
static atp_CODE_T charset = CHR7BIT; /* latin1, msdos, or 7-bit ascii.      */
static int ScrnRows = 0;           /* initialized by TTYinfo() or atprc     */
static int ScrnCols = 0;           /* initialized by TTYinfo() or atprc     */

/************ end of globals **********/


/* 
 * public access functions
 *
 */


/*
 * toglpcb - toggle support for PCBoard long subjects and features.
 */
void
toglpcb(void)
{
    pcbext = pcbext ? FALSE : TRUE ;
    yellow();
    high();
    printf("%s %s .\n", "PCBoard long subjects are", pcbext ? txt[85] : txt[86]);
}


/* 
 * toglfido - toggle tagline styles between FIDO and PCBoard.
 */
void
toglfido(void)
{
    fido = fido ? FALSE : TRUE ;
}


/* 
 * toglauto - toggle tagline styles between FIDO and PCBoard.
 */
void
toglauto(void)
{
    autotag = autotag ? FALSE : TRUE;
}


/*
 * test_line_graphics - displays a small test pattern.
 */
void
test_line_graphics(void)
{
    graphics = graph_togl(graphics);
}

/*@-strictops */
/*
 * set_CurBoard - restricted write access to Bbs name buffer.
 */
void
set_CurBoard(const char *str, const curbrd_access_t perm)
{
    if (cb_RdCn_main_init <= perm && perm <= cb_Read)
	strcpy(rc_CurBoard, str);
#ifdef ATPDBG
    else
	assert(cb_RdCn_main_init <= perm && perm <= cb_Read);
#endif
}


/*
 * set_ScrnRows - restricted write access to ScrnRows variable.
 */
void
set_ScrnRows(const int siz, const scrn_access_t perm)
{
    if (ss_getwinders <= perm && perm <= ss_TTYinfo)
	ScrnRows = siz;
#ifdef ATPDBG
    else
	assert(ss_getwinders <= perm && perm <= ss_TTYinfo);
#endif
}


/*
 * set_ScrnCols - restricted write access to ScrnCols variable.
 */
void
set_ScrnCols(const int siz, const scrn_access_t perm)
{
    if (ss_getwinders <= perm && perm <= ss_TTYinfo)
	ScrnCols = siz;
#ifdef ATPDBG
    else
	assert(ss_getwinders <= perm && perm <= ss_TTYinfo);
#endif
}


/*
 * set_pmail - restricted write access to pmail flag.
 */
void
set_pmail(const atp_BOOL_T mode, const pmail_access_t perm)
{
    if (pmPurge_D <= perm && perm <= pmLoad)
	pmail = mode;
#ifdef ATPDBG
    else
	assert(pmPurge_D <= perm && perm <= pmLoad);
#endif
}
/*@=strictops */

/*
 * get_pmail - read access for pmail flag.
 */
atp_BOOL_T
get_pmail(void)
{
    return pmail;
}


/*
 * get_ScrnRows - read access for ScrnRows variable.
 */
int
get_ScrnRows(void)
{
    return ScrnRows;
}


/*
 * get_ScrnCols - read access for ScrnCols variable.
 */
int
get_ScrnCols(void)
{
    return ScrnCols;
}


/*
 * get_charset - read access for charcter set variable.
 */
atp_CODE_T
get_charset(void)
{
    return charset;
}


/*
 * get_caps - allows global read access to caps variable.
 */
atp_BOOL_T
get_caps(void)
{
    return caps;
}


/*
 * set_caps - allows global write access to caps variable.
 */
void
set_caps(const atp_BOOL_T val)
{
    caps = val;
}


/*
 * get_HeadLetter - allows global read access to HeadLetter variable.
 */
atp_BOOL_T
get_HeadLetter(void)
{
    return HeadLetter;
}


/*
 * ansi_toggle - flips mode from ansi to non-ansi.
 */
void
ansi_toggle(void)
{
    ansi = ansi ? FALSE : TRUE;
    yellow();
    high();
    printf("%s %s .\n", txt[84], ansi ? txt[85] : txt[86]);
}


/*
 * head_toggle - called by ReadShell().
 */
void
head_toggle(void)
{
    blue();
    high();
    HeadLetter = HeadLetter ? FALSE : TRUE ;
    printf("%s ", txt[101]);    /* "Automatic H." */
    printf("%s\n", HeadLetter ? txt[85] : txt[86]);     /* "on" / "off"   */
    green();
    fflush(stdout);
}


/* used by DoLoad(), Read(), Reply(), */

/*
 * test_for_caps - called by PutHeader(). checks to/from fields for upper case.
 */
void
test_for_caps(const char *Header)
{
    int i ;
    const char CONSPTR pntr = Header + HForWhom;
    caps = TRUE;
    for (i = HForWhom; i < HSubject; i++) /*@-strictops */
        if (pntr[i] >= 'a' && pntr[i] <= 'z') /*@=strictops */
            caps = FALSE;
}


/*
 * get_atprc_str - returns copies of the atprc string variables.
 */
char *
get_atprc_str(const atprc_str_t token)
{
    char *strnm = NULL;
    switch (token) {
    case usr1nm:
        strnm = strdup(User1Name);
        break;
    case editr:
        strnm = strdup(Editor);
        break;
    case archivr:
        strnm = strdup(Archiver);
        break;
    case unarchvr:
        strnm = strdup(UnArchiver);
        break;
    case qwklst:
        strnm = strdup(qwklist);
        break;
    case bltlst:
        strnm = strdup(bltlist);
        break;
    case spellr:
        strnm = strdup(speller);
        break;
    default:
        /* switch() case error */
	error_switch_case((int)token, __FILE__, __LINE__);
    }
    test_fatal_malloc(strnm, __FILE__, __LINE__);
    return strnm;
}


/*
 * Clean - builds the full work directory pathname and cleans work directory.
 *
 * ( System dependent functions are in system.c )
 */
void
Clean(void)
{   /*@-strictops */
    if (WorkPath[0] == NUL_CHAR) {  /*@=strictops */
        strcpy(rc_WorkPath, HomePath);
        strcat(rc_WorkPath, WORK_DIR);
    }
    if (access(WorkPath, F_OK) == FAILURE)
        (void)my_mkdir(WorkPath);
    else {
        printf("%s %s...\n", txt[57], WorkPath);        /* "Cleaning" */
        Erase(WorkPath);
    }
}


/*
 * PrePath, adds the last separator to a pathname.
 * Must be room enough for that in the string !
 */
static void
PrePath(char *path)
{
    int l;

    ADDRIVE(path);
    l = strlen(path); /*@-strictops */
    if (path[l - 1] != SEP && path[l - 1] != SEPDOS) {  /*@=strictops */
	path[l] = SEP;
	path[l + 1] = NUL_CHAR;
    }
}

/*
 * MakeHomePath - creates the Home directory pathname string.
 *
 *  In the Unix version we look for for the environment variable
 *  ATP. If that is not found we look for the HOME variable and
 *  make our home path with that. If that fails we use the current
 *  working directory as out home path. If that fails, we exit
 *  after printing an error message. Note that Unix doesn't pass
 *  the path in argv[0], only the name of the program. The MS-DOS
 *  version assumes that argv[0] contains the current path.
 *  MakeHomePath is invoked from main() and is passed argv[0]. 
 */
atp_BOOL_T
MakeHomePath(void)
{
    atp_BOOL_T ret_code = FALSE;
    char *pt;
    if (strlen(getenv("PATH")) > (size_t) MAXPATHS) {
        printf("PATH environment too long: %d\n", (int) MAXPATHS);
    } else if ((pt = (char *) getenv("ATP")) != NULL
               || (pt = (char *) getenv("HOME")) != NULL
               || (pt = GETCWD(rc_HomePath, MAXPATHS)) != NULL) {
        size_t path_len;
        strcpy(rc_HomePath, pt);
        PrePath(rc_HomePath);
        path_len = strlen(rc_HomePath);
        while (path_len != (size_t)0) {          /*@-strictops */
            if (rc_HomePath[path_len] == SEP || 
	        rc_HomePath[path_len] == ':' ||
                rc_HomePath[path_len] == SEPDOS) /*@=strictops */
                break;
            rc_HomePath[path_len] = NUL_CHAR;
            path_len--;
        }
        pt = rc_HomePath;           /*@-strictops */
        while (*pt != NUL_CHAR) {  
            if (*pt == SEPDOS)      /*@=strictops */
                *pt = SEP;
            pt++;
        }
        ret_code = TRUE;
    } else {
        printf("\nERROR: MakeHomePath() module readlib.c\n");
    }
    return ret_code;
}

/*
 * init_termtype - set terminal type from TERM environment variable. 
 */
void
init_term_type(void)
{
    luxptr = getenv("TERM");
    if (luxptr == NULL)
        strcpy(rc_termtype, " ");
    else
        strcpy(rc_termtype, luxptr);
}

/*
 * init_charset, set character set to latin1 or 7-bit or msdos per atprc.
 */
static void
init_charset(char *value)
{
    if (strnicmp(value, "iso", 3) == SUCCESS
	|| strnicmp(value, "lat", 3) == SUCCESS) {
	charset = ISOLAT1;
    } else if (strnicmp(value, "dos", 3) == SUCCESS
	       || strnicmp(value, "msd", 3) == SUCCESS) {
	charset = CHRDOS;
    } else {
	charset = CHR7BIT;
    }
}

/*
 * init_warning, warn about old atp version 1.3 atprc files.
 */
static void
init_warning(void)
{
    printf("\n\n\n READ THE DOCS: you must update your atprc file!\n\n");
    printf(" \"taglist\"  not valid opton for this release. \n");
    printf(" Taglines are now stored in textfile \"taglines.atp\".\n\n");
    abort();
}

/*
 * init_screen, set screen row and columns from atprc.
 */
static void
init_screen(const char *var, const char *value)
{
    if (Numeric(value)) {
        int tmpint = atoi(value);
        tmpint = (tmpint < 0) ? -tmpint : tmpint;
        if (2 < tmpint && tmpint < 301) {       /* range check */
            if (strcmp(var, "screenlen") == SUCCESS)
                set_ScrnRows(tmpint, ss_init_screen);
            else
                set_ScrnCols(tmpint, ss_init_screen);
        }
    }
}

/* default confernce truncation number */ 
static long TruncNum  = 50L;

/*
 * get_TruncNum - returns the current value for truncation length.
 */
long
get_TruncNum(void)
{
    return TruncNum;
}

/*
 * init_truncate, set truncation length from atprc for "clean" command.
 */
static void
init_truncate(const char *value)
{
    if (Numeric(value)) {
        long tmplong = atol(value);
        tmplong = (tmplong < 0L) ? -tmplong : tmplong;
        if (tmplong < 1L)
            tmplong = 1L;
        TruncNum = tmplong;
    }
}



/*
 * init_program, parses atprc for editor/archiver/unarchiver/speller.
 */
static atp_ERROR_T
init_program(char *var, char *value, char *aLine)
{
    atp_ERROR_T ret_code = ATP_OK;
    char *tptr;
    if ((tptr = strstr(aLine, " = ")) != NULL
        && (tptr = strstr(tptr, value)) != NULL) {
        if (strcmp(var, "archiver") == SUCCESS) {
            strcpy(Archiver, tptr);
        } else if (strcmp(var, "unarchiver") == SUCCESS) {
            strcpy(UnArchiver, tptr);
        } else if (strcmp(var, "speller") == SUCCESS) {
            strcpy(speller, tptr);
        } else if (strcmp(var, "editor") == SUCCESS) {
            strcpy(Editor, tptr);
        } else
            ret_code = ATP_ERROR;
    } else
        ret_code = ATP_ERROR;
    return ret_code;
}


/*
 * init_workpath, setup various paths used by ATP from atprc entries.
 */
static void
init_path(const char *var, const char *value)
{
    char *newpath = NULL;
    if (strcmp(var, "workpath") == SUCCESS)
        newpath = rc_WorkPath;
    else if (strcmp(var, "mail") == SUCCESS)
        newpath = rc_MailPath;
    else if (strcmp(var, "reply") == SUCCESS)
        newpath = rc_ReplyPath;
    if (newpath != NULL) { /*@-strictops */
        if (value[0] != SEP && value[1] != ':') { /*@=strictops */
	    /*@-returnval */
            GETCWD(newpath, MAXPATHS);
	    /*@=returnval */
            PrePath(newpath);
        }
        strcat(newpath, value);
        PrePath(newpath);
        if (strcmp(var, "workpath") == SUCCESS)
            strcat(newpath, WORK_DIR);
    }
}


/*
 * affect, initialize configuration variables to their global values.
 * returns ATP_ERROR on an invalid line. Ignores unknown variables.
 */
static atp_ERROR_T
affect(char *var, const char *sign, char *value1, char *value2, char *aLine)
{
    atp_ERROR_T ret_code = ATP_OK;
    (void)strlwr(var);
    if (strcmp(sign, "=") != SUCCESS) {
	ret_code = ATP_ERROR;
    } else {
	if (strcmp(var, "workpath") == SUCCESS) {
	    init_path(var, value1);
	} else if (strcmp(var, "mail") == SUCCESS) {
	    init_path(var, value1);
	} else if (strcmp(var, "reply") == SUCCESS) {
	    init_path(var, value1);
	} else if (strcmp(var, "ansi") == SUCCESS) {
	    /* Starting in ansi mode or not... */
	    ansi = stricmp(value1, "off") == SUCCESS ? FALSE : TRUE;
	} else if (strcmp(var, "bell") == SUCCESS) {
	    silent = stricmp(value1, "on") == SUCCESS ? FALSE : TRUE;
	} else if (strcmp(var, "user") == SUCCESS) {
	    /* Get User First and Last Name      */
	    sprintf(User1Name, "%s %s", value1, value2);
	} else if (strcmp(var, "tagline") == SUCCESS) {
	    init_tagpers(value1, sign, aLine);
	} else if (strcmp(var, "tagstyle") == SUCCESS) {
	    /* default tag style Fido or PCB */
	    init_tagstyle(value1);
	} else if (strcmp(var, "color") == SUCCESS) {
	    /* Starting in ansi color mode or not... */
	    color = stricmp(value1, "on") == SUCCESS ? TRUE : FALSE;
	} else if (strcmp(var, "autotag") == SUCCESS
		   && stricmp(value1, "OFF") == SUCCESS) {
	    autotag = FALSE;
	} else if (strcmp(var, "graphics") == SUCCESS
		   && stricmp(value1, "ON") == SUCCESS) {
	    graphics = TRUE;
	} else if (strcmp(var, "pcb") == SUCCESS &&
		   stricmp(value1, "OFF") == SUCCESS) {
	    pcbext = FALSE;
	} else if (strcmp(var, "header") == SUCCESS
		   && stricmp(value1, "OFF") == SUCCESS) {
	    HeadLetter = FALSE;
	} else if (strcmp(var, "charset") == SUCCESS) {
	    init_charset(value1);
	} else if (strcmp(var, "taglist") == SUCCESS) {
	    /* warn that old atprc was found */
	    init_warning();
	} else if (strcmp(var, "screenlen") == SUCCESS) {
	    /* default screenlength */
	    init_screen(var, value1);
	} else if (strcmp(var, "screencol") == SUCCESS) {
	    /* default screenwidth */
	    init_screen(var, value1);
	} else if (strcmp(var, "truncate") == SUCCESS) {
	    /* default database truncation length */
	    init_truncate(value1); /*@-strictops */
	} else if (value1[0] != NUL_CHAR) { /*@=strictops */
	    char *tptr = NULL;
	    if (strcmp(var, "qlist") == SUCCESS
		&& (tptr = strstr(aLine, value1)) != NULL) {
		/* get command for listing packets */
		assert(tptr != NULL);
		strcpy(qwklist, tptr);
	    } else if (strcmp(var, "blist") == SUCCESS
		       && (tptr = strstr(aLine, value1)) != NULL) {
		/* get command for listing packets */
		assert(tptr != NULL);
		strcpy(bltlist, tptr);
	    } else if (strcmp(var, "speller") == SUCCESS
		       || strcmp(var, "archiver") == SUCCESS
		       || strcmp(var, "unarchiver") == SUCCESS
		       || strcmp(var, "editor") == SUCCESS) {
		ret_code = init_program(var, value1, aLine);
	    }
	}
    }
    return ret_code;
}

/*
 * open_atprc, opens atprc configuration file for ReadConfig.
 */
static FILE *
open_atprc(char *Path)
{
    FILE *cfg;
    strcpy(Path, HomePath);
    strcat(Path, ".");
    strcat(Path, CONFIG_FILE);
    /* try to open cfg "dot" file */
    if ((cfg = fopen(Path, "rb")) == NULL) {
        strcpy(Path, HomePath);
        strcat(Path, CONFIG_FILE);
        /* try again non-dot file     */
        if ((cfg = fopen(Path, "rb")) == NULL) {
            /* "Unable to open config file" */
            printf("\n%s %s or .%s ! \n", txt[52], Path, CONFIG_FILE);
	    /*@+voidabstract */
            cfg = NULL;
	    /*@=voidabstract */
        }
    }
    return cfg;
}


 /*
 * parse_config, called by ReadConfig().
 */
static atp_ERROR_T
parse_config(FILE * cfg, char *Path) /* !! */
{
    atp_ERROR_T our_status = ATP_OK; /* !! do we need both of these? */
    atp_ERROR_T ret_code = ATP_OK;
    int count = 0;

    char RCLine[MAXPATHS], var[100], sign[100], value1[100], value2[100];
    /*@-strictops */
    while (ret_code == ATP_OK && our_status == ATP_OK && /*@i1*/ fget(RCLine, 255, cfg)) {
        count++; /*@=strictops */
        StripLSpace(RCLine); /*@-strictops */
        if (RCLine[0] != '#' && (strlen(RCLine) >= (size_t) 2)) { /*@=strictops */
            var[0] = sign[0] = value1[0] = value2[0] = NUL_CHAR;
            sscanf(RCLine, "%s %s %s %s", var, sign, value1, value2); /*@-strictops */
            if ((our_status = affect(var, sign, value1, value2, RCLine)) == ATP_ERROR) {
                /* "error in config file in line..." */ /*@=strictops */
                printf("%s %s %s %d\n", txt[54], Path, txt[55], count);
                printf("%s %d : %s\n", txt[56], count, RCLine);
                ret_code = ATP_ERROR;
                break;
            }
        }
    }
    return ret_code;
}


/*
 * ReadConfig - read the configuration file by invoking parse_config().
 */
atp_ERROR_T
ReadConfig(void)
{
    char Path[MAXPATHS];
    atp_ERROR_T ret_code = ATP_ERROR;
    FILE *cfg;
    printf("%s...\n", txt[53]); /* "Reading config file" */
    speller[0] = NUL_CHAR;
    strcpy(Archiver, "zip");    /* initialize default packer */
    strcpy(UnArchiver, "unzip");        /* initialize default unpacker */
    strcpy(Editor, "vi");       /* initialize default editor */
#ifdef UNIXCMDS
    strcpy(qwklist, "ls *.q* ");        /* initialize qwklist command */
    strcpy(bltlist, "ls blt*.* ");      /* initialize qwklist command */
#else
    strcpy(qwklist, "dir *.q* ");
    strcpy(bltlist, "dir blt*.* ");
#endif
    if ((cfg = open_atprc(Path)) != NULL) {
        ret_code = parse_config(cfg, Path);
        fclose(cfg);
    }
    return ret_code;
}

#if defined(TIOCGWINSZ) && defined(SIGWINCH)
/*@-paramuse */
/* 
 * getwinders - signal handler for SIGWINCH.
 */
RETSIGTYPE
getwinders(int dummy_arg)
{
    struct winsize siz;
#if !defined(HAVE_LIBTERMCAP)
    static int firstWinSz = FALSE;
#endif
#ifndef HAVE_SIGACTION
    signal(SIGWINCH, getwinders);
#endif
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &siz) < 0) {
	printf("\nTIOCGWINSZ error getwinder()\n");
	(void) sleep(3);
    } else {
/*
   We need a sanity check the first time this is called because if we
   are using a tty and not the console, the row and column sizes may be
   set to zero. This will cause Display() to fail by truncating output.
   Note that after initialization, this routine is only likely to be
   invoked by a SIGWINCH generated by running ATP under X Window system.

   If we don't have termcap, we must do sanity check *here* because 
   because without termcap, sanity checks in TTYinfo are bypassed 
 */
#if !defined(HAVE_LIBTERMCAP)
	if (firstWinSz || (!firstWinSz && siz.ws_row > 2 && siz.ws_col > 2)) {
	    set_ScrnRows(siz.ws_row, ss_getwinders);
	    set_ScrnCols(siz.ws_col, ss_getwinders);
	    firstWinSz = TRUE;
	}
#else
	set_ScrnRows(siz.ws_row, ss_getwinders);
	set_ScrnCols(siz.ws_col, ss_getwinders);
#endif
    }
}
/*@=paramuse */
#endif



/*
 * BEGIN REUP MEMORY MANAGEMENT ROUTINES
 *
 */

static size_t RbufSize = MYBUF;     
static size_t RbufRecs = (MYBUF+(size_t)(BLKSIZE - 1))/(size_t)BLKSIZE;

/*
 * get_RbufSize - returns the size of the message buffer.
 */
size_t
get_RbufSize(void)
{
    assert(RbufSize == (RbufRecs * block_SIZE));
    return RbufSize;
}

/*
 * get_RbufRecs - returns the message buffer size in number of 128 byte records.
 */
size_t
get_RbufRecs(void)
{
    assert(RbufRecs == (RbufSize / block_SIZE));
    return RbufRecs;
}

/* 
 * reup - reallocate rbuf message buffer. returns TRUE = success. FALSE = fail.
 */
atp_BOOL_T
reup(const size_t bfsz)
{
    atp_BOOL_T ret_code = FALSE;
    char *tptr = NULL;

    assert(mybuf <= bfsz && rbuf != NULL);
    if (bfsz <= maxbuf && (tptr = (char *)realloc(rbuf, bfsz + block_SIZE)) != NULL) {
        /* realloc succeeded */
        rbuf = tptr;
        /* bfsz = msgheader + message */
        RbufSize = bfsz;
        /* round upward the number of blocks in RbufRecs */
        RbufRecs = (size_t) ((bfsz + block_SIZE - (size_t)1) / block_SIZE);
        ret_code = TRUE;
    }
#ifdef ATPDBG2
    printf("Inside reup()  bufsize = %lu \n", (unsigned long)bfsz), sleep(2);
#endif
    return ret_code;
}

/* end of rdconfig.c */
