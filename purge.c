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
purge.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "reader.h"
#include "readlib.h"
#include "qlib.h"
#include "ansi.h"
#include "makemail.h"
#ifdef __LCLINT__
#include "purge.lh"
#endif

/* 
 * BEGIN CLIP FAMILY ROUTINES
 *
 * Family of functions to truncate ATP databases to "n" most recent
 *  messages, and to purge messages marked as "killed". 
 *  
 */

static FILE *idxptr;
static FILE *ndxptr;
static FILE *ncnfptr;
static FILE *ocnfptr;
enum errlvl {
    el_invalid = -13, el_0 = 0, el_1, el_2, el_3, el_4
};

/* 
 * clip_open_fin, cleans up after a call to clip_open().
 */
static atp_BOOL_T
clip_open_fin(enum errlvl err_level, char *co_tmptr, char *co_tmpbuf)
{
    atp_BOOL_T ret_code;
    switch (err_level) {
    case el_4:
	fclose(ncnfptr);
	strcpy(co_tmptr, "tmc");
	do_unlink(co_tmpbuf);
	/*@fallthrough@*/ /* this is supposed to fallthrough, no break needed */
    case el_3:
	fclose(ndxptr);
	strcpy(co_tmptr, "tmi");
	do_unlink(co_tmpbuf);
	/*@fallthrough@*/ /* no break needed */
    case el_2:
	fclose(idxptr);
	/*@fallthrough@*/ /* no break needed */
    case el_1:
	ret_code = FALSE;
	break;
    case el_0:
	ret_code = TRUE;
	break;
    default:
	/* switch() case error */
	error_switch_case((int)err_level, __FILE__, __LINE__);
	ret_code = FALSE;
    }
    return ret_code;
}



/*
 * clip_open, opens files for use by clip() to prune message base.
 *
 * This routine opens 2 files and creates 2 files. The names of
 * these files are unique in suffix only. We build new path names
 * by changing the suffix. The parameter co_tmpbuf points to the
 * basic pathname which we will build upon.
 *   
 */
static atp_BOOL_T 
clip_open(char *co_tmpbuf, long *num_messages)
{
    /* init to an invalid err_level */
    enum errlvl err_level = el_invalid;
    char *co_tmptr = co_tmpbuf;

    /* isolate the suffix */
    while (*co_tmptr != NUL_CHAR)
	co_tmptr++;
    co_tmptr -= 3;
    /* 
       create path name to old index file, then open the file. 
     */
    strcpy(co_tmptr, "idx");
    if ((idxptr = fopen(co_tmpbuf, "rb")) == NULL) {
	/* Unable to open file */
	fprintf(stderr, "%s: %s\n", txt[51], co_tmpbuf);
	err_level = el_1;
    } else {
	/*
	   Determine the length of the old index file and the number
	   of messages which it references.
	 */
	long file_len;
	fseek(idxptr, 0L, SEEK_END);
	file_len = ftell(idxptr);
	if (file_len < (long)IDXSIZE) {
	    /*  Error reading file (empty) */
	    fprintf(stderr, "%s (%s): %s\n", txt[58], txt[9], co_tmpbuf);
	    err_level = el_2;
	} else {
	    *num_messages = file_len / IDXSIZE;
	    /* 
	       Create path name for new temporary index file,
	       then open the new index file. 
	     */
	    strcpy(co_tmptr, "tmi");
	    if ((ndxptr = fopen(co_tmpbuf, "wb")) == NULL) {
		/* Unable to create file */
		fprintf(stderr, "%s: %s\n", txt[50], co_tmpbuf);
		err_level = el_2;
	    } else {
		/* 
		   Create path name for new temporary conference file,
		   then open new conference file. 
		 */
		strcpy(co_tmptr, "tmc");
		if ((ncnfptr = fopen(co_tmpbuf, "wb")) == NULL) {
		    /* Unable to create file */
		    fprintf(stderr, "%s: %s\n", txt[50], co_tmpbuf);
		    err_level = el_3;
		} else {
		    /* 
		       Create path name for old conference file, 
		       then open old conference file.
		     */
		    strcpy(co_tmptr, "cnf");
		    if ((ocnfptr = fopen(co_tmpbuf, "rb")) == NULL) {
			/* Unable to open file */
			fprintf(stderr, "%s: %s\n", txt[51], co_tmpbuf);
			err_level = el_4;
		    } else
			err_level = el_0;
		}
	    }
	}
    }
    /* clean up */
    return clip_open_fin(err_level, co_tmptr, co_tmpbuf);
}

