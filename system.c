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
system.c
*/

/*
 * Operating System system-dependant functions for ATP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"
#include "readlib.h"
#include "ansi.h"
#include "qlib.h"

#ifdef DJGPP
#include <pc.h>
#include <dos.h>
#endif

#if defined(__MSDOS__)
#ifdef _MSC_
#include <direct.h>
#else
#include <dir.h>
#endif
#include <io.h>
#include <dos.h>
#endif

#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif

/*
 * Erase - deletes ALL files in a directory pointed to by PathName.
 * This is system-dependant.
 */
void
Erase(const char *PathName)
{
    char Pattern[MAXPATHS];

#if defined(DIRENTRY)
    DIR *dirp;
    struct DIRENTRY *firp;

    dirp = opendir(PathName);
    if (dirp != NULL) {
	while ((firp = readdir(dirp)) != NULL)  /*@-strictops */
	    if (firp->d_name[0] != '.') {       /*@=strictops */
		sprintf(Pattern, "%s%c%s", PathName, SEP, firp->d_name);
		do_unlink(Pattern);
	    }
	(void)closedir(dirp);
    } else {
	printf("Can't seem to open %s\n", PathName);
    }

#elif defined(UNIXCMDS)
    sprintf(Pattern, "rm -f %s/*", PathName);
    system(Pattern);

#elif !defined(WIN32)		/* for dos, this method saves you from an annoying prompt */
    /* see below for WIN32 variation */
    int ok;
    char tmp[130];
    char cwd[130];
    struct ffblk FileDoc;

    GETCWD(cwd, 128);
    sprintf((char *) Pattern, "%s\\*.*", PathName);

    if ((ok = findfirst((char *) Pattern, &FileDoc, 0)) == ATP_OK) {
	while (ok == ATP_OK) {
	    sprintf(tmp, "%s\\%s", PathName, FileDoc.ff_name);
	    do_unlink(tmp);
	    ok = findnext(&FileDoc);
	}
	CHPATH(cwd);
    }
/* WIN32 below */
#else

    int ok;
    long filegrouphandle;
    char tmp[MAXPATHS];
    char cwd[MAXPATHS];

    struct _finddata_t FileDoc;

    GETCWD(cwd, MAXPATHS - 2);
    sprintf((char *) Pattern, "%s\\*.*", PathName);

    if ((ok = _findfirst((char *) Pattern, &FileDoc)) == ATP_OK) {
	filegrouphandle = ok;
	while (ok == ATP_OK) {
	    sprintf(tmp, "%s\\%s", PathName, FileDoc.name);
	    do_unlink(tmp);
	    ok = _findnext(filegrouphandle, &FileDoc);
	}
	_findclose(filegrouphandle);
	CHPATH(cwd);
    }
#endif
}				/* end of Erase() */


#if !defined(HAVE_STRNICMP)

/*
 * strnicmp - case insensitive counted string comparison.
 */
int
strnicmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && *s2) {
	if (tolower(*s1) != tolower(*s2))
	    return (tolower(*s1) > tolower(*s2) ? 1 : -1);
	s1++;
	s2++;
	n--;
    }
    if (n) {
	if (*s1)
	    return (1);
	if (*s2)
	    return (-1);
    }
    return (0);
}


/*
 * strnicmp - case insensitive string comparison.
 */
int
stricmp(const char *s1, const char *s2)
{
    while (*s1 != NUL_CHAR && *s2 != NUL_CHAR) {
	if (tolower(*s1) != tolower(*s2))
	    return (tolower(*s1) > tolower(*s2) ? 1 : -1);
	s1++;
	s2++;
    }
    if (*s1)
	return (1);
    if (*s2)
	return (-1);
    return (0);
}

#endif

#if !defined(HAVE_STRUPR)
/*@-strictops */
/*
 * strlwr - converts string to lower case.
 */
char *
strlwr(char *s)
{
    char *s1;

    s1 = s;
    while (*s1 != NUL_CHAR ) {
	*s1 = tolower(*s1);
	s1++;
    }
    return (s);
}

/*
 * strupr - converts string to upper case.
 */
