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
qlib.c
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

#define _ATP_QLIB_C 1
#include "reader.h"
#include "readlib.h"
#include "ansi.h"
#include "qlib.h"
#include "makemail.h"
#ifdef __LCLINT__
#endif

/*
 * StripLSpace - strip white space on left side of string.
 */
void
StripLSpace(char *ptr)
{
    while (/*@i2*/ *ptr == SPC_CHAR || *ptr == TAB_CHAR)
	ShiftLeft(ptr, 1);
}


/*
 * free_buffer - overwrite old buffer with garbage before discarding.
 */
void
free_buffer(void *s, size_t len)
{
    assert(s != NULL);
    if (s != NULL) {
	(void)memset(s, (int)'G', len);
	free(s);
    }
}


/*
 * free_string - overwrite an old string with garbage before discarding.
 */
void
free_string(char *s)
{
    assert(s != NULL);
    free_buffer(s, strlen(s)+(size_t)1);
}


/*
 * do_unlink - unlink file and report any errors via perror().
 */
void
do_unlink(const char *fname)
{
    if (unlink(fname) == FAILURE) {
	puts("do_unlink(),");
	perror(fname);
	(void)sleep(3);
    }
}


/*
 * readCnum - convert 13 bit little-endian integer in to ordinary integer.
 */
int
readCnum(const char *ptr)
{
    /* only the first 5 bits of the high byte are guaranteed valid */
    const byte valid_high_bits = (byte)0x1f;
    byte p, q;
    unsigned int j;

    p = (byte) *ptr;
    ptr++;
    q = (byte) *ptr;
    q &= valid_high_bits;
    j = (unsigned int) q;
    j <<= 8;
    j += (unsigned int) p;

    return (j);
}


/* 
 * findCindex - find actual index into array so as to locate conference name.
 */
int
findCindex(const int n)
{
    int i = LastConf;
    for (; i >= 0; i--) {
	if (ConfNumbers[i] == n)
	    break;
    }
    return i;
}

/*
 * verify_new_file - checks access rights and handles upper case under Unix.
 */
atp_ERROR_T
verify_new_file(char *Src, const char *SrDir, const char *pilgrim)
{
    atp_ERROR_T ret_code = ATP_OK;
    sprintf(Src, "%s%c%s", SrDir, SEP, pilgrim);
    if (access(Src, F_OK) == FAILURE) {
#ifdef SYS_UNIX
	/* try the upper-case variant of the name */
	const size_t path_len = strlen(SrDir);
	/*@-returnval */
	strupr(Src + path_len + (size_t)1);
	if (access(Src, F_OK) == FAILURE) {
	    strlwr(Src + path_len + (size_t)1);
	    ret_code = ATP_ERROR;
	}
	/*@=returnval */
#else
	ret_code = ATP_ERROR;
#endif
    }
    return ret_code;
}

/*
 * WriteIndex - add an index struct to the index file.
 */
atp_ERROR_T
WriteIndex(FILE * wi_fx, const long wi_count, const unsigned long wi_Size, const long wi_Offset)
{
    atp_ERROR_T ret_code = ATP_OK;
    struct MyIndex Yndex;

    Yndex.LastRead = wi_count ? 0L : VIRGINDX;	/* no count implies new index */
    Yndex.MaxMsg = 0L;		/* future use */
    Yndex.MsgNum = wi_count;
    Yndex.Offset = wi_Offset;
    Yndex.Size = wi_Size;

    if (fwrite(&Yndex, index_SIZE, one_record, wi_fx) != one_record) {
	/* "error writing index file " */
	printf("%s !\n", txt[74]);
	(void) sleep(3);
	ret_code = ATP_ERROR;
    }
    return ret_code;
}


/*
 * OpenCon - open a ".cnf" or an "idx"  file for read/write.
 */
