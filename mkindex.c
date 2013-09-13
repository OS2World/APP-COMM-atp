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
mkindex.c
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_UTIME_H)
# include <utime.h>
#endif

#include "reader.h"
#include "readlib.h"
#include "ansi.h"
#include "qlib.h"
#include "makemail.h"

/*
 * BEGIN THE MAKE INDEX ROUTINES
 *
 */

/*
 * MkInReadMsg, does the low-level read when called from MkInLoopRd().
 */
ATP_INLINE atp_ERROR_T
MkInReadMsg(const unsigned long MaxChars, FILE * rm_fs, const size_t NbBlocs)
{
    atp_ERROR_T ret_code = ATP_OK;
    byte *ptr = (byte *) rbuf + block_SIZE;

    /* Read the message..... */
    if (fread(ptr, block_SIZE, NbBlocs, rm_fs) != NbBlocs) {
	printf("max char %lu\n", MaxChars);
	printf("%s messages.dat\n", txt[58]);	/* "error reading file " */
	ret_code = ATP_ERROR;
    } else {
	/* Translate to normal Line Feed */
	unsigned long i = 0L;
	for (; i < MaxChars; i++) {
            /*@-strictops */
	    if ((byte)*ptr == QWK_LINE_FEED) /*@=strictops */
		*ptr = '\n';
	    ptr++;
	}
	/*@-strictops */
	while (*(--ptr) == SPC_CHAR) /*@=strictops */
	    *ptr = NUL_CHAR ;		/* delete padding spaces */
    }
    return ret_code;
}

static /*@i5*/ FILE *fs = NULL, *fd = NULL, *fx = NULL, *pfd = NULL, *pfx = NULL;
static unsigned NamLen = 0;
static char Qmail[HDRSIZE];
static char *UserName;
static atp_BOOL_T persmail;


/*
 * MkInUserName, try to get user name from control.dat, or atprc.
 */
ATP_INLINE atp_BOOL_T
MkInUserName(void)
{
    atp_BOOL_T ret_code = TRUE;
    if ((UserName = get_cntrl_str(usrnm)) == NULL
	&& (UserName = get_atprc_str(usr1nm)) == NULL) {
	/* this point should never be reached */
	ret_code = FALSE;
    } else {
	NamLen = strlen(UserName);
    }
    return ret_code;
}


/*
 * MkInOpen, first routine called by MkIndex.
 */
static atp_BOOL_T
MkInOpen(const char *SrcDir, const char *DestDir, long *old_last)
{
    atp_BOOL_T ret_code = FALSE;
    char Src[MAXPATHS];		/* Source file Message.DAT */
    char Pdst[MAXPATHS];	/* Personal conference file */
    char Pidx[MAXPATHS];	/* Personal index file     */

    /* Load new CONTROL.DAT file. */  /*@-strictops */
    if (RdCn_ReadControl(SrcDir) != ATP_OK) { /*@=strictops */	
	/* "error in control.dat" */
	printf("%s !\n", txt[14]);
    } else {
	yellow();
	/* "adding msgs and creating indexes" */
	printf("%s...\n", txt[71]);
	cyan();
	/*@-usedef */
	/* Open MESSAGES.DAT */
	verify_new_file(Src, SrcDir, MSG_FILE);
	/*@=usedef */
	printf("Open %s\n", Src);

	if ((fs = fopen(Src, "rb")) == NULL) {
	    /* "unable to open file" */
	    printf("%s %s...\n", txt[51], Src);
	} else if (fread(Qmail, header_SIZE, one_record, fs) != one_record) {
	    /* "error reading file" */
	    printf("%s %s\n", txt[58], Src);
	    fclose(fs);
	} else {
	    /* Open personal conference and index files  */
	    sprintf(Pdst, "%s%c%d.cnf", DestDir, SEP, PERS_CONF);

	    /* Open Conference file */  /*@-strictops */
	    if (OpenCon(&pfd, &fs, Pdst) != ATP_ERROR) {  /*@=strictops */
		fseek(pfd, 0L, SEEK_END);	/* Go to end of personal idx file... */
		*old_last = ftell(pfd);		/* Take size of existing messages */
		sprintf(Pidx, "%s%c%d.idx", DestDir, SEP, PERS_CONF);

		/* Open Conference Index */ /*@-strictops */
		if (OpenCon(&pfx, &fs, Pidx) != ATP_ERROR) {  /*@=strictops */
		    fseek(pfx, 0L, SEEK_END);
		    MoveWork(DestDir, SrcDir);
		    persmail = FALSE;
		    ret_code = MkInUserName();
		}
	    }
	}
    }
    return ret_code;
}

