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
read.c
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
#ifdef __LCLINT__
#endif

/*
 * str2mem - copies a string without the trailing null character.
 * used to correctly fill the Qmail header.
 */
void
str2mem(char *mem, const char *str)
{
    while (*str != NUL_CHAR) {
        *mem = *str;
        str++;
        mem++;
    }
}


/*
 * Numeric - returns TRUE if the string is a number or FALSE if not.
 */
atp_BOOL_T
Numeric(const char *str)
{
    atp_BOOL_T ret_code = FALSE;
    if (*str != NUL_CHAR) {
        while (*str != NUL_CHAR && '0' <= *str && *str <= '9')
            str++;
        if (*str == NUL_CHAR)
            ret_code = TRUE;
    }
    return ret_code;
}


/*
 * Date - displays the date and time.
 */
void
Date(void)
{
    time_t t;
    const size_t length_of_date_string = DATE_LEN ;
    char date_string[DATE_LEN+1];

    blue();
    high();
    /*@i1*/ time(&t);
    tzset();
    /* this macro is used only by msdos and its relatives */
    ADJUST_DOS_TIME
    (void) memcpy(date_string, ctime(&t), length_of_date_string);
    date_string[ length_of_date_string ] = (char)0;
    printf("%s", date_string);
    green();
    printf("\n");
}


/*
 * show_again - re-displays the current message.
 */
void
show_again(void)
{
    if (get_FirstDone())
        Display(NEXT, NULL, 0);
    else
        ReadNext(NEXT);
}


/*
 * enter_msg - invokes Reply() routines to post a message.
 */
void
enter_msg(char *tmp)
{
    if (get_CurConf() == RCONF_IDX && tmp[1] == NUL_CHAR && get_FirstDone())
        Reply(CHANGE, tmp);
    else
        Reply(ENTER, tmp);
}


/*
 * reply_msg - post reply to current message.
 */
void
reply_msg(const char *tmp)
{
    if (get_FirstDone() && get_CurConf() != RCONF_IDX)
        Reply(REPLY, tmp);
}


/*
 * xpost_msg - copy a perviously entered reply to another message area. 
 */
void
xpost_msg(const char *tmp)
{
    if (get_CurConf() == RCONF_IDX && get_FirstDone())
        Reply(XPOST, tmp);
}


/*
 * change_msg - re-edit a message in the reply conference.
 */
void
change_msg(const char *tmp)
{
    if (get_CurConf() == RCONF_IDX && get_FirstDone())
        Reply(CHANGE, tmp);
}


/* 
 * mark_private - marks the current message as private.
 */
void
mark_private(void)
{
    if (get_CurConf() == RCONF_IDX)
        ReadNext(PRIVATE);
}


/*
 * FindActive, search conference array for next active conference.
 * it will wrap around to start if need be.
 */
static int
FindActive(const int curcon)
{
    int i;
    for (i = curcon + 1; i <= LastConf; i++) {
        if (ConfActive[i])
            return (i);
    }
    for (i = 0; i <= curcon; i++) {
        if (ConfActive[i])
            return (i);
    }
    return FAILURE;
}


/*
 * more - prompts "more  ?" and wait for yes/oui/ja and returns TRUE for yes.
 *  returns FALSE for no, 'def ' manages the CR default answer.
 */
atp_BOOL_T
more(atp_BOOL_T def)
{
    atp_BOOL_T ret_code = TRUE;
    atp_BOOL_T loop_flag = TRUE;
    char *response, tmp[80];

    do {
	high();
	yellow();
	luxptr = NULL;
	sprintf(tmp, "\r %s ? %s : ", txt[60], def? txt[106] : txt[107]);
	do {
	    response = readline(tmp, no_scroll);
	} while (response == NULL);
	strcpy(tmp, response);
	free_string(response);
	clear();
	printf("\r %-75s \r", " ");	/* erase prompt */
	switch ((int) tmp[0]) {
	case 'o':
	case 'y':
	case 'O':
	case 'Y':
	case 'j':
	case 'J':
	    ret_code = TRUE;
	    loop_flag = FALSE;
	    break;
	case 'N':
	case 'n':
	    ret_code = FALSE;
	    loop_flag = FALSE;
	    break;
	case '+':
	case '-':
	case 0:		/* CR */
	case 10:
	    if (def) {
		ret_code = TRUE;
	    } else {
		ret_code = FALSE;
	    }
	    loop_flag = FALSE;
	}
    } while (loop_flag);
    return ret_code;
}


