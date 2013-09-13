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
qlib.h
*/

#ifndef _ATP_QLIB_H
#define _ATP_QLIB_H 1

#include "atptypes.h"

/* contents of qwk packets */
#define MSG_FILE    "messages.dat" /* Message filename prepared by Qmail */
#define CNTRL_FILE  "control.dat"  /* List of conferences by Qmail       */
#define DOORID      "door.id"	   /* Info for the BBS QWK door 	 */
#define NEWFILES    "newfiles.dat" /* List of new files   by Qmail       */
#define WELCOME     "welcome"      /* 1st Screen of te BBS               */
#define NEWS        "news"         /* news file, ascii mode              */
#define TAGFILE     "taglines.atp" /* taglines file                      */
#define PERS_CONF   9000           /* Conference number for personal mail*/
#define REPL_CONF   9001           /* Conference number for replies      */
#define PCONF_IDX   LastConf
#define RCONF_IDX   (LastConf-1)
#define t_PCONF_IDX   t_LastConf
#define t_RCONF_IDX   (t_LastConf-1)
#define VIRGINDX    -1L            /* Index.LastRead flag for virgin index */

/* message header constants */
#define BLKSIZE   128
#define HDRSIZE   128
#define HStatus     0
#define HNumMsg     1 
#define HConfNum    1 
#define HMsgDate    8 
#define HMsgTime   16
#define HForWhom   21
#define HAuthor    46
#define HSubject   71
#define HPassWrd   96
#define HRefMsg   108 
#define HSizeMsg  116
#define HMsgActiv 122
#define HBinConfN 123
#define HUnused1  125
#define HUnused2  126
#define HNetTag   127

#include "editline/edpublic.h" 

#endif /* qlib.h */

/* unused below */
#if 0 
/* note that these structures are for reference purposes only */
struct MsgHeaderType {		/* received message header structure */
    byte Status,		/*                                   */
     NumMsg[7],			/* Numero du message,envoi = conf !  */
     MsgDate[8],		/* mm-dd-yy                          */
     MsgTime[5],		/* HH:MM                             */
     ForWhom[25],		/* Destinataire                      */
     Author[25],		/* Nous mme...                     */
     Subject[25],		/*                                   */
     PassWord[12],		/* Si sender ou group password       */
     RefMsg[8],			/* Message rfrenc              */
     SizeMsg[6],		/* en ascii, nb blocs de 128 bytes   */
     MsgActive,			/* 0xE1 = active  0xE2 = inactive    */
     BinConfN[2],		/* 16 bit unsigned binary word       */
     Unused1,			/* space                             */
     Unused2,			/* space                             */
     NetTag;			/* space                             */
};

struct QmailRepType {		/* send message header structure i   */
    byte Status;		/* '+' = private  ' ' = public       */
    byte ConfNum[7];		/* Numero de la conference concerne  */
    byte MsgDate[13];		/* mm-dd-yyHH:MM                     */
    byte ForWhom[25];		/* Destinataire                      */
    byte Author[25];		/* Nous mme...                     */
    byte Subject[25];		/*                                   */
    byte PassWord[12];		/* Si sender ou group password       */
    byte RefMsg[8];		/* Message rfrenc              */
    byte SizeMsg[6];		/* en ascii, nb blocs de 128 bytes   */
    byte MsgActive;		/* 0xE1 = active                     */
    byte BinConfN[2];		/* 16 bit conference number binary.  */
    byte Unused1;		/* space                             */
    byte Unused2;		/* space                             */
    byte NetTag;		/* space                             */
};
#endif