/* forward declarations */
#ifdef no_proto
static void clip_close();
static atp_BOOL_T clip_loop();
#else
static void clip_close(atp_BOOL_T result_flag, char *tmpbuf1);
static atp_BOOL_T clip_loop(char *cl_tmpbuf, const long idx_offset, long *read_last);
#endif


/* 
 * clip_do, does housekeeping before and after a call to clip_loop().
 */
static atp_BOOL_T
clip_do(const long trunc_num, char *cd_tmptr, atp_BOOL_T purge_mode, const long num_messages)
{
    atp_BOOL_T ret_code = FALSE;
    struct MyIndex msg_idx;
/*
   We now know the number of messages in the file. Next we must
   calculate the current message number of the message that will
   start the new message base: msg_start = num_messages - trunc_num
 */
    if (purge_mode || trunc_num < num_messages) {
	long msg_start = 0L;
	long idx_offset = 0L;
	if (!purge_mode) {
	    msg_start = num_messages - trunc_num;
	    idx_offset = msg_start * IDXSIZE;
	}
	/* calculate new last_read pointer */
	rewind(idxptr);
	if (fread(&msg_idx, index_SIZE, one_record, idxptr) != one_record) {
	    /* read error */
	    fprintf(stderr, "%s: %s:%d\n", txt[58], __FILE__, __LINE__);
	} else {
	    long last_read = msg_idx.LastRead;
	    if (last_read >= num_messages)
		last_read = num_messages - 1;	/* repair an error by resetting message pointer */
	    if (last_read > msg_start)
		last_read -= msg_start;
	    else
		last_read = 0L;
#ifdef ATPDBG3
	    printf("LAST: %ld   START: %ld\n", last_read, msg_start);
	    sleep(5);
#endif

	    /* LOOP through message base */
	    ret_code = clip_loop(cd_tmptr, idx_offset, &last_read);
	    if (ret_code) {
		printf("\n\nwait ... \r");
		fflush(stdout);

		/* fix-up last_read pointer */
		rewind(ndxptr);
		fwrite(&last_read, sizeof(msg_idx.LastRead), one_record, ndxptr);
	    }
	}
    }
    /* fini: */
    return ret_code;
}

/*
 * clip, truncate ATP databases and purge "killed" messages.
 *  Returns TRUE on success, and FALSE on failure. Called by
 *  function Purge() 
 */
static atp_BOOL_T
clip(const char *bbspath, const int Tflg, const long trunc_num)
{
    atp_BOOL_T ret_code = FALSE;
    /* shortest valid names, e.g. "0.cnf" or "0.idx" */
    const size_t minimum_valid_name_length = 5;

    if (trunc_num > 0L) {
	long num_messages = 0L;
	char tmpbuf[MAXPATHS];
	const atp_BOOL_T purge_mode = (Tflg == (int)'C') ? TRUE : FALSE;
	strcpy(tmpbuf, bbspath);

	if (strlen(tmpbuf) < minimum_valid_name_length) {
	    printf("%s %s\n", txt[111], tmpbuf);
	} else if (clip_open(tmpbuf, &num_messages)) {
	    ret_code = clip_do(trunc_num, tmpbuf, purge_mode, num_messages);
	    clip_close(ret_code, tmpbuf);
	}
    }
    return ret_code;
}


/*
 * clip_verify_msg_size, compare index with message header.
 */
