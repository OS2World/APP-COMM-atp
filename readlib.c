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
readlib.c
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 

#include "reader.h"
#include "readlib.h"
#include "ansi.h"
#include "makemail.h"
#include "qlib.h"

static atp_BOOL_T FirstDone  = FALSE;  /* Set if 1st message in conf is read       */

/*
 * set_FirstDone - used to set the FirstDone flag.
 */
void
set_FirstDone(const atp_BOOL_T mode, const fdone_access_t perm)
{
/*@-strictops */
    if (perm == fd_ReadNext || perm == fd_PurgeAdjust )
	FirstDone = mode;
#ifdef ATPDBG
    else
        assert(perm == fd_ReadNext || perm == fd_PurgeAdjust );
#endif
/*@=strictops */
}


/*
 * get_FirstDone - return the FirstDone flag.
 */
atp_BOOL_T
get_FirstDone(void)
{
    return FirstDone;
}

/* internal structure of atp index files    */
static struct MyIndex Index;         

/*
 * get_MsgLastRead - returns Index.LastRead.
 */
long
get_MsgLastRead(void)
{
    return Index.LastRead;
}

/*
 * get_MsgNum - returns Index.MsgNum.
 */
long
get_MsgNum(void)
{
    return Index.MsgNum;
}

/*
 * get_MsgOffset - returns Index.Offset.
 */
long
get_MsgOffset(void)
{
    return Index.Offset;
}

/*
 * get_MsgSize - returns Index.Size.
 */
unsigned long
get_MsgSize(void)
{
    return Index.Size;
}

/*
 * get_ptrIndex - returns a pointer to the Index structure.
 */
struct MyIndex *
get_ptrIndex(const indxptr_access_t perm)
{
    struct MyIndex *tp = &Index;
/*@-strictops */
    if (perm != ip_ReadSeek) {
	tp = NULL;
	assert(perm == ip_ReadSeek);
    }
/*@=strictops */
    return tp;
}

/*
 * zero_Index - sets Index.LastRead and Index.MsgNum to 0L.
 */
void
zero_Index(const indxptr_access_t perm)
{
/*@-strictops */
    if (perm == ip_PurgeAdjust)
	Index.LastRead = Index.MsgNum = 0L;
#ifdef ATPDBG
    else
	assert(perm == ip_PurgeAdjust);
#endif
/*@=strictops */
}


/* 
 * SkNumRead, helper function for SeekNum() to load buffer after seek.
 */
static atp_BOOL_T
SkNumRead(void)
{
    atp_BOOL_T ret_code = FALSE;
    const size_t nrecs = (size_t) ((Index.Size / block_SIZE) + 1);	/* same as (Index.Size+128)/128) */
    if (nrecs <= (get_RbufSize() / block_SIZE) || reup(nrecs * block_SIZE)) {
	if ((fread(rbuf, block_SIZE, nrecs, fmsg)) != nrecs) {
	    printf("%s ? \n", txt[18]);		/* "read error" */
	} else {
	    rbuf[(size_t) Index.Size + block_SIZE] = NUL_CHAR;		/* terminate buffer like a string */
	    FirstDone = TRUE;	/* signal that there is a full buffer */
	    ret_code = TRUE;	/* good seek */
	}
    }
    return ret_code;
}

/* Number of messages in conference */
static long int TotMsg = 0L;

/*
 * get_TotMsg - returns TotMsg the number of messages in a conference.
 */
long
get_TotMsg(void)
{
    return TotMsg;
}

/*
 * set_TotMsg - sets TotMsg the number of messages in conference.
 */
void
set_TotMsg(const long val, const totmsg_access_t perm)
{
/*@-strictops */
    if (perm == tm_PurgeAdjust || perm == tm_AddReply)
	TotMsg = val;
#ifdef ATPDBG
    else
	assert(perm == tm_PurgeAdjust || perm == tm_AddReply);
#endif
/*@=strictops */
}

/*
 * SeekNum - seek to specific message number.
 */
