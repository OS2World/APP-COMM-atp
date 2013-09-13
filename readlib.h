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
readlib.h
*/
   
#ifndef _ATP_READLIB_H
#define _ATP_READLIB_H 1

#include "atptypes.h"

#define SCREEN_ROWS 24 
#define SCREEN_COLS 80
#define FIND_BUF_SIZE 255

#ifndef __CEXTRACT__
/* ansi.c */
extern void bblack ( void );
extern void blink ( void );
extern void blue ( void );
extern void clear ( void );
extern void cls ( void );
extern void cyan ( void );
extern void deleol ( void );
extern atp_BOOL_T graph_togl ( atp_BOOL_T flag );
extern void green ( void );
extern void high ( void );
extern void magenta ( void );
extern void red ( void );
extern void sig_chint( void );
extern void sig_chwin( void );
extern void sig_ignore ( int signo );
extern void sig_init ( void );
extern void sig_stopfind ( void );
extern int tputs_cat ( char *bfr, char *attr );
extern int tputs_cpy ( char *bfr, char *attr );
extern char * tputs_ptr ( char *attr );
extern void white ( void );
extern void yellow ( void );

/* atpmain.c */
/* extern int main ( int argc, char *argv[] );*/
extern void error_switch_case(int e_mode, const char *e_file, int e_line);
extern atp_BOOL_T ReadNext ( const read_command_t mode );

/* chosetag.c */
extern void ChooseTag ( void );
extern /*@only@*/ byte * get_CurTag ( void );
extern atp_BOOL_T get_tag_flag ( void );
extern void init_tagpers ( const char *value, const char *sign, const char *aLine );
extern void init_tagstyle ( const char *value );
extern void Tag ( const char *line );
extern void tag_set_edit_reply_mode(const atp_BOOL_T er_mode);
extern void tag_set_by_main ( void );
extern void togltag ( void );

/* display.c */
extern void Check4LongSubj ( void );
extern void Display ( const read_command_t mode, struct fentry *bptr, const size_t blen );
extern char * get_long_subj ( void );
extern char * get_sot ( void );
extern void PrintHeader ( FILE * fp );
extern void PutHeader ( const read_command_t mode, struct fentry **sptr, size_t blen );
extern void rot_toggle ( void );
extern atp_BOOL_T subj_is_long ( void );

/* makemail.c */
extern atp_BOOL_T Cnf2Msg ( const char *filemsg );
extern atp_ERROR_T KodeMessage ( const char *fname, char *Qmail );
extern atp_BOOL_T OpenRepFile ( const pakrep_t mode );

/* mkindex.c */
extern atp_ERROR_T MkIndex ( const char *SrcDir, const char *DestDir );

/* purge.c */
extern void Purge ( void );

/* qlib.c */ 
extern void test_fatal_malloc(void *ptr, const char *fname, int line_num);
extern int findCindex ( const int n );
extern void free_buffer (/*@only@*/ void *s, size_t len );
extern void free_string (/*@only@*/ char *s ); 
extern void do_unlink ( const char *fname );
extern char * get_cntrl_str ( const cntrl_str_t token );
extern int get_ActvCnt (void);
extern void ActvConf ( void );
extern void Chk4RepCnf ( const char *tpath );
extern void ListConf ( void );
extern void MoveWork ( const char *DestDir, const char *SrcDir );
extern atp_ERROR_T OpenCon ( FILE ** oc_pf, FILE ** oc_fs, const char *dspath );
extern atp_ERROR_T RdCn_ReadControl ( const char *Path );
extern void RdCn_exit_free ( void );
extern atp_BOOL_T RdCn_main_init ( void );
extern int readCnum ( const char *ptr );
extern void StripLSpace ( char *ptr );
extern atp_ERROR_T verify_new_file (char /*@out@*/ *Src, const char *SrDir, const char *pilgrim );
extern void WelcomeMsg ( const welcome_msg_t msg_name, const char *mfile );
extern atp_ERROR_T WriteIndex ( FILE *wi_fx, const long wi_count, const unsigned long wi_Size, const long wi_Offset );