ATP_INLINE void
clip_verify_msg_size(const size_t nrecs, const long old_num)
{
    const size_t msg_size = atoi(rbuf + HSizeMsg);
    if (msg_size != nrecs) {
	fprintf(stderr, "Warning: index and header size mismatch: #%ld             \n", old_num + 1);
	fprintf(stderr, "header size says %lu blocks\n", (unsigned long) msg_size);
	fprintf(stderr, " index size says %lu blocks\n", (unsigned long) nrecs);
    }
#ifdef ATPDBG3
    fprintf(stderr, "msg_size = %lu  nrecs = %lu\n", (unsigned long)msg_size, (unsigned long)nrecs);
#endif
}


/* used only by clip_loop() and clip_close() */
static atp_BOOL_T wrote_one;

/*
 * clip_loop, loops through message base, truncating and purging.
 */
static atp_BOOL_T
clip_loop(char *cl_tmpbuf, const long idx_offset, long *read_last)
{
    atp_BOOL_T ret_code = TRUE;
    const byte high_bit_mask = HIGH_ASCII;
    long old_num = 0L, old_size = 0L, old_offset = 0L, last_read;
    struct MyIndex msg_idx;

    /*      
       Point to the index record for new first message.
       Get its offset in the message data base and its size 
     */
    wrote_one = FALSE;
    last_read = *read_last;
    fseek(idxptr, idx_offset, SEEK_SET);
    while (fread(&msg_idx, index_SIZE, one_record, idxptr) == one_record) {
	size_t nrecs;
	unsigned char kill_it;
#ifdef ATPDBG3
	printf("Offset = %ld\n", msg_idx.Offset);
	printf("MsgNum = %ld\n", msg_idx.MsgNum);
	printf("MsgSize = %ld\n", msg_idx.Size);
#endif
	/* read in the new message to the buffer */
	nrecs = (size_t) (msg_idx.Size / block_SIZE) + 1;
	if (nrecs > get_RbufRecs() && !reup(nrecs * block_SIZE)) {
	    /* can't reallocte buffer */
	    /* don't unlink source message base */
	    ret_code = FALSE;
	    break;
	}
	/* clobber old header */
	(void)memset(rbuf, (int)'G', header_SIZE);
	fseek(ocnfptr, msg_idx.Offset, SEEK_SET);
	fread(rbuf, block_SIZE, nrecs, ocnfptr);

	/*  verify that message size in index and and header match */
	clip_verify_msg_size(nrecs, old_num);

	/* check if status byte is marked as killed */
	kill_it = (byte) (/*@i1*/ (byte)rbuf[0] & high_bit_mask);

	/* Now  we update the new index and conference files */
	if (/*@i1*/ kill_it != NUL_CHAR) {
	    if ((old_num <= last_read) && last_read)
		last_read--;	/* adjust last message read */
	    printf("%s :    KILLING message %ld        \r", cl_tmpbuf, msg_idx.MsgNum + 1);
	} else {
	    if (fwrite(rbuf, block_SIZE, nrecs, ncnfptr) != nrecs) {
		printf("\n (ncnfptr) Write error in clip()%c (disk full?)\n", BELL);
		ret_code = FALSE;	/* don't unlink source files */
		break;
	    }
	    msg_idx.Offset = old_size + old_offset;
	    msg_idx.MsgNum = old_num;
	    old_num++;
	    old_size = msg_idx.Size;
	    old_offset = msg_idx.Offset + block_SIZE;
	    if (fwrite(&msg_idx, index_SIZE, one_record, ndxptr) != one_record) {
		printf("\n (ndxptr) Write error in clip()%c (disk full?)\n", BELL);
		ret_code = FALSE;	/* don't unlink source files */
		break;
	    }
	    wrote_one = TRUE;
	    printf("%s : processing message %ld        \r", cl_tmpbuf, msg_idx.MsgNum + 1);
	}
	fflush(stdout);
    }
    *read_last = last_read;
    return ret_code;
}


/*
 * clip_close, housekeeping called from clip() before exiting.
 */
