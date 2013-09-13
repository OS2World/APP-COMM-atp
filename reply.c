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
reply.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>

#include "reader.h"
#include "readlib.h"
#include "ansi.h"
#include "makemail.h"
#include "qlib.h"

/************************* begin Reply() routines *********************/

/* size for reply prompt buffer */
#define RPMSIZE 300

#if defined(HAVE_LIBTERMCAP)
/*
 * do_tc_prompt, called by reply_prompt() on termcap based systems. 
 */
static void
do_tc_prompt(char *rpmbuf, const int subject_length, const char *p)
{                                 /*@-strictops */
    if (*cur_right != NUL_CHAR) { /*@=strictops */
	char *t;
	int k, i, buf_leng = 0;
	strcat(rpmbuf, "[ ");
	t = tputs_ptr(cur_right);
	for (i = 0; i < subject_length; i++)
	    strcat(rpmbuf, t);
	strcat(rpmbuf, " ]");
	buf_leng = strlen(rpmbuf);
	for (i = 0; i < RPMSIZE; i++)
	    rpmbuf[buf_leng + i] = (char) SPC_CHAR;
	rpmbuf[buf_leng] = NUL_CHAR;
	strcat(rpmbuf, "\r");
	k = strlen(p) + 2;
	for (i = 0; i < k; i++)
	    strcat(rpmbuf, t);
    }
}
#endif

static const char *prm_ansi[] =
{"error", PRMTTO, PRMTFR, PRMTSB, PRMTSL, PRMTSC, NULL};

enum rep_prompt_type {
    REPERR = 0, REPTO, REPFR, REPSB, REPSBL, REPSEC
};

/*
 * reply_prompt, create prompts for user when entering a message.
 *
 *  modes for reply prompts:  to, from, subject, long_subject, security.
 */
static char *
reply_prompt(enum rep_prompt_type rpm)
{
    static char rpmbuf[RPMSIZE];
    const char *p;
    rpmbuf[0] = NUL_CHAR;
    strcat(rpmbuf, "\r");
    if (ansi) {
	strcat(rpmbuf, prm_ansi[(unsigned)rpm]);
    } else {
#if defined(HAVE_LIBTERMCAP)
	int subject_length = SHORT_SUBJ_LEN;
#endif
	switch (rpm) {
	case REPTO:
	    p = PRTO;
	    break;
	case REPFR:
	    p = PFROM;
	    break;
	case REPSB:
	    p = PSUBJ;
	    break;
	case REPSBL:
	    p = PSUBJ;
#if defined(HAVE_LIBTERMCAP)
	    subject_length = LSUBJ_BODY_LEN;
#endif
	    break;
	case REPSEC:
	    p = PSECR;
	    break;
	default:
	    /* switch() case error */
	    error_switch_case((int)rpm, __FILE__, __LINE__);
	    p = " ERROR in reply_prompt() ";
	}
	strcat(rpmbuf, p);
#if defined(HAVE_LIBTERMCAP)
	do_tc_prompt(rpmbuf, subject_length, p);
#endif
    }
    return rpmbuf;
}


/* 
 * ask_speller, asks user if reply needs spelling check.
 */
static atp_ERROR_T
ask_spell(const reply_type_t mode, const char *fname)
{
    atp_ERROR_T ret_code = ATP_OK;
    char CONSPTR speller = get_atprc_str(spellr);
    char *intmp;                                     /*@-strictops */

    if (mode != XPOST && speller[0] != NUL_CHAR ) {  /*@=strictops */
	do {
	    intmp = readline("Check spelling [Y/n] ?", do_scroll);
	} while (intmp == NULL);
	StripDel(intmp);
	intmp[0] = tolower(intmp[0]);  /*@-strictops */
	if (intmp[0] != 'n') {         /*@=strictops */
	    fork_execvp(speller, fname);
	    restore_terminal();	/* restore terminal parameters */
#ifdef USE_FORK
	    if (get_reperror())
		ret_code = ATP_ERROR;
#endif
	}
	free_string(intmp);
    }
    free_string(speller);
    return ret_code;
}


/* 
 * ask_user, confirm disposition of reply with the user.
 * bufr *must* point to a buffer 
 */