/*
 * YesNo - prompts "Y/N : " and wait for yes/oui/ja then returns TRUE for yes.
 */
atp_BOOL_T
YesNo(atp_BOOL_T def, const char *prmt)
{
    atp_BOOL_T ret_code = TRUE;
    atp_BOOL_T loop_flag = TRUE;
    char *response, tmp[80];

    do {
	luxptr = NULL;
	sprintf(tmp, "\r%s %s : ", prmt, def? txt[106] : txt[107]);
	do {
	    response = readline(tmp, no_scroll);
	} while (response == NULL);
	strcpy(tmp, response);
	free_string(response);
	clear();
	printf("\r %-75s \r", " ");	/* erase prompt */
	switch ((int) tmp[0]) {
	case 'o':
	case 'y':
	case 'O':
	case 'Y':
	case 'j':
	case 'J':
	    ret_code = TRUE;
	    loop_flag = FALSE;
	    break;
	case 'N':
	case 'n':
	    ret_code = FALSE;
	    loop_flag = FALSE;
	    break;
	case 0:		/* CR */
	case 10:
	    if (def)
		ret_code = TRUE;
	    else
		ret_code = FALSE;
	    loop_flag = FALSE;
	}
    } while (loop_flag);
    return ret_code;
}


/* 
 * Chk4RepPkt - returns TRUE if bbsname.rep exists.
 */
atp_BOOL_T
Chk4RepPkt(void)
{
    atp_BOOL_T ret_code = FALSE;
    char RepFile[MAXPATHS];

    sprintf(RepFile, "%s%s.rep", ReplyPath, CurBoard);
    if (access(RepFile, F_OK) == SUCCESS) {
	char prmbuf[80];
	red();
	high();
	/* "Warning ! file already exist " "delete it ?" */
	printf("%s   %s %s %s...!\n", txt[2], txt[62], RepFile, txt[63]);
	sprintf(prmbuf, "            %s ", txt[64]);
	if (YesNo(YES, prmbuf))
	    do_unlink(RepFile);
	else
	    ret_code = TRUE;
    }
    return ret_code;
}


/*
 * chkerror - si un rigolo tape ce mot dans le shell.
 */
void
chkerror(void)
{
    magenta();
    high();
    printf("\n");
    switch (get_charset()) {
    case ISOLAT1:
	printf("Calice! Espèce");
	break;
    case CHRDOS:
	printf("Calice! EspŠce");
	break;
    default:
	printf("Calice! Espece");
    }
    printf(" de petit connard, on t'a pas appris la politesse  ???%c\n", BELL);
    printf("Va te faire foutre !!!\n\n");
    clear();
    fflush(stdout);
}


/*
 * LoadStat, called by DoLoad().
 */
static atp_BOOL_T
LoadStat(char *OldDat, char *NewDat)
{
    atp_BOOL_T ret_code = TRUE;
    struct stat Oldst, Newst;
    Newst.st_mtime = Oldst.st_mtime = 0L;
    if (stat(OldDat, &Oldst) == FAILURE) {
        printf("Load() Can't stat OldDat %s:%d\n",__FILE__,__LINE__);
        (void) sleep(3);
    }
    if (stat(NewDat, &Newst) == FAILURE) {
        printf("Load() Can't stat NewDat %s:%d\n",__FILE__,__LINE__);
        (void) sleep(3);
    }
    if (Newst.st_mtime <= Oldst.st_mtime) {
        char CONSPTR prmbuf = NewDat;
        red();
        high();
        /* "New packet older..." */
        printf("%s\n%s.\n", txt[2], txt[11]);
        /* " Do you  want to load it? " */
        sprintf(prmbuf, "%s...", txt[12]);
        if (!YesNo(NO, prmbuf))
            ret_code = FALSE;
    }
    return ret_code;
}

/* used by DoLoad(), AutoJoin(), Read(), */
static atp_BOOL_T newmail = TRUE;

/* used by many routines in this file */
static atp_BOOL_T IsEmpty = TRUE;       /* Flag false if a BBS is loaded */

/*
 * get_isempty - allows global read access to IsEmpty variable.
 */
atp_BOOL_T
get_isempty(void)
{
    return IsEmpty;
}

/*
 * UpdateConf - update last message read in index file.
 */