static int iconf = 0;
static long oldcount = 0;
static long count = 0;

/*
 * MkInShow, print the conference number and name while building Index.
 */
static void
MkInShow(void)
{
    printf("Conference %d \t[ %-15s ]%c  %3ld New %s %s \n",
	   ConfNumbers[iconf], ConfNames[iconf],
	   persmail ? '*' : ' ', count - oldcount,
	   (count - oldcount) > 1 ? "Messages" : "Message",
	   persmail ? txt[103] : txt[108]);
}


/*
 * MkInClose, cleanup and close files before exiting MkIndex().
 */
static void
MkInClose( atp_ERROR_T ret_code, const int conf, const long old_last, const char *DestDir)
{
    char tmpbuf[MAXPATHS];  /*@-strictops */
    if (ret_code == ATP_OK && conf >= 0) /*@=strictops */
	MkInShow();
    /* outerr: */
    printf("\n");
    fclose(fs);
    fclose(fd);
    fclose(pfd);
    fclose(fx);
    fclose(pfx);
    /* if no pre-existing or new personal messages, delete personal cnf. */
    if (old_last == 0L) {
	sprintf(tmpbuf, "%s%c%d.idx", DestDir, SEP, PERS_CONF);
	do_unlink(tmpbuf);
	sprintf(tmpbuf, "%s%c%d.cnf", DestDir, SEP, PERS_CONF);
	do_unlink(tmpbuf);
    }
}


static int conf;

/*
 * MkInDoOpenFiles, setup and execute the open routines.
 *  called by MkInReOpen().
 */
static atp_BOOL_T
MkInDoOpenFiles(const char *Dst, char *Idx, const char *DestDir, long *last)
{
    atp_BOOL_T ret_code = TRUE;
    /* Open Conference file */ /*@-strictops */
    if (OpenCon(&fd, &fs, Dst) == ATP_ERROR) { /*@=strictops */
	fclose(pfx);
	fclose(pfd);
	ret_code = FALSE;
    } else {
	fseek(fd, 0L, SEEK_END);	/* Go to end of file... */
	*last = ftell(fd);	/* Take size of existing messages */
	sprintf(Idx, "%s%c%d.idx", DestDir, SEP, conf);
	/* Open Conference Index */  /*@-strictops */
	if (OpenCon(&fx, &fs, Idx) == ATP_ERROR) {  /*@=strictops */
	    fclose(pfx);
	    fclose(pfd);
	    ret_code = FALSE;
	} else {
	    fseek(fx, 0L, SEEK_END);
	    count = (ftell(fx) / IDXSIZE);
	    oldcount = count;
	    printf("%d : %ld\r", iconf, count - oldcount);
	    fflush(stdout);
	}
    }
    return ret_code;
}

/*
 * MkInReopen, closes old conference dbase and opens a new conference dbase.
 */
static atp_BOOL_T
MkInReOpen(atp_BOOL_T *ffllgg, const char *DestDir, long *last)
{
    char Dst[MAXPATHS];		/* Conference data  file   */
    char Idx[MAXPATHS];		/* Conference index file   */
    if (*ffllgg) {		/* after first pass, close file pointers */
	fclose(fx);
	fclose(fd);
    }
    *ffllgg = TRUE;		/* first pass flag set true */
    if (conf >= 0)		/* close up old before starting new */
	MkInShow();
    persmail = FALSE;
    count = oldcount = 0;
    conf = readCnum(Qmail + HBinConfN);
    /* here is where we do bounds checking */
    for (iconf = 0; iconf <= LastConf; iconf++)
	if (conf == ConfNumbers[iconf])
	    break;
    if (iconf > LastConf) {
	/* this unmungs the number by overwriting probable space character */
	Qmail[HBinConfN + 1] = (byte) 0;
	(void)memcpy((char *) rbuf, (char *) Qmail, header_SIZE);
	conf = readCnum(Qmail + HBinConfN);
	for (iconf = 0; iconf <= LastConf; iconf++)
	    if (conf == ConfNumbers[iconf])
		break;
	if (iconf > LastConf) {	/* last resort force valid conf num. */
	    const unsigned high_byte = 0xff00;
	    const unsigned low_byte = 0x00ff;
	    unsigned uconf;
	    iconf = 0;
	    conf = ConfNumbers[0];
	    uconf = (unsigned) conf;
	    Qmail[HBinConfN] = (byte) (uconf & low_byte);
	    Qmail[HBinConfN + 1] = (byte) ((uconf & high_byte) >> 8);
	    (void)memcpy(rbuf, Qmail, header_SIZE);
	}
    }
    /* Open new files  */
    sprintf(Dst, "%s%c%d.cnf", DestDir, SEP, conf);
    return MkInDoOpenFiles(Dst, Idx, DestDir, last);
}


