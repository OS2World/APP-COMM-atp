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
ansi.c
*/


#include <stdio.h>
#include <string.h>
#include "reader.h"
#include "readlib.h"
#include "ansi.h"

/* define the "escape" character */

#undef ESC
static const char ESC = (char)27 ;

#ifdef HAVE_LIBTERMCAP
#if 0 && defined(NEED_TCVARS)  /* not needed at present */
char PC, *UP, *BC;
short ospeed = 0;
/* this is a place holder for alignment purposes only */
short atp_tc_dumb = 0;
#endif

/* 
 * outc, wrapper for use by tputs().
 */
static int
outc(int c)
{
    int r;
    r = putchar(c);
    return r;
}

#define tc_out(s) tputs( s, 1, outc )
#endif

/*
 * graph_togl - visually check if display supports VT102 line graphics.
 */
atp_BOOL_T
graph_togl(atp_BOOL_T flag)
{
    if (flag) {
	flag = FALSE;
	printf("\n\t\t +------------------------------+\n");
	printf("\t\t |                              |\n");
	printf("\t\t |  VT102 graphics are now OFF  |\n");
	printf("\t\t |                              |\n");
	printf("\t\t +------------------------------+\n\n");
    } else {
	flag = TRUE;
	printf("\n\016\t\t lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk\n");
	printf("\t\t xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax\n");
	printf("\t\t xaa\017 VT102 graphics are now ON\016 ax\n");
	printf("\t\t xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax\n");
	printf("\t\t mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj\n\n\017");
    }
    return flag;
}

/* termcap support provided for cls(), clear(), high(), blink(), reverse() */

/*
 * cls - clear screen and home cursor.
 */
void
cls(void)
{
    if (ansi)
	printf("%c[2J%c[H", ESC, ESC);
#ifdef HAVE_LIBTERMCAP
    else if (clear_scr)
	tc_out(clear_scr);
    /* tputs (clear_scr, 1, putchar ) ; */
#endif
#if defined(DJGPP) || defined(__TURBOC__)
    else {
	gotoxy(1, 1);
	clrscr();
    }
#endif
}

/*
 * clear - reset ansi screen color and highlighting attributes.
 */
void
clear(void)
{
    if (ansi)
	printf("%c[0m", ESC);
#ifdef HAVE_LIBTERMCAP
    else if (clear_attr)
	tc_out(clear_attr);
#endif
}

/*
 * high - ansi highlighting/boldface.
 */
void
high(void)
{				/* HigLitht  (or BoldFace )  */
    if (ansi)
	printf("%c[1m", ESC);
#ifdef HAVE_LIBTERMCAP
    else if (bold_on)
	tc_out(bold_on);
#endif
}

/*
 * blink - invoke ansi blink mode.
 */
void
blink(void)
{
    if (ansi)
	printf("%c[5m", ESC);
#ifdef HAVE_LIBTERMCAP
    else if (blink_on)
	tc_out(blink_on);
#endif
}

/* 
 * deleol - ansi del to end of line.
 */
void
deleol(void)
{
    if (ansi)
	printf("%c[K", ESC);
#ifdef HAVE_LIBTERMCAP
    else if (del_eol)
	tc_out(del_eol);
#endif
#if defined(DJGPP) || defined(__TURBOC__)
    else
	clreol();
#endif
}

#ifdef ATPUNUSED
void
reverse()
{				/* Reverse video mode         */
    if (ansi)
	printf("%c[7m", ESC);
#ifdef HAVE_LIBTERMCAP
    else if (rev_on)
	tc_out(rev_on);
#endif
}

/*------------- Foreground colors ---------------------*/
void
black()
{
    if (color && ansi)
	printf("%c[30m", ESC);
}
#endif
/*
 * red - ansi color.
 */
void
red(void)
{
    if (color && ansi)
	printf("%c[31m", ESC);
}

/*
 * green - ansi color.
 */
void
green(void)
{
    if (color && ansi)
	printf("%c[32m", ESC);
}

/*
 * yellow - ansi color.
 */