char *
strupr(char *s)
{
    char *s1;

    s1 = s;
    while (*s1 != NUL_CHAR) {
	*s1 = toupper(*s1);
	s1++;
    }
    return (s);
}
/*@=strictops */
#endif

#if !defined(HAVE_STRSTR) || defined(NEED_STRSTR)
/*
 *  Copyright (C) 1991, 1992 Free Software Foundation, Inc.
 *  This function is part of the GNU C Library. 
 */

/*
 * strstr - find a substring in another string (from GNU C library).
 *
 * Return the first ocurrence of NEEDLE in HAYSTACK. 
 */
char *
strstr(const char *haystack, const char *needle)
{
    const char CONSPTR needle_end = strchr(needle, NUL_CHAR);
    const char CONSPTR haystack_end = strchr(haystack, NUL_CHAR);
    const size_t needle_len = needle_end - needle;
    const size_t needle_last = needle_len - 1;
    const char *begin;

    if (needle_len == 0)
	return (char *) haystack;	/* ANSI 4.11.5.7, line 25.  */
    if ((size_t) (haystack_end - haystack) < needle_len)
	return NULL;

    for (begin = &haystack[needle_last]; begin < haystack_end; ++begin) {
	const char *n = &needle[needle_last];
	const char *h = begin;

	do {
	    if (*h != *n)
		goto loop;	/* continue for loop */
	} while (--n >= needle && --h >= haystack);

	return (char *) h;

      loop:;
    }
    return NULL;
}
#endif

#if !defined(HAVE_TMPNAM) || defined(NEEDTMPNAM)

#undef L_tmpnam
#define L_tmpnam 30

/* 
 * mytmprnam - provides the posix tmpnam() function.
 *  assumes the existence of the non-posix mktemp() function. 
 */
char *
mytmprnam(char *buf)
{
    static char tbf[(L_tmpnam + 1)];
    char *tbptr;

    tbf[0] = NUL_CHAR;
#if defined(SYS_UNIX)
    strcpy(tbf, "/usr/tmp/");
#endif
    strcat(tbf, "tmXXXXXX");
    tbptr = mktemp(tbf);
    if (tbptr == NULL) {	/* try to return something anyway */
	tbptr = tbf;
    }
    if (buf != NULL) {
	strcpy(buf, tbptr);
	tbptr = buf;
    }
    return tbptr;
}

#endif

/* unix terminal functions, see system.h for more info */
#if defined(SYS_UNIX)
/*
 * restore_terminal - reset terminal from previously saved attributes.
 */
static struct ATPSRTERM savedterm;
void
restore_terminal(void)
{
    (void)ATPSETERM;
}

/*
 * save_terminal - store terminal attributes for later retrieval.
 */
void
save_terminal(void)
{
    (void)ATPGETERM;
}

#endif


/*
 * BEGIN PROCESS CREATION ROUTINES
 *
 */

#if defined(USE_FORK)
static char *eargv[101], tbf[MAXPATHS];

/*@-strictops */
/*
 * build_eargv, build array of arguments for execvp.
 */
static atp_ERROR_T
build_eargv(const char *prg, const char *prgfile)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    char *tp = tbf;

    /* create command line */
    sprintf(tbf, "%s %s", prg, prgfile);
    StripLSpace(tbf);

    /* decompose command line into arguments */
    if (tbf[0] != NUL_CHAR) {
	int i = 1;
	for (; i < 100; i++) {
	    eargv[i] = 0;
	    while (*tp != SPC_CHAR && *tp != TAB_CHAR && *tp != NUL_CHAR)
		tp++;
	    if (*tp != NUL_CHAR)
		*tp = NUL_CHAR;
	    else
		break;
	    tp++;
	    while (*tp == SPC_CHAR || *tp == TAB_CHAR) {
		*tp = NUL_CHAR;
		tp++;
	    }
	    if (*tp != NUL_CHAR)
		eargv[i] = tp;
	    else
		break;
	    tp++;
	}
	eargv[0] = tbf;
	eargv[100] = 0;
	ret_code = ATP_OK;
    }
    return ret_code;
}
/*@=strictops */
#endif


#ifdef USE_FORK
static pid_t Proc_ID = NO_CHILD;
static atp_BOOL_T reperror = FALSE ;

