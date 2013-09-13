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
makemail.c 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "reader.h"
#include "readlib.h"
#include "makemail.h"
#include "qlib.h"
#include "ansi.h"


static FILE *RepMsg;

/*
 * Creation du bloc d'entete  du fichier message correspondant au serveur
 * FirstHeader, creates the opening header block for reply packet file.
 */
static void
FirstHeader(void)
{
    char Block[BLKSIZE], tbuf[QWK_FIELD_LEN + 1];
    (void) memcpy(tbuf, CurBoard, field_len);	/* Preserve lowercase name for */
    tbuf[field_len] = NUL_CHAR;
    (void) strupr(tbuf);	/* later use ....                */
    (void) memset(Block, (int) SPC_CHAR, block_SIZE);	/* Fill with spaces      */
    str2mem(Block, tbuf);	/* Insert BBS name for header    */
    if (fwrite(Block, block_SIZE, one_record, RepMsg) != one_record) {
	printf("FirstHeader() %s\n", txt[73]);
	(void) sleep(3);
    } else
	fflush(RepMsg);		/* Write this block to the file  */
}


/*
 * DoRepExport, called by DoRep() to actually write out the reply.
 */
static atp_BOOL_T
DoRepExport(const char *filemsg, const unsigned long MaxChars, const size_t NbBlocs)
{
    /* only export messages not marked as "killed" */
    atp_BOOL_T mrep = TRUE;
    unsigned long lndx;
    byte *ptr = (byte *) (rbuf + block_SIZE);
    /* Translate to QWK style "pi" line feed  */
    for (lndx = 0L; lndx < MaxChars; lndx++, ptr++) {  /*@-strictops */
	if (*ptr == (byte)'\n')
	    *ptr = QWK_LINE_FEED;
	else if (*ptr == DOS_SPACE)                    /*@=strictops */
	    *ptr = SPC_CHAR;
    }
    if (fwrite(rbuf, block_SIZE, NbBlocs + 1, RepMsg) != NbBlocs + 1) {
	/* "error writing file " */
	printf("%s %s\n", txt[73], filemsg);
	mrep = FALSE;
    }
    return mrep;
}

static FILE *MsgFile, *IdxFile;
static char MessageFile[MAXPATHS];

/*
 * DoRepFill, load message into rbuf buffer and process.
 *
 * Note: Either a message will be skipped or packed in one atomic operation.
 *       The only time count will be -1 is if there is some failure
 *       in exporting a message that is marked for export.
 */
ATP_INLINE int
DoRepFill(const char *filemsg, const char *msg_total_blocks, const char *export_status)
{
    int count = -1;
    const size_t text_blocks = (size_t) (atoi(msg_total_blocks) - 1);
    const unsigned long TextChars = text_blocks * block_SIZE;
    const unsigned long current_max = (unsigned long) get_RbufSize();

    if (current_max <= TextChars && !reup((size_t) TextChars + block_SIZE)) {
        /* message too big */
        printf("%s\n%s %lu\n", txt[1], txt[72], TextChars);
    } else if (TextChars && fread(rbuf + header_SIZE, block_SIZE, text_blocks, MsgFile) != text_blocks) {
        /* error reading file */
        printf("max char %lu\n%s %s\n", TextChars, txt[58], MessageFile);
        /*@-strictops */
    } else if ((byte) (*export_status) > HIGH_ASCII) {
        count = 0;              /* message skipped */
    } else if (DoRepExport(filemsg, TextChars, text_blocks))
        count = 1;              /* message exported */

    assert(-1 <= count && count <= 1);
    return count;
}


/*
 * DoRep, reads from reply conference then writes to messages.rep.
 *
 * Note: There are two ways to exit from the do loop. The first is if a
 *       call to DoRepFill fails (result code of -1). The second is if fread()
 *       fails (which is presumed to indicate that we are at the end of the
 *       replies file). This function should return TRUE if export count is at
 *       least 1, and result is not an error (-1);
 */