static void
ask_user(const reply_type_t mode, char *bufr)
{				/*@-strictops */
    if (mode == XPOST)		/*@=strictops */
	bufr[0] = 'x';
    else
	for (;;) {
	    char *intmp;
	    CLSCRN();
	    printf("\n");
	    if (get_tag_flag()) {
		strcpy(bufr, "tag ?");
		Tag(bufr);
		printf("Fido style tagline is %s.\n\n", fido ? "ON" : "OFF");
	    } else {
		printf("\nTaglines are currently disabled.\n");
		printf("Type \"tag on\" to enable them.\n");
	    }
	    printf("select:\n\t`Save' to save reply.\n");
	    printf("\t`Edit' to re-edit reply.\n");
	    printf("\t`Fido' to toggle tagline style and save reply.\n");
	    printf("\t`Tag' allows you to change tagline (see ATP man page)\n");
	    printf("\t`Abort' to cancel reply.\n\n");
	    do {
		intmp = readline("Choose: Save, Edit, Fido, Tag, Abort  [Save] ? ", do_scroll);
	    } while (intmp == NULL);
	    strcpy(bufr, intmp);
	    free_string(intmp);
	    StripDel(bufr);
	    StripLSpace(bufr);
	    bufr[0] = tolower(bufr[0]);   /*@-strictops */
	    if (bufr[0] == NUL_CHAR || bufr[0] == 's' || bufr[0] == 'f' || bufr[0] == 'a' || bufr[0] == 'e')
		break;                  /*@=strictops */
	    if (strnicmp(bufr, "tag", (size_t)3) == 0) {
          	/* call tagline routines */
		tag_set_edit_reply_mode(TRUE);
		Tag(bufr);
		tag_set_edit_reply_mode(FALSE);
	    }
	}/* end of for(;;) loop */
}


/*
 * scpy, copies len bytes to src while also deleting left spaces.
 */
static void
scpy(char *dest, const char *src, const size_t len)
{
    int i = (int) len ;
    (void) memcpy( dest, src, len);
    do {
	*(dest + i) = NUL_CHAR;
	i--;			/*@-strictops */
    } while ( 0 <= i && *(dest + i) == SPC_CHAR);	/*@=strictops */
}

/*
 * set_lowcase, subroutine for get_from().
 */
static atp_BOOL_T
set_lowcase(const reply_type_t mode, char *Header)
{
    atp_BOOL_T lowcase = fido;		/* True or False boolean valued */
    /*@-strictops */
    if (mode == REPLY) {
	char CONSPTR sl_tmp = Header + HForWhom;
	size_t i = 0;
	lowcase = FALSE;
	for (; i < field_len; i++)
	    if (sl_tmp[i] >= 'a' && sl_tmp[i] <= 'z')
		lowcase = TRUE;
    } else if (mode == ENTER) {         /*@=strictops */ 
	/* PCBoard likes uppercase to/from field */
	lowcase = get_caps() ? FALSE : TRUE  ;
    }
    return lowcase;
}

/*
 * error_too_long, prints message if author or recipient address is too long.
 */
static void
error_too_long(void)
{
    red();
    printf("      %s", txt[34]);
    green();
    printf("\n");
}

/*
 * error_msg_aborted, message printed when reply is aborted.
 */
static void
error_msg_aborted(void)
{
    blue();
    printf("      %s.", txt[33]);
    green();
    printf("\n");
}


static char tmp[MAXPATHS], dummy[MAXPATHS];
/*
 * get_from, get who the message is from.
 */
static atp_ERROR_T
get_from(const reply_type_t mode, char *Qmail)
{
    atp_ERROR_T ret_code;
    char CONSPTR UserName = get_cntrl_str(usrnm);
    assert(UserName != NULL);
    printf("\n");
    for (;;) {
	atp_BOOL_T lowcase;
	char CONSPTR Header = rbuf;
	char *intmp = NULL;
	green();
	strcpy(dummy, UserName);
	luxptr = dummy;                        /*@-strictops */ 
	if (mode == CHANGE || mode == XPOST) { /*@=strictops */ 
	    scpy(dummy, Header + HAuthor, field_len);
	}
	lowcase = set_lowcase(mode, Header);
	if (!lowcase)
	    (void)strupr(dummy);
	do {
	    intmp = readline(reply_prompt(REPFR), do_scroll);
	} while (intmp == NULL);
	strcpy(tmp, intmp);
	add_history(intmp);
	free_string(intmp);
	luxptr = NULL;
	StripDel(tmp);
	if (strlen(tmp) <= field_len)
	    break;
	/* "Entry too long!  field_len chars max */
	error_too_long();
	printf("\n");
    }
    free_string(UserName);     /*@-strictops */
    if (tmp[0] != NUL_CHAR) {  /*@=strictops */
	/* write name in the from field */
	str2mem(Qmail + HAuthor, tmp);
	ret_code = ATP_OK;
    } else {
	ret_code = ATP_ERROR;
	/* message aborted! */
	error_msg_aborted();
    }
    return ret_code;
}

/* 
 * get_to, get who the message is being sent to.
 */