atp_ERROR_T
OpenCon(FILE ** oc_pf, FILE ** oc_fs, const char *dspath)
{
    atp_ERROR_T ret_code = ATP_OK;

    if (access(dspath, F_OK) == FAILURE ) {	/* Create if not exist */
	*oc_pf = fopen(dspath, "wb");
	fclose(*oc_pf);
    }
    if ((*oc_pf = fopen(dspath, "r+b")) == NULL) {
	ret_code = ATP_ERROR;
	/* "unable to open file" */
	printf("%s %s...\n", txt[51], dspath);
	if (*oc_fs != NULL)
	    fclose(*oc_fs);
    }
    return ret_code;
}


#if defined(HAVE_FTIME)		/* msdos Borland Turbo C++ */
/*
 * fcopy_ftime, sync destination file time to souce file time.
 */
ATP_INLINE atp_ERROR_T
fcopy_ftime(int dst, int src)
{
    struct ftime dsrctime;

    getftime(src, &dsrctime);
    setftime(dst, &dsrctime);
    return ATP_OK;
}

#elif defined(HAVE_UTIME)	/* Unix, MSC */

/*
 * fcopy_utime, sync destination file time to souce file time.
 */
ATP_INLINE atp_ERROR_T
fcopy_utime(const char *destin, const char *source)
{
    atp_ERROR_T ret_code = ATP_OK;
    struct stat srctime;
    struct utimbuf restime;

    if (stat(source, &srctime) == FAILURE) {
	printf("%s %s\n", txt[49], source);	/* "unable to read file" */
	ret_code = ATP_ERROR;
    } else {
	restime.actime = srctime.st_atime;
	restime.modtime = srctime.st_mtime;
	/*@-returnval */
	utime(destin, &restime);
	/*@=returnval */
    }
    return ret_code;
}
#endif

static const size_t fcopy_buf_size = FCPYBUF;

/*
 * fcopy_loop, the heart of of the fcopy team actually does the copying.
 */
ATP_INLINE atp_ERROR_T
fcopy_loop(int dst, int src, char *buf)
{
    atp_ERROR_T ret_code = ATP_OK;
    int rb;

    for (;;) {
	if ((rb = read(src, buf, fcopy_buf_size)) > 0) {
	    if (write(dst, buf, (size_t) rb) != rb) {
		perror("fcopy_loop() write:");
		(void) sleep(1);
		ret_code = ATP_ERROR;
		break;
	    }
	} else {
	    if (rb < 0) {
		perror("fcopy_loop() read:");
		(void) sleep(1);
		ret_code = ATP_ERROR;
	    }
	    break;
	}
    }
    return ret_code;
}


#ifdef __TURBOC__
/* turbo c: suppress unused parameter warnings for next function only */
#pragma argsused
#endif

/*
 * fcopy_fin, finish the work for fcopy and and then clean up.
 *  note: the arguments 'source' and 'destin' aren't used when HAVE_FTIME 
 *        is defined. This is not an really error.
 */
ATP_INLINE atp_ERROR_T
fcopy_fin(int dst, int src, const char *destin, const char *source, char *buf)
{
    atp_ERROR_T ret_code;

    /*@-strictops */
    if ((ret_code = fcopy_loop(dst, src, buf)) == ATP_OK) { /*@=strictops */
#if defined(HAVE_FTIME)		/* msdos Borland Turbo C++ */
	ret_code = fcopy_ftime(dst, src);
#elif defined(HAVE_UTIME)	/* Unix, MSC */
	ret_code = fcopy_utime(destin, source);
#endif
    }
    (void) close(src);
    (void) close(dst);
    return ret_code;
}

#ifdef __TURBOC__
/* turbo c: suppress unused parameter warnings for next function only */
#pragma argsused
#endif

/*
 * fcopy, fast copy of a file: returns ATP_OK on success ATP_ERROR on error.
 *  It is helpful to be able to duplicate the source file's time stamp
 *  too. While not absolutely needed for ATP, there are times when
 *  it is useful when dealing with mail packets from systems whose time
 *  clock is out of synch with yours. This is a fine point and if you
 *  are porting ATP to a new system, you may comment out the time functions
 *  in fcopy_fin() to ease initial porting with no serious side effects. 
 */