static void
clip_close(atp_BOOL_T result_flag, char *tmpbuf1)
{
    char *tmptr1, *tmptr2, tmpbuf2[MAXPATHS];
    const atp_BOOL_T nrml_exit = result_flag;

/* close open files */
    fclose(idxptr);
    fclose(ndxptr);
    fclose(ocnfptr);
    fclose(ncnfptr);

/* build path to temp files */
    strcpy(tmpbuf2, tmpbuf1);
    tmptr1 = tmpbuf1;
    tmptr2 = tmpbuf2;
    /* seek to end of buffer */
    while (*tmptr1 != NUL_CHAR) {
	tmptr1++;
	tmptr2++;
    }
    tmptr1 -= 3;		/* strlen("cnf") */
    tmptr2 -= 3;		/* strlen("tmc") */
    strcpy(tmptr1, "cnf");
    strcpy(tmptr2, "tmc");

/* rename and unlink temp files */
    if (nrml_exit) {
#ifdef CHECK_MODE_DEBUG
	const atp_BOOL_T check_mode = TRUE; /* used only for debugging */
	if (check_mode) {
	    do_unlink(tmpbuf2);	/* just unlink temp conf file */
	} else 
#endif
	{
	    do_unlink(tmpbuf1);	/* unlink xxx.cnf */
	    if (wrote_one)
		rename(tmpbuf2, tmpbuf1);	/* link xxx.tmc to xxx.cnf */
	    else
		do_unlink(tmpbuf2);	/* empty conference file */
	}
	strcpy(tmptr1, "idx");
	strcpy(tmptr2, "tmi");
#ifdef CHECK_MODE_DEBUG
	if (check_mode) {
	    do_unlink(tmpbuf2);	/* just unlink temp index file */
	} else 
#endif
	{
	    do_unlink(tmpbuf1);	/* unlink xxx.idx */
	    if (wrote_one)
		rename(tmpbuf2, tmpbuf1);	/* link xxx.tmi to xxx.idx */
	    else
		do_unlink(tmpbuf2);
	}
	printf("done.     \n");
    } else {			/* not normal exit */
	/* unlink temporary files on error */
	do_unlink(tmpbuf2);	/* unlink xxx.tmc (temp conference file) */
	strcpy(tmptr2, "tmi");
	do_unlink(tmpbuf2);	/* unlink xxx.tmi (temp index file) */
	printf("\n");
    }
}

/*
 * PurgeAdjust, does preliminary houskeeping for Purge_CT() and Purge_D().
 */
static void
PurgeAdjust(int con_num)
{
    if (con_num == get_CurConf()) {
	zero_Index(ip_PurgeAdjust);
	set_TotMsg(0L, tm_PurgeAdjust);
	if (get_FilesOpen()) {
	    fclose_fmsg_fidx();
	    set_FirstDone(FALSE, fd_PurgeAdjust);
	}
    }
}


/*
 * Purge_D, deletes a given conference.
 */
static void
Purge_D(char *cnf, char *idx, int con_num)
{
    PurgeAdjust(con_num);
    if (unlink(cnf) == FAILURE) {
	perror("Purge_D()");
	printf("Unable to remove file %s\n", cnf);
    } else if (unlink(idx) == FAILURE) {
	perror("Purge_D()");
	printf("Unable to remove file %s\n", idx);
    }
    if (con_num == findCindex(PERS_CONF))
	set_pmail(FALSE, pmPurge_D);
    deleol();
    blue();
    printf("Conference %d is now empty.", ConfNumbers[con_num]);
    green();
    printf("\n\n");
}


/*
 * Purge_CT, performs "clean" or "truncate" on a give conference.
 */
