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
atptypes.h
*/

#ifndef _ATPTYPES_H
#define _ATPTYPES_H 1

#undef TRUE
#undef FALSE

struct fentry {
    char *findpt;
    struct fentry *fnext;
};

struct MyIndex {

    long LastRead;		/* Last Message Read, Used once...            */
    long MaxMsg;		/* Max Messages allowed in conf. (future use) */
    long MsgNum;		/* Number of This message                     */
    long Offset;		/* Offset in the Conference file              */
    unsigned long Size;		/* Size of the message,with Header            */
};

/* the enum read_command numbers are not arbitrary in the lower 2 bits */
typedef enum {
    NEXT = 0, AGAIN, PREVIOUS, FIND = 4, SCAN = 8, QUICK = 12,
    KILL = 5, NUKE = 9, CROSS = 13, PRIVATE = 17
} read_command_t;

/* tokens for strings gathered from the atprc file */
typedef enum {
    usr1nm, editr, archivr, unarchvr, qwklst, bltlst, spellr
} atprc_str_t ;

/* character set modes */
typedef enum {
    CHR7BIT, ISOLAT1, CHRDOS
} atp_CODE_T;

typedef enum {
    ENTER, REPLY, CHANGE, XPOST
} reply_type_t ;

typedef enum {
    update_last_read_ptr, reset_last_read_ptr
} update_command_t;

/* 
 * The following enums are access constants to track who can
 * change certain variables. Note that these access constants are
 * arbitrary values; there is no particular significance to the
 * magic numbers other than the hope that they are unique and not
 * likely to occur accidentally i.e. not TRUE, FALSE, -1, 0, etc.
 */

typedef enum {
    pmPurge_D = -632, pmPurge_CT, pmMkIndex, pmLoad
} pmail_access_t ;

typedef enum {
    ss_getwinders = -773, ss_init_screen, ss_TTYinfo
} scrn_access_t ;

typedef enum {
    fd_ReadNext = 1129, fd_PurgeAdjust
} fdone_access_t;

typedef enum {
    scnf_GetConf = 1833
} scnf_access_t;

typedef enum {
    rexist_AddReply = -51, rexist_ActvConf, rexist_Chk4RepCnf
} rexist_access_t;

typedef enum {
    tm_PurgeAdjust = 16901, tm_AddReply
} totmsg_access_t;

typedef enum {
    ip_ReadSeek = -7021, ip_PurgeAdjust
} indxptr_access_t;

typedef enum {
    cb_RdCn_main_init = 226, cb_DoLoad_MkIndex, cb_Read
} curbrd_access_t ;

/* end of access constants */


/* arbitrary and constants used by OpenRepFile() */
typedef enum {
    add_reply = 1492, pack_them = 1609
} pakrep_t;

typedef enum {
    wm_hello, wm_news, wm_goodbye, wm_newfiles, wm_door_id, wm_bulletin
} welcome_msg_t;

typedef enum {
    usrnm, boardnm
} cntrl_str_t ;

#ifndef BYTE_DECL
typedef unsigned char byte;
#define BYTE_DECL
#endif

#ifndef CNF_NAM_SIZ
typedef char conf_name[16];
#define CNF_NAM_SIZ sizeof(conf_name)
#endif

#ifdef __LCLINT__
typedef bool atp_BOOL_T ;
#else
typedef enum { FALSE = 0 , TRUE = 1 } atp_BOOL_T;
#endif

typedef enum { ATP_ERROR = -1, ATP_OK = 0 } atp_ERROR_T;

#define YES TRUE
#define NO FALSE

#define SUCCESS  0 
#define FAILURE -1

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

#endif
