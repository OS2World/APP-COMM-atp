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
atpmain.c
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
 * SaveMsgFin, subroutine called by SaveMsg() to do the actual write. 
 */
static void
SaveMsgFin(const char *fname)
{
    FILE *fp;
    const atp_BOOL_T new_file_flag = access(fname, W_OK) == FAILURE ? TRUE : FALSE;
    if ((fp = fopen(fname, "a")) == NULL) {
        red();
        high();
        printf("%s %s\n", txt[51], fname);      /* "unable to open file" */
    } else {
        unsigned long i = 0;
        const unsigned long MsgSize = get_MsgSize();
        const atp_CODE_T chrset = get_charset();
        byte *ptr = (byte *) (rbuf + header_SIZE);
        PrintHeader(fp);
        while (i < MsgSize) {
	    /*@-strictops */
            if (*ptr != (byte)NUL_CHAR) {
                if (chrset == ISOLAT1)
                    fputc(codelu[(unsigned) (*ptr)], fp);
                else if (chrset == CHR7BIT) /*@=strictops */
                    fputc(code7bit[(unsigned) (*ptr)], fp);
                else
                    fputc(*ptr, fp);
            }
            i++;
            ptr++;
        }
        fprintf(fp, "\n");
        blue();
        high();
        /* printf("Message %s",new_file_flag ? "Saved in" : "Appended to"); */
        printf("%s %s", txt[41], new_file_flag? txt[42] : txt[43]);
        clear();
        cyan();
        printf(" %s\n", fname);
        fclose(fp);
    }
}



/*
 * SaveMsg, save or append current message in text file.
 */
static void
SaveMsg(const char *str)
{
    if (get_isempty()) {
	EmptyMsg();
    } else {
	char dummy[MAXPATHS];
	char fname[MAXPATHS];
	fname[0] = NUL_CHAR;
	sscanf(str, "%s %s", dummy, fname);
	/*@-strictops */
	if (fname[0] == NUL_CHAR) {	/*@=strictops */
	    char *ptr;
	    high();
	    yellow();
	    do {
		ptr = readline(txt[39], do_scroll);
	    } while (ptr == NULL);
	    clear();
	    fflush(stdout);
	    strcpy(fname, ptr);
	    free_string(ptr);
	}
	/*@-strictops */
	if (fname[0] == NUL_CHAR)	/*@=strictops */
	    printf("%s.\n", txt[40]);	/* "Aborted" */
	else
	    SaveMsgFin(fname);
    }
}

/*
 * ReadSeek, set-up and housekeeping before attempting to display message.
 */
static atp_BOOL_T
ReadSeek(read_command_t mode)
{
    atp_BOOL_T ret_code = TRUE;
    /* if needed, seek to new record in conference index file */
    if (((int)mode & (int)RPT_MASK) != 0) {
        const long here = ftell(fidx) - (long) (((int)mode & (int)RPT_MASK) * index_SIZE);
        if (here < 0L) {
            printf("%s\n", txt[15]);    /* "First message !" */
            ret_code = FALSE;
        } else if (fseek(fidx, here, SEEK_SET) != SUCCESS) {
            printf("%s.\n", txt[16]);   /* "Seek error" */
            ret_code = FALSE;
        }
    }
    /* get new index information and check for end of conference */
    /* use new index information to seek to new message in msg file */
    if (ret_code && fread(get_ptrIndex(ip_ReadSeek), index_SIZE, one_record, fidx) != one_record) {
        ret_code = FALSE;
        printf("%s\n", txt[17]);        /* "End of messages." */
	/*@-strictops */
        if (mode != SCAN && mode != FIND && mode != QUICK)
            AutoJoin();
	/*@=strictops */
    }
    if (ret_code && fseek(fmsg, get_MsgOffset(), SEEK_SET) != SUCCESS) {
        ret_code = FALSE;
        printf("%s!\n", txt[16]);       /* "Seek Error" */
    }
    return ret_code;
}


/*
 * error_switch_case - prints message when internal switch case error occurs.
 */
void
error_switch_case(int emode, const char *e_file, int e_line)
{
    fprintf(stderr, "%s %d: %s:%d\n", txt[109], emode, e_file, e_line);
    (void) sleep(3);
}


/*
 * change_status, re-mark message status based on NUKE, KILL, or PRIVATE.
 */