/*
 * get_reperror - returns value of reperror reply error flag.
 */
atp_BOOL_T
get_reperror(void)
{
    return reperror;
}

/*
 * reset_reperror - resets value of reperror reply error flag to FALSE.
 */
void
reset_reperror(void)
{
    reperror = FALSE;
}
/*@-paramuse */

/* 
 * kill_child - signal handler for SIGINT to catch a signal in Reply().
 */
RETSIGTYPE
kill_child(int dummy_arg)
{
#ifndef HAVE_SIGACTION
    signal(SIGINT, SIG_IGN);
#endif
    if (Proc_ID > 0) {
	(void)kill(Proc_ID, SIGTERM);
	(void)sleep(2);
	(void)kill(Proc_ID, SIGKILL);
#ifndef HAVE_SIGACTION
	signal(SIGINT, kill_child);
#endif
    }
    restore_terminal();		/* restore terminal parameters */
    reperror = TRUE;
}
/*@=paramuse */
#endif				/* USE_FORK */


/* 
 * fork_execvp - spawn process via fork()/execvp() or system().
 */
void
fork_execvp(const char *prg, const char *prgfile)
{
#ifdef USE_FORK
    Proc_ID = NO_CHILD;                            /*@-strictops */
    if (build_eargv(prg, prgfile) == ATP_OK) {     /*@=strictops */
	if ((Proc_ID = fork()) == 0) {
	    /* child */
	    (void)execvp(tbf, eargv);
	    /* this should never be reached */
	    exit(EXIT_FAILURE);
	}
	if (Proc_ID < 0) {
	    reperror = TRUE;
	} else {
	    /* parent */
	    /* set new handler for SIGINT */
	    pid_t npid;
	    sig_chint();
	    do {
		int childstatus;
		npid = wait(&childstatus);
	    } while (npid != Proc_ID);
	    sig_ignore(SIGINT);
	}
	/* reset Proc_ID to indicate no active child */
	Proc_ID = NO_CHILD;
    }
#else
    char forkbuf[MAXPATHS];
    /* build command for call system() */
    sprintf(forkbuf, "%s %s", prg, prgfile);
    /* execute command */
    system(forkbuf);
#endif
}
/* END OF PROCESS CREATION ROUTINES */



/* 
 * replacement for missing getcwd() function 
 */

#if !defined(HAVE_GETCWD) || defined(NEEDGETCWD)

#ifdef no_proto
char *getwd();
#else
EXTERNC char *getwd(char *p);
#endif

/*
 * mygetcwd - provides posix getcwd() assuming getwd() is available.
 */
char *
mygetcwd(char *buf, size_t siz)
{
    char tmp[MAXPATHS];

    memset(tmp, 0, (size_t) (siz - 1));
    getwd(tmp);
#ifdef DJGPP
    addrive(tmp);
#endif
    if (siz <= MAXPATHS)
	tmp[siz - 1] = 0;
    strcpy(buf, tmp);
    return (buf);
}
#endif

/* ************************************************************************ */
/* THE FOLLOWING SECTION IS DEVOTED TO MS-DOS, OS/2, AND WINDOWS STUFF ONLY */
/* ************************************************************************ */

#ifdef __MSDOS__

/* 
 * DosChPath - (msdos) changes active path to new drive and directory.
 */
atp_ERROR_T
DosChPath(const char *path)
{
    int driv;
    atp_ERROR_T ret_code = ATP_ERROR;

    /* get drive letter from path */
    driv = toupper(path[0]);

    /*  test for valid drive designator at start of path */
    if ((path[2] == SEP || path[2] == '/') && path[1] == ':' && driv > '@' && driv < 'Q') {
	/* convert to numeric */
	const int drive_mask = 15;
	driv &= drive_mask;
#ifndef _MSC_
	driv--;
#endif
	if (setdisk(driv) < driv) {
	    /* can't set drive */
	    printf("ERROR: can't set drive to '%c:'\n", driv);
	    sleep(4);
	} else if (chdir(path) == ATP_OK) {
	    ret_code = ATP_OK;
	} else {
	    /* can't change to new path */
	    printf("ERROR: can't change path to '%s:'\n", path);
	    sleep(4);
	}
    }
    return ret_code;
}
#endif