static atp_BOOL_T
DoRep(const char *filemsg)
{
    int count = 0, result = -1;
    atp_BOOL_T mrep = FALSE;
    char Qmail[HDRSIZE];

    do {

        if ((fread(Qmail, header_SIZE, one_record, MsgFile)) == one_record) {
            (void) memcpy(rbuf, Qmail, block_SIZE);
            result = DoRepFill(filemsg, Qmail + HSizeMsg, Qmail + HStatus);
            count += result;
        } else
            break;

    } while (0 <= result);

    if (count != 0 && result != -1)
        mrep = TRUE; /* no errors and at least one reply exported */
    return mrep;
}


/*
 * Cnf2Msg - convert reply ".cnf" file into ".msg" file for export.
 *
 */
atp_BOOL_T
Cnf2Msg(const char *filemsg)
{
    atp_BOOL_T mrep = FALSE;

    /* Open MsgFile and IdxFile */
    if (OpenRepFile(pack_them)) {
        if ((RepMsg = fopen(filemsg, "wb")) == NULL) {
            /* "unable to open file" */
            fprintf(stderr, "%s %s\n", txt[51], filemsg);
            perror("Does 'workpath' specified in atprc exist? ");
        } else {
            FirstHeader();
            /* copy .cnf to messpath */
            rewind(MsgFile);
            mrep = DoRep(filemsg);
            fclose(RepMsg);
            if (!mrep)
                do_unlink(filemsg);
        }
        /* close message path */
        fclose(MsgFile);
        fclose(IdxFile);
    }
    return mrep;
}


/*
 * display_ORF_result, prints result message for OpenRepFile.
 */
ATP_INLINE void 
display_ORF_result(atp_BOOL_T ret_code, pakrep_t mode, atp_BOOL_T FileExist)
{
    if (!ret_code) {
	/* "unable to open file" */
	printf("%s %s\n", txt[51], MessageFile); /*@-strictops */
    } else if (mode == add_reply) {              /*@=strictops */
	if (FileExist)
	    /* "Adding message to file" */
	    printf("%s %s \n", txt[76], MessageFile);
	else
	    /* "creating file" */
	    printf("%s %s \n", txt[77], MessageFile);
    }
}

static char IndexFile[MAXPATHS];

/*
 * Ouverture ou creation du fichier messages.
 * OpenRepFile - open or create the message file for replies.
 */
atp_BOOL_T
OpenRepFile(const pakrep_t mode)
{
    atp_BOOL_T FileExist = FALSE, ret_code = TRUE;

    /* valid modes are "pack_them" and "add_reply" */   /*@-strictops */
    assert(mode == pack_them || mode == add_reply);	/*@=strictops */
    make_c_i_paths(MessageFile, IndexFile, REPL_CONF);

    if (access(MessageFile, F_OK) == SUCCESS)
	FileExist = TRUE;

    /* see if rep file exists and it is open */
    if (FileExist && get_saved_conf() == RCONF_IDX) {
	MsgFile = fmsg;
	IdxFile = fidx;                                                  /*@-strictops */
	assert(MsgFile != NULL && IdxFile != NULL);
    } else if (OpenCon(&MsgFile, NULL, MessageFile) == ATP_ERROR 
	       || OpenCon(&IdxFile, &MsgFile, IndexFile) == ATP_ERROR) { /*@=strictops */
	ret_code = FALSE;                                       
    }
    display_ORF_result(ret_code, mode, FileExist);
    return ret_code;
}


/***** begin KodeMessage routines *****/

/*
 * KodeSubj, handle PCBoard style long subjects lines in replies.
 *   Note that sprintf() returns a char pointer under SunOS but
 *   an integer under POSIX. Therefore for portability, char_cnt 
 *   must be calculated in a separate operation. 
 */