static atp_ERROR_T
get_to(const reply_type_t mode, char *Qmail)
{
    atp_ERROR_T ret_code;
    for (;;) {
	char *intmp = NULL;
	char CONSPTR Header = rbuf;
	green();
	luxptr = dummy; /*@-strictops */
	if (mode == CHANGE || mode == XPOST)
	    scpy(dummy, Header + HForWhom, field_len);
	else if (mode != ENTER)
	    scpy(dummy, Header + HAuthor, field_len);
	else
	    strcpy(dummy, "All");
	if (!fido || (get_caps() && (mode == ENTER)))	/* PCBoard likes uppercase to/from field */
	    (void)strupr(dummy);
	do {
	    intmp = readline(reply_prompt(REPTO), do_scroll);
	} while (intmp == NULL);
	if (!fido || (get_caps() && (mode == ENTER)))	/* PCBoard likes uppercase to/from field */
	    (void)strupr(intmp); /*@=strictops */
	strcpy(tmp, intmp);
	add_history(intmp);
	free_string(intmp);
	luxptr = NULL;
	StripDel(tmp);
	if (strlen(tmp) <= field_len)
	    break;
	/* "Entry too long!  field_len chars max */
	error_too_long();
	printf("\n");
    }
    /*@-strictops */
    if (stricmp(tmp, "N") == SUCCESS || tmp[0] == NUL_CHAR ) { /*@=strictops */
	ret_code = ATP_ERROR;
	/* "message aborted" */
	error_msg_aborted();
    } else {
	ret_code = ATP_OK;
	str2mem(Qmail + HForWhom, tmp);
    }
    return ret_code;
}

/* Long subject for PCBoard */
static char RepSubjBuf[100];   

/*
 * get_reply_lsubj - strdups the reply long subject buffer.
 */ 
char *
get_reply_lsubj(void)
{
    char *sb = NULL;                 /*@-strictops */
    if (RepSubjBuf[0] != NUL_CHAR ) {  /*@=strictops */
	sb = strdup(RepSubjBuf);
	test_fatal_malloc(sb, __FILE__, __LINE__);
    }
    return sb;
}


/*
 * setup_long_subject, initializes reply subject buffer.
 */
static void
setup_long_subject(const char *Header)
{
    if (PCBLONG)
	Check4LongSubj();
    if (subj_is_long()) {
	char CONSPTR tp = get_long_subj();
	assert( tp != NULL );
	strcpy(RepSubjBuf, tp);
	free_string(tp);
    } else {
	scpy(RepSubjBuf, Header + HSubject, field_len);
    }
}

/* 
 * get_subject, get subject of the message.
 */
static atp_ERROR_T
get_subj(const reply_type_t mode, char *Qmail)
{
    atp_ERROR_T ret_code;
    char *intmp = NULL;
    char CONSPTR Header = rbuf;
    green();               /*@-strictops */
    if (mode != ENTER) {   /*@=strictops */
	setup_long_subject(Header);
	luxptr = RepSubjBuf;
    }
    do {
	intmp = readline(reply_prompt((PCBLONG ? REPSBL : REPSB)), do_scroll);
    } while (intmp == NULL);
    strcpy(tmp, intmp);
    add_history(intmp);
    free_string(intmp);
    /* remember that luxptr is global so re-initialize ! */
    luxptr = NULL;
    StripDel(tmp);  /*@-strictops */
    if (stricmp(tmp, "N") == SUCCESS || tmp[0] == NUL_CHAR ) {  /*@=strictops */
	/* "message aborted" */
	error_msg_aborted();
	ret_code = ATP_ERROR;
    } else {
	ret_code = ATP_OK;
	/* length of long subject line */
	if (strlen(tmp) > (size_t) LSUBJ_BODY_LEN)
	    tmp[LSUBJ_BODY_LEN] = NUL_CHAR;
	/* make sure that it fits header field */
	if (strlen(tmp) > (size_t) SHORT_SUBJ_LEN) {
	    strcpy(RepSubjBuf, tmp);
	    tmp[SHORT_SUBJ_LEN] = NUL_CHAR;
	} else {
	    /* don't use long subject if length <= field_len */
	    RepSubjBuf[0] = NUL_CHAR;
	}
	str2mem(Qmail + HSubject, tmp);
    }
    return ret_code;
}

/* 
 * get_security, get message security: public or private.
 */
static atp_ERROR_T
get_security(const reply_type_t mode, char *Qmail)
{
    atp_ERROR_T ret_code;
    char CONSPTR Header = rbuf;
    for (;;) {
	char *intmp = NULL;
	green();	/*@-strictops */
	if ((mode != ENTER) &&
	    (Header[HStatus] == '+' || Header[HStatus] == '*')){/*@=strictops */
	    luxptr = "Receiver Only (private)";
	    add_history("None (public)");
	} else {
	    luxptr = "None (public)";
	    add_history("Receiver Only (private)");
	}
	do {
	    intmp = readline(reply_prompt(REPSEC), do_scroll);
	} while (intmp == NULL);
	strcpy(tmp, intmp);
	add_history(intmp);
	free_string(intmp);
	luxptr = NULL;
	StripDel(tmp);
	StripLSpace(tmp);
	if (strlen(tmp) <= field_len)
	    break;
	red();
	printf("%s", txt[35]);	/* "Too long! R=...etc..." */
	green();
	printf("\n");
    }                           /*@-strictops */
    if (tmp[0] != NUL_CHAR) {
	(void)strupr(tmp);
	if (tmp[0] == 'R' || strnicmp(tmp,"priv", (size_t)4) == SUCCESS )   /*@=strictops */
	    Qmail[HStatus] = (char) PRIVATE_MSG;
	else
	    Qmail[HStatus] = (char) PUBLIC_MSG;
	ret_code = ATP_OK;
    } else {
	ret_code = ATP_ERROR;
	/* "message aborted" */
	error_msg_aborted();
    }
    return ret_code;
}

