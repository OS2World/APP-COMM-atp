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
display.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "reader.h"
#include "readlib.h"
#include "makemail.h"
#include "qlib.h"
#include "ansi.h"

/*
 * START OF NEW DISPLAY ROUTINES
 */

/* length of QWK author, recipient, and subject fields */
const size_t field_len = (size_t) QWK_FIELD_LEN ;

/*
 * nfprint, print a string not null-terminated in file fp.
 */
static void
nfprint(FILE * fp, const char *str, size_t len)
{
    assert(fp != NULL);
    while (len > (size_t)0) {
	fputc(*str, fp);
	str++;
	len--;
    }
}
#define nprint(s,n) nfprint(stdout,(s),(n))
static const char CONSPTR Space = "\t\t";

/*
 * prn_hdr_first_line, prints to a file, called by PrintHeader().
 */
static void
prn_hdr_first_line(char *Header, FILE * fp)
{
    fprintf(fp, "\n%s", txt[19]);	/* "from:" */
    nfprint(fp, Header + HAuthor, field_len);
    fprintf(fp, Space);
    fprintf(fp, "%s", txt[23]);	/* "number:" */
    nfprint(fp, Header + HNumMsg, (size_t)7);
}

/*
 * prn_hdr_second_line, prints to a file, called by PrintHeader(). 
 */
static void
prn_hdr_second_line(char *Header, FILE * fp)
{
    fprintf(fp, "\n%s", txt[20]);	/* "to :" */
    nfprint(fp, Header + HForWhom, field_len);
    fprintf(fp, Space);
    fprintf(fp, "%s", txt[24]);	/* "ref#" */
    nfprint(fp, Header + HRefMsg, (size_t)7);
    fprintf(fp, "\n%s", txt[21]);	/* "subj." */
    nfprint(fp, Header + HSubject, field_len);
    fprintf(fp, Space);
    fprintf(fp, "%s", txt[25]);	/* "Conf : " */
    fprintf(fp, "%s", ConfNames[findCindex(readCnum(Header + HBinConfN))]);
    fprintf(fp, "\n%s", txt[22]);	/* "date :" */
    nfprint(fp, Header + HMsgDate, (size_t)8);
    fprintf(fp, "%s", txt[26]);	/* "time:" */
    nfprint(fp, Header + HMsgTime, (size_t)5);
    /* stored 0 based, printed 1 based. */
    fprintf(fp, "\t[%ld/%ld]", get_MsgNum() + 1L, get_TotMsg());
}

/*
 * PrintHeader - prints the Qmail message header in file fp.
 */
void
PrintHeader(FILE * fp)
{
    char Header[HDRSIZE];
    int i;
    byte *pntr;
    (void)memcpy(Header, rbuf, header_SIZE);
    pntr = (byte *) Header + HMsgDate;

    /* translate header info */     /*@-strictops */
    if (get_charset() == ISOLAT1) { /*@=strictops */
	for (i = HMsgDate; i < HPassWrd; i++, pntr++)
	    *pntr = codelu[(unsigned) *pntr]; /*@-strictops */
    } else if (get_charset() == CHR7BIT) {    /*@=strictops */
	for (i = HMsgDate; i < HPassWrd; i++, pntr++)
	    *pntr = code7bit[(unsigned) *pntr];
    }
    /* First line */
    prn_hdr_first_line(Header, fp);

    /* Second line */
    prn_hdr_second_line(Header, fp);
    /*@-strictops */
    if (Header[HStatus] == '+' || Header[HStatus] == '*') { /*@=strictops */
	fprintf(fp, "\t\t");
	fprintf(fp, "  %s\n", txt[27]);		/* "PRIVATE MESSAGE" */
    }
    fprintf(fp, "\n\n");
}

#define SBUF_LEN 100
static char SubjBuf[SBUF_LEN];   /* Long subject for PCBoard */

/*
 * subj_is_long - returns TRUE if the long subject buffer is full.
 */
atp_BOOL_T
subj_is_long(void)
{       /*@-strictops */
	return SubjBuf[0] == NUL_CHAR ? FALSE : TRUE ; /*@=strictops */
}