void
yellow(void)
{
    if (color && ansi)
	printf("%c[33m", ESC);
}

/*
 * blue - ansi color.
 */
void
blue(void)
{
    if (color && ansi)
	printf("%c[34m", ESC);
}

/*
 * magenta - ansi color.
 */
void
magenta(void)
{
    if (color && ansi)
	printf("%c[35m", ESC);
}

/*
 * cyan - ansi color.
 */
void
cyan(void)
{
    if (color && ansi)
	printf("%c[36m", ESC);
}

/*
 * white - ansi color.
 */
void
white()
{
    if (color && ansi)
	printf("%c[37m", ESC);
}

/*------------- BackGround colors ---------------------*/

/*
 * black - ansi background color.
 */
void
bblack()
{
    if (color && ansi)
	printf("%c[40m", ESC);
}

#ifdef ATPUNUSED
void
bred()
{
    if (color && ansi)
	printf("%c[41m", ESC);
}
void
bgreen()
{
    if (color && ansi)
	printf("%c[42m", ESC);
}
void
byellow()
{
    if (color && ansi)
	printf("%c[43m", ESC);
}
void
bblue()
{
    if (color && ansi)
	printf("%c[44m", ESC);
}
void
bmagenta()
{
    if (color && ansi)
	printf("%c[45m", ESC);
}
void
bcyan()
{
    if (color && ansi)
	printf("%c[46m", ESC);
}
void
bwhite()
{
    if (color && ansi)
	printf("%c[47m", ESC);
}
#endif

#ifdef ATPUNUSED
/*----------- Cursor position ---------------------------*/

void
locate(x, y)
int x, y;
{
    if (ansi)
	printf("%c[%d;%dH", ESC, y, x);
}

void
up(nb)				/* Cursor Up default 1 */
int nb;
{
    if (ansi) {
	if (!nb)
	    nb = 1;
	printf("%c[%dA", ESC, nb);
    }
}

void
dn(nb)				/* Cursor down default 1 */
int nb;
{
    if (ansi) {
	if (!nb)
	    nb = 1;
	printf("%c[%dB", ESC, nb);
    }
}

void
right(nb)			/* Cursor right default 1 */
int nb;
{
    if (ansi) {
	if (!nb)
	    nb = 1;
	printf("%c[%dC", ESC, nb);
    }
}

void
left(nb)			/* Cursor left default 1 */
int nb;
{
    if (ansi) {
	if (!nb)
	    nb = 1;
	printf("%c[%dD", ESC, nb);
    }
}
#endif

#ifdef HAVE_LIBTERMCAP

#define o2bSIZE 200
static int o2b_ct;
static char *o2b_p;
static char o2b_bf[o2bSIZE + 1];

/* 
 * out_2_buf, expand termcap attribute in buffer.
 * passed to tputs() to expand a termcap attribute string into a buffer
 */
static int
out_2_buf(int n)
{
    if (o2bSIZE <= o2b_ct) {
	o2b_bf[o2bSIZE] = NUL_CHAR;
	n = EOF;
    } else {
	*o2b_p = (char) n;
	o2b_ct++;
	o2b_p++;
	*o2b_p = NUL_CHAR;
    }
    return n;
}

/* 
 * tputs_ptr - returns a pointer to the expanded attribute.
 */
char *
tputs_ptr(char *attr)
{
    o2b_bf[0] = NUL_CHAR;
    o2b_p = o2b_bf;
    o2b_ct = 0;
    /*@-strictops */
    if (attr != NULL && *attr != NUL_CHAR ) /*@=strictops */
	tputs(attr, 1, out_2_buf);
    return (char *) o2b_bf;
}

/*
 * tputs_cpy - copies a termcap attribute to a string buffer.
 */
int
tputs_cpy(char *bfr, char *attr)
{
    strcpy(bfr, tputs_ptr(attr));
    return strlen(o2b_bf);
}

/*
 * tputs_cat - strcats a termcap attribute to a string buffer.
 */