/* index into array */
static int TargetConfIdx;
/*
 * get_address_fin, finishes building message header for replies.
 */
static void
get_address_fin(const reply_type_t mode, char *Qmail)
{
    char CONSPTR Header = rbuf;
    /* Msg goes to the target conference */
    sprintf(dummy, "%-7d", ConfNumbers[TargetConfIdx]);
    str2mem(Qmail + HConfNum, dummy);

    /* get reference number if available */          /*@-strictops */
    if (mode == CHANGE )
	scpy(dummy, Header + HRefMsg, (size_t)7);
    else if (mode == ENTER || mode == XPOST)         /*@=strictops */
	dummy[0] = NUL_CHAR;
    else
	scpy(dummy, Header + HNumMsg, (size_t)7);
    str2mem(Qmail + HRefMsg, dummy);
}


static const char *months[] =
{"none", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
 "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
static const char *monums[] =
{"00-", "01-", "02-", "03-", "04-", "05-", "06-",
 "07-", "08-", "09-", "10-", "11-", "12-"};
  
/*
 * le date et l'heure d'entete du message.     
 * tetedate, returns a pointer to the reply date/time string.
 */
static char *
tetedate(void)
{
    int i;
    time_t t;
    char tbuf[30];
    static char tbuf2[30];
    t = time(NULL);
    tzset();
    /* this macro is used only by msdos and its relatives */
    ADJUST_DOS_TIME
    sprintf(tbuf, "%s", ctime(&t));
    tbuf[7] = NUL_CHAR;
    tbuf[10] = NUL_CHAR;
    tbuf[16] = NUL_CHAR;
    tbuf[21] = '-';
    tbuf[24] = NUL_CHAR;
    /* default case */
    sprintf(tbuf2, "01-01-7001:00");
    for (i = 1; i < 13; i++) {
	const int found_a_match = stricmp(tbuf+4, months[i]);
	if (found_a_match == SUCCESS) {
	    sprintf(tbuf2, "%s%s%s%s", monums[i], tbuf+8, tbuf+21, tbuf+11);
	    break;
	}
    } /*@-strictops */
    if (tbuf2[3] == SPC_CHAR) /*@=strictops */
	tbuf2[3] = '0';
    return tbuf2;
}

/*
 * Remise du Header aux paramŠtres par d‚faut  
 * ResetHeader, put the default parameters in the reply header.
 */
static void
ResetHeader(char *Qmail)
{
    const unsigned high_byte = 0xff00;
    const unsigned low_byte = 0x00ff;
    byte CONSPTR ptr = (byte *) Qmail;
    byte word16[2];		/* space for 16 bit unsigned word  */
    char qbuf[30];		/* temporary buffer                */

    /* Given index number, retrieve real conference number */
    const unsigned RealConfN = (unsigned)ConfNumbers[TargetConfIdx];
#ifdef ATPDBG
    const int temp = ConfNumbers[TargetConfIdx];
    assert( -1 < temp );
#endif

    /* fill new header with spaces ; struct remplie d'espaces */
    (void)memset((void *) Qmail, (int)SPC_CHAR, header_SIZE);

    /* Mark message as "public message, unread" */
    Qmail[HStatus] = PUBLIC_MSG;

    /* Add ascii conference number to header */
    sprintf(qbuf, "%-7d", ConfNumbers[TargetConfIdx]);
    str2mem(Qmail + HConfNum, qbuf);

    /* Now enter the time as formatted by a call to tetedate() */
    str2mem(Qmail + HMsgDate, tetedate());

    /* create a 16bit little endian unsigned conference number */
    word16[0] = (byte) (RealConfN & low_byte );
    word16[1] = (byte) ((RealConfN & high_byte ) >> 8);
    (void)memcpy((void *) &Qmail[HBinConfN], (void *) word16, (size_t) 2);

    /* Mark message as "active", 0xE1 = active  0xE2 = non active  */
    *(ptr + HMsgActiv) = ACTIVE_MSG;
    *(ptr + HUnused1) = SPC_CHAR;	/* space */
    *(ptr + HUnused2) = SPC_CHAR;	/* space */
    *(ptr + HNetTag) = SPC_CHAR;	/* space */
}


/* 
 * get_address, build subject/author/recepient/security fields for message.
 */
static atp_ERROR_T
get_address(const reply_type_t mode, char *Qmail)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    ResetHeader(Qmail);

    /* get who the message is from */      /*@-strictops */ 
    if (get_from(mode, Qmail) == ATP_OK

    /* get who the message is being sent to */
	&& get_to(mode, Qmail) == ATP_OK

    /* get subject of message */
	&& get_subj(mode, Qmail) == ATP_OK

    /* get security of message */
	&& get_security(mode, Qmail) == ATP_OK) {

    /* finish up */
	get_address_fin(mode, Qmail);
	ret_code = ATP_OK;
    }
    return ret_code;
}