/*
 * get_long_subj - returns a strdup() of the long subject buffer.
 */
char *
get_long_subj(void)
{
    char *tp;
    assert(LSUBJ_LEN < SBUF_LEN);
    SubjBuf[LSUBJ_LEN] = NUL_CHAR;
    tp = strdup(SubjBuf);
    test_fatal_malloc(tp, __FILE__, __LINE__);
    return tp ;
}

/*
 * put_long_subj, called from PutHdrScan().
 */
static void
put_long_subj(size_t blen)
{
    const size_t fentry_size = sizeof(struct fentry);
    struct fentry *dum1 = NULL, *dum2;

    /* find string in long subject buffer */
    if (blen != (size_t)0 )
	findstr((byte *) SubjBuf, NULL, &dum1);
    
    /* print long Subject */
    hprint(SubjBuf, (size_t)0, strlen(SubjBuf), &dum1, blen);

    /* free linked list */
    while (dum1 != NULL) {
	dum2 = dum1->fnext;
	free_buffer(dum1, fentry_size);
	dum1 = dum2;
    }
}

static char *UserName ;
static size_t UserLen ; 
/*
 * PutHdrScan, fully display each message header, called from PutHeader().
 */
static void
PutHdrScan(struct fentry **sptr, size_t blen, const char *Header)
{
    struct fentry **dum1, *dum2 = NULL;
    /* don't dereference null pointer */
    dum1 = (sptr == NULL) ? &dum2 : sptr;

    /* print first line */
    {
	high();
	green();
	printf("\n%s", txt[19]);	/* " From :" */

	/* print Author */
	yellow();
	*dum1 = hprint(rbuf, (size_t)HAuthor, field_len, sptr, blen);

	/* print name of conference */
	{
	    const int conf_array_idx = findCindex(readCnum(Header + HBinConfN));
#if 0 && defined(ATPDBG)
/*xxx !! */ if (!(0 <= conf_array_idx && conf_array_idx < RCONF_IDX)) {
		printf("\nconf_array_idx: %d  LastConf: %d\n", conf_array_idx, LastConf);
		(void)sleep(4);
	    }
	    assert(0 <= conf_array_idx && conf_array_idx < RCONF_IDX);
#endif
	    magenta();
	    if (conf_array_idx < 0)
		printf(" : %s", txt[28]);	/* unknown conference */
	    else
		printf(" : %s", ConfNames[conf_array_idx]);
	}
    }

    /* print second line */
    {
	green();
	printf("\n%s", txt[20]);	/* "To:" */
	red();
	if (strnicmp(UserName, Header + HForWhom, UserLen) == SUCCESS)
	    blink();
	/* print ForWhom */
	*dum1 = hprint(rbuf, (size_t)HForWhom, field_len, sptr, blen);
	clear();
	green();
	high();
	printf(" : %s", txt[23]);	/* "Number" */
	red();
	nprint(Header + HNumMsg, (size_t)7);
	green();
	printf(" %s", txt[24]);	/* " Ref # " */
	yellow();
	nprint(Header + HRefMsg, (size_t)7);
	green();
	printf("\n%s", txt[22]);	/* "date:" */
	red();
	nprint(Header + HMsgDate, (size_t)8);
	green();
	printf("%s", txt[26]);	/* "time:" */
	red();
	nprint(Header + HMsgTime, (size_t)5);
	clear();
	printf("    : ");
	blue();
	/* MsgNum is stored 0 based, but printed 1 based. */
	printf("Index  : %ld/%ld", get_MsgNum() + 1L, get_TotMsg());

	/* get message status and display it */
	{
	    const byte statu = Header[HStatus];
	    /*@-strictops */
	    if (statu == (byte) '+' || statu == (byte) '*' || statu >= HIGH_ASCII) {
		red();
		printf("\t");
		high();
		blink();
		if (statu >= HIGH_ASCII)	/*@=strictops */
		    printf(" %s%c", txt[105], BELL);	/* "KILLED MESSAGE" */
		else
		    printf(" %s%c", txt[27], BELL);	/* "PRIVATE MESSAGE" */
	    }
	}
    }

    /* print subject line */
    {
	clear();
	green();
	printf("\n%s", txt[21]);	/* "Subject" */
	cyan();			/*@-strictops */
	if (SubjBuf[0] == NUL_CHAR) {	/*@=strictops */
	    /* print normal subject */
	    *dum1 = hprint(rbuf, (size_t)HSubject, field_len, sptr, blen);
	} else {
	    /* print long subject */
	    put_long_subj(blen);
	}
	cyan();
	printf("\n\n");
    }
}