/*
 * MkIn_calc_block_count, calculate/adjust record count for splitting messages.
 */
ATP_INLINE unsigned long
MkIn_calc_block_count(size_t *NbBlocs, size_t TotBlocs)
{
    unsigned long MaxChars;
    char tqbuf[10];
    if ( TotBlocs < *NbBlocs)
	*NbBlocs = (size_t) TotBlocs;
    MaxChars = *NbBlocs * block_SIZE;
    sprintf(tqbuf, "%lu", (unsigned long)(*NbBlocs + (size_t)1));
    str2mem(rbuf + HSizeMsg, "      ");		/* 6 spaces */
    str2mem(rbuf + HSizeMsg, tqbuf);
    return MaxChars;
}

static atp_BOOL_T persave;
static long pcount;

/*
 * MkInPers, write out personal messages to their own conference area.
 */
static atp_ERROR_T
MkInPers(const size_t blk_ct, long *old_last, const unsigned long MaxChars, const char *DestDir)
{
    atp_ERROR_T ret_code = ATP_OK;
    if (fwrite(rbuf, block_SIZE, blk_ct, pfd) != blk_ct) {
	/* "error writing file " */
	perror("MkInPers()");
	printf("%s %s%c%d.cnf\n", txt[73], DestDir, SEP, PERS_CONF);
	ret_code = ATP_ERROR;
    } else
	*old_last += (long) MaxChars + header_SIZE;
    return ret_code;
}


/*
 * MkInWritePidx, write out personal message conference index.
 */
ATP_INLINE atp_BOOL_T
MkInWritePidx(unsigned long MaxChars, long former_last)
{
    atp_BOOL_T ret_code;
    /*@-strictops */
    if (WriteIndex(pfx, pcount, MaxChars, former_last) == ATP_ERROR) {
	ret_code = FALSE;	/*@-strictops */
    } else {
	pcount++;
	ret_code = TRUE;
    }
    return ret_code;
}


/*
 * MkInLoopRd, loop for reading in raw messages from packet.
 */
ATP_INLINE atp_ERROR_T
MkInLoopRd(size_t TotBlocs, size_t NbBlocs, 
	   long *old_last, long *rlast, const char *DestDir)
{
    atp_ERROR_T ret_code = ATP_OK;
    long last = *rlast;
    unsigned long MaxChars = NbBlocs * block_SIZE ;

    /* loop till complete message is read */
    for (;;) {
	/* if message will be split ... */
	if (TotBlocs != NbBlocs)
	    MaxChars = MkIn_calc_block_count(&NbBlocs, TotBlocs);

	/* 'last' initialized to -912L, error in ftell() sets 'last' to -1L */
	assert( last != -912L );
        /*@-strictops */
	/* Write message index */
	if (last < 0L || WriteIndex(fx, count, MaxChars, last) == ATP_ERROR) {	/*@=strictops */ 
	    /* "msg too big" */
	    printf("Error writing fidx MkIndex(): last = %ld\n", last);
	    ret_code = ATP_ERROR;
	    break;
	}
	count++;
	/* Write Personal Index */
	if (persave && !MkInWritePidx(MaxChars, *old_last)){
	    printf("Error writing pidx MkIndex()\n");	/* "msg too big" */
	    ret_code = ATP_ERROR;
	    break;
	}
	/* Display running total to screen. */
	printf("%d : %ld\r", iconf, count - oldcount);
	fflush(stdout);

	/*@-strictops */
	/* Read the message ... */
	if (MaxChars && (ret_code = MkInReadMsg(MaxChars, fs, NbBlocs)) == ATP_ERROR)/*@=strictops */
	    break;

	/* Write the message ... */
	if (fwrite(rbuf, block_SIZE, NbBlocs + 1, fd) != NbBlocs + 1) {	/* And copy it.         */
	    printf("%s %d.cnf\n", txt[73], conf);	/* "error writing file " */
	    ret_code = ATP_ERROR;
	    break;
	}
	/*@-strictops */
	/* Write any personal messages to personal area ... */
	if (persave && (ret_code = MkInPers(NbBlocs + 1, old_last, MaxChars, DestDir)) == ATP_ERROR) {/*@=strictops */
	    break;
	} else {
	    /* keep track of our progress ... */
	    last += (long) (MaxChars + header_SIZE);
	    if (TotBlocs >= NbBlocs)
		TotBlocs -= NbBlocs;
	    if (TotBlocs <= 0)
		break;
	}
    }				/* end of for(;;) */
    *rlast = last;
    return ret_code;
}