void
UpdateConf(const update_command_t mode)
{
    assert(/*@i2*/ mode == update_last_read_ptr || mode == reset_last_read_ptr);
    if (get_ActvCnt() < 1 || !ConfActive[get_CurConf()]) {
	deleol();
	printf("No active conference to update.\n");
	if (get_FilesOpen()) {
	    printf("ERROR: %s:%d UpdateConf() -- open files, inactive Conf\n", __FILE__, __LINE__);
	    printf("ActvCnt = %d  CurAct = %d \n", get_ActvCnt(), (int) ConfActive[get_CurConf()]);
	};
    } else if (!get_FilesOpen()) {
	printf("ERROR: module UpdateConf() -- bad file pointer for fseek()\n");
    } else if (fseek(fidx, 0L, SEEK_SET) != SUCCESS) {
	printf("UpdateConf() Seek error...\n");
    } else {
	struct MyIndex Idx;
	fread((char *) &Idx, index_SIZE, one_record, fidx);
	if (((Idx.LastRead < get_MsgNum()) || /*@i1*/ mode == reset_last_read_ptr) && get_FirstDone()) {
	    long here;
	    Idx.LastRead = get_MsgNum();
	    rewind(fidx);
	    fwrite((char *) &Idx, index_SIZE, one_record, fidx);
	    printf("%s.\n", txt[66]);	/* "Last read pointer updated" */
	    here = Idx.LastRead * IDXSIZE;
	    fseek(fidx, here, SEEK_SET);
	}
    }
}


/*
 * DoLoad, update BBS archive from files in working directory.
 * called from Load(). OldDat points to buffers of size MAXPATHS.
 * NewDat points to the new control.dat file in the work directory.
 */

static int CurConf = 0;

/*
 * get_CurConf - returns the number of the current conference.
 */
int
get_CurConf(void)
{
    return CurConf;
}


/*
 * set_CurConf - sets the number of the current conference.
 */
void
set_CurConf(const int num, const scnf_access_t perm)
{
    if (/*@i1*/ scnf_GetConf == perm)
	CurConf = num;
#ifdef ATPDBG
    else
	assert(/*@i1*/ scnf_GetConf == perm);
#endif
}


/*
 * DoLoad_MkIndex, after DoLoad() sets up, make index and clean up.
 */
static void
DoLoad_MkIndex(const char *BbsDir, const char *BbsName)
{
    if (/*@i1*/ MkIndex(WorkPath, BbsDir) == ATP_OK) {
        /* update array of active conferences */
        set_CurBoard(BbsName, cb_DoLoad_MkIndex);
        ActvConf();
        (void) Chk4RepPkt();
        Chk4RepCnf(BbsDir);
        if (get_FilesOpen()) {
            fclose_fmsg_fidx();
        }
        IsEmpty = FALSE;
        newmail = TRUE;
        set_caps(fido ? FALSE : TRUE);
        if (get_pmail())
            CurConf = findCindex(PERS_CONF);
        else
            CurConf = FindActive(LastConf);
        if (CurConf < 0)
            CurConf = 0;
    }
}


/*
 * DoLoad, called by LoadExtract(), does setup for DoLoad_MkIndex().
 */
static void
DoLoad(char *BbsName, char *OldDat, char *NewDat)
{
    char BbsDir[MAXPATHS];
    /* Access to a board subdir */
    sprintf(BbsDir, "%s%s", HomePath, BbsName);
    if (access(BbsDir, R_OK | W_OK) == FAILURE)
        my_mkdir(BbsDir);
    sprintf(OldDat, "%s%s%c%s", HomePath, BbsName, SEP, CNTRL_FILE);

    /* If there exists an old control.dat */
    if (access(OldDat, F_OK) == FAILURE || LoadStat(OldDat, NewDat)) {
        if (!IsEmpty && ConfActive[CurConf])
            UpdateConf(update_last_read_ptr);
        /* Create Index Files */
        DoLoad_MkIndex(BbsDir,BbsName);
        GetConf(CurConf);
    }
    assert(!get_isempty() || (fmsg == NULL && fidx == NULL));
}

#ifdef NONUM
/*
 * For problems with Searchlight BBS,
 * don't use it unless you *really* need it.  
 */
static void
SearchlightCruft(char *BbsName, int i)
{
    char *tptr = BbsName;
    if (!Numeric(tptr))         /* strip trailing packet sequence numbers */
        while (isdigit(BbsName[--i]))
            BbsName[i] = NUL_CHAR;
}
#endif

/*
 * LoadExtract, called by Load().
 */