static void
change_status(read_command_t mode)
{
    byte tchar;
    fread(&tchar, (size_t) 1, one_record, fmsg);        /* read status into variable */
    switch ((int)mode) {
    case KILL:
	/*@-strictops -usedef */
        /* toggle kill status */
        tchar = (byte)(tchar ^ HIGH_ASCII);
        break;
    case NUKE:
        /* mark message as killed */
        tchar = (byte)(tchar | HIGH_ASCII);
        break;
	/*@=usedef */
    case PRIVATE:
        /* toggle private status */
        tchar = (byte) ((tchar == (byte)PRIVATE_MSG) ? PUBLIC_MSG : PRIVATE_MSG);
        break;
	/*@=strictops */
    default:
        /* switch() case error */
        error_switch_case((int)mode, __FILE__ , __LINE__ );
    }
    /* re-seek and write back new status */
    fseek(fmsg, get_MsgOffset(), SEEK_SET);
    fwrite(&tchar, (size_t) 1, one_record, fmsg);
    fflush(fmsg);
    fseek(fmsg, get_MsgOffset(), SEEK_SET);
}


/*
 * ReadNext - print next or previous or current message.
 */
atp_BOOL_T
ReadNext(const read_command_t mode)
{
    atp_BOOL_T ret_code = FALSE;
/*@-strictops */
    assert(mode == CROSS || mode == KILL || mode == NEXT || mode == NUKE
           || mode == PREVIOUS || mode == PRIVATE || mode == QUICK || mode == SCAN);
    if (get_isempty()) {
        EmptyMsg();
    } else if (!get_FilesOpen()) {
        printf("%s \n", txt[44]);       /* no mail in conf */
        if (mode != SCAN && mode != QUICK)
            AutoJoin();
    } else if (ReadSeek(mode)) {
        const size_t nrecs = (size_t) ((get_MsgSize() / block_SIZE) + 1) /* Msg + Head */ ;
        if (mode == KILL || mode == NUKE || mode == PRIVATE)
            change_status(mode);
        if (mode == QUICK) {    /* just read header and ignore message body */
/*@=strictops */
            fread(rbuf, block_SIZE, one_record, fmsg);
            PutHeader(QUICK, NULL, 0);
            ret_code = TRUE;
        } else if (nrecs <= (get_RbufSize() / block_SIZE) || reup(nrecs * block_SIZE)) {
            if (fread(rbuf, block_SIZE, nrecs, fmsg) == nrecs) {
                /* flag that there is a full buffer */
                set_FirstDone(TRUE, fd_ReadNext);
                Display(mode, NULL, 0);
                ret_code = TRUE;
            } else {
                printf("%s? \n", txt[18]);      /* Read Error */
            }
        } else {                /* memory allocation error */
            printf("%s  %lu bytes\n", txt[1], (unsigned long) (nrecs * block_SIZE));
        }
    }
    return ret_code;
}


/*
 * GetBbsFin, called at the end of GetBbs.
 */
static void
GetBbsFin(char *packet, char *tmp, const char *base, const char *name)
{
    struct stat Oldst;
    struct stat Newst;
#ifdef ATPDBG
    Oldst.st_mtime = Oldst.st_ctime = Oldst.st_atime = 0L;
    Newst.st_mtime = Newst.st_ctime = Newst.st_atime = 0L;
#endif
    sprintf(packet, "%s%s", MailPath, tmp);
    if (stat(packet, &Newst) != 0) {
        printf("GetBbs() Can't stat packet\n");
        (void)sleep(3);
    }
    if (stat(base, &Oldst) != 0) {
        printf("GetBbs() Can't stat base\n");
        (void)sleep(3);
    }
#ifdef ATPDBG
    assert(Oldst.st_mtime > 0L);
    assert(Newst.st_mtime > 0L);
#endif
    if (Newst.st_mtime > Oldst.st_mtime) {
        sprintf(tmp, "Load %s", name);
        Load(tmp);
    } else {
        printf("\n");
        blue();
        high();
        printf("%s. %s.", txt[46], txt[47]);    /* mail older than last update */
        green();
        fflush(stdout);
        printf("\n");
        sprintf(tmp, "Read %s", name);
        Read(tmp);
    }
}


/*
 * GetBbs, called by main() with argv[1] and looks for a new QWK packet.
 * if packet is older than message base it just joins the BBS 
 */