/* 
 * MkInGetMemory, try to re-allocate more memory while reading in new messages.
 */
ATP_INLINE size_t
MkInGetMemory(size_t NbBlocs)
{
    reup( NbBlocs * block_SIZE + block_SIZE ); 
    return (get_RbufRecs() - (size_t)1) ; 
}

/*
   This is where we read messages.dat the main message file from
   the BBS. The file is parsed into new seperate .cnf files for each
   conference, each with its own index. While scanning the main file
   we also look for a match between UserName (global variable) and
   Qmail.ForWhom field in each message header. When a match is found
   the message is also saved to a personal conference file with its
   own index, and a flag "persmail" is set which shows that personal
   mail has been received. Note that the index files sent in the QWK
   packet are ignored and never used by this program.
 */


/*
 * MkIndex - tosses messages.dat thus creating conferences and indexes. 
 */
atp_ERROR_T
MkIndex(const char *SrcDir, const char *DestDir)
{
    atp_ERROR_T ret_code = ATP_OK;
    long old_last = 0L;		/* size of pre-existing personal conference */

    if (MkInOpen(SrcDir, DestDir, &old_last)) {
	atp_BOOL_T ffllgg = FALSE;
	long last = -912L;	/* initialize to arbitrary debugging constant */
	conf = -1;
	pcount = (ftell(pfx) / IDXSIZE);
	for (;;) {
	    size_t NbBlocs, TotBlocs;

	    if (fread(Qmail, header_SIZE, one_record, fs) != one_record)
		break;		/* End of file         */
            /*@-strictops */
	    if (Qmail[HStatus] == NUL_CHAR || (byte) Qmail[HStatus] == DOS_SPACE)
		break;		/* End of file  J mail ?  */  /*@=strictops */

	    /* Another conference ? */
	    (void)memcpy(rbuf, Qmail, header_SIZE);
	    if ((readCnum(Qmail + HBinConfN)) != conf 
	        && !MkInReOpen(&ffllgg, DestDir, &last)) {
		ret_code = ATP_ERROR ;
		break ;
	    }
	    /* check for a personal message */
	    persave = FALSE;
	    if (strnicmp(UserName, Qmail + HForWhom, NamLen)==SUCCESS){
		set_pmail(TRUE,pmMkIndex);	/* set global personal mail flag true */
		persmail = TRUE;	/* set local personal mail flag true  */
		persave = TRUE;
	    }
	    /* check buffer size and re-allocate if needed */
	    TotBlocs = NbBlocs = atoi(Qmail + HSizeMsg) - 1;
	    if (NbBlocs + (size_t)1 > get_RbufRecs() )
		NbBlocs = MkInGetMemory(NbBlocs);
	    ret_code = MkInLoopRd(TotBlocs, NbBlocs, &old_last, &last, DestDir);
	    /*@-strictops */
	    if (ret_code == ATP_ERROR) /*@=strictops */
		break;
	}			/* end of for(;;) */
	MkInClose(ret_code, conf, old_last, DestDir);
	free_string(UserName);
    }
    return ret_code;
}


/* end of mkindex.c */