/*
 * PutHdrQuick, displays abbreviated message headers, one per line.
 */
static void
PutHdrQuick(const char *Header)
{
    byte statu;
    high();
    yellow();
    nprint(Header + HAuthor, field_len);
    red();
    if (strnicmp(UserName, Header + HForWhom, strlen(UserName)) == SUCCESS
	&& get_CurConf() != PCONF_IDX)
	blink();
    nprint(Header + HForWhom, field_len);
    clear();
    red();
    nprint(Header + HSubject, field_len);
    blue();
    printf("%ld", get_MsgNum() + 1L);	/* 0 based, print 1 based. */
    statu = Header[HStatus]; /*@-strictops */
    if (statu == (byte)'+' || statu == (byte)'*' || statu >= HIGH_ASCII) {
	red();
	high();
	blink();
	if (statu >= HIGH_ASCII) /*@=strictops */
	    printf("%c\n", 'K');	/* "KILLED MESSAGE" */
	else
	    printf("%c\n", 'P');	/* "PRIVATE MESSAGE" */
    } else
	printf("\n");
    clear();
    cyan();
}



/*
 * PutHeader - prints the Qmail message header pointed by buf.
 */
void
PutHeader(const read_command_t mode, struct fentry **sptr, size_t blen)
{
    char Header[HDRSIZE];
    int i;
    byte *pntr;                            /*@-strictops */
    assert(mode == SCAN || mode == QUICK); /*@=strictops */
    UserName = get_cntrl_str(usrnm);
    UserLen = strlen(UserName);
    (void)memcpy(Header, rbuf, header_SIZE);
    pntr = (byte *) Header + HMsgDate; /*@-strictops */
    if (get_charset() == ISOLAT1) {
	for (i = HMsgDate; i < HPassWrd; i++, pntr++)	/* translate header info */
	    *pntr = codelu[(unsigned) *pntr];
    } else if (get_charset() == CHR7BIT) { /*@=strictops */
	for (i = HMsgDate; i < HPassWrd; i++, pntr++)	/* translate header info */
	    *pntr = code7bit[(unsigned) *pntr];
    }
    /* check for lowercase letters in to/from fields */
    test_for_caps(Header);

    /* normal scan */ /*@-strictops */
    if (mode == SCAN) /*@=strictops */
	PutHdrScan(sptr, blen, Header);
    /* quick scan */
    else {
	SubjBuf[0] = NUL_CHAR;
	PutHdrQuick(Header);
    }
    free_string(UserName);
    UserName = NULL;
}


/* static const byte norm[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\140abcdefghijklmnopqrstuvwxyz"; */
static const byte rot[] = "@NOPQRSTUVWXYZABCDEFGHIJKLM[\\]^_\140nopqrstuvwxyzabcdefghijklm";

/* 
 * rot_buf, used by Display() to decode rot13 messages.
 */
static void
rot_buf(byte * rb_ptr, const byte * rb_badptr)
{
    const byte RB_MASK = (byte)0x3F ; 
    int B;
    while (rb_ptr < rb_badptr) {                               /*@-strictops */
	if ((byte)'A' <= *rb_ptr && *rb_ptr <= (byte)'z') {
	    B = (*rb_ptr & RB_MASK);  /* mask out high bits */ /*@=strictops */
	    assert(0 < B && B < 59);
	    *rb_ptr = rot[B];
	}
	rb_ptr++;
    }
}


static const int quote_depth = 6;

/*
 * IsQuoted, returns TRUE if the string contains ">" in the 6 left chars.
 */