int
tputs_cat(char *bfr, char *attr)
{
    strcat(bfr, tputs_ptr(attr));
    return strlen(o2b_bf);
}

/* tputs_dup not currently used in ATP */
#if 0
/*
** tputs_dup - strdups a termcap attribute.
 */
char *
tputs_dup(char *attr)
{
    char *t;
    t = strdup(tputs_ptr(attr));
    return t;
}
#endif
#endif

/*-----------------------------------------------------------------------*/
/*         Below we have the signal related functions.                   */



#ifdef HAVE_SIGACTION
static struct sigaction sdefault, signore, stpfind;
#if defined(USE_FORK)
static struct sigaction new_sinter; 
#endif
#if defined(TIOCGWINSZ) && defined(SIGWINCH)
static struct sigaction new_swinch; 
#endif
#endif

#if defined(TIOCGWINSZ) && defined(SIGWINCH)

/* 
 * sig_chwin - set-up handler for SIGWINCH.
 */
void
sig_chwin(void)
{
#if defined(HAVE_SIGACTION)
    sigaction(SIGWINCH, &new_swinch, NULL);
#else
    signal(SIGWINCH, getwinders);
#endif
}

#endif				/* TIOCGWINSZ */



/* 
 * sig_init - initialize sigaction structures.
 */
#ifdef HAVE_SIGACTION
void
sig_init(void)
{
    sigset_t sigst;
    sigemptyset(&sigst);
#if defined(TIOCGWINSZ) && defined(SIGWINCH)
    new_swinch.sa_handler = getwinders;
    new_swinch.sa_mask = sigst;
    new_swinch.sa_flags = 0;
#endif
#if defined(USE_FORK)
    new_sinter.sa_handler = kill_child;
    new_sinter.sa_mask = sigst;
    new_sinter.sa_flags = 0;
#endif
    sdefault.sa_handler = SIG_DFL;
    signore.sa_handler = SIG_IGN;
    stpfind.sa_handler = stopfind;
    sdefault.sa_mask = signore.sa_mask = stpfind.sa_mask = sigst;
    sdefault.sa_flags = signore.sa_flags = stpfind.sa_flags = 0;
}

#endif				/* HAVE_SIGACTION */



#if defined(USE_FORK) && defined(SIGINT)

/* 
 * sig_chint - set-up handler for SIGINT.
 */
void
sig_chint(void)
{
#if defined(HAVE_SIGACTION)
    sigaction(SIGINT, &new_sinter, NULL);
#else
    signal(SIGINT, kill_child);
#endif
}

#endif


/* 
 * sig_stopfind - set-up SIGINT handler for findtxt().
 */
void
sig_stopfind(void)
{
#if defined(SIGINT)
#if defined(HAVE_SIGACTION)
    sigaction(SIGINT, &stpfind, NULL);
#else
    signal(SIGINT, stopfind);
#endif
#endif
}


#if 0
void
sig_default(int signo)
{
#ifdef HAVE_SIGACTION
    sigaction(signo, &sdefault, NULL);
#else
    signal(signo, SIG_DFL);
#endif
}
#endif

/*
 * sig_ignore - ignore a signal.
 */
void
sig_ignore(int signo)
{
#ifdef HAVE_SIGACTION
    sigaction(signo, &signore, NULL);
#else
    signal(signo, SIG_IGN);
#endif
}

/*-----------------------------------------------------------------------*/
/*-- ALL CODE BELOW IS UNUSED BY ATP ------------------------------------*/

#if 0 && defined(LISTDIR)
#include "editline/editline.h"
extern char *Screen;
extern size_t ScreenSize;

static void
list_dir(char *path)
{
    char **av;
    int ac = 5;

    ac = rl_list_possib((char *) path, (char ***) &av);
    if (ac) {
	ScreenSize = SCREEN_INC;
	Screen = NEW(char, ScreenSize);
	columns(ac, av);
	TTYflush();
	DISPOSE(Screen);
	sleep(3);
	while (--ac >= 0)
	    free(av[ac]);
	free(av);
    }
}
#endif