static void
Purge_CT(char *cnf, const int cmd, const int con_num)
{
    long trunc_siz = get_TruncNum();
    struct stat st;
    PurgeAdjust(con_num);
    if (cmd == (int)'T') {
	char *ntmp, tmp[80];
	sprintf(tmp, "%ld", trunc_siz);
	luxptr = tmp;
	for (;;) {
	    deleol();		/* clear line */
	    do {
		ntmp = readline("\renter truncation count: ", do_scroll);
	    } while (ntmp == NULL);
	    if (Numeric(ntmp)) {
		trunc_siz = atol(ntmp);
		free_string(ntmp);
		luxptr = NULL;
		printf("\n");
		break;
	    }
	}
    }
    clip(cnf, cmd, trunc_siz);	/* purge and/or truncate conference */
    if (stat(cnf, &st) == FAILURE || st.st_size == 0) {
	if (con_num == findCindex(PERS_CONF))
	    set_pmail(FALSE, pmPurge_CT);
    } else if (con_num == get_CurConf() && /*@i1*/ GetConf(con_num) != ATP_OK) {	/* reopen conference */
	printf("Error in purge() can't reopen conference\n");
    }
}


/* 
 * PurgeAsk, queries user for "delete", "truncate", "clean".
 */
static int
PurgeAsk(const long c_len, const char *idx, const int con_num)
{
    struct stat st;
    int ret_code = 0;

    if (stat(idx, &st) == SUCCESS) {
	char *ntmp, tmp[80];
	const long i_len = st.st_size;
	const long num_msgs = i_len / IDXSIZE;
	long avg_siz; 
	if (num_msgs != 0L)
	    avg_siz = (((c_len * 10) / num_msgs) + 5L) / 10;	/* round up */
	else
	    avg_siz = 0L;
	/* here we could attempt to rebuild index if num_msgs == 0 */
	/* not implemented yet */
	green();
	high();
	printf("\nConf. %d [ ", ConfNumbers[con_num]);
	yellow();
	printf("%s", ConfNames[con_num]);
	green();
	printf(" ] takes ");
	yellow();
	printf("%ld Kbytes\n", (long) (c_len + 512L) >> 10);
	green();
	{
	    printf("\n     Index length = %ld bytes\n", i_len);
	    printf("     Data length = %ld bytes\n", c_len);
	    printf("     Number of messages = %ld \n", num_msgs);
	    printf("     Average size = %ld bytes\n\n", avg_siz);
	}
	deleol();
	do {
	    ntmp = readline("\r (D)elete (C)lean (T)runcate (N)ext (S)top : ", no_scroll);	/* empty loop */
	} while (ntmp == NULL);
	tmp[0] = ntmp[0];
	tmp[1] = NUL_CHAR;
	free_string(ntmp);
	deleol();
	(void)strupr(tmp);
	ret_code = (int)tmp[0] ; 
    }
    return ret_code;
}

/*
 * PurgeEnd, does pre-exit housekeeping for Purge().
 */
static void
PurgeEnd(void)
{
    /* update array of active conferences */
    ActvConf();
    if (!ConfActive[get_CurConf()])
	AutoJoin();
    else
	GetConf(get_CurConf());
}

/*
 * Purge - used to prune message bases when "clean" command is invoked.
 */
void
Purge(void)
{
    if (get_isempty()) {
	EmptyMsg();
    } else {
	int i;
	if (ConfActive[get_CurConf()])
	    UpdateConf(update_last_read_ptr);	/* Save current conf pointer */
	magenta();
	high();			/* Selective erasing of conferences. */
	printf("\n%s\n", txt[102]);	/* Effacement sélectif des conférences */
	clear();
	red();
	printf("----------------------------------------\n");
	for (i = 0; i <= LastConf; i++) {
	    struct stat st;
	    char cnf[MAXPATHS];
	    char idx[MAXPATHS];
	    make_c_i_paths(cnf, idx, ConfNumbers[i]);
	    if (stat(cnf, &st) == SUCCESS) {
		const long c_len = st.st_size;
		const int ask = PurgeAsk(c_len, idx, i);
		switch (ask) {
		case 'S':	/* stop */
		    i = LastConf + 1;	/* break from outer loop */
		    break;
		case 'D':
		    Purge_D(cnf, idx, i);
		    break;
		case 'C':
		case 'T':
		    Purge_CT(cnf, ask, i);
		    break;
		case 'N':
		case 0:
		default:
		    break;
		}
	    }
	}
	PurgeEnd();
    }
}