static size_t
KodeSubj(char CONSPTR tampbuf)
{
    size_t text_start_offset = 0;
    char *long_subj = NULL ;

    /* if a long subject line was entered, format it PCBoard style */
    if (PCBLONG && (long_subj = get_reply_lsubj()) != NULL) {
	byte CONSPTR net_flag = (byte *) tampbuf;
	char *buf_ptr = tampbuf;
	size_t char_cnt;

	/* write head string and subject to reply buffer */
	sprintf(tampbuf, "%s%s", LSUBJ_HEAD, long_subj);
	free_string(long_subj);

	/* guarantee temination before taking length */
	tampbuf[LSUBJ_HEAD_LEN + LSUBJ_BODY_LEN] = NUL_CHAR;
	char_cnt = strlen(tampbuf);
	buf_ptr += char_cnt;

	/* adjust for FIDO network if needed */
	*net_flag = (byte) (fido ? SPC_CHAR : DOS_SPACE);

	/* pad message with space characters */
	while (char_cnt < (size_t)(LSUBJ_HEAD_LEN + LSUBJ_LEN)) {
	    *buf_ptr++ = SPC_CHAR;
	    char_cnt++;
	}
#ifdef ATPDBG
	assert((STRING_LEN(LSUBJ_HEAD) + STRING_LEN(LSUBJ_TAIL) + LSUBJ_LEN) == LSUBJ_BUF_LEN);
	assert(STRING_LEN(LSUBJ_HEAD) == 10);
	assert(STRING_LEN(LSUBJ_TAIL) == 8);
	assert(buf_ptr == (tampbuf + 64));
#endif
	strcpy(buf_ptr, LSUBJ_TAIL);
	strcat(buf_ptr, "\n");
	text_start_offset = strlen(tampbuf);
	assert( text_start_offset == (LSUBJ_BUF_LEN + STRING_LEN("\n")));
    }
    return text_start_offset;
}

static size_t CountBlocks; 
static size_t CountTotalChars; 
static size_t CountRecordChars;
static char *tampon, *tamp;

/*
 * KodeMsgTag, add the tagline to the reply message.
 */
static void
KodeMsgTag(void)
{
    /*@only@*/ byte CONSPTR base_ptr = get_CurTag();

    if (base_ptr != NULL) {
	const byte *ptr = base_ptr;
	const atp_CODE_T chrset = get_charset();

	/*@-strictops */
	while (*ptr != (byte) NUL_CHAR) {
	    if (chrset == ISOLAT1)	/*@=strictops */
		/* translate ISO Latin1 to DOS chars */
		*tamp = codepc[(unsigned) *ptr];
	    else
		*tamp = *ptr;
	    tamp++;
	    ptr++;
	    CountRecordChars++;
	    CountTotalChars++;
	    if (CountRecordChars > block_SIZE) {
		CountBlocks++;
		CountRecordChars = 1;
	    }
	}
	free_string((char *) base_ptr);
    }
}

/*
 * KodeMsgFin, finish up by adding tagine and padding out last block
 */
static void
KodeMsgFin(char *Qmail)
{

    /* Add the the tagline. Rajout de la signature du programme */
    if (get_tag_flag())
	KodeMsgTag();

    /* Ajuste la taille du dernier bloc … 128 octets    */
    /* Adjust the size of the last block to 128 bytes      */
    while (CountRecordChars < block_SIZE) {
	*tamp = SPC_CHAR;
	tamp++;
	CountRecordChars++;
	CountTotalChars++;
    }

    /* Inscrit dans le Header le nombre blocs de 128 octets */
    /* Inscribe in the header the number of 128 byte blocks */
    {
	char tmp[20];
	sprintf(tmp, "%lu", (unsigned long) CountBlocks);
	str2mem(Qmail + HSizeMsg, tmp);
    }
}

/*
 * KodeXlat, translate the message from host to MSDOS character set.
 */