#if defined(_OS2) || defined(__MSDOS__)

/* 
 * addrive - (msdos) prepends a drive designator to a path name.
 */
void
addrive(char *path)
{
    if ((path[2] != SEP && path[2] != SEPDOS) || path[1] != ':') {
	char tmp[MAXPATHS];
	int driv;
#ifdef __EMX__
	driv = _getdrive();
#else
	driv = getdisk();
	driv = driv + 'A';
#endif
	if (path[0] != SEP && path[0] != SEPDOS)
#ifdef DJGPP
	    sprintf(tmp, "%c:%c%s", driv, '/', path);
#else
	    sprintf(tmp, "%c:%c%s", driv, SEP, path);
#endif
	else
	    sprintf(tmp, "%c:%s", (char) driv, path);
	strcpy(path, tmp);
    }
}

#endif

#ifdef NEEDISKS

#ifdef __MSDOS__

/* 
 * getdisk - (msdos) get and return current active drive.
 */
int
getdisk(void)
{
    const int drive_mask = 15;
    int temp;
#ifdef _MSC_
    _dos_getdrive(&temp);
    return temp;
#else
    union REGS regs;

    regs.h.ah = 0x19;
    regs.x.dx = 0;
    regs.h.al = 0;

    temp = intdos(&regs, &regs);
    return (temp & drive_mask);
#endif
}

/*
 * setdisk - (msdos) set the current active disk drive.
 */
int
setdisk(int driv)
{
    int temp;
    const int mask = 0x7F;
#ifdef _MSC_
    _dos_setdrive(driv, &temp);
#else
    union REGS regs;

    regs.h.ah = 0x0e;
    regs.h.dl = (char) (driv & mask);
    regs.h.al = 0;

    temp = intdos(&regs, &regs);
#endif
    return (temp);
}

#endif
#endif

#ifdef NEED_SLEEP
/* Microsoft QuickC 2.0 doesnt' seem to have sleep() in its library */

#ifdef __MSDOS__

#define DOS_TICK_CNTR 0x046c

#define time_t long

/*
 * sleep - (msdos) provides sleep routine for Microsoft C.
 */
void
sleep(unsigned sec)
{
    time_t *ticker = (time_t *) DOS_TICK_CNTR;
    time_t tick, tock;

    tock = (time_t) ((181 * sec) + 5) / 10;	/* fixed point math with round up */
    tick = *ticker;
    tock += tick;

    do {
	A_DVPAUZ;		/* if we are desqview aware this is defined to dv_pause(); */
	tick = *ticker;
    } while (tick < tock);
}

#else
void
sleep(int sec)
{
    time_t tick1, tick2, tock;

    tick1 = time(&tock);
    do {
	tick2 = time(&tock);
    } while ((tick2 - tick1) < sec);

}
#endif
#endif				/* need sleep */


#ifdef DOSTIME
/*
 * msdos_time_init - (msdos) pseudo-posix time zone routine for MSDOS.
 */
void
msdos_time_init(void)
{
    char *tp;
    if (!((tp = getenv("TZ")) != NULL && *tp
          && strlen(tp) > 3 && atoi(tp + 3) != 0)) {
        putenv("TZ=TML0CAL");
    }
    tzset();
}
#endif


#if 0

/******************************************************************/
/******* THE FUNCTIONS BELOW ARE CURRENTLY UNUSED BY ATP! *********/
/******************************************************************/
/******* THE FUNCTIONS BELOW ARE CURRENTLY UNUSED BY ATP! *********/
/******************************************************************/
/******* THE FUNCTIONS BELOW ARE CURRENTLY UNUSED BY ATP! *********/
/******************************************************************/

#if defined(NEEDREADLINE)

#ifndef UNIXCMDS
#define STOPIT (int)'\r'
#else
#define STOPIT (int)'\n'
#endif
/*
 **readline - display prompt and retrieve user's response.
 */