/*
 * AddReply, add the reply to a Qmail reply.msg file.
 */
static void
AddReply(const reply_type_t mode, const char *fname, char *Qmail)
{
    if (OpenRepFile(add_reply) && KodeMessage(fname, Qmail) == ATP_OK) {
	do_unlink(fname);
	set_ReplyExist(TRUE, rexist_AddReply);
	if (autotag)
	    ChooseTag();
	if (get_saved_conf() == RCONF_IDX) {
	    const long total_messages = get_TotMsg() + 1L ;
	    set_TotMsg(total_messages, tm_AddReply);
	    /*    CurConf = RCONF_IDX; */
	    if (mode == CHANGE)
		ReadNext(NUKE);
	    if (mode == XPOST)             /*@=strictops */
		ReadNext(CROSS);
	    else {
		sprintf(dummy, "%ld", get_TotMsg());
		GoToNum(dummy);
	    }
	}
    }
}



/*
 * Rep_conf_stoi, finds conference integer index based on string argument.
 * 
 */
static int
Rep_conf_stoi(const char *cmdargs)
{
    int i;
    if (Numeric(cmdargs)) {
	i = findCindex(atoi(cmdargs));
    } else if (stricmp(cmdargs, "MAIN") == SUCCESS) { /* not numeric */
	i = 0;
    } else {
	/* search for match in conf list */
	const size_t cmdarg_len = strlen(cmdargs);
	for (i = 0; i <= LastConf; i++)
	    if (strnicmp(cmdargs, ConfNames[i], cmdarg_len) == SUCCESS)
		break;
    }
    return i;
}

static const int not_valid_target  = (int)ATP_ERROR ;
/*
 * Rep_valid_range, verifies that a conference index is in the proper range.
 */
static int
Rep_valid_range(const int target)
{
    return ((0 <= target && target < RCONF_IDX) ? target : not_valid_target );
}

static const int no_args = -1 ;

/*
 * Rep_mode_valid, verifies that requested Reply() operation is legal.
 */
static atp_BOOL_T
Rep_mode_valid(const reply_type_t mode, const int target)
{
    atp_BOOL_T is_valid = FALSE;
    const int saved_conf = get_saved_conf();
    assert(-1 < saved_conf);
    if (target == no_args) {
	switch (mode) {
	case XPOST:
	    break;
	case ENTER:
	    is_valid = (saved_conf != PCONF_IDX && saved_conf != RCONF_IDX) ?
		TRUE : FALSE;
	    break;
	case CHANGE:
	    is_valid = (saved_conf == RCONF_IDX) ? TRUE : FALSE;
	    break;
	case REPLY:
	    is_valid = (saved_conf != RCONF_IDX) ? TRUE : FALSE;
	    break;
	default:
	    /* switch() case error */
	    error_switch_case((int)mode, __FILE__, __LINE__);
	}
    } else
	is_valid = (Rep_valid_range(target) == not_valid_target) ?
	    FALSE : TRUE;
    return is_valid;
}


/*
 * Rep_conf_header, read conference number from header, find index and validate.
 */
static int
Rep_conf_header(const char *Header)
{
    const int target = findCindex(readCnum(Header + HBinConfN));
    return Rep_valid_range(target);
}

/*
 * get_target_conf, get proper conference number to inscribe in reply header.
 */
static int
get_target_conf(const reply_type_t mode)
{
    int target = not_valid_target;
    /*@-strictops */
    if (mode == CHANGE || get_saved_conf() == PCONF_IDX) {
	/* rbuf points to message header */
	target = Rep_conf_header(rbuf);
    } else if (mode == REPLY || mode == ENTER) { /*@=strictops */
	target = get_saved_conf();
    } else {
	fprintf(stderr, "Unknown mode: %d  %s:%d\n", (int)mode, __FILE__, __LINE__);
	(void)sleep(3);
    }
    return target;
}

/*
 * Rep_mode_parse, initializes context for Reply().
 */
static int
Rep_mode_parse(const reply_type_t mode, const char *cmdline)
{
    int target;
    /* check for command line arguments */
    const atp_BOOL_T m_args = ((sscanf(cmdline, "%s %s", dummy, tmp)) > 1) ? TRUE : FALSE;

    if (m_args) {
	/* convert string to number */
	target = Rep_conf_stoi(tmp);

	/* verify combination of mode and target */
	if (!Rep_mode_valid(mode, target)) {
	    target = not_valid_target;
	}
    } else if (Rep_mode_valid(mode, no_args)) {
	target = get_target_conf(mode);
    } else
	target = not_valid_target;
    return target;
}
/* end of mode parse routines */


/*
 * call_editor, invoke editor for reply.
 */
static atp_ERROR_T
call_editor(const reply_type_t mode, const char *fname)
{
    atp_ERROR_T ret_code = ATP_OK;
    /*@-strictops */
    if (mode != XPOST) { /*@=strictops */
	char CONSPTR Editor = get_atprc_str(editr);
	assert( Editor != NULL );
	white();
	/* "calling editor" */
	printf("\n%s\n", txt[38]);
	fork_execvp(Editor, fname);
	free_string(Editor);
	/* reset terminal attributes */
	restore_terminal();
#ifdef USE_FORK
	if (get_reperror())
	    ret_code = ATP_ERROR;
#endif
    }
    return ret_code;
}