static void
KodeXlat(FILE * TmpaFile)
{
    byte ch;
    unsigned LinCt = 0;		/* counts line length */
    const atp_CODE_T chrset = get_charset();
    rewind(TmpaFile);
    while (fread(&ch, (size_t)1, one_record, TmpaFile) == one_record) {
	if (LinCt >= REPLY_LINE_LEN)
	    ch = '\n'; 
	/*@-strictops */                            /*@-usedef */
	LinCt = (ch == (byte)'\n') ? 0 : LinCt + 1; /*@=usedef */

	if (chrset == ISOLAT1 && ch >= HIGH_ASCII) 
	    ch = codepc[(unsigned)ch];	/* translate back to DOS set */
	ch = (byte)((ch == DOS_SPACE || ch == (byte)CNTRL_Z) ? SPC_CHAR : ch);

	/* skip ^M carriage returns */
	if (ch != (byte)CR) {		 /*@=strictops */
	    *tamp = ch;
	    tamp++;
	    CountRecordChars++;
	    CountTotalChars++;
	    if (CountRecordChars > block_SIZE) {
		CountBlocks++;
		CountRecordChars = 1;
	    }
	}
    }
}


/* Ecrit les fichiers conf et index.
 * KodeWrite, write out newly encoded message and update .cnf and idx files.
 */
static void
KodeWrite(char *Qmail, long klast)
{
    /* Ecrit l'entete du message, suivi du message          */
    /* Write the reply header followed by the reply itself  */

    printf("%s...\n", txt[75]);	/* "sauvegarde du message" */
    rewind(MsgFile);
    fseek(MsgFile, 0L, SEEK_END);
    if (fwrite(Qmail, header_SIZE, one_record, MsgFile) != one_record ||
    fwrite(tampon, CountTotalChars, one_record, MsgFile) != one_record) {
	printf("KodeWrite() %s\n", txt[73]);
	(void) sleep(3);
    } else {
	fflush(MsgFile);

	/* Update the index */

	fseek(IdxFile, 0L, SEEK_END);	/*@-strictops */
	if (WriteIndex(IdxFile, (long) (ftell(IdxFile) / IDXSIZE), (block_SIZE * (CountBlocks - 1)), klast) == ATP_OK)	/*@=strictops */
	    fflush(IdxFile);
    }
}


/*
 * KodeXlatFin, clean up the tail end of the message.
 */
static void
KodeXlatFin(void)
{
    /* point to last character */
    if (tamp > tampon)
	tamp--;

    /* guarantee a non-empty message */
    if (CountTotalChars == (size_t) 0 || (CountTotalChars == (size_t) 1 && /*@i@*/ *tampon == '\n')) {
	assert(tamp == tampon);
	*tamp = SPC_CHAR;
	CountTotalChars = CountRecordChars = (size_t) 1;
    }
    /*@-strictops */
    if (*tamp != '\n') {	/*@=strictops */
	tamp++;
	CountRecordChars++;
	CountTotalChars++;
    }
    /* mark end of text proper */
    *tamp = DOS_SPACE;
    tamp++;
    if (CountRecordChars > block_SIZE) {
	CountBlocks++;
	CountRecordChars = 1;
    }
}


/*
 * KodeDoFin, finish up and close files for KodeDo().
 */
static void
KodeDoFin(long ksave_cnf, long ksave_idx)
{
    if (get_saved_conf() == RCONF_IDX) {
	fseek(fmsg, ksave_cnf, SEEK_SET);
	fseek(fidx, ksave_idx, SEEK_SET);
    } else {
	fclose(MsgFile);
	fclose(IdxFile);
    }
    /* update list of active conferences */
    ActvConf();
}


/* Codage du message dans le tampon.
 * KodeDo, encode the reply with QWK standard characters.
 */
static void
KodeDo(FILE * TmpaFile, char *Qmail)
{
    long int klast, ksavec = 0L, ksavei = 0L;
    CountBlocks = 2;

    /* if we are already in the Reply Conference, save position */
    if ( get_saved_conf() == RCONF_IDX) {
	ksavec = ftell(fmsg);
	ksavei = ftell(fidx);
    }
    /* determine size of MsgFile */
    fseek(MsgFile, 0L, SEEK_END);
    klast = ftell(MsgFile);

    /* handle PCBoard long subjects */
    CountTotalChars = CountRecordChars = KodeSubj(tampon);
    tamp = tampon + CountRecordChars;

    /* translate the message from host to MSDOS character set */
    KodeXlat(TmpaFile);

    /* clean-up the tail end of the message */
    KodeXlatFin();

    /* apend tagline and finish up */
    KodeMsgFin(Qmail);
   
    /* write out and update cnf and idx files */
    KodeWrite(Qmail, klast);

    /* Close and erase the work file */
     KodeDoFin(ksavec,ksavei);
}