static void
GetBbs(const char *name)
{
    char tmp[MAXPATHS];
    char packet[MAXPATHS];
    strcpy(tmp, name);
    if (strstr(name, ".qw") == NULL)
        strcat(tmp, ".qwk");
    sprintf(packet, "%s%s", MailPath, tmp);
    if (access(packet, R_OK) == FAILURE) { /* No new mail found */
        printf("%s\n", txt[31]);        /* "no new mail" */
        Read(name);
    } else {
        char *tpt;
        char base[MAXPATHS];
        strcpy(packet, tmp);
        if ((tpt = strrchr(packet, '.')) != NULL)
            *tpt = NUL_CHAR;
        sprintf(base, "%s%s%c%s", HomePath, packet, SEP, CNTRL_FILE);
        if (access(base, F_OK) == FAILURE) {
            printf("\n%s. ", txt[45]);  /* New BBS in this base. */
            sprintf(tmp, "load %s", name);
            Load(tmp);
        } else {
            GetBbsFin(packet,tmp,base,name);
        }
    }
}


/*
 * scruter, scan through the message headers.
 */
static void
scruter(read_command_t mode)
{
    int m;
    long msgnumb = get_MsgNum();
    /*@-strictops */
    assert( mode == SCAN || mode == QUICK );
    if (mode == SCAN) /*@=strictops */
        m = get_ScrnRows() / 6;
    else
        m = get_ScrnRows() - 1;
    do {
        int i = 0;
        CLSCRN();
        for (; i < m && ReadNext(mode); i++)
            /* save current message number */
            msgnumb = get_MsgNum();
        if (i < m)
            break;
    } while (more(YES));

    /* fill msg buffer from current message */
    if (msgnumb >= 0L) {
        char seeker[100];
        sprintf(seeker, "%ld", msgnumb ? msgnumb : msgnumb + 1L);
        /* fill message buffer with last message */
        if (SeekNum(seeker) && msgnumb == 0L)
            rewind(fidx);
    }
}


/*
 * showterms, display GNU license.
 */
static void
showterms(void)
{
    int i = 0;
    while (terms[i] != NULL && i < 50) {
        printf("%s\n", terms[i]);
        i++;
    }
}

/*
 * Qlist, sets up shell command line for listing spooled QWK packets.
 * sorts them by creation time using shell tools
 */
static void
Qlist(void)
{
    char CONSPTR list_our_qwk_packets = get_atprc_str(qwklst);
    char our_current_dir[MAXPATHS];
    char our_mail_dir[MAXPATHS];
    int len;
    assert(list_our_qwk_packets != NULL);
    strcpy(our_mail_dir, MailPath);
    len = strlen(our_mail_dir);
    our_mail_dir[--len] = NUL_CHAR;
    /*@-usedef */
    GETCWD(our_current_dir, MAXPATHS);  /* save current directory      */
    /*@=usedef */
    CHPATH(our_mail_dir);               /* change to Mail directory    */
    (void)system(list_our_qwk_packets);       /* execute command line        */
    CHPATH(our_current_dir);            /* restore current directory   */
    free_string(list_our_qwk_packets);
}

/*
 * Blist, lists bulletins from a BBS using shell commands.
 */
static void
Blist(void)
{
    char CONSPTR command_line_to_list_bulletins = get_atprc_str(bltlst);
    char our_current_dir[MAXPATHS];
    char the_current_bbs_dir[MAXPATHS];

    sprintf(the_current_bbs_dir, "%s%s", HomePath, CurBoard);
    /*@-usedef */
    GETCWD(our_current_dir, MAXPATHS);      /* save current directory      */
    /*@=usedef */
    CHPATH(the_current_bbs_dir);            /* change to BBS directory     */
    (void)system(command_line_to_list_bulletins); /* list avaliable bulletins    */
    CHPATH(our_current_dir);                /* restore current directory   */
    free_string(command_line_to_list_bulletins);
}

/*
 * build_prompt, creates various prompts for use by readline().
 */