static atp_BOOL_T
IsQuoted(const int c, const byte * str)
{
    atp_BOOL_T ret_code = FALSE;
    int i = 0; /*@-strictops */
    for (; *str != (byte)NUL_CHAR && !ret_code && i < quote_depth; str++,i++) /*@=strictops */
	ret_code = ((const int)*str == c) ? TRUE : FALSE;
    return ret_code;
}


static atp_BOOL_T xterm_flag;
static char *start_of_text ;

/*
 * get_sot - returns pointer to message start_of_text.
 */
char *
get_sot(void)
{
	return start_of_text ;
}

/*
 * loop_pre_hdr, setup for output display.
 */
static void
loop_pre_hdr(read_command_t mode, struct fentry **bptr, const size_t blen)
{
    if (stricmp(atp_termtype, "xterm") == SUCCESS)
	xterm_flag = TRUE;   /*@-strictops */
    if (mode != SCAN)        /*@=strictops */
	CLSCRN();
    /* support for PC Board BBS long subject lines */
    SubjBuf[0] = NUL_CHAR;
    start_of_text = rbuf + header_SIZE;
    if (PCBLONG)
	Check4LongSubj();
    PutHeader(SCAN, bptr, blen);
}

/* used by Display for rot13 */
static atp_BOOL_T rot_13 = FALSE;

/*
 * rot_toggle - toggles rot13 flag when called by ReadShell().
 */
void
rot_toggle(void)
{
    rot_13 = TRUE;
    if (get_FirstDone())
	Display(NEXT, NULL, 0);
    else
	ReadNext(NEXT);
    rot_13 = FALSE;
}


/*
 * loop_pre_buf, setup for output display.
 */
static byte *
loop_pre_buf(byte CONSPTR badptr)
{
    /* point to last char of message */
    byte *ptr = badptr - 1;
    ptr--;

    /* strip trailing spaces and carriage returns */ /*@-strictops */
    while (*ptr == (byte)NUL_CHAR || *ptr == (byte)SPC_CHAR || *ptr == (byte)'\n') { /*@=strictops */
	*ptr = NUL_CHAR;
	/* don't munge header */
	if ((--ptr) < (byte *) (rbuf + header_SIZE))
	    break;
    }

    /* insure message terminates with newline */
    if ((++ptr) < badptr)
	*ptr = '\n';

    /* point to beginning of message text, skipping header */
    ptr = (byte *)start_of_text ;

    /* if rot13 flag is set, decode buffer */
    if (rot_13)
	rot_buf(ptr, badptr);
    cyan();
    return ptr;
}

/*
   Display(), prints the current message stored in rbuf message buffer.
   'bptr' points to a linked list of strings found by "find" routine.
   'blen' is the length of the string found. 
   set bptr to NULL and blen to 0 when calling normally. 
   'mode' is:   0       display full message
   SCAN    display message header only
   KILL    display one page only   
   NUKE    display one page only
   PRIVATE display one page only

   algorithm: display the message header, then read in characters of
   message text, building output lines. Start at beginning of rbuf
   working toward the end, reading one byte at at time. While doing
   this, if 'bptr' is not NULL, check for a match with the current
   position in rbuf. If 'bptr' and ptr match, highlight "found" text
   by putting ansi reverse video screen codes in the line buffer. Then
   after 'blen' characters have been processed turn off reverse video by
   putting cancel screen codes in the line buffer. When the line buffer
   is full, dump it to screen. Check to make sure that no more than one
   screen full of lines is displayed at a time by promptng user as each
   screen is filled.

 */
/* globals used by Display() routines */
#define OUTBUF_SIZE 800
static byte OutLineBuf[OUTBUF_SIZE];

/*
 * more_output, terminate output line with '\n' and prompt for more output.
 */
static atp_BOOL_T
more_output(const byte *output_line)
{
    int c_cnt = 0;
    /*@-strictops */
    while (*output_line != (byte) NUL_CHAR)
	output_line++, c_cnt++;
    if (c_cnt != 0 && *--output_line != (byte) '\n')	/*@=strictops */
	printf("\n");
    return more(YES);
}