char *
readline(char *prmt, int scrollflag)
{

    static char rdlnbuf[128];
    extern char *luxptr;
    char *buf, *tempr;
    int cin = 0, idx = 0, i, firsthere = TRUE;

    buf = (char *) malloc(128);	/* inialize buffers */
    rdlnbuf[0] = buf[0] = 0;

    printf("%s", prmt);		/* display prompt */
    if (luxptr != NULL) {
	strcpy(rdlnbuf, luxptr);
	printf("%s\n", luxptr);
	up(1);
	right(14);
	buf[0] = 1;
    }
    fflush(stdout);
    while ((cin = getch()) != STOPIT) {
	if (firsthere) {
	    for (i = idx; i < field_len; i++)
		printf(" ");	/* clear line */
	    for (i = idx; i < field_len; i++)
		printf("\b");
	    buf[0] = 1;
	    firsthere = FALSE;
	}
	if (cin > 31) {
	    printf("%c", cin);
	    buf[idx] = (char) cin;
	    idx++;
	    buf[idx] = 0;
	    if (idx > 24) {
		idx--;
		printf("\b");
	    };
	} else if (cin == '\b' && idx > 0) {	/* do backspace */
	    printf("\b \b");
	    buf[--idx] = 0;
	} else if (cin == 0) {
	    buf[idx] = 0;
#ifdef __TURBOC__		/* check for special keys */
	    tempr = NULL;
	    if (luxptr == NULL) {
		switch (cin = getch()) {
		case 0x3b:
		    tempr = "help";
		    break;
		case 0x3c:
		    tempr = "tag help";
		    break;
		case 0x3d:
		    tempr = "tag list";
		    break;
		case 0x3e:
		    tempr = "qlist";
		    break;
		case 0x49:
		    tempr = "-";
		    break;
		case 0x51:
		    tempr = "+";
		    break;
		case 0x4f:
		    tempr = "last";
		    break;
		case 0x47:
		    tempr = "1";
		    break;
		case 0x3f:
		    tempr = "show terms";
		    break;
		case 0x4c:
		    tempr = "N";
		    break;
		default:
		    break;
		}
		if (tempr != NULL) {
		    strcpy(buf, tempr);
		    printf("\n");
		    return buf;
		}
	    }
#endif
	} else {
	    buf[idx] = 0;
	}
	fflush(stdout);
    }
    printf("\n");
    if (buf[0] == 1)
	strcpy(buf, rdlnbuf);
    return buf;

};

void				/* dummy function for compatibility */
rl_initialize(void)
{
}

#endif

#ifdef NEEDFIXPATH
void
fixpath(char *path)
{

    char *tptr;

    tptr = path;
    while (*tptr) {
	if (*tptr == '/')
	    *tptr = '\\';
	tptr++;
    }
}
#endif

#if defined(NEEDTEMPNAM)
char *
mytempnam(const char *workpath, const char *rep)
{

    char *tp1, *tp2;
    static char tbuf[MAXPATHS];
    char buffy[15];

    strcpy(buffy, "atpXXXXXX");
    tp2 = tbuf;
    tp1 = mktemp(buffy);
    if (tp1 != NULL) {
	strcpy(tbuf, workpath);
	while (*tp2)
	    tp2++;
	tp2--;
	if (*tp2 != SEP) {
	    tp2++;
	    *tp2 = SEP;
	    tp2++;
	    *tp2 = NUL_CHAR;
	}
	strcat(tbuf, rep);
	strcat(tbuf, tp1);
	tp1 = tbuf;
    }
    return tp1;
}
#endif

#if defined(NEEDSYSTEM)
#ifdef UNIXCMDS
#define ATPSWC "-c"
#else
#define ATPSWC "/C"
#endif

int
mysystem(char *command)
{
    pid_t mpid, npid;
    int statloc;
    char *shelbuf;

    if ((shelbuf = malloc(MAXPATHS)) == NULL)
	return (0);
    shelbuf = getenv(SHELL);

    mpid = fork();

    if (mpid == 0) {		/* child */
	execl(shelbuf, shelbuf, ATPSWC, command, NULL);
	free(shelbuf);
	abort();
    } else if (mpid < 0) {
	printf("Unable to fork %s\n", command);
	free(shelbuf);
	return (0);
    } else {			/* parent */
      wmore:
	npid = wait(&statloc);
	if (npid != mpid)
	    goto wmore;
    }
    free(shelbuf);
    return (1);
}

#endif
#endif				/* UNUSED BY ATP */