static void
build_prompt(char *p)
{
    strcpy(p, "\r");
    p++;
    if (ansi) {
        if (get_pmail() && (get_CurConf() == findCindex(PERS_CONF))) {
            sprintf(p, PRMT2, CurBoard, ConfNames[get_CurConf()]);
        } else {
            sprintf(p, PRMT1, CurBoard, ConfNames[get_CurConf()]);
        }
    } else {                    /* termcap */
#ifndef HAVE_LIBTERMCAP
        sprintf(p, SRMPT, CurBoard, ConfNames[get_CurConf()]);
#else
        if (get_pmail() && (get_CurConf() == findCindex(PERS_CONF))) {
            /* build a fancy blinking prompt */
            /* A_HIGH "%s [ " A_BLINK "%s" A_CLEAR A_HIGH " ] > " */
            /* STUB   sprintf (prompt, PRMT2, CurBoard, ConfNames[get_CurConf()]); */
	    /*@-returnvalint */
            tputs_cat(p, bold_on);
            strcat(p, CurBoard);
            strcat(p, " [ ");
            tputs_cat(p, blink_on);
            strcat(p, ConfNames[get_CurConf()]);
            tputs_cat(p, clear_attr);
            tputs_cat(p, bold_on);
	    /*@=returnvalint */
            strcat(p, " ] > ");
        } else {
            int ci;
            ci = tputs_cpy(p, bold_on);
            sprintf(p + ci, SRMPT, CurBoard, ConfNames[get_CurConf()]);
        }
#endif
    }
}


/*
 * do_ext_shell, shell out of atp into external shell.
 */
static void
do_ext_shell(char *tmp)
{
    /*@-strictops */
    if (tmp[1] != NUL_CHAR) {  /*@=strictops */
        (void)system(tmp + 1);
    } else {
        char CONSPTR ptr = getenv(SHELL);
        if (ptr != NULL)
            (void)system(ptr);
    }
    /* reset terminal paramaters */
    restore_terminal();
}


/*
 * sing_args, called by ReadShell().
 */
static void
sing_args(char *tmp)
{
    switch (tmp[0]) {
    case '!':
        do_ext_shell(tmp);
        break;
    case 'c':
    case 'C':
        change_msg(tmp);
        break;
    case 'e':
    case 'E':
        enter_msg(tmp);
        break;
    case 'j':
    case 'J':
        chconf(tmp);
        break;
    case 'r':
    case 'R':
        reply_msg(tmp);
        break;
    case 's':
    case 'S':
        SaveMsg(tmp);
        break;
    case 'x':
    case 'X':
        xpost_msg(tmp);
        break;
    default:
        break;
    }
}



/*
 * sing_noargs, called by ReadShell().
 */
static void
sing_noargs(char *tmp)
{
    switch ((int) tmp[0]) {
    case '?':
        Help();
        break;
    case 'a':
    case 'A':
        show_again();
        break;
#ifdef ATPDBG
    case 'i':
        printf("CurCon: %d   Index.LastRead: %ld\n", get_CurConf(), get_MsgLastRead());
        printf("ScrnRows: %d   ScrnCols: %d \n", get_ScrnRows(), get_ScrnCols());
        break;
#endif
    case 'k':
    case 'K':
        ReadNext(KILL);
        break;
    case 'm':                   /* toggle ANSI mode on/off */
    case 'M':
        ansi_toggle();
        break;
    case 'p':
    case 'P':
        mark_private();
        break;
    case 'n':
    case 'N':
        AutoJoin();
        break;
    case '+':
    case 0:
        ReadNext(NEXT);
        break;
    case '-':
        ReadNext(PREVIOUS);
        break;
    }
}


/*
 * seek_last, called by ReadShell().
 */
static void
seek_last(void)
{
    char go_to_num[32];
    sprintf(go_to_num, "%ld", get_TotMsg());
    GoToNum(go_to_num);
}

/*
 * cmds[], used by ReadShell() to spawn system commands.
 */

#if defined(UNIXCMDS)
#define MAXCMDS  24             /* should equal count of the following array */
static const char CONSPTR cmds[MAXCMDS] =
{"cd", "cp", "echo", "free", "grep", "ls", "ln", "lpr", "man", "sort", "sync",
 "mkdir", "more", "mv", "pwd", "ps", "cwd", "rm", "rmdir", "set", "cat", "less", "df", "du"
};
#else
#define MAXCMDS 16
static const char CONSPTR cmds[MAXCMDS] =
{"del", "dir", "chkdsk", "md", "rd",
 "cd", "mem", "more", "mkdir", "set", "print", "rmdir", "sort", "copy", "xcopy", "type"};
#endif


/*
 * sys_cmds, invoke shell commands.
 */
static atp_BOOL_T
sys_cmds(char *tmp)
{
    atp_BOOL_T ret_code = FALSE;
    int i = 0;
    for (; i < MAXCMDS; i++) {
        if (strnicmp(tmp, cmds[i], strlen(cmds[i])) == SUCCESS) {
            (void)system(tmp);
            ret_code = TRUE;
            break;
        }
    }
    return ret_code;
}