/*
 * loop_out, the work horse for Display().
 */
static atp_BOOL_T
loop_out(read_command_t mode, struct fentry *bptr, const size_t blen, byte CONSPTR badptr, byte * ptr, int *line_ct)
{
    atp_BOOL_T QuotedColor = FALSE;
    atp_BOOL_T vidon = FALSE;
    const atp_CODE_T chrset = get_charset();
    const int line_limit = OUTBUF_SIZE - 30;
    const int spaces_per_tab = 8;
    int count = 0, bufct = 0, spc_ct = 0;
    byte CONSPTR linbuf = OutLineBuf;
    byte *pt = linbuf;
    int line = 5;		/* offset output by six for header */

    *pt = NUL_CHAR;			/* initialize line buffer */

    while (ptr < badptr) {
	assert((byte *) (rbuf + header_SIZE) <= ptr);
	/* skip over bells and null characters */        /*@-strictops */
	if (*ptr == (byte)NUL_CHAR || (*ptr == (byte)BEEP && silent)) {
	    ptr++;                                       /*@=strictops */
	} else {
            const int rows = get_ScrnRows();
	    const int columns = get_ScrnCols()-1;
	    /* begin highlighting code */
	    if (bptr != NULL && blen > 0) {	/* re-synch pointers */
		while (bptr != NULL && (((char *) ptr) > (bptr->findpt + blen))) {
		    bptr = bptr->fnext;
		}
	    }
	    if (bptr != NULL && blen > 0) {
		int ht;
		if (ptr == (byte *) bptr->findpt) {	/* Beginning of the found string */
		    if (ansi) {	/* highlight found string */
			strcpy((char *) pt, A_REV_ON);
			bufct += 4, pt += 4;
		    } else {	/* termcap */
#ifdef HAVE_LIBTERMCAP
			ht = tputs_cpy((char *) pt, rev_on);
#else
			strcpy((char *) pt, "**");
			ht = 2;
#endif
			bufct += ht, pt += ht;
		    }
		    vidon = TRUE;
		}
		if (ptr == (byte *) (bptr->findpt + blen)) {	/* End of the found string */
		    if (ansi) {	/* turn off highlighting */
#ifdef VT220
			/* If NOT an xterm, use vt220 stuff */
			if (!xterm_flag) {
			    strcpy((char *) pt, VT220_REV_OFF);
			    bufct += 5;
			    pt += 5;
			} else
#endif
			{
			    strcpy((char *) pt, A_REV_OFF);
			    bufct += 3, pt += 3;
			}
		    } else {	/* non-ansi */
#ifdef HAVE_LIBTERMCAP
			ht = tputs_cpy((char *) pt, rev_off);
#else
			strcpy((char *) pt, "**");
			ht = 2;
#endif
			bufct += ht, pt += ht;
		    }
		    vidon = FALSE;
		}
	    }
/* end highlighting code */

	    /* blow long line saftey valve to avoid buffer overrun */
	    if (bufct > line_limit)
		*ptr = '\n';

	    /* begin fill output line buffer */               /*@-strictops */
	    if (*ptr == (byte)'\t') {	/* expand tabs */     /*@=strictops */
		/* calculate number of spaces to pad */
		spc_ct = spaces_per_tab - (count % spaces_per_tab);
		/* do housekeeping, track various counts */
		count += spc_ct, bufct += spc_ct;
		if (count >= columns) {
		    count -= columns;
		    line++;
		}
		/* fill output buffer with expanded tab */
		while (spc_ct != 0) {
		    *pt = (byte) SPC_CHAR;
		    pt++, spc_ct--;
		} /*@-strictops */
	    } else if (*ptr != (byte)NUL_CHAR ) {/* Non 0 chars */
		if (chrset != CHRDOS) {
		    if (graphics && vtspecial(*ptr)) {	/* use VT102 line graphics codes */
			*pt++ = '\016'; /*@=strictops */
			*pt++ = codevt[(unsigned) (*ptr)];
			*pt = '\017';
			bufct += 2;               /*@-strictops */
		    } else if (chrset == ISOLAT1) /*@=strictops */
			*pt = codelu[(unsigned) (*ptr)];	/* use iso lookup table */
		    else	/* use 7 bit character set */
			*pt = code7bit[(unsigned) (*ptr)];
		} else
		    *pt = *ptr;
		pt++;
		count++;	/* Security for line length ... */
		bufct++;
	    }			/* end fill output line buffer */
	    /* begin process output line buffer to screen when ready */ /*@-strictops */
	    if (*ptr == (byte)'\n' || count >= columns) {             
		if (*ptr != (byte)'\n' && ((ptr + 1) < badptr) && *(ptr + 1) == (byte)'\n')
		    ptr++;	/* avoid to '\n' in a row */            /*@=strictops */
		count = bufct = 0;
		*pt = NUL_CHAR;
		line++;
		pt = linbuf;
		/* Process colors on quoted line */
		if (IsQuoted( (int)'>', pt) || IsQuoted( (int)'|', pt)) {
		    if (!color)
			high();                                    /*@-strictops */
		    while (*pt != (byte)'>' && *pt != (byte)'|') { /*@=strictops */
			(void)putchar(*pt);
			pt++ ;               /*@-strictops */
			assert((pt - linbuf) < quote_depth);
		    }                        /*@=strictops */
		    QuotedColor = TRUE;
		    if (color)
			red();	/* Quote char in red... */
		    (void)putchar(*pt);   
		    pt++ ;                           /*@-strictops */
		    if (*pt != (byte)SPC_CHAR)       /*@=strictops */ 
			(void)putchar(SPC_CHAR);
		    if (color)
			green();	/* Quoted line in green.. */
		    else
			clear();
		    fputs((char *) pt, stdout);
		    cyan();
		} else {
		    if (QuotedColor) {
			cyan();
			QuotedColor = FALSE;
		    }
		    fputs((char *) linbuf, stdout);
		}
		pt = linbuf;
	    }			/* end process output line buffer */
	    /* prompt user for more output (yes/no) */        /*@-strictops */ 
	    if ((++ptr) < badptr && *ptr != (byte)NUL_CHAR && line > (rows - ROW_OFFSET)) {
		line = 0;                                     
		if (mode == CROSS || mode == KILL || mode == NUKE || mode == PRIVATE || !more_output(linbuf))
		    break;                                    /*@=strictops */ 
		cyan();		/* restore default color */
	    }			/* end prompt user  */
	}
    }				/* end while ptr < badptr */

    /* flush output line buffer if needed */
    if (bufct != 0) {
	*pt = NUL_CHAR;
	printf("%s",(char *) linbuf);
    }
    assert(ptr <= badptr);
    *line_ct = line;
    return vidon;
}