atp_BOOL_T
SeekNum(const char *str)
{
    atp_BOOL_T ret_code = FALSE;
    const long int nb = atol(str) - 1;	/* message number to seek to */
    if (get_isempty()) {
	EmptyMsg();
    } else if (nb < 0 || nb >= TotMsg) {
	red();
	high();
	printf("%s\n", txt[81]);	/* "Number out of range ! " */
	printf("%s ", txt[82]);	/* "Valid msgs are" */
	magenta();
	printf("1 ");
	red();
	printf("%s ", txt[83]);	/* "to" */
	magenta();
	printf("%ld\n", TotMsg);
    } else if (fseek(fidx, (nb * (long) IDXSIZE), SEEK_SET) != SUCCESS) {
	printf("%s !\n", txt[16]);	/* "seek error" */
    } else if (fread(&Index, index_SIZE, one_record, fidx) != one_record) {
	printf("%s\n", txt[17]);	/* "End of messages " */
    } else if (rewind(fmsg), fseek(fmsg, Index.Offset, SEEK_SET) != SUCCESS) {
	printf("%s !\n", txt[16]);	/* "seek error" */
    } else {
	ret_code = SkNumRead();	/* load buffer with new message */
    }
    return ret_code;
}


/*
 * GoToNum - go to the msg number represented by the string and print it.
 *  ( Internal count, not BBS numbers ! )
 */
void
GoToNum(const char *str)
{
    if (SeekNum(str))
	Display(NEXT, NULL, 0);
}
/* Flag if these files are open */
static atp_BOOL_T FilesOpen  = FALSE;  

/*
 * get_FilesOpen - returns TRUE if any files are open.
 */
atp_BOOL_T
get_FilesOpen(void)
{
    /* get_isempty => !FilesOpen */
    assert( !get_isempty() || !FilesOpen);
    return FilesOpen;
}

/*
 * fclose_fmsg_fidx - error checking wrapper to close fmsg and fidx streams.
 */
void
fclose_fmsg_fidx(void)
{
    if (fidx == NULL || fmsg == NULL) {
	fprintf(stderr, "%s:%d  fclose() on NULL pointer\n", __FILE__, __LINE__);
	abort();
    }
    fclose(fidx);
    fclose(fmsg);
    FilesOpen = FALSE;
    /*@i1*/ fidx = fmsg = NULL;
}

/*
 * StripRSpace, strip white space on right side of string.
 */
ATP_INLINE void
StripRSpace(const char *limit, char *ptr)
{
    /*@-strictops */
    while ( limit <= ptr && (*ptr == SPC_CHAR || *ptr == TAB_CHAR)) { /*@=strictops */
	*ptr = NUL_CHAR;
	ptr--;
    }
}

#define TBUF_SIZE 300
/*
 * fget - get a string like fgets but without any space or lf on the right.
 */
atp_BOOL_T
fget(char *s, const int n, FILE *fp)
{
    atp_BOOL_T ret_code = FALSE;
    int ch1 = EOF, ct = 0; 
    char *p2, tbuf[TBUF_SIZE + 1];

    if (s != NULL && n <= TBUF_SIZE) {
	p2 = tbuf;
	while (ct < (n - 1) && ct < TBUF_SIZE) {	/* get 'n' chars */
	    ch1 = fgetc(fp);
	    if (ch1 == EOF || ch1 == (int)'\n')
		ct = TBUF_SIZE;
	    else if (ch1 != (int)'\r') {
		*p2 = (char) ch1;
		ct++;
		p2++;
	    }
	}
	*p2 = NUL_CHAR;
	StripRSpace(tbuf, --p2);
	while (ct < TBUF_SIZE) {	/* keep getting chars till end of line is found */
	    ch1 = fgetc(fp);
	    if (ch1 == EOF)
		break;
	    if (ch1 == (int)'\n')
		break;
	    ct++;
	}
	/*@-usedef */
	strcpy(s, tbuf);
	/*@=usedef */
	if (ch1 != EOF)
	    ret_code = TRUE;
    }
    return ret_code;
}


/*
 * GetConfFin, finish up for GetConf().
 */