/*
 * get_shell_input, displays prompt and reads input for main command shell.
 */
static char *
get_shell_input(char *oldtmp)
{
    char prompt[200];
    char *tmp = oldtmp;
    if (tmp != NULL) {
	free_string(tmp);
	tmp = NULL;
    }
    luxptr = NULL;
    /* clear highlighting attributes */
    clear();
    /* clear line */
    deleol();
    build_prompt(prompt);
    do {
	tmp = readline(prompt, do_scroll);
    } while (tmp == NULL);
    add_history(tmp);
    clear();
    fflush(stdout);
    StripDel(tmp);
    StripLSpace(tmp);
    return tmp;
}


/*
 * do_quit, cleans up before exiting main ReadShell() loop.
 */
static void
do_quit(char *tmp)
{
    if (!get_isempty())
        UpdateConf(update_last_read_ptr);       /* Save current conf pointer */
    query_rep_exists();
    free_string(tmp);
    clear();
    return;
}


/*
 * is_quit, parses for quit command and then invokes do_quit().
 */
static atp_BOOL_T
is_quit(char *tmp)
{
    atp_BOOL_T ret_code = FALSE;
    /*@-strictops */
    if ((stricmp(tmp, "quit") == SUCCESS
         || (tmp[0] != NUL_CHAR && (tmp[1] == SPC_CHAR || tmp[1] == NUL_CHAR)
             && (tmp[0] == 'q' || tmp[0] == 'Q' || tmp[0] == 'G' || tmp[0] == 'g')))) {
    /*@=strictops */
        do_quit(tmp);
        ret_code = TRUE;
    }
    return ret_code;
}


/*
 * ReadShell, the ATP command interpreter.
 */
static void
ReadShell(void)
{
    char *tmp = NULL;
    while (1) {
	assert((fmsg != NULL || fidx == NULL) && (fidx != NULL || fmsg == NULL));
	assert( fmsg == NULL || get_FilesOpen());
	assert( !get_FilesOpen() || fmsg != NULL );
        tmp = get_shell_input(tmp);
        if (!sys_cmds(tmp)) {
            if (strnicmp(tmp, "q!", 2) == SUCCESS)
                tmp[1] = NUL_CHAR;
            if (Numeric(tmp))
                GoToNum(tmp);
            else if (strnicmp(tmp, "show ", 5) == SUCCESS)
                showterms();
            else if (strnicmp(tmp, "aide", 4) == SUCCESS)
                Help();
            else if (strnicmp(tmp, "cls", 3) == SUCCESS)
                CLSCRN();
            else if (strnicmp(tmp, "clean", 5) == SUCCESS)
                Purge();
            else if (strnicmp(tmp, "conf", 4) == SUCCESS)
                ListConf();
            else if (strnicmp(tmp, "date", 4) == SUCCESS)
                Date();
            else if (strnicmp(tmp, "fido", 4) == SUCCESS)
                togltag();
            else if (strnicmp(tmp, "find ", 5) == SUCCESS)
                findtxt(tmp, FIND);
            else if (strnicmp(tmp, "pcb", 3) == SUCCESS)
                toglpcb();
            else if (strnicmp(tmp, "ts", 2) == SUCCESS)
                findtxt(tmp, FIND);
            else if (strnicmp(tmp, "help", 4) == SUCCESS)
                Help();
            else if (stricmp(tmp, "last") == SUCCESS)
                seek_last();
            else if (stricmp(tmp, "next") == SUCCESS)
                findtxt(tmp, NEXT);
            else if (strnicmp(tmp, "rot", 3) == SUCCESS)
                rot_toggle();
            else if (strnicmp(tmp, "scan", 4) == SUCCESS)
                scruter(SCAN);
            else if (strnicmp(tmp, "qscan", 5) == SUCCESS)
                scruter(QUICK);
            else if (strnicmp(tmp, "ql", 2) == SUCCESS)
                Qlist();
            else if (strnicmp(tmp, "bl",2) == SUCCESS && strnicmp(tmp, "blt-",4) != SUCCESS)
                Blist();
            /* toggle VT102 line graphics */
            else if (strnicmp((char *) tmp, "graph", 4) == SUCCESS)
                test_line_graphics();
            else if (strnicmp(tmp, "head", 4) == SUCCESS) {
                head_toggle();
            } else if (strnicmp(tmp, "time", 4) == SUCCESS)
                Date();
            else if (strnicmp(tmp, "welcome", 4) == SUCCESS)
                WelcomeMsg(wm_hello,NULL);
            else if (strnicmp(tmp, "hello", 5) == SUCCESS)
                WelcomeMsg(wm_hello,NULL);
            else if (strnicmp(tmp, "goodbye", 5) == SUCCESS)
                WelcomeMsg(wm_goodbye,NULL);
            else if (strnicmp(tmp, "news", 4) == SUCCESS)
                WelcomeMsg(wm_news,NULL);
            else if (strnicmp(tmp, "file", 4) == SUCCESS)
                WelcomeMsg(wm_newfiles,NULL);
            else if (strnicmp(tmp, "door", 4) == SUCCESS)
                WelcomeMsg(wm_door_id,NULL);
            else if (strnicmp(tmp, "blt-", 4) == SUCCESS)
                WelcomeMsg(wm_bulletin,tmp);
            else if (strnicmp(tmp, "load", 4) == SUCCESS)
                Load(tmp);      /* Load new mail */
            else if (strnicmp(tmp, PRVERR, 5) == SUCCESS)
                chkerror();     /* internal check */
            else if (strnicmp(tmp, "rev", 3) == SUCCESS)
                Read(tmp);      /* read existing mail */
            else if (strnicmp(tmp, "s ", 3) == SUCCESS)
                SaveMsg(tmp);      /* save message */
            else if (strnicmp(tmp, "tag", 3) == SUCCESS)
                Tag(tmp);       /* change tagline */
            else if (stricmp(tmp, "reset") == SUCCESS)
                UpdateConf(reset_last_read_ptr);
            else if (is_quit(tmp))
                return;
            /*@-strictops */
            else if (tmp[0] == NUL_CHAR || tmp[1] == NUL_CHAR || tmp[1] == SPC_CHAR) {
                sing_args(tmp);
                if (tmp[0] == NUL_CHAR || tmp[1] == NUL_CHAR)
            /*@=strictops */
                    sing_noargs(tmp);
            }
        }
    }
}