/*
 * Display - page message buffer to the screen.
 */
void
Display(const read_command_t mode, struct fentry *bptr, const size_t blen)
{
    if (get_isempty()) {
	EmptyMsg();
    } else {
	/* first output header */
	loop_pre_hdr(mode, &bptr, blen);
        /*@-strictops */
	if (mode != SCAN) {     /*@=strictops */
	    int line = -8382;	/* arbitrary init constant */
	    atp_BOOL_T vidon;

	    /* badptr points one byte past end of message text */
	    byte CONSPTR badptr = (byte *) (rbuf + (size_t) get_MsgSize() + header_SIZE);
	    byte CONSPTR ptr = loop_pre_buf(badptr);

	    /* loop till end of message processing and displaying output lines */
	    vidon = loop_out(mode, bptr, blen, badptr, ptr, &line);
	    assert( -1 < line );
	    if (line != 0 && line < (get_ScrnRows() - 2))
		printf("\n");
#ifdef VT220
	    if (vidon && !xterm_flag) {
		printf(VT220_REV_OFF);
	    } else
#endif
#ifdef HAVE_LIBTERMCAP
	    if (vidon)
		printf("%s", ansi ? A_REV_OFF : tputs_ptr(rev_off));
#else
	    if (vidon && ansi)
		printf("%s", A_REV_OFF);
#endif
	    fflush(stdout);	/* fail safe cancel of highlighting */
	}
    }
}