/* 
 * McAdjust, adjust for Mc prefix as in McWilliams.
 */
ATP_INLINE void 
McAdjust(size_t *i, const char **str, char **dst, const size_t len)
{
    const char CONSPTR McPrefix = "Mc";
    const size_t mick_len = (size_t) STRING_LEN("Mc");
    const size_t too_far = (size_t) (len - mick_len);

    assert( len >= mick_len );
    if (*i < too_far && strnicmp(((*str) + 1), McPrefix, mick_len) == SUCCESS) {
	strcpy(((*dst) + 1), McPrefix);
	(*i) += mick_len;
	(*dst) += (int) mick_len;
	(*str) += (int) mick_len;
    }
}

/*
 **Formatte le nom en minuscules, avec les initiales Upcase, dans dst, null
 **terminated.
 * Initials, build and format author's name for reply headers. 
 */
static void
Initials(const char *str, char *dst, const size_t len)
{
    atp_BOOL_T convert_to_upper = TRUE;
    size_t i;

    /* build a name with first letter capitalized */
    for (i = 0; i < len; i++, str++, dst++) {
	*dst = *str;		/*@-strictops */
	if (*dst == '.' || *dst == '-' || *dst == SPC_CHAR) {	/*@=strictops */
	    convert_to_upper = TRUE;	/* Next char converted to Upcase */
	    McAdjust(&i, &str, &dst, len);	/* adjust for "Mc" prefix */
	} else if (convert_to_upper) {
	    *dst = toupper(*dst);
	    convert_to_upper = FALSE;
	} else {
	    *dst = tolower(*dst);
	}
    }

    /* build a null-terminated string */
    do {
	*dst = NUL_CHAR;
	dst--;			/*@-strictops */
    } while (*dst == SPC_CHAR);	/*@=strictops */

}


/* 
 * Response modes for RefMessage():
 *  John Doe wrote to all, John Doe wrote to me, John Doe wrote to Guy Beau.
 */
enum ref_hdr_msg {
  John_Doe_wrote_to_ALL, John_Doe_wrote_to_me, John_Doe_wrote_to_Guy_Beau
};


/*
 * get_response_mode, get relationship of who is writing to whom.
 */
static enum ref_hdr_msg
get_response_mode(const char *Msg)
{
    enum ref_hdr_msg response_mode;
    char CONSPTR UserName = get_cntrl_str(usrnm);
    assert(UserName != NULL);

    if (strnicmp(Msg + HForWhom, "ALL", (size_t) 3) == SUCCESS)
        response_mode = John_Doe_wrote_to_ALL;
    else if (strnicmp(Msg + HForWhom, UserName, strlen(UserName)) == SUCCESS)
        response_mode = John_Doe_wrote_to_me;
    else
        response_mode = John_Doe_wrote_to_Guy_Beau;
    free_string(UserName);
    return response_mode;
}


/*
 * R‚alise l'entˆte de la lettre...
 * RefMessage, create a message reference string for a reply.
 */
static void
RefMessage(FILE * fp, char *Msg)
{
    if (get_HeadLetter()) {
	enum ref_hdr_msg response_mode;
	int mon, day;
	char Destin[32];
	char Ref[32];

	Msg[HMsgDate + 2] = '-';
	Msg[HMsgDate + 5] = '-';
	sscanf(Msg + HMsgDate, "%d-%d", &mon, &day);
	Initials(Msg + HAuthor, Destin, field_len);
	Destin[field_len] = NUL_CHAR;
	Initials(Msg + HForWhom, Ref, field_len);
	Ref[field_len] = NUL_CHAR;

	response_mode = get_response_mode(Msg);

	if (strnicmp(Msg + HAuthor, "UUCP", (size_t)4) != SUCCESS ) {
	    /* "Cher"           */
	    fprintf(fp, "%s %s,\n", txt[96], Destin);
	    fprintf(fp, "%s", txt[97]);
	    /* "Dans un msg du" */
	    fprintf(fp, " %d %s, ", day, Months[mon - 1]);

	    switch (response_mode) {
	    case John_Doe_wrote_to_ALL:
		/* "vous ‚crivez" */
		fprintf(fp, "%s :\n\n", txt[98]);
		break;
	    case John_Doe_wrote_to_me:
		/* "vous m'‚crivez" */
		fprintf(fp, "%s :\n\n", txt[99]);
		break;
	    case John_Doe_wrote_to_Guy_Beau:
		/* "destin‚ …" */
		fprintf(fp, "%s", txt[100]);
		fprintf(fp, " %s, ", Ref);
		/* "vous ‚crivez"   */
		fprintf(fp, "%s :\n\n", txt[98]);
		break;
	    default:
		/* switch() case error */
		error_switch_case((int)response_mode, __FILE__, __LINE__);
	    }
	}
    }
}