ATP_INLINE atp_ERROR_T
GetConfFin(FILE *t_fidx, char *tmp, const int num)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    FILE *t_fmsg;
    cnf_path(tmp, ConfNumbers[num]);
    if ((t_fmsg = fopen(tmp, "r+b")) == NULL) {
	fclose(t_fidx);
	/* "Error Reading file" */
	printf("%s %s ! \n", txt[58], tmp);
    } else if (fread(&Index, index_SIZE, one_record, t_fidx) != one_record) {	/* Go  to Last message read */
	printf("Can't read index in GetConf()\n");
	fclose(t_fmsg);
	fclose(t_fidx);
    } else {
	if (FilesOpen) {
	    fclose_fmsg_fidx();
	}
	fmsg = t_fmsg;
	fidx = t_fidx;
	FilesOpen = TRUE;
	if (Index.LastRead == VIRGINDX) {
	    /* virgin index, seek first message */
	    rewind(fidx);
	} else
	    fseek(fidx, (long) Index.LastRead * (long) IDXSIZE, SEEK_SET);
	set_CurConf(num, scnf_GetConf);
	FirstDone = FALSE;
	ret_code = ATP_OK;
    }
    return ret_code ;
}


/*
 * GetConfStat, stat conference before joining.
 */
ATP_INLINE atp_ERROR_T
GetConfStat(FILE *t_fidx, char *tmp, const int num)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    struct stat stats;

    if (stat(tmp, &stats) != SUCCESS) {
	red();
	perror("GetConfStat()");
	fclose(t_fidx);
    } else if ((TotMsg = (long int) (stats.st_size / IDXSIZE)) < 1L) {
	red();
	/* "No mail on conf" */
	printf("%s %s\n", txt[44], tmp);
	fclose(t_fidx);
    } else
	ret_code = GetConfFin(t_fidx, tmp, num);
    return ret_code;
}


/*
 * GetConf - join conference and open global files etc.
 */
atp_ERROR_T
GetConf(const int num)
{
    atp_ERROR_T ret_code = ATP_ERROR;

    if (!get_isempty()) {
	char tmp[MAXPATHS];
	FILE *t_fidx;
	assert(ConfNumbers != NULL);
	idx_path(tmp, ConfNumbers[num]);
	if (access(tmp, F_OK) == FAILURE) {	/* File does not exists, no mail. */
	    red();
	    /* "No mail on conf" */
	    printf("%s %s\n", txt[44], tmp);
	} else if ((t_fidx = fopen(tmp, "r+b")) == NULL) {	/* Ok file exists, read it . */
	    /* "unable to read file" */
	    printf("%s %s\n", txt[49], tmp);
	} else {
	    ret_code = GetConfStat(t_fidx, tmp, num);
	}
    }
    return ret_code;
}



static atp_BOOL_T ReplyExist = FALSE;

/*
 * set_ReplyExist - sets boolean ReplyExist flag.
 */
void
set_ReplyExist(const atp_BOOL_T mode, const rexist_access_t perm)
{
/*@-strictops */
    if (rexist_AddReply <= perm && perm <= rexist_Chk4RepCnf)
	ReplyExist = mode;
#ifdef ATPDBG
    else
	assert(rexist_AddReply <= perm && perm <= rexist_Chk4RepCnf);
#endif
/*@=strictops */
}


#define REP_FILE_EXISTS Chk4RepPkt()
/*
 * PackReply, creates reply packets for uploading to BBS.
 */
static void
PackReply(void)
{
    char MsgFile[MAXPATHS];
    sprintf(MsgFile, "%s%c%s.msg", WorkPath, SEP, CurBoard);
    
    if ( REP_FILE_EXISTS || !Cnf2Msg(MsgFile)) {
	printf("No replies packed.\n");
    } else {
        /* make bbs.msg from 9001.cnf */
	char RepFile[MAXPATHS];
	char Command[MAXPATHS];
	char CONSPTR Archiver = get_atprc_str(archivr);
	assert( Archiver != NULL );
	yellow();
	high();
	sprintf(RepFile, "%s%s.rep", ReplyPath, CurBoard);
	printf("%s %s\n", txt[65], RepFile);	/* "packing replies in" */
	green();
	fflush(stdout);
	sprintf(RepFile, "%s%s.rep", ReplyPath, CurBoard);
	sprintf(Command, "%s %s", RepFile, MsgFile);
	fork_execvp(Archiver, Command);
	free_string(Archiver);
	ReplyExist = FALSE;
	do_unlink(MsgFile);
    }
}