static atp_ERROR_T
fcopy(const char *source, const char *destin)
{
#ifdef SYS_UNIX
    const mode_t perms = 0666;
#else
    const int perms = (S_IREAD | S_IWRITE);
#endif
    char *buf;
    int src, dst;
    atp_ERROR_T ret_code = ATP_ERROR;
    const int acc = (O_RDWR | O_CREAT | O_TRUNC);

    if ((buf = (char *) malloc(fcopy_buf_size)) == NULL) {
	printf("%s", txt[1]);	/* memory allocation failed */
    } else {
	if ((src = open(source, O_RDONLY)) == FAILURE) {
	    printf("%s %s\n", txt[49], source);		/* "unable to read file" */
	} else if ((dst = open(destin, acc, perms)) == FAILURE) {
	    printf("%s %s\n", txt[50], source);		/* "unable to create file" */
	} else {
	    ret_code = fcopy_fin(dst, src, destin, source, buf);
	}
	free_buffer(buf, fcopy_buf_size);
    }
    return ret_code;
}


/*
 * Work2Home, moves bulletins/news/etc. to atp home BBS directory.
 */
static void
Work2home(const char *DstDir, const char *SrDir, const char *pilgrim)
{
    if ( /*@i2 */ *pilgrim != NUL_CHAR && *pilgrim != SPC_CHAR) {
	/* Source file Message.DAT */
	char Src[MAXPATHS];
	if (verify_new_file(Src, SrDir, pilgrim) == ATP_OK) {
	    char Dst[MAXPATHS];
	    sprintf(Dst, "%s%c%s", DstDir, SEP, pilgrim);
	    /* Conference data  file   */
	    if (access(Dst, F_OK) == SUCCESS && unlink(Dst) == FAILURE) {
		perror("Work2home()");
		printf("Can't remove destination %s %c\n", Dst, BELL);
		/*@-returnval */
		(void) sleep(3);
		/*@=returnval */
	    }
#if defined(HAVE_LINK)
	    /* try linking first */
	    if (link(Src, Dst) == FAILURE)
		/* unix will try copying too */
#endif          
		/*@-strictops */
		if (fcopy(Src, Dst) == ATP_ERROR)      /*@=strictops */
		{
		    printf("Can't link source %s\n", Src);
		    printf("Check destination permissions %s\n", Dst);
		    exit(EXIT_FAILURE);
		}
	    do_unlink(Src);
	}
    }
}



/* 
 * BEGIN READ CONTROL.DAT ROUTINES
 *
 */ 

/* maximum number of characters for call to fget() */
#define FGET_RD_SIZE 126

/* common to all ReadControl() subroutines */
static FILE *rdc_fp;

/*
 * RdCn_skiplines, skips and ignores lines in control.dat.
 */
static atp_BOOL_T
RdCn_skiplines(int ct, char *buf)
{
    int i = 0; 
    atp_BOOL_T ret_code = TRUE;
    assert(ct > 0);
    for (; i < ct && ret_code; i++)
	ret_code = fget(buf, FGET_RD_SIZE, rdc_fp);
    return ret_code;
}

/* global data exported by this module */
int         LastConf    = -1;  /* index to last conference in arrays */ 
conf_name  *ConfNames   = NULL; 
int        *ConfNumbers = NULL; 
atp_BOOL_T *ConfActive  = NULL;


static int t_LastConf = -1; 	
static conf_name  *t_ConfNames   = NULL; 
static int        *t_ConfNumbers = NULL; 
static atp_BOOL_T *t_ConfActive  = NULL;

/*
 * RdCn_t_invalidate, trash temp data and temp pointers.
 */
static void
RdCn_t_invalidate(void)
{
    /* invalidate temporary data and pointers */
    t_LastConf = -1;
    t_ConfActive = NULL;
    t_ConfNumbers = NULL;
    t_ConfNames = NULL;
}