/* rdconfig.c */
extern void ansi_toggle ( void );
extern void Clean ( void );
extern char * get_atprc_str ( const atprc_str_t token );
extern atp_BOOL_T get_caps ( void );
extern atp_CODE_T get_charset ( void );
extern atp_BOOL_T get_HeadLetter ( void );
extern atp_BOOL_T get_pmail ( void );
extern RETSIGTYPE getwinders ( int dummy_arg );
extern size_t get_RbufRecs ( void );
extern size_t get_RbufSize ( void );
extern int get_ScrnCols ( void );
extern int get_ScrnRows ( void );
extern long get_TruncNum ( void );
extern void head_toggle ( void );
extern void init_term_type ( void );
extern atp_BOOL_T MakeHomePath ( void );
extern atp_ERROR_T ReadConfig ( void );
extern atp_BOOL_T reup (const size_t bfsz );
extern void set_caps (const atp_BOOL_T val );
extern void set_CurBoard ( const char *str, const curbrd_access_t perm );
extern void set_pmail ( const atp_BOOL_T mode, const pmail_access_t perm );
extern void set_ScrnCols ( const int siz, const scrn_access_t perm );
extern void set_ScrnRows ( const int siz, const scrn_access_t perm );
extern void test_for_caps ( const char *Header );
extern void test_line_graphics ( void );
extern void toglauto ( void );
extern void toglfido ( void );
extern void toglpcb ( void );

/* read.c */
extern void AutoJoin ( void );
extern void change_msg ( const char *tmp );
extern void chconf ( const char *buf );
extern atp_BOOL_T Chk4RepPkt ( void );
extern void chkerror ( void );
extern void Date ( void );
extern void enter_msg ( char *tmp );
extern int get_CurConf ( void );
extern atp_BOOL_T get_isempty ( void );
extern void Load ( const char *name );
extern void mark_private ( void );
extern atp_BOOL_T more ( atp_BOOL_T def );
extern atp_BOOL_T Numeric ( const char *str );
extern void Read ( const char *name );
extern void reply_msg ( const char *tmp );
extern void set_CurConf ( const int num, const scnf_access_t perm );
extern void show_again ( void );
extern void str2mem ( char *mem, const char *str );
extern void UpdateConf ( const update_command_t mode );
extern void xpost_msg ( const char *tmp );
extern atp_BOOL_T YesNo ( atp_BOOL_T def, const char *prmt );

/* readlib.c */
extern void EmptyMsg ( void );
extern void fclose_fmsg_fidx ( void );
extern atp_BOOL_T fget ( char *s, const int n, FILE * fp );
extern struct fentry * findstr ( byte * s1, byte * s2, struct fentry **sptr );
extern void findtxt ( char *fndargs, const read_command_t mode );
extern atp_ERROR_T GetConf ( const int num );
extern atp_BOOL_T get_FilesOpen ( void );
extern atp_BOOL_T get_FirstDone ( void );
extern long get_MsgLastRead ( void );
extern long get_MsgNum ( void );
extern long get_MsgOffset ( void );
extern unsigned long get_MsgSize ( void );
extern struct MyIndex * get_ptrIndex ( const indxptr_access_t perm );
extern long get_TotMsg ( void );
extern void GoToNum ( const char *str );
extern struct fentry * hprint ( char *hbuf, size_t offst, size_t len, struct fentry **bptr, const size_t blen );
extern void query_rep_exists ( void );
extern atp_BOOL_T SeekNum ( const char *str );
extern void set_FirstDone ( const atp_BOOL_T mode, const fdone_access_t perm );
extern void set_ReplyExist ( const atp_BOOL_T mode, const rexist_access_t perm );
extern void set_TotMsg (const long val, const totmsg_access_t perm );
extern void ShiftLeft ( char *p1, const int n );
extern void stopfind ( int dummy_arg );
extern void StripDel ( char *tp );
extern void view ( const char *Path, const char *File );
extern void zero_Index ( const indxptr_access_t perm );

/* reply.c */
extern char * get_reply_lsubj ( void );
extern int get_saved_conf ( void );
extern void Reply ( const reply_type_t mode, const char *line );

/* system.c */
extern void Erase ( const char *PathName );
atp_BOOL_T get_reperror( void );
extern void reset_reperror( void );
extern RETSIGTYPE kill_child( int dummy_arg );
extern void fork_execvp ( const char *prg, const char *prgfile );
extern char * strlwr ( char *s );
extern char * strupr ( char *s );

/* text.c */
extern void Help ( void );
extern void Title ( void );

#endif
#endif /* _ATP_READLIB_H */

