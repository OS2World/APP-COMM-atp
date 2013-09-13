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
reader.h  
*/

#ifndef _ATP_READER_H
#define _ATP_READER_H 1

#include "globals.h"
#include "atptypes.h"

#if defined(HAVE_TERMCAP_H)
#ifndef HAVE_LIBTERMCAP
#define HAVE_LIBTERMCAP
#endif
#endif

#define PCBLONG (pcbext)

/* various constants */
#define SEPDOS '\\'
#define EOS     0  /* End of string */
#define DEL_CHAR '\177'
#define SPC_CHAR '\040'
#define CR_CHAR  '\015'
#define LF_CHAR  '\012'
#define TAB_CHAR '\011'
#define BS_CHAR  '\010'
#define NUL_CHAR '\0'
#define HIGH_ASCII (byte)0x80
#define LF 0x0A
#define CR 0x0D
#define SUB_CHAR 0x1A
#define CNTRL_Z SUB_CHAR
#define DOS_SPACE (byte)0xFF
#define ASTERISK '*' 
#define PRIVATE_MSG ASTERISK
#define PUBLIC_MSG SPC_CHAR
#define ACTIVE_MSG (byte)0xE1
#define INACTIVE_MSG (byte)0xE2
#define QWK_LINE_FEED (byte)0xE3
#define QWK_FIELD_LEN 25

#define MSG_EXT     ".msg"         /* Extension of reply file            */
#define CNF_EXT     ".cnf"         /* Extension of conference file       */
#define IDX_EXT     ".idx"         /* Extension of index files           */

#define BEEP '\07'
#define BELL (silent? (char) SPC_CHAR :(char) BEEP )
#define IDXSIZE sizeof(struct MyIndex)
#define STRING_LEN(X) (sizeof(X)-1)
#define DATE_LEN 24
#define SHORT_SUBJ_LEN 25
#define LSUBJ_HEAD_LEN 10
#define LSUBJ_NAME_LEN 8
#define LSUBJ_BODY_LEN 54
#define LSUBJ_TAIL_LEN 8
#define LSUBJ_BUF_LEN  LSUBJ_HEAD_LEN + LSUBJ_BODY_LEN + LSUBJ_TAIL_LEN
#define LSUBJ_LEN 54
#define LSUBJ_TAIL "      N\n"
#define LSUBJ_HEAD " @SUBJECT:"

/* test for MS-DOS block graphic line drawing character */
#define vtspecial(c)  ((((c)>(byte)0xaf)&&((c)<(byte)0xe0)) ? 1 : 0 )

#define CONFIG_FILE "atprc"
#define WORK_DIR    "atpwork"
#define TAGFILE     "taglines.atp" /* taglines file                      */


/* various memory model configurations */
/* MS-DOS 16 bit */
#ifdef _ATP16_

/* large model */
#define MAXCONF     4000  /* Increase if you need more conferences         */
#define MYBUF   (size_t)12800U  /* MAX message len buffer received message */
#define MAXBUF  (size_t)61440U  /* Worst case buffer size 60K              */
#define MAXSEND (size_t)61440U  /* MAX message len buffer sent message     */
                           /* (40000 = about 600 lines with 65 chars/line) */
#else /* use huge model 32 bit */

/*  huge model  */
#define MAXCONF       8191        /* Can not exceed 8191 but could be less.  */
/* #ifndef no_proto */
#define MYBUF   (size_t)(256*128)    /* about 32K */
#define MAXBUF  (size_t)(MYBUF*8)    /* about 260K */
#define MAXSEND (size_t)  65408      /* MAX message len buffer sent message  */
/*                               
 *#else
 *#define MYBUF 32768L
 *#define MAXBUF 179200L
 *#define MAXSEND 65408L
 *#endif
 */ 
#endif  /* memory models */

#ifndef CLSCRN
#define CLSCRN cls
#endif 

/* read.c */
/* lower 2 bits used for display mode repeat constant */
#define RPT_MASK 0x03 
#define BBS_NM_LEN 74
#define NO_CHILD -1

/* text.c */
extern const char CONSPTR txt[];       /* Language-dependant text messages ( general)*/
extern const char CONSPTR taghlp[];    /* Language-dependant text messages ( tag help)*/
extern const char CONSPTR Months[];    /* Language-dependant text messages (Dates)*/
extern const char CONSPTR terms[];     /* Gnu copyleft message */

/* editline.c */
#include "editline/edpublic.h"


/* various macros. */

#if defined(__cplusplus)

ATP_INLINE void
cnf_path(char *targ, const int cnum)
{
    sprintf(targ, "%s%s%c%d%s", HomePath, CurBoard, SEP, cnum, CNF_EXT);
}

ATP_INLINE void
idx_path(char *targ, const int cnum)
{
    sprintf(targ, "%s%s%c%d%s", HomePath, CurBoard, SEP, cnum, IDX_EXT);
}

ATP_INLINE void
make_c_i_paths(char *C, char *I, const int N)
{
    sprintf(C, "%s%s%c%d%s", HomePath, CurBoard, SEP, N, CNF_EXT);
    sprintf(I, "%s%s%c%d%s", HomePath, CurBoard, SEP, N, IDX_EXT);
}

#else
#define cnf_path(targ,cnum) sprintf(targ, "%s%s%c%d%s", HomePath, CurBoard, SEP, (cnum), CNF_EXT);
#define idx_path(targ,cnum) sprintf(targ, "%s%s%c%d%s", HomePath, CurBoard, SEP, (cnum), IDX_EXT);
#define make_c_i_paths(C,I,N) \
  sprintf(C, "%s%s%c%d%s", HomePath, CurBoard, SEP, (N), CNF_EXT); \
  sprintf(I, "%s%s%c%d%s", HomePath, CurBoard, SEP, (N), IDX_EXT);

#endif

#endif /* _ATP_READER_H */