/*
 * RdCn_test_failed_malloc, free partial mallocs after failure.
 */
static void
RdCn_test_failed_malloc(atp_BOOL_T success)
{
    if (!success) {
        if (t_ConfActive != NULL)
            free(t_ConfActive);
        if (t_ConfNumbers != NULL)
            free(t_ConfNumbers);
        assert(t_ConfNames == NULL);
        RdCn_t_invalidate();
        clear();
        fprintf(stderr, "%s  RdCn_do_malloc()\n", txt[1]);
    }
}


/*
 * RdCn_do_malloc, static helper function for ReadControl().
 * malloc global arrays 
 */
static atp_BOOL_T
RdCn_do_malloc(const int index_to_last_element_of_array)
{
    const size_t array_size = (size_t) (index_to_last_element_of_array + 1);
    const size_t new_bcnt = array_size * (size_t)sizeof(atp_BOOL_T);
    const size_t new_icnt = array_size * (size_t)sizeof(int);
    const size_t new_scnt = array_size * (size_t)CNF_NAM_SIZ;
    atp_BOOL_T result = FALSE;
    assert(-1 < index_to_last_element_of_array);

    if ((t_ConfActive = (atp_BOOL_T *) malloc(new_bcnt)) != NULL)
	if ((t_ConfNumbers = (int *) malloc(new_icnt)) != NULL)
	    if ((t_ConfNames = (conf_name *) malloc(new_scnt)) != NULL)
		result = TRUE;

    RdCn_test_failed_malloc(result);
    return result;
}

/*
 * RdCn_do_free, de-allocates old arrays used by previous BBS message base. 
 */
static void
RdCn_do_free(const int CLast, atp_BOOL_T *CActive, int *CNumbers, conf_name *CNames)
{
    const size_t old_bcnt = (size_t) ((CLast+1) * sizeof(atp_BOOL_T));
    const size_t old_icnt = (size_t) ((CLast+1) * sizeof(int));
    const size_t old_scnt = (size_t) ((CLast+1) * CNF_NAM_SIZ);
    if (CActive != NULL)
	free_buffer(CActive, old_bcnt);
    if (CNumbers != NULL)
	free_buffer(CNumbers, old_icnt);
    if (CNames != NULL)
	free_buffer(CNames, old_scnt);
}

/*
 * RdCn_get_conf_names, static helper function for ReadControl().
 */
static atp_BOOL_T
RdCn_get_conf_names(char *buf)
{
    int i, numconf = 0 ; 
    atp_BOOL_T ret_code = TRUE;
    for (i = 0; i < t_RCONF_IDX; i++) {
	if (fget(buf, FGET_RD_SIZE, rdc_fp) && (numconf = atoi(buf)) >= 0
	    && fget(t_ConfNames[i], 16, rdc_fp)) {
	    StripLSpace(t_ConfNames[i]);
	    t_ConfNumbers[i] = numconf;
	} else {
	    ret_code = FALSE;
	    break;
	}
    }
    /* initialize personal and replies conference name and index */
    if (ret_code) {
	t_ConfNumbers[t_PCONF_IDX] = PERS_CONF;
	t_ConfNumbers[t_RCONF_IDX] = REPL_CONF;
	/* "personal" */
	strncpy(t_ConfNames[t_PCONF_IDX], txt[103], (size_t)(CNF_NAM_SIZ - 1));
	/* "replies " */
	strncpy(t_ConfNames[t_RCONF_IDX], txt[104], (size_t)(CNF_NAM_SIZ - 1));
	*(t_ConfNames[t_PCONF_IDX]+(CNF_NAM_SIZ - 1)) = (char) 0 ;
	*(t_ConfNames[t_RCONF_IDX]+(CNF_NAM_SIZ - 1)) = (char) 0 ;
    }
    return ret_code;
}

static char Welcome[16];
static char News[16];
static char GoodBye[16];
static char NewFiles[] = "newfiles.dat";
static char DoorId[] = "door.id";
static char nothing[] = "";