/*
 *  Ecriture du message proprement dit, cod‚ selon le format PcBoard, c.a.d les cr/lf remplac‚s par  '\343'
 * 
 * KodeMessage - encodes reply in PcBoard/QWK format.
 *  note: CR/LF are replaced with '\343', the DOS "pi" character.
 */
atp_ERROR_T
KodeMessage(const char *fname, char *Qmail)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    FILE *TmpaFile;

    /* Ouvre le fichier cr‚‚ par l'‚diteur */
    /* Open the reply file created by the editor    */

    if ((TmpaFile = fopen(fname, "r")) == NULL) {
	printf("%s %s\n", txt[51], fname);	/* "unable to open file" */
    } else {
	long int repsize;
	fseek(TmpaFile, 0L, SEEK_END);
	if ((repsize = ftell(TmpaFile)) >= 0L) {
	    const size_t tampon_size = (size_t)(repsize+1024L); /* !! why the 1024L? */
	    if (repsize > (long) (MAXSEND - 256)) {
		printf("\nERROR: Reply file exceeds %lu bytes%c\n", (unsigned long) MAXSEND, BELL);
		fclose(MsgFile);
	    } else if ((tampon = (char *) malloc(tampon_size)) == NULL) {
		printf("%s\n", txt[1]);		/* memory allocation error */
		fclose(MsgFile);
	    } else {
		KodeDo(TmpaFile, Qmail);
		ret_code = ATP_OK;
		free_buffer(tampon, tampon_size);
		tampon = NULL;
	    }
	}
	fclose(TmpaFile);
    }
    return ret_code;
}
/***** end of KodeMessage routines *****/

/* 
 * codepc - table converts a LINUX screen code to an MS-DOS screen code.
 */
const unsigned char codepc[] =
{

    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x20, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,

    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,	/* 87 */
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,	/* 8f */
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,	/* 97 */
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,	/* 9f */
    0x20, 0xad, 0x9b, 0x9c, 0xfe, 0x9d, 0x7c, 0xfe,	/* a7 */
    0xfe, 0xfe, 0xa6, 0xae, 0xaa, 0x2d, 0xfe, 0xfe,	/* af */
    0xf8, 0xf1, 0xfd, 0xfe, 0xfe, 0xe6, 0xfe, 0xf9,	/* b7 */
    0xfe, 0xfe, 0x97, 0xaf, 0xac, 0xab, 0xfe, 0xa8,	/* bf */
    0xfe, 0xfe, 0xfe, 0xfe, 0x8e, 0x8f, 0x92, 0x80,	/* c7 */
    0xfe, 0x90, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,	/* cf */
    0xfe, 0xa5, 0xfe, 0xfe, 0xfe, 0xfe, 0x99, 0xfe,	/* d7 */
    0xfe, 0xfe, 0xfe, 0xfe, 0x9a, 0xfe, 0xfe, 0xe1,	/* df */
    0x85, 0xa0, 0x83, 0xfe, 0x84, 0x86, 0x91, 0x87,	/* e7 */
    0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,	/* ef */
    0xfe, 0xa4, 0x95, 0xa2, 0x93, 0xfe, 0x94, 0xf6,	/* f7 */
    0xfe, 0x97, 0xa3, 0x96, 0x81, 0xfe, 0xfe, 0x98};


/* 
 * codelu - table converts an MS_DOS screen code to a LINUX screen code.
 */