static void
LoadExtract(char *BbsName, char *tmp1)
{
    if (access(tmp1, R_OK) == FAILURE) {
        red();
        /* No mail found        */
        printf("%s : %s\n", txt[6], tmp1);
        printf("%s\n", txt[7]); /* Try the read command */
    } else {
        char tmp2[MAXPATHS];
        /* Don't forget to clean work directory ! */
        Clean();
        yellow();
        high();
        /* " Extracting Messages..." */
        printf("%s\n", txt[8]);
        green();
        fflush(stdout);
	/*@-usedef */
        /* save current directory    */
        GETCWD(tmp2, MAXPATHS);
	/*@=usedef */
        /* change to work directory  */
        if (CHPATH(WorkPath) != SUCCESS) {
            yellow();
            printf("Error: can't access workpath: %s\n", WorkPath);
        } else {
            char CONSPTR unarc = get_atprc_str(unarchvr);
            assert(unarc != NULL);
            fork_execvp(unarc, tmp1);
            free_string(unarc);
            CHPATH(tmp2);       /* restore current directory */
            yellow();
            if(verify_new_file( tmp2, WorkPath, CNTRL_FILE) == ATP_ERROR)
                printf("txt[49]: %s\n", tmp2);
            else
                DoLoad(BbsName, tmp1, tmp2);
        }
    }
}


/*
 * Load - get a qwk packet; toss it then display it.
 */
void
Load(const char *name)
{
    char BbsName[50], tmp1[MAXPATHS];
    /* reset global flag to indicate no personal mail yet */
    set_pmail(FALSE, pmLoad);	/* should this be moved ??? */
    query_rep_exists();
    BbsName[0] = NUL_CHAR;
    sscanf(name, "%s %s", tmp1, BbsName);
    if (BbsName[0] == NUL_CHAR) {
	red();
	printf("%s  %s\n", txt[87], txt[88]);	/* "usage" " load.." */
    } else {
	int i = -1;
	printf("\n%s %s\n", txt[5], BbsName);	/* "Loading" */
	if (strstr(BbsName, ".qw") == NULL)
	    strcat(BbsName, ".qwk");
	sprintf(tmp1, "%s%s", MailPath, BbsName);
	/*
	 *    Now that we've used BbsName to id the packet file, 
	 *    strip any trailing suffix. 
	 */
	while (BbsName[++i] != NUL_CHAR)/* empty loop */
	    ;
	while (BbsName[--i] != '.')	/* empty loop */
	    ;
	BbsName[i] = NUL_CHAR;
#ifdef NONUM
	/* do not use, this is a kludge only for Searchlight BBS problems */
	SearchlightCruft(BbsName, i);
#endif
	LoadExtract(BbsName, tmp1);
    }
}


/*
 * VerifyBBS, check for existing BBS and control.dat file.
 *
 */
static atp_BOOL_T
VerifyBBS(const char *BbsName, char *buf)
{
    atp_BOOL_T ret_code = FALSE;
    sprintf(buf, "%s%s%c%s", HomePath, BbsName, SEP, CNTRL_FILE);
    if (access(buf, F_OK) == FAILURE) {
        printf("%s : %s\n", txt[13], buf);      /* "No bbs found " */
    } else {
        sprintf(buf, "%s%s", HomePath, BbsName);
        if (/*@i1*/ RdCn_ReadControl(buf) != ATP_OK) {
            printf("%s: %s\n", buf, txt[14]);   /* "Error in CONTROL.DAT." */
        } else {
            ret_code = TRUE;
        }
    }
    return ret_code;
}


/*
 * ReadExecute, after setup by Read() this changes to another BBS.
 */
static void
ReadExecute(const char *BbsName, char *tmp_buffer)
{
    if (VerifyBBS(BbsName, tmp_buffer)) {
        printf("%s %s\n", txt[5], BbsName);     /* "Loading" */
        set_CurBoard(BbsName, cb_Read);
        (void) Chk4RepPkt();
        Chk4RepCnf(tmp_buffer);
        if (get_FilesOpen()) {
            fclose_fmsg_fidx();
        }
        /* update boolean array of active/inactive flags */
        ActvConf();
        IsEmpty = FALSE; /* should this be set inside of GetConf()?  !!! */
        newmail = FALSE;
        set_caps(fido ? FALSE : TRUE);
        CurConf = FindActive(LastConf);
        if (CurConf < 0)
            CurConf = 0;
    }
}


/*
 * Read - join a BBS without extracting new mail.
 */