/*
 * query_rep_exists - prompts user if he wishes to delete reply conference.
 */
void
query_rep_exists(void)
{
    if (ReplyExist) {
	char prompt[80];
	red();
	high();
	printf("%s\n%s %s.\n", txt[2], txt[3], CurBoard);	/* Warning, you have  */
	sprintf(prompt, "%s", txt[4]);	/* Replies,pack them ? */
	if (YesNo(YES, prompt))
	    PackReply();
    }
}


/*
 * puts_xlate, translate line from DOS to LINUX character set and output it.
 */
static void
puts_xlate(char *buf, char *tmp)
{
    /* translate DOS character set to LINUX character set */
    const atp_CODE_T chrset = get_charset();
    byte *ptr, *bptr;
    ptr = (byte *) buf;
    bptr = (byte *) tmp;
    *bptr = NUL_CHAR;
/*@-strictops */
    while (*ptr != (byte)NUL_CHAR) {
	if (graphics && vtspecial(*ptr)) {	/* use VT102 line graphics codes */
	    *bptr = '\016';
	    bptr++;
	    *bptr = codevt[(unsigned) *ptr];
	    bptr++;
	    *bptr = '\017';
	} else
	    *bptr = chrset == ISOLAT1 ? codelu[(unsigned) (*ptr)]
		: code7bit[(unsigned) (*ptr)];
/*@=strictops */
	ptr++;
	bptr++;
	*bptr = NUL_CHAR;
    }
    puts(tmp);
}


/*
 * view_lines, output lines for view().
 */
static void
view_lines(char *buf, char *tmp, FILE * fp)
{
    const atp_CODE_T chrset = get_charset();
    int line = 0;
    clear();
    cyan();
    printf("\n\n");
    while (fget(buf, 250, fp)) {
	line++;
	if (line > (get_ScrnRows() - 2)) {
	    line = 0;
	    if (!more(YES))
		break;
	    cyan();
	}
	if (/*@i1*/ chrset == CHRDOS)
	    puts(buf);
	else
	    puts_xlate(buf, tmp);
    }
}

/*
 * view - page a file to screen while prompting user with "more".
 *
 *  note that the tmp buffer must be large enough to
 *  handle all ansi escape sequences and VT102 line
 *  graphics sequences (or you will bomb on a seg fault).
 */
void
view(const char *Path, const char *File)
{
    const size_t VIEWBUF = 1000;
    char *tmp;
    if ((tmp = (char *) malloc(VIEWBUF)) == NULL) {
	/* malloc failure */
	fprintf(stderr, "%s %s:%d view()\n\n", txt[1], __FILE__, __LINE__);
    } else {
	const size_t VIEWLINE = 600;
	char *buf;
	FILE *fp;
	sprintf(tmp, "%s%c%s", Path, SEP, File);
	if ((fp = fopen(tmp, "rb")) == NULL) {
	    red();
	    printf("%s %s\n", txt[67], tmp);	/* "No file" */
	} else if ((buf = (char *) malloc(VIEWLINE)) == NULL) {
	    /* malloc failure */
	    fprintf(stderr, "%s %s:%d view()\n\n", txt[1], __FILE__, __LINE__);
	    fclose(fp);
	} else {
	    view_lines(buf, tmp, fp);
	    fclose(fp);
	    free_buffer(buf, VIEWLINE);
	}
	free_buffer(tmp, VIEWBUF);
    }
}



/*
 * EmptyMsg - informs the user that there is no BBS currently loaded.
 */
void
EmptyMsg(void)
{
    red();
    high();
    printf("%s ", txt[68]);	/* "Hey !" */
    clear();
    magenta();
    printf("%s\n", txt[69]);	/* There is no bbs loaded !" */
    printf("%s...\n", txt[70]);	/* use 'read' 'load' or 'help' command" */
    clear();
}

/* 
 * ShiftLeft - shifts a string to the left.
 *      shifts a string left at p1, if n==2 then string is shifted
 *      two spaces left
 */
void
ShiftLeft(char *p1, const int n)
{
    char *p2;

    p2 = p1;
    p2++;
    if (n == 2)
	p2++;
    while (/*@i1*/ *p2 != NUL_CHAR) {
	*p1 = *p2;
	p2++;
	p1++;
    }
    *p1 = NUL_CHAR;
    return;
}


