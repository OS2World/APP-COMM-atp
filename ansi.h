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
ansi.h
*/

#ifndef _ATP_ANSI_H
#define _ATP_ANSI_H 1

#if defined(HAVE_TERMCAP_H)
#  include <termcap.h>
#elif defined(SYS_UNIX)

#if !defined(EXTERNC)
# if defined(__cplusplus)
#   define EXTERNC extern "C"
# else 
#   define EXTERNC extern
# endif
#endif

extern char PC;
extern char *UP;
extern char *BC;
extern short ospeed;
EXTERNC int tgetnum (const char *name);
EXTERNC int tgetflag (const char *name);
EXTERNC char *tgetstr (const char *name, char **area);
EXTERNC int tgetent (char *buffer, const char *trment);
EXTERNC char *tgoto (const char *cstring, int hpos, int vpos);
EXTERNC void tputs (const char *string, int nlines, int (*outfun)(int));
EXTERNC char *tparam (const char *ctlstring, char *buffer, int size, ...);
#endif

/* termcap pointers in editline.c */
#if 0
/* not extern for this module */
extern char *backspace ;
extern char *home ;
#endif
extern char *blink_on  ;
extern char *bold_on ; 
extern char *clear_attr ;
extern char *clear_scr ;
extern char *cur_right ;
extern char *del_eol;
extern char *rev_on    ;
extern char *rev_off   ;

/***** externals in ansi.c *******/
#ifndef _ATP_READLIB_H
/* attributes used by atp */
extern  void blink(void );
extern  void clear(void );
extern  void cls(void );
extern  void deleol(void );
extern  void high(void );


/* colors used in atp */
extern  void blue(void );
extern  void bblack(void );
extern  void cyan(void );
extern  void green(void );
extern  void magenta(void );
extern  void red(void );
extern  void white(void );
extern  void yellow(void );

extern atp_BOOL_T graph_togl(atp_BOOL_T);
extern void sig_init( void );
extern void sig_chwin( void );
extern void sig_chint( void );
extern void sig_stopfind( void );
extern void sig_ignore( int signo );

extern int    tputs_cat ( char *bfr, char *attr );
extern char * tputs_ptr ( char *attr );
extern int    tputs_cpy ( char *bfr, char *attr );
extern char * tputs_dup ( char *attr );
#endif

/* define prompts and ansi sequences for prompts */

#ifndef SRMPT
#define	SRMPT  "%s [ %s ] > "  /* format string for simple prompt  */
#define PFROM  "      From: " 
#define PRTO   "        To: "
#define PSUBJ  "   Subject: "
#define PSECR  "  Security: "
#define PTAIL  "[  ]        "
#define A_BLINK  "\33[5m"
#define A_CLEAR  "\33[0m"
#define A_GREEN  "\33[32m"
#define A_HIGH   "\33[1m"
#define A_WHIT   "\33[37m"
#define A_REV_ON "\33[7m"
#define A_REV_OFF "\33[m"
#define VT220_REV_OFF "\33[27m"
#define LEFT35 "\33[35D"
#define RGHT25 "\33[25C"
#define LEFT57 "\33[57D"
#define RGHT54 "\33[54C"
#define PRVERR "\155\145\162\144\145"
#define PRTAIL "[ " RGHT25 " ]        " LEFT35 A_WHIT  
#define LRTAIL "[ " RGHT54 " ] " LEFT57 A_WHIT  

#ifdef no_proto
#define PRMT1  "\33[32m\33[1m%s [ %s ] > "   
#define PRMT2  "\33[32m\33[1m%s [ \33[5m%s\33[0m\33[32m\33[1m ] > " 
#define PRMTTO  "\33[32m        To: [ \33[25C ]        \33[35D\33[37m"
#define PRMTFR  "\33[32m      From: [ \33[25C ]        \33[35D\33[37m"
#define PRMTSB  "\33[32m   Subject: [ \33[25C ]        \33[35D\33[37m"
#define PRMTSL  "\33[32m   Subject: [ \33[54C ] \33[57D\33[37m"
#define PRMTSC  "\33[32m  Security: [ \33[25C ]        \33[35D\33[37m"
#else /* ANSI C compiler */
#define PRMT1  A_GREEN A_HIGH SRMPT
#define PRMT2  A_GREEN A_HIGH "%s [ " A_BLINK "%s" A_CLEAR A_GREEN A_HIGH " ] > "
#define PRMTFR A_GREEN PFROM PRTAIL   
#define PRMTTO A_GREEN PRTO  PRTAIL   
#define PRMTSB A_GREEN PSUBJ PRTAIL
#define PRMTSL A_GREEN PSUBJ LRTAIL
#define PRMTSC A_GREEN PSECR PRTAIL 
#endif

#define SRMTFR "      From: "
#define SRMTTO "        To: "
#define SRMTSB "   Subject: "
#define SRMTSC "  Security: " 

#endif


/* not used in atp */
#if 0
extern  void reverse(void );
extern  void black(void );
extern  void bred(void );
extern  void bgreen(void );
extern  void byellow(void );
extern  void bblue(void );
extern  void bmagenta(void );
extern  void bcyan(void );
extern  void bwhite(void );
extern  void locate(int x,int y);
extern  void up(int nb);
extern  void dn(int nb);
extern  void right(int nb);
extern  void left(int nb);
#endif

#endif /* _ATP_ANSI_H */