/*
 * output_quoted_line, add quote delimiter character to start of reply line. 
 */
static void
output_quoted_line(unsigned long i, byte * ptr, const char *quote, FILE * fq)
{
    size_t col = strlen(quote);
    const atp_CODE_T chrset = get_charset();
    const unsigned long MsgSize = get_MsgSize();
    fprintf(fq, "%s", quote);
    while (i < MsgSize) {
	/*@-strictops */
	if (*ptr == (byte)'\n') {
	    fprintf(fq, "\n%s", quote);
	    col = strlen(quote);
	} else if (*ptr != (byte)NUL_CHAR) {
	    if (col < 78) {	/* Trunc quoted lines to 78 cols */
		if (chrset == ISOLAT1)
		    fprintf(fq, "%c", codelu[(unsigned) (*ptr)]);
		else if (chrset == CHR7BIT)
		    fprintf(fq, "%c", code7bit[(unsigned) (*ptr)]);
		else
		    fprintf(fq, "%c", *ptr == DOS_SPACE ? SPC_CHAR : *ptr);
		    /*@=strictops */
	    }
	    col++;
	}
	i++;
	ptr++;
    }
}


/*
 * QuoteDo, open reply file for quoting. 
 */
static void
QuoteDo(const char *quote, const char *tmprname)
{
    FILE *fq;
    if ((fq = fopen(tmprname, "w")) == NULL) {
	/* "Unable to create file" */
	printf("%s %s !\n", txt[50], tmprname);
    } else {
	unsigned long start_of_text_offset = 0L;
	char *ptr = rbuf;

	/* Dear User, in a message to... */
	RefMessage(fq, ptr);

	/* point to start of message text */
	ptr += header_SIZE;

	/* adjust start of message for embedded long subject lines */
	if (PCBLONG && subj_is_long()) {
	    ptr = get_sot(); /* get start_of_text */ /*@-strictops */
	    start_of_text_offset = (long) (ptr - (rbuf + header_SIZE)); /*@=strictops */
	}
	/* write quoted lines to file */
	output_quoted_line(start_of_text_offset, (byte *) ptr, quote, fq);
	fclose(fq);
    }
}

/*
 * QuoteMsg, copy the current msg in file tmprname with quotes.
 */
static void
QuoteMsg(const char *tmprname)
{
    const char CONSPTR Msg = rbuf;
    char quote[10];
    if (strnicmp(Msg + HAuthor, "SYSOP", (size_t)5) == SUCCESS) {
	strcpy(quote, "Sys> ");
    } else if (strnicmp(Msg + HAuthor, "UUCP", (size_t)4) == SUCCESS) {
	strcpy(quote, "> ");
    } else {
	int col;
	char tmp1[50], tmp2[50], tmp3[50], tmp4[50], tmp5[50];
	(void)memcpy(tmp1, Msg + HAuthor, field_len);
	tmp1[field_len] = NUL_CHAR;
	col = sscanf(tmp1, "%s %s %s %s", tmp2, tmp3, tmp4, tmp5);

	assert( 0 <= col && col < 5 );
	switch (col) {
	case 0:
	    strcpy(quote, " > ");
	    break;
	case 1:                            /*@-strictops */
	    if (tmp2[1] == NUL_CHAR)       /*@=strictops */
		sprintf(quote, "%c> ", tmp2[0]);
	    else
		sprintf(quote, "%c%c> ", tmp2[0], tmp2[1]);
	    break;
	case 3:
	    sprintf(quote, "%c%c%c> ", tmp2[0], tmp3[0], tmp4[0]);	/* Initials.. */
	    break;
	case 4:
	    sprintf(quote, "%c%c> ", tmp2[0], tmp5[0]);		/* Initials.. */
	    break;
	default:
	    sprintf(quote, "%c%c> ", tmp2[0], tmp3[0]);		/* Initials.. */
	}
    }
    printf("\n%s %s ...\n", txt[59], quote);	/* "Quoting message with " */
    QuoteDo(quote, tmprname);
}


/*
 * do_strip, copy old reply message minus the tagline, called by StripTag().
 */
static void
do_strip(byte *ptr, FILE * fq, unsigned long i)
{
    const unsigned long MsgSize = get_MsgSize();
    const atp_CODE_T chrset = get_charset();
    while (i < MsgSize) {
	/*@-strictops */ 
	if (*ptr == DOS_SPACE) {
	    fprintf(fq, "\n");
	    break;
	}
	if (*ptr == (byte)'\n')
	    fprintf(fq, "\n");
	else if (*ptr == (byte)NUL_CHAR)/* do nothing, i.e. skip the zero character */
	    ;			/* empty statement */
	else if (chrset == ISOLAT1)
	    fprintf(fq, "%c", codelu[(unsigned) (*ptr)]);
	else if (chrset == CHR7BIT) /*@=strictops */
	    fprintf(fq, "%c", code7bit[(unsigned) (*ptr)]);
	else
	    fprintf(fq, "%c", *ptr);
	i++;
	ptr++;
    }
}