static char *BoardName = NULL ;	/* Name of BBS read from control.dat */
static char t_BoardName[BBS_NM_LEN+2];	/* Name of BBS read from control.dat */
static char *UserName = NULL ; 	        /* user name taken from control.dat */ 
static char t_UserName[50]; 	/* temp user name taken from control.dat */ 

/*
 * fatal_malloc - exit with error message on malloc error.
 */
void 
test_fatal_malloc(void *ptr, const char *filename, int line_num)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s  %s:%d\n", txt[1], filename, line_num);
        exit(EXIT_FAILURE);
    }
}


/*
 * get_cntrl_str - returns strdup of control.dat variable based on token.
 *  tokens are:  usrnm, boardnm
 */
char *
get_cntrl_str(const cntrl_str_t token)
{
    char *strnm = NULL;
    switch (token) {
    case usrnm:
	strnm = strdup(UserName);
	break;
    case boardnm:
	strnm = strdup(BoardName);
	break;
    default:
	/* switch() case error */
	error_switch_case((int)token, __FILE__, __LINE__);
	strnm = nothing ;
    }
    test_fatal_malloc(strnm, __FILE__, __LINE__);
    return strnm;
}


/* 
 * MoveBulletins, move bulletins from workdir to atp bbs dir. 
 */
static void
MoveBulletins(const char *DestDir, const char *SrcDir)
{
    char **bulletin_name = NULL;
    size_t bulletin;
#ifdef DIRENTRY
    const size_t b_count = FindMatches(SrcDir, "blt-", &bulletin_name);
#else				/* msdog and windoze */
    size_t b_count;
    char mwbuf[MAXPATHS];
    sprintf(mwbuf, "%s%c%s", SrcDir, SEP, "blt-");
    b_count = FindMatches((char *) mwbuf, &bulletin_name);
#endif
    if (bulletin_name != NULL) {
	for (bulletin = 0; bulletin < b_count; bulletin++, bulletin_name++) {
	    Work2home(DestDir, SrcDir, *bulletin_name);
	    free_string(*bulletin_name);
	}
	bulletin_name -= b_count;
	free(bulletin_name);
    }
}


/* 
 * MoveWork - wrapper for Work2Home().
 */
void
MoveWork(const char *DestDir, const char *SrcDir)
{
    /* Update Control.dat in bbs dir */
    Work2home(DestDir, SrcDir, CNTRL_FILE);

    /* Update Newfiles.dat in bbs dir */
    Work2home(DestDir, SrcDir, NewFiles);

    /* Update Welcome file in bbs dir */
    Work2home(DestDir, SrcDir, Welcome);

    /* Update GoodBye file in bbs dir */
    Work2home(DestDir, SrcDir, GoodBye);

    /* Update News file in bbs dir */
    Work2home(DestDir, SrcDir, News);

    /* Update GoodBye file in bbs dir */
    Work2home(DestDir, SrcDir, DoorId);

    /* Move bulletins to bbs dir */
    MoveBulletins(DestDir, SrcDir);
}

/*
 * get_wm_ptr, returns strdup() of control.dat auxillary file names.
 */
static char *
get_wm_ptr( welcome_msg_t msg_name )
{
    char *wm_ptr = NULL;
    switch (msg_name) {
    case wm_hello:
	wm_ptr = Welcome;
	break;
    case wm_news:
	wm_ptr = News;
	break;
    case wm_goodbye:
	wm_ptr = GoodBye;
	break;
    case wm_newfiles:
	wm_ptr = NewFiles;
	break;
    case wm_door_id:
	wm_ptr = DoorId;
	break;
    default:
	/* switch() case error */
	error_switch_case((int)msg_name, __FILE__, __LINE__);
    }
    return wm_ptr;
}

/*
 * WelcomeMsg - displays the BBS welcome file.
 */