void
Read(const char *cmd_line)
{
    char BbsName[50];
    char tmp_buffer[MAXPATHS];
    query_rep_exists();
    BbsName[0] = NUL_CHAR;
    sscanf(cmd_line, "%s %s", tmp_buffer, BbsName);
    if (BbsName[0] == NUL_CHAR) {
        red();
        printf("%s %s\n", txt[87], txt[89]);    /* "usage " "rev..." */
    } else {
        if (!IsEmpty && ConfActive[CurConf])
            UpdateConf(update_last_read_ptr);
        ReadExecute(BbsName, tmp_buffer);
        GetConf(CurConf);
    }
#ifdef ATPDBG
    assert(!get_isempty() || !get_FilesOpen());
    assert(!get_FilesOpen() || ( fmsg != NULL && fidx != NULL ));
#endif
}

static const int not_found = -1 ;
/*
 * chcon_string_parse, turn a conference string into a conference index.
 */
static int
chcon_string_parse(char *Name, const char *buf, char *tmp)
{
    int i = 0 ;
    strcpy(tmp, strstr(buf, Name));
    strcpy(Name, tmp);
    for (i = 0; i <= LastConf; i++) {
        if (stricmp(Name, ConfNames[i]) == SUCCESS)
            break;
    }
    if (i > LastConf) {
        const size_t m = strlen(Name);
        for (i = 0; i <= LastConf; i++) {
            if (strnicmp(Name, ConfNames[i], m) == SUCCESS)
                break;
        }
    }
    return ( i > LastConf ? not_found : i ) ;
}


/*
 * chcon_scan, turn absolute conf number or name into a conference index.
 */
static int
chcon_scan(char *Name, const char *buf, char *tmp)
{
    int i;
    if (Numeric(Name)) {
        /* a number is given */
        i = atoi(Name);
        if ((i = findCindex(i)) > -1)
            strcpy(Name, ConfNames[i]);
    } else {
        /* a string name is given   */
        i = chcon_string_parse(Name, buf, tmp);
    }
    return i;
}


/*
 * chconf_join, actually invoke GetConf() to change current conference.
 */
static void
chconf_join(int new_conference)
{
    /* Save current conf pointer */
    UpdateConf(update_last_read_ptr);
    if (/*@i1*/ GetConf(new_conference) == ATP_OK) {
        printf("\n ");
        blue();
        high();
        printf("*  %s %s %s. *", txt[29], ConfNames[CurConf], txt[30]);
        green();
        printf("\n\n");
        /* "Conference " "joined" */
        clear();
        fflush(stdout);
    }
}


/*
 * chconf - change active conference.
 */
void
chconf(const char *buf)
{
    char Name[100];
    char tmp[MAXPATHS];
    if (IsEmpty) {
        EmptyMsg();
    } else {
        Name[0] = NUL_CHAR;
        sscanf(buf, "%s %s", tmp, Name);
        if (Name[0] == NUL_CHAR) {
            red();
            printf("%s\n", txt[87]);    /* "Usage" */
            printf("\tj %s\n\t\t%s\n", txt[90], txt[91]);       /* "Conf #" "or" */
            printf("\tj %s\n", txt[92]);        /* "conf name" */
            printf("%s\n", txt[93]);    /* "type conf to list..." */
        } else {
            int new_conference = chcon_scan(Name, buf, tmp);
            if ( stricmp(Name, "MAIN") == SUCCESS)
                new_conference = 0;     /* Special case for "main board" */
            if (new_conference > LastConf || new_conference < 0) {
                /* "Unknown conference" */
                printf("\n%s\n", txt[28]);
            } else {
                cnf_path(tmp, ConfNumbers[new_conference]);
                if (access(tmp, F_OK) == FAILURE) {
                    red();      /* "No mail found" */
                    printf("%s : %s  %s\n", txt[6], tmp, ConfNames[new_conference]);
                } else
                    chconf_join(new_conference);
            }
        }
    }
}


/*
 * AutoJoin - loads next valid conferences.
 */
#define ILR get_MsgLastRead()

void
AutoJoin(void)
{
    if (IsEmpty) {
	EmptyMsg();
    } else if (get_ActvCnt() == 0) {
	chconf("j MAIN");
    } else {
	char tmp[MAXPATHS];
	int i = CurConf;
	int j = get_ActvCnt();

	for (; j != 0; j--) {
	    i = FindActive(i);
	    sprintf(tmp, "j %d\n", ConfNumbers[i]);
	    chconf(tmp);
	    if (ILR < 0L || ILR != (get_TotMsg() - 1L) || !newmail) {
		return;
	    } else
		printf("\t%s.\n", txt[31]);	/* "No new mail" */
	}
	newmail = FALSE;
	printf("\n");
    }
}