/* 
 * StripTag, strip tagline from re-edited reply.
 */
static void
StripTag(const char *tmprname)
{
    FILE *fq;
    if ((fq = fopen(tmprname, "w")) == NULL) {
	/* "Unable to create file" */
	printf("%s %s !\n", txt[50], tmprname);
	(void)sleep(4);
    } else {
	unsigned long i = 0;
	byte *ptr = (byte *) (rbuf + header_SIZE);
	/* adjust for long embedded subjects */
	if (PCBLONG && subj_is_long()) {       /*@-strictops */
	    assert( strlen(RepSubjBuf) > field_len ); /* ??? */
	    while (*ptr != (byte)LF_CHAR)      /*@=strictops */
		i++, ptr++;
	    i++, ptr++;               /*@-strictops */
	    if(*ptr == (byte)LF_CHAR) /*@=strictops */
	        i++, ptr++;
	}
	/* setup is complete, now do the actual work */
	do_strip(ptr, fq, i);
	fflush(fq);
	fclose(fq);
    }
}


/*
 * editpp_fname, pre-process reply file for quoting and tagline removal.
 */
static void
editpp_fname(const reply_type_t mode, const char *fname, atp_BOOL_T reedit)
{
    /*@-strictops */
    if (mode == REPLY && !reedit)
	/* Quote original new message */
	QuoteMsg(fname);
    else if ((mode == CHANGE || mode == XPOST) && !reedit)
	/* remove tagline from old message */
	StripTag(fname);
    /*@=strictops */
}


/*
 * edit_fname, wrapper sets up housekeeping for call_editor().
 * returns either ATP_OK or ATP_ERROR
 */
static atp_ERROR_T
edit_fname(const reply_type_t mode, const char *fname, char *Qmail)
{
    atp_BOOL_T reedit = FALSE;
    atp_ERROR_T ret_code = ATP_ERROR;

    do {
	/* get subject, author, recepient, and security for message */ /*@-strictops */
	if (get_address(mode, Qmail) == ATP_OK) {

	    /* pre-process reply: quote message or strip tagline */
	    editpp_fname(mode, fname, reedit);

	    /* finally, here is where we invoke the editor for the reply */
	    if (call_editor(mode, fname) == ATP_OK

	    /* here is where we ask if spelling needs checking */
		&& ask_spell(mode, fname) == ATP_OK) {
                /*@=strictops */
		/* query user whether to save, abort or re-edit message */
		ask_user(mode, tmp);
		switch (tmp[0]) {
		case 'e':
		    /* re-edit message */
		    reedit = TRUE;
		    break;
		case 'a':
		    /* abort message */
		    error_msg_aborted();
		    do_unlink(fname);
		    reedit = FALSE;
		    ret_code = ATP_ERROR;
		    break;
		case 'f':
		    /* flip tagline style */
		    /*@-casebreak */ /* yes, we fall-through, no break */
		    togltag();
		default:
		    reedit = FALSE;
		    /*@=casebreak */
		    ret_code = ATP_OK;
		    /* adjust header for fido or not */
		    if (!fido) {
			int i = 0;
			for (; i < 50; i++)
			    Qmail[HForWhom + i] = toupper(Qmail[HForWhom + i]);
		    }
		}
	    }
	}
    } while (reedit);
    return ret_code;
}

/* initialize to guaranteed invalid conference index */ 
static const int conf_is_invalid = -17715;
static int SaveConf = -17715;

/*
 * get_saved_conf - returns value of SaveConf variable.
 */
int
get_saved_conf(void)
{
	return SaveConf;
}

/*
 * Reply - reply to current message; enter message; or re-edit a message.
 */
void
Reply(const reply_type_t mode, const char *line)
{
    char *intmp, fname[MAXPATHS];
    char Qmail[HDRSIZE];

    /* save current conference */
    SaveConf = get_CurConf();

    /* determine which conference to post message in based on mode */
    if ((TargetConfIdx = Rep_mode_parse(mode, line)) != (int)ATP_ERROR) {
#ifdef ATPDBG
	assert( 0 <= TargetConfIdx && TargetConfIdx < RCONF_IDX );
#endif
	/* create temporary file for reply */
	if ((intmp = tmpnam(NULL)) != NULL)
	    strcpy(fname, intmp);
	else
	    sprintf(fname, "%s%c%s", WorkPath, SEP, "reply.tmp");

	/* edit the reply file */                        /*@-strictops */
	if (edit_fname(mode, fname, Qmail) == ATP_OK) {  /*@=strictops */

	    /* add the reply to the reply conference database */
	    AddReply(mode, fname, Qmail);
	}
    }
    restore_terminal();
#ifdef USE_FORK
    if (get_reperror()) {
	CLSCRN();
	/* clear error condition */
	reset_reperror();
    }
#endif
    /* restore current conference */
    /* CurConf = SaveConf; */
    /* invalidate saved conference */
    SaveConf = conf_is_invalid;
    return;
}
/********************** end of Reply() routines *******************/