void
WelcomeMsg( const welcome_msg_t msg_name, const char *mfile)
{
    char tmp[MAXPATHS];
    if (get_isempty()) {
	EmptyMsg();
	/*@-strictops */
    } else if (msg_name == wm_bulletin
	       || (mfile = get_wm_ptr(msg_name)) != NULL) {
	sprintf(tmp, "%s%s%c%s", HomePath, CurBoard, SEP, mfile);
	if (*mfile != NUL_CHAR && access(tmp, R_OK) == SUCCESS) {
	    /*@=strictops */
	    sprintf(tmp, "%s%s", HomePath, CurBoard);
	    view(tmp, mfile);
	} else {
	    printf("%s %s\n", txt[67], tmp);	/* "No file" */
	}
    }
}

/*
 * RdCn_get_misc_names, static helper function for ReadControl().
 *  gets Welcome, News, Goodbye names
 */
static void
RdCn_get_misc_names(char *buf)
{
    strcpy(Welcome, "welcome");
    strcpy(News, "news");
    strcpy(GoodBye, "script0");
    if (fget(buf, 15, rdc_fp)) {
	buf[15] = NUL_CHAR;
	strcpy(Welcome, strlwr(buf));
	if (fget(buf, 15, rdc_fp)) {
	    buf[15] = NUL_CHAR;
	    strcpy(News, strlwr(buf));
	    if (fget(buf, 15, rdc_fp)) {
		buf[15] = NUL_CHAR;
		strcpy(GoodBye, strlwr(buf));
	    }
	}
    }
}

/*
 * RdCn_get_user_names, static helper function for ReadControl().
 */
static atp_BOOL_T
RdCn_get_user_names(char *buf)
{
    atp_BOOL_T ret_code = FALSE;
    t_UserName[0] = NUL_CHAR ;
    if (fget(t_UserName, 50, rdc_fp) && RdCn_skiplines(3, buf))
	ret_code = t_UserName[0] == NUL_CHAR ? FALSE : TRUE ;
    else {
	char CONSPTR tp = get_atprc_str(usr1nm);
	strcpy(t_UserName,tp);
	free_string(tp);
	ret_code = t_UserName[0] == NUL_CHAR ? FALSE : TRUE ;
    }
    return ret_code;
}


/*
 * RdCn_switch_to_new_bbs, assign new pointers and free old arrarys.
 */
static void 
RdCn_switch_to_new_bbs(void)
{
    RdCn_do_free(LastConf, ConfActive, ConfNumbers, ConfNames);
    if (UserName != NULL)
	free_string(UserName);
    if (BoardName != NULL)
	free_string(BoardName);
    BoardName = strdup(t_BoardName);
    UserName = strdup(t_UserName);
    /* memory allocation error test */
    if (BoardName == NULL || UserName == NULL)
	test_fatal_malloc(NULL, __FILE__, __LINE__);
    LastConf = t_LastConf;
    ConfActive = t_ConfActive;
    ConfNumbers = t_ConfNumbers;
    ConfNames = t_ConfNames;
    RdCn_t_invalidate();
}


/*
 * RdCn_Fin, finish up reading control.dat.
 */
static atp_ERROR_T
RdCn_Fin(char *tmp)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    assert(t_LastConf == -1 && t_ConfNumbers == NULL && t_ConfNames == NULL && t_ConfActive == NULL);
    /* get new LastConf and add 2 for the Personal and Reply conferences */
    t_LastConf = atoi(tmp) + 2;
    if (0 <= t_LastConf && t_LastConf <= MAXCONF && RdCn_do_malloc(t_LastConf)) {
	if (RdCn_get_conf_names(tmp)) {
	    RdCn_switch_to_new_bbs();
	    assert(t_LastConf == -1 && t_ConfNumbers == NULL && t_ConfNames == NULL && t_ConfActive == NULL);
	    RdCn_get_misc_names(tmp);
	    printf("\n %s\n\n", BoardName);
	    ret_code = ATP_OK;
	} else {
	    RdCn_do_free(t_LastConf, t_ConfActive, t_ConfNumbers, t_ConfNames);
	    RdCn_t_invalidate();
	}
    }
    return ret_code;
}


