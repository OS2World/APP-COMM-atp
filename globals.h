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
globals.h
*/

#ifndef _ATP_GLOBALS_H
#define _ATP_GLOBALS_H 1

#include "system.h"

/* global variables */

extern FILE *fidx ;
extern FILE *fmsg ;
extern char *rbuf ;

/* scalar constants */
extern const size_t mybuf;
extern const size_t maxbuf; 
extern const size_t one_record;
extern const size_t field_len;
extern const size_t header_SIZE;
extern const size_t block_SIZE;
extern const size_t index_SIZE;

/* global path data */
extern const char CONSPTR HomePath;
extern const char CONSPTR MailPath;
extern const char CONSPTR ReplyPath;
extern const char CONSPTR WorkPath;
extern const char CONSPTR CurBoard;
extern const char CONSPTR atp_termtype;
extern const char *luxptr;           

#if defined(_ATP_RDCONFIG_C)
#define CONSTBOOL extern
#else
#define CONSTBOOL extern const
#endif

/* boolean flag variables set from atprc */
CONSTBOOL atp_BOOL_T autotag;   /* flag for random taglines             */
CONSTBOOL atp_BOOL_T graphics;  /* flag for VT102 line graphics mode    */
CONSTBOOL atp_BOOL_T ansi;      /* flag true if ansi output is on       */
CONSTBOOL atp_BOOL_T silent;    /* flag for no tty bell                 */
CONSTBOOL atp_BOOL_T fido;      /* flag for tagline style               */
CONSTBOOL atp_BOOL_T color;     /* flag true if ansi output is on       */
CONSTBOOL atp_BOOL_T pcbext;    /* flag true to display long PCB subjs  */

#if defined(_ATP_QLIB_C) 
# define QLIB_CONST extern
# define QLCPTR *
#else
# define QLIB_CONST extern const 
# define QLCPTR CONSPTR
#endif

QLIB_CONST int LastConf ;
QLIB_CONST conf_name  QLCPTR ConfNames ;
QLIB_CONST int        QLCPTR ConfNumbers /* [MAXCONF] */;
QLIB_CONST atp_BOOL_T QLCPTR ConfActive /* [MAXCONF] */; 

#endif