const unsigned char codelu[] =
{

    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x09, 0x0a, 0x20, 0x20, 0x0d, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x1b, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7e,

    0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7,	/* 87 */
    0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,	/* 8f */
    0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9,	/* 97 */
    0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0x50, 0x66,	/* 9f */
    0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba,	/* a7 */
    0xbf, 0xad, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,	/* af */
    0xfe, 0xfe, 0xfe, 0xa6, 0xa6, 0xa6, 0xa6, 0xfe,	/* b7 */
    0xfe, 0xa6, 0xa6, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,	/* bf */
    0xfe, 0xad, 0xad, 0xa6, 0xad, 0x2b, 0xa6, 0xa6,	/* c7 */
    0xfe, 0xfe, 0xad, 0xad, 0xa6, 0xad, 0x2b, 0xad,	/* cf */
    0xad, 0xad, 0xad, 0xfe, 0xfe, 0xfe, 0xfe, 0x2b,	/* d7 */
    0x2b, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,	/* df */
    0xa3, 0xdf, 0xfe, 0xfe, 0x53, 0x73, 0xb5, 0x74,	/* e7 */
    0xa7, 0x4f, 0x4f, 0x64, 0xad, 0x6f, 0xc6, 0xfe,	/* ef */
    0xad, 0xb1, 0xfe, 0xfe, 0xa6, 0xa6, 0xf7, 0x7e,	/* f7 */
    0xb0, 0xb7, 0xb7, 0x76, 0x6e, 0xb2, 0xfe, 0x20};


/* 
 * codevt - table generates MS_DOS line characters with VT102 codes.
 */
const unsigned char codevt[] =
{
/* dummy 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 3 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 4 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 5 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 6 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 7 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* dummy a */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 0xb0 */ 'a', 'a', 'a', 'x', 'u', 'u', 'u', 'k',
/* 0xb8 */ 'k', 'u', 'x', 'k', 'j', 'j', 'j', 'k',
/* 0xc0 */ 'm', 'v', 'w', 't', 'q', 'n', 't', 't',
/* 0xc8 */ 'm', 'l', 'v', 'w', 't', 'q', 'n', 'v',
/* 0xd0 */ 'v', 'w', 'w', 'm', 'm', 'l', 'l', 'n',
/* 0xd8 */ 'n', 'j', 'l', 'a', 'a', 'a', 'a', 'a'};


/* 
 * code7bit - table maps 8 bit MS_DOS codes to a 7bit approximation.
 */
const unsigned char code7bit[] =
{

    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x09, 0x0a, 0x20, 0x20, 0x0d, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,

/* 80 */ 'C', 'u', 'e', 'a', 'a', 'a', 'a', 'c',
/* 88 */ 'e', 'e', 'e', 'i', 'i', 'i', 'A', 'A',
/* 90 */ 'E', 'a', 'A', 'o', 'o', 'o', 'u', 'u',
/* 98 */ 'y', 'O', 'U', 'c', 'L', 'v', 'P', 'f',
/* a0 */ 'a', 'i', 'o', 'u', 'n', 'N', 'a', 'o',
/* a8 */ '?', '~', '~', '-', '-', '!', '<', '>',
/* b0 */ '#', '#', '#', '|', '+', '+', '+', '+',
/* b8 */ '+', '+', '|', '+', '+', '+', '+', '+',
/* c0 */ '+', '+', '+', '+', '-', '+', '+', '+',
/* c8 */ '+', '+', '+', '+', '+', '-', '+', '+',
/* d0 */ '+', '+', '+', '+', '+', '+', '+', '+',
/* d8 */ '+', '+', '+', '#', '#', '#', '#', '#',
/* e0 */ '8', 'B', 'G', 'p', 'S', 's', 'm', 't',
/* e8 */ 'o', 'O', 'O', 'd', '8', 'o', 'E', '-',
/* f0 */ '=', '#', '<', '>', '|', '|', '/', '=',
/* f8 */ 'o', 'o', '.', 'v', 'n', '2', '*', ' '};

/* end of makemail.c */