/* 
 * RdCn_ReadControl - parses control.dat file.
 */
atp_ERROR_T
RdCn_ReadControl(const char *Path)
{
    atp_ERROR_T ret_code = ATP_ERROR;
    char tmp[MAXPATHS];
    /*@-strictops */
    if (verify_new_file(tmp, Path, CNTRL_FILE) == ATP_OK
	&& (rdc_fp = fopen(tmp, "rb")) != NULL
	&& fget(t_BoardName, BBS_NM_LEN, rdc_fp)
	&& RdCn_skiplines(5, tmp)
	&& RdCn_get_user_names(tmp)
	&& fget(tmp, FGET_RD_SIZE, rdc_fp)) {
	ret_code = RdCn_Fin(tmp);
    }
    /*@=strictops */
    assert(t_LastConf == -1 && t_ConfNumbers == NULL && t_ConfNames == NULL && t_ConfActive == NULL);
    fclose(rdc_fp);
    return ret_code;
}

/*
 * RdCn_main_init - sets up "empty" bbs on program initialization.
 */
atp_BOOL_T
RdCn_main_init(void)
{
    atp_BOOL_T ret_code;
    char CONSPTR tusrnm = get_atprc_str(usr1nm);
    assert(t_LastConf == -1 && t_ConfNumbers == NULL && t_ConfNames == NULL && t_ConfActive == NULL);
    strcpy(t_UserName, tusrnm);
    free_string(tusrnm);
    strcpy(t_BoardName, txt[10]);
    t_LastConf = 0;
    ret_code = RdCn_do_malloc(t_LastConf);
    if (ret_code) {
	strcpy(t_ConfNames[0], txt[9]);
	*t_ConfActive = FALSE;
	*t_ConfNumbers = (int)0;
	RdCn_switch_to_new_bbs();
	set_CurBoard(BoardName, cb_RdCn_main_init);
    }
    assert(t_LastConf == -1 && t_ConfNumbers == NULL && t_ConfNames == NULL && t_ConfActive == NULL);
    return ret_code ;
}

/*
 * RdCn_exit_free - called by main_exit() in atpmain.c before shutting down.
 */ 
void
RdCn_exit_free(void)
{
   assert(ConfActive != NULL && ConfNumbers != NULL && ConfNames != NULL);
   RdCn_do_free(LastConf, ConfActive, ConfNumbers, ConfNames);
}

static int ActvCnt = 0 ;
/*
 * get_ActvCnt - returns number of active conferences.
 */ 
int
get_ActvCnt(void)
{
    assert( -1 < ActvCnt );
    return ActvCnt;
}


/*
 * ActvConf - update array which tracks active conferences.
 */
void
ActvConf(void)
{
    int i;
    char tmp[MAXPATHS];
#ifdef __MSDOS__
    struct ffblk merv;

    ActvCnt = 0;
    for (i = 0; i <= LastConf; i++)
	ConfActive[i] = FALSE;
    sprintf(tmp, "%s%s%c*.idx", HomePath, CurBoard, SEP);

    if (!findfirst(tmp, &merv, 0)) {
	i = findCindex(atoi(merv.ff_name));
	if (i >= 0) {
	    ConfActive[i] = TRUE;
	    ActvCnt = 1;
	}
	while (!findnext(&merv)) {
	    i = findCindex(atoi(merv.ff_name));
	    if (i >= 0) {
		ConfActive[i] = TRUE;
		ActvCnt++;
	    }
	}
    }
#else				/* unix et al */
    ActvCnt = 0;

    for (i = 0; i <= LastConf; i++) {
	cnf_path(tmp, ConfNumbers[i]);
	if (access(tmp, F_OK) == SUCCESS) {
	    ConfActive[i] = TRUE;
	    ActvCnt++;
	} else {
	    ConfActive[i] = FALSE;
	}
    }
#endif
    set_ReplyExist( ConfActive[RCONF_IDX], rexist_ActvConf);
}