/* 
 * StripDel - scans a line removing backspace and delete characters.
 */
void
StripDel(char *tp)
{
    atp_BOOL_T done = FALSE;
    char *p;
    p = tp;
    while (/*@i1*/ *p != NUL_CHAR && !done) {
	if (p == tp) {		/* first character on line */
	    switch (*p) {
	    case BS_CHAR:	/* backspace */
	    case DEL_CHAR:
		ShiftLeft(p, 1);	/* del       */
		break;
	    case CR_CHAR:	/* carriage return */
	        /*@-casebreak*/ 
		*p = NUL_CHAR;  /* no break needed here ... */
	    case NUL_CHAR:      /* yes, fall through is intentional! */
		done = TRUE;
	        /*@=casebreak*/ 
		break;
	    default:
		p++;
	    }
	} else {
	    switch (*p) {
	    case BS_CHAR:
	    case DEL_CHAR:
		p--;
		ShiftLeft(p, 2);
		break;
	    case CR_CHAR:	/* carriage return */
	        /*@-casebreak */ 
		*p = NUL_CHAR;
	    case NUL_CHAR:    /* no break needed, fall through intentional */
		done = TRUE;
	        /*@=casebreak */ 
		break;
	    default:
		p++;
	    }
	}
    }
    return;
}


/*
 * tup8bit, table for international capitalization.
 */