/*
 * main_malloc, do RdCn_main_init() then malloc rbuf message buffer. 
 */
static atp_BOOL_T
main_malloc(void)
{
    atp_BOOL_T ret_code = TRUE;
    if ( !RdCn_main_init() || (rbuf = (char *) malloc(mybuf + block_SIZE)) == NULL) {
        printf("%s\n", txt[1]); /* Not enough memory */
        printf("%lu bytes\n", (unsigned long)mybuf);
        ret_code = FALSE;
    }
    return ret_code;
}

/*
 * main_init, system specific initialization routines for main().
 */
static void
main_init(void)
{
    save_terminal();            /* save terminal parameters */
    init_term_type();
#ifdef DOSTIME
    msdos_time_init();
#endif
#ifdef HAVE_SIGACTION
    sig_init();
#endif
#if defined(TIOCGWINSZ) && defined(SIGWINCH)
    sig_chwin();
#endif
    sig_ignore(SIGINT);
    TTYinfo();
#if defined(SPAWNO) && defined(__MSDOS__)
    /* msdos function only, support for Ralph Brown's swap library */
    init_SPAWNO((WorkPath[0] ? WorkPath : (char *) 0), 0xff);
    __spawn_ext = 0;
#endif
}

/*
 * main_title, display the opening title screen.
 */
static void
main_title(char *bbs_nm)
{
    Clean();
    clear();
    bblack();
    Title();
    cyan();
    fflush(stdout);
    rl_initialize();
    if (bbs_nm)
        GetBbs(bbs_nm);
    /* setup the default TagLines */
    tag_set_by_main();
}

/*
 * main_exit, called from main() before exiting to do clean-up.
 */
static void
main_exit(void)
{
    free_buffer(rbuf, get_RbufSize() + block_SIZE);
    if (get_FilesOpen()) {
        fclose_fmsg_fidx();
    }
    RdCn_exit_free();
    clear();
    fflush(stdout);
    restore_terminal();         /* restore terminal parameters */
    Clean();
    rl_cleanup();
}

/*
 * main - calls init routines then invokes command interpreter.
 */
int
main(int argc, char *argv[])
{
    int ret_code = FAILURE;
    if (MakeHomePath() && /*@i1*/ ReadConfig() == ATP_OK && main_malloc()) {
        main_init();
        main_title(argc > 1 ? argv[1] : (char *) 0 );
        ReadShell();
        main_exit();
        ret_code = SUCCESS;
    }
    return ret_code;
}