/*
 * Chk4RepCnf - checks if reply conference exists and prompts for deletion.
 */
void
Chk4RepCnf(const char *tpath)
{
    char Tcnfbuf[MAXPATHS];
    char Tidxbuf[MAXPATHS];

    sprintf(Tcnfbuf, "%s%c%d.cnf", tpath, SEP, REPL_CONF);
    sprintf(Tidxbuf, "%s%c%d.idx", tpath, SEP, REPL_CONF);
    set_ReplyExist(FALSE, rexist_Chk4RepCnf);
    /* does reply conference exist ? */
    if (access(Tcnfbuf, F_OK) == SUCCESS ) {
	char prmbuf[80];
	red();
	high();
	/* "Warning ! file already exist " "delete it ?" */
	printf("%s   %s %d.cnf %s %s...!\n", txt[2], txt[62], REPL_CONF, ConfNames[RCONF_IDX], txt[63]);
	sprintf(prmbuf, "            %s ", txt[64]);
	if (YesNo(YES, prmbuf)) {
	    do_unlink(Tcnfbuf);
	    do_unlink(Tidxbuf);
	    ConfActive[RCONF_IDX] = FALSE;
	} else
	    set_ReplyExist(TRUE, rexist_Chk4RepCnf);
    }
}


/*
 * ListConfOut, called bye display_conf_names().
 */
static const int pers = -2;
static const int reps = -1;

ATP_INLINE int
ListConfOut(int conf_array_index)
{
    if (conf_array_index == pers) {		/* List personal conference first */
	if (get_pmail())
	    blink();
	conf_array_index = PCONF_IDX;
    }
    if (conf_array_index == reps)		/* List reply conference next */
	conf_array_index = RCONF_IDX;
    red();
    high();
    printf("\n\t\t%4d ", ConfNumbers[conf_array_index]);
    clear();
    blue();
    printf("......... ");
    magenta();
    if ((conf_array_index == PCONF_IDX) && get_pmail())
	blink();
    high();
    printf("%-15s ", ConfNames[conf_array_index]);
    clear();
    blue();
    printf("......... ");
    if ((conf_array_index == PCONF_IDX) && get_pmail())
	blink();
    green();
    high();
    if (!ConfActive[conf_array_index]) {
	printf("%s", txt[79]);	/* "Inactive" */
    } else {
	magenta();
	printf("%s", txt[80]);	/* "active" */
    }
    if (conf_array_index == PCONF_IDX) {
	clear();
	conf_array_index = pers;
    } else if (conf_array_index == RCONF_IDX) {
	conf_array_index = reps;
    }
    return conf_array_index;
}


/*
 * display_bbs_name, called by ListConf().
 */
static void
display_bbs_name(void)
{
    int i;
    const int underline_width = (int) (strlen(txt[78]) + strlen(BoardName) + 1);
    CLSCRN();
    magenta();
    high();
    printf("\r%s %s\n", txt[78], BoardName);	/* "Liste des conf. sur" */
    clear();
    red();
    printf("\r");
    for (i = 0; i < underline_width; i++)
	(void)putchar('-');
    (void)putchar('\n');
}


/*
 * display_conf_names, called by ListConf().
 */
static void
display_conf_names(void)
{
    int line, cnf_index;
    ActvConf();
    for (cnf_index = pers, line = 0; cnf_index < RCONF_IDX; line++, cnf_index++) {
	if (line > (get_ScrnRows() - 6)) {
	    line = -2;
	    printf("\n");
	    if (!more(YES))
		break;
	    cyan();
	    CLSCRN();
	}
	cnf_index = ListConfOut(cnf_index);
    }
}


/*
 * ListConf - prints conference numbers/names/status.
 */
void
ListConf(void)
{
    if (get_isempty()) {
	EmptyMsg();
    } else {
	display_bbs_name();
	display_conf_names();
	clear();
	printf("\n");
    }
}
/********************* end of ReadControl() section ***********************/