static const byte tup8bit[] =
{

    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,

    0x80, 0x9a, 0x90, 0x83, 0x8e, 0x85, 0x8f, 0x80,	/* 87 */
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,	/* 8f */
    0x90, 0x92, 0x92, 0x93, 0x99, 0x95, 0x96, 0x97,	/* 97 */
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,	/* 9f */
    0xa0, 0xa1, 0xa2, 0xa3, 0xa5, 0xa5, 0xa6, 0xa7,	/* a7 */
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,	/* af */
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,	/* b7 */
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,	/* bf */
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,	/* c7 */
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,	/* cf */
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,	/* d7 */
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,	/* df */
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,	/* e7 */
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,	/* ef */
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,	/* f7 */
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

static char find_buf[FIND_BUF_SIZE + 1];
static struct fentry *entry = NULL;         /* remembers last link */

/*
 * set_list_tail, point to end of fentry linked list.
 */
ATP_INLINE void
set_list_tail(struct fentry **sptr)
{
    if (*sptr == NULL)
        entry = NULL;
}


/*
 * set_target, point to search string.
 */
ATP_INLINE byte *
set_target(byte *needle)
{
    if (needle == NULL)
        needle = (byte *) find_buf;
    return needle;
}


/*
 * new_fentry, allocate an new item on linked list of found text.
 */
ATP_INLINE atp_BOOL_T
new_fentry(char *haystack, struct fentry **sptr)
{
    atp_BOOL_T malloc_failure = FALSE;
    struct fentry *tentry;      /* temporary pointer   */

    /* return ptr to position in string haystack where needle found */
    if ((tentry = (struct fentry *) malloc(sizeof(struct fentry))) != NULL) {
        tentry->fnext = NULL;
        tentry->findpt = (char *) haystack;
        if (*sptr != NULL) {
            assert(entry != NULL);
            entry->fnext = tentry;
        } else
            *sptr = tentry;     /* first in linked list */
        entry = tentry;
    } else {
        printf("findstr(), %s\n", txt[1]);
        (void) sleep(3);
        malloc_failure = TRUE;
    }
    return malloc_failure;
}

/* 
 * findstr - search conference for text string.
 * function to look for search string needle in text string haystack. 
 */
struct fentry *
findstr(byte * haystack, byte * needle, struct fentry **sptr)
{
    byte *hay = haystack, *ndl;
    atp_BOOL_T new_has_failed = FALSE;

    assert(sptr != NULL && haystack != NULL);
    set_list_tail(sptr);
    ndl = needle = set_target(needle);

    /* 
     *  while the end of string haystack has not been reached, 
     *  look for search string needle, strip off lowercase if needed, 
     *  while needle matches within haystack, DTS pre-upshifted string
     *  needle in calling routine 
     *//*@-strictops */
    while (*haystack != (byte) NUL_CHAR) {
        while (*hay != (byte) NUL_CHAR && tup8bit[(unsigned) *hay] == *ndl) {
            hay++;              /* advance string compare pointers */
            ndl++;
            /* if end of search string needle is reached, *//*@=strictops */
            if (*ndl == (byte) NUL_CHAR
                &&(new_has_failed = new_fentry((char *)haystack, sptr))==TRUE)
                    break;
        }
        if ( /*@i1 */ *hay == (byte) NUL_CHAR || new_has_failed)    /* if end of string haystack reached prematurely, */
            break;              /* return last entry in linked list */
        hay = ++haystack;       /* if mismatch occurs, advance haystack string pointer */
        ndl = needle;           /* and reset search string pointer needle */
    }
    return entry;               /* if end of string haystack reached, */
}                               /* return last entry in list */


static volatile atp_BOOL_T searchflag ;

#ifdef __TURBOC__
#pragma argsused
#endif
/*@-paramuse */
/* 
 * stopfind - signal handler allows ^C aborting findtxt() during search.
 */
void
stopfind(int dummy_arg)
{
#ifndef HAVE_SIGACTION
    signal(SIGINT, stopfind);
#endif
    searchflag = FALSE;
}
/*@=paramuse */

/* this has to be static for proper initialization with old compilers */
static const size_t hdr_offst[3] = {HAuthor, HForWhom, HSubject};

/* 
 * do_search_buf, search buffer message header then message body.
 */
ATP_INLINE void
do_search_buf(struct fentry **sentry_adr )
{
    char tchar = NUL_CHAR;
    int i;

    /* search header author, recipient, and subject fields */
    for (i = 0; i < 3; i++) {
	tchar = rbuf[hdr_offst[i] + field_len];
	rbuf[hdr_offst[i] + field_len] = NUL_CHAR;
	(void) findstr((byte *) (rbuf + hdr_offst[i]), (byte *) find_buf, sentry_adr);
	rbuf[hdr_offst[i] + field_len] = tchar;
    }

    /* search message body */
    (void) findstr((byte *) (rbuf + header_SIZE), (byte *) find_buf, sentry_adr);
}

static size_t find_len = 0;
static long find_nmb;		/* message count for search */

/*
 * do_search, called after findtxt performs setup to execute search.
 */
static void
do_search(void)
{
    struct fentry *tentry, *sentry = NULL;

    /* setup ^C handler to enable abort search feature */
    searchflag = TRUE;
    sig_stopfind();

    /* walk through message base */
    while (find_nmb <= TotMsg && searchflag) {
	char nmbuf[30];
	sprintf(nmbuf, "%ld", find_nmb);
	find_nmb++;
	/* seek to next message in conference */
	if (!SeekNum(nmbuf)) {
	    break;
	}
	/* search buffer for text */
	do_search_buf( &sentry ) ;
	if (sentry != NULL) {
	    Display(NEXT, sentry, find_len);
	    break;
	}
    }

    /* reset signal handler */
    sig_ignore(SIGINT);

    /* free linked list of found text */
    while (sentry != NULL) {
	const size_t sentry_size = sizeof(struct fentry);
	tentry = sentry->fnext;
	free_buffer(sentry, sentry_size);
	sentry = tentry;
    }
}


/* 
 * iso2dosup - translate latin1 or msdos string to msdos upper case.
 */
ATP_INLINE size_t
iso2dosup(byte * dpt)
{
    const atp_CODE_T chrset = get_charset();
    size_t ct = 0;
    while (/*@i1*/ *dpt != (byte)NUL_CHAR && ct < (size_t) 256) {
	/* must search with DOS char. set! */
	if (/*@i2*/ chrset == ISOLAT1 && *dpt >= HIGH_ASCII) {
	    /* translate back to DOS set */
	    *dpt = codepc[(unsigned)*dpt];
	}
	*dpt = tup8bit[(unsigned)*dpt];	/* international capitalization */
	ct++, dpt++;
    }
    return ct;
}


/*
 * findtxt - wrapper sets up for do_search(); valid modes are FIND and NEXT.
 */
void
findtxt(char *fndargs, const read_command_t mode )
{
/*@-strictops */
    assert(mode == FIND || mode == NEXT);
    if (mode == FIND) {
	while (*fndargs != SPC_CHAR && *fndargs != NUL_CHAR)
	    fndargs++;
	if (*fndargs != NUL_CHAR) {
	    fndargs++;
/*@=strictops */
	    if (strlen(fndargs) >= (size_t) FIND_BUF_SIZE)
		*(fndargs+FIND_BUF_SIZE) = NUL_CHAR;
	    strcpy(find_buf, fndargs);
	    find_len = iso2dosup((byte *) find_buf);
	} else {
	    find_len = 0;
	    find_buf[0] = NUL_CHAR;
	}
	find_nmb = 1L;
    }
    if (/*@i1*/ find_buf[0] != NUL_CHAR && 1L <= find_nmb && find_nmb <= TotMsg)
	do_search();
}


/*
 * hprint - print message header entries and highlighting if need be.
 */
struct fentry *
hprint(char *hbuf, size_t off_set, size_t len, struct fentry **bptr, const size_t blen)
{
    const atp_CODE_T chrset = get_charset();
    char linbuf[200], *pt, *chkptr;
    byte *ptr = (byte *) (hbuf + off_set); 
    struct fentry *tpr = *bptr ;
    atp_BOOL_T vidon = FALSE;
    size_t i; 

    linbuf[0] = NUL_CHAR ;
    pt = linbuf;
    chkptr = linbuf + 180;	/* security for buffer overun */
    for (i = 0; i < len; i++) {
/* begininning of highlighting code */
	if (blen != (size_t)0 && tpr != NULL) {
	    if (ptr == (byte *) tpr->findpt) {	/* Beginning of the found string */
		if (ansi) {
		    strcpy(pt, "\033[7m");
		    pt += 4;	/* reverse video on */
		} else {	/* termcap */
#ifdef HAVE_LIBTERMCAP
		    pt += tputs_cpy(pt, rev_on);
#else
		    strcpy(pt, "**");
		    pt += 2;
#endif
		}
		vidon = TRUE;
	    }
	    if (ptr == (byte *) (tpr->findpt + blen)) {		/* End of the found string */
		if (ansi) {
#ifdef VT220
		    if (stricmp(atp_termtype, "xterm") != SUCCESS) {
			strcpy(pt, "\033[27m");		/* reverse video off */
			pt += 2;
		    } else
#endif
			strcpy(pt, "\033[m");	/* reverse video off */
		    pt += 3;
		} else {
#ifdef HAVE_LIBTERMCAP
		    pt += tputs_cpy(pt, rev_off);
#else
		    strcpy(pt, "**");
		    pt += 2;
#endif
		}
		tpr = tpr->fnext;
		vidon = FALSE;
	    }
	}
/* end of highlighting code */
/*@-strictops */
	if (*ptr != (byte)NUL_CHAR) {
	    if (chrset != CHRDOS) {
		if (vtspecial(*ptr) && graphics) {	/* use VT102 line graphics codes */
/*@=strictops */
		    *pt++ = '\016';
		    *pt++ = codevt[(unsigned) (*ptr)];
		    *pt = '\017';
		} else if (/*@i1*/ chrset == ISOLAT1)
		    *pt = codelu[(unsigned) (*ptr)];	/* use iso lookup table */
		else		/* use 7 bit character set */
		    *pt = code7bit[(unsigned) (*ptr)];
	    } else
		*pt = *ptr;
	} else
	    break;		/* done */
	if (pt > chkptr)
	    break;		/* buffer full, done */
	pt++, ptr++;
    }
    if (vidon) {		/* turn off reverse video */
	if (ansi) {
#ifdef VT220
	    if (stricmp(atp_termtype, "xterm") != SUCCESS) {
		strcpy(pt, "\033[27m");		/* reverse video off */
		pt += 2;
	    } else
#endif
		strcpy(pt, "\033[m");	/* reverse video off */
	    pt += 3;
	}
#ifdef HAVE_LIBTERMCAP
	else {			/* termcap */
	    pt += tputs_cpy(pt, rev_off);
	}
#endif
    }
    *pt = NUL_CHAR;			/* mark end of string */
    printf("%s", linbuf);
    fflush(stdout);
    return (tpr);
}