/*
 * BEGIN LONG SUBJECT ROUTINES
 */

/*
 * adjust_sot, adjust start of text pointer.
 */
static char *
adjust_sot(char *SOSubj, char *badptr)
{
    char *p = SOSubj;
    char *SOText = p + LSUBJ_BODY_LEN + LSUBJ_TAIL_LEN;
    char *Saved = NULL;

    /*@-strictops */
    while (p < SOText && p < badptr) {
	if (*(SOText - 2) == 'N' && strnicmp(SOText - 8, LSUBJ_TAIL, 8) == SUCCESS)
	    break;
	if (*p == '\n') {
	    SOText++;
	    if (!Saved)
		Saved = p + 1;
	}
	p++;
    }
    if (strnicmp(p - 7, LSUBJ_TAIL, 8) != SUCCESS && Saved != NULL) {
	if (Saved > (SOSubj + field_len))
	    SOText = Saved;
	else
	    SOText = rbuf + header_SIZE;
    }
    if (*SOText == '\n')
	SOText++;		/*@=strictops */

    return SOText;
}


/*
 * find_long_subj, parse first message block for embedded long subject.
 */
static char *
find_long_subj(char *txt_start, char *badptr)
{
    char *subj_start = txt_start;
    size_t ct = 0;
    badptr -= LSUBJ_BUF_LEN;	/*@-strictops */
    txt_start--;
    for (; ct < block_SIZE && txt_start < badptr; txt_start++, ct++) {
	if (ct == (size_t) 0 || *txt_start == '@') {
	    if (++txt_start < badptr && strnicmp(txt_start, "SUBJECT:", LSUBJ_NAME_LEN) == SUCCESS) {
		subj_start = txt_start + LSUBJ_NAME_LEN;
		if (*subj_start == SPC_CHAR)	/*@=strictops */
		    subj_start++;
		break;
	    } else
		txt_start--;	/* adjust backward after look-ahead */
	}
    }
    return subj_start;
}


/*
 * process_subjbuf, strip newlines, padding and junk from subject buffer.
 */
static void
process_subjbuf(void)
{
    char *subptr = SubjBuf;
    char *endmarker = SubjBuf + LSUBJ_BODY_LEN;
    *endmarker = NUL_CHAR;
    while (subptr < endmarker) {
        /*@-strictops */
	if (*subptr == '\n' || *subptr == '\r') {
#if 0
	    assert(*subptr == '\r' || (subptr - SubjBuf) > field_len); /*@=strictops */
#endif
	    ShiftLeft(subptr, 1);
	    endmarker--;
	} else {
	    subptr++;
	}
    } /*@-strictops */
    for (; (char *)SubjBuf < subptr;) {
	subptr--;
	if (*subptr != SPC_CHAR && *subptr != TAB_CHAR)  /*@=strictops */
	    break;
	*subptr = NUL_CHAR;
    }
}

/* 
 * Check4LongSubj - checks message for PCBoard-style long subject line.
 */
void
Check4LongSubj(void)
{
    char *SOSubj;
    char CONSPTR badptr = rbuf + (size_t) get_MsgSize() + header_SIZE;

/* reset variables to normal state, i.e. no long subject */
    SubjBuf[0] = NUL_CHAR;
    start_of_text = rbuf + header_SIZE;

/* check first line of message for @SUBJECT (ignore white space) */
    SOSubj = find_long_subj(start_of_text, badptr);

/* find end of long subject, i.e. the start of message text */
    if (SOSubj != start_of_text && SOSubj < (start_of_text = adjust_sot(SOSubj, badptr))) {
	assert(SOSubj < start_of_text && start_of_text < badptr);
	assert(start_of_text < (SOSubj + block_SIZE + (LSUBJ_BUF_LEN - 2)));

	/* zero out subject buffer and copy long subject into it */
	(void) memset(SubjBuf, 0, (size_t) SBUF_LEN);	/*@-strictops */
	(void) memcpy(SubjBuf, SOSubj, (size_t) (start_of_text - SOSubj)); /*@=strictops */
	process_subjbuf();
    }
}
    
