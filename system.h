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
system.h
*/

#ifndef _ATP_SYSTEM_H
#define _ATP_SYSTEM_H 1

#include <ctype.h>
#include <time.h>
#include <signal.h>
#if !defined(ATPDBG) || defined(_QC)
#define NDEBUG 1
#endif
#include <assert.h>
#include "atptypes.h"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

char *mygetcwd( char *buf, size_t siz );

#if defined(SYS_UNIX) && !defined(unix)
#define unix 1
#endif

#if !defined(SYS_UNIX) && !defined(unix) /* non-unix includes */
# if defined(_MSC_) /* microsoft c */
#   include <malloc.h>
#   include <direct.h>
# else /* borland et al dos */
#   include <alloc.h>
#   include <dir.h>
# endif
# include <process.h>
# include <io.h>
# include <dos.h>
#endif

#if defined(__cplusplus)
# define ATP_INLINE static inline
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__) && (__GNUC__ > 1 )
# define ATP_INLINE static inline
#else
# define ATP_INLINE static
#endif

#if defined(MEMDEBUG)
#include <memdebug.h>
#elif defined(DBMALLOC)
#include <dbmalloc.h>
#endif

#if defined(GC_DEBUG) || defined(MYGC)
#undef calloc
#undef realloc
#undef malloc
#undef free 
#define malloc(s) GC_MALLOC((s))
#define realloc(n,s) GC_REALLOC((n),(s))
#define free(s) GC_FREE((s))
/* #define free(s) ;*/
#define calloc(n,s) GC_MALLOC((n)*(s))
#include <gc.h>
#endif

#if defined(__EMX__)
#ifndef _OS2
#define _OS2 1
#endif
#define unix 1
#define CHPATH _chdir2
#define GETCWD _getcwd2
#define ADDRIVE(X) addrive(X)
#define HAVE_DIRENT_H 1
#define HAVE_GETCWD 1
#define HAVE_PERROR 1
#define HAVE_SIGACTION 1
#define HAVE_STRDUP 1
#define HAVE_STRNICMP 1
#define HAVE_STRSTR 1
#define HAVE_STRUPR 1
#define HAVE_TMPNAM 1
#define HAVE_UNISTD_H 1
#define HAVE_UTIME 1
#define DOSTIME 1
#include <utime.h>
#include <io.h>
extern void addrive(char *path);
#endif

/* D.J.Delore gcc for ms-dos */
#ifdef DJGPP
#ifndef __MSDOS__
#define __MSDOS__
#endif
#include <gppconio.h>
#define HAVE_DIRENT_H
#define NEEDISKS 1
#define HAVE_PERROR 1
#define HAVE_STRDUP 1
#define HAVE_STRNICMP 1
#define HAVE_STRSTR 1
#define HAVE_STRUPR 1
/* #define HAVE_TMPNAM <-- this is partly broken under djgpp 1.12 */
#define HAVE_UNISTD_H 1
#define HAVE_UTIME 1
#include <utime.h>
#define HAVE_GETCWD 1
#define GETCWD getcwd
#endif

#ifdef unix
#  include <sys/param.h>
#  if defined(HAVE_UNISTD_H) || !defined(SYS_UNIX)
#    include <unistd.h>
#  endif
#endif

#if defined(unix)
# if defined(HAVE_DIRENT_H)
#   include <sys/types.h>
#   include <dirent.h>
#   define DIRENTRY dirent
# else
#   if defined(HAVE_SYS_DIR_H)
#      include <sys/dir.h>
#   elif defined(HAVE_SYS_NDIR_H)
#      include <sys/ndir.h>
#   elif defined(HAVE_NDIR_H)
#      include <ndir.h>
#   else
#      include <dir.h>
#   endif
#   define DIRENTRY direct
# endif

#  ifndef S_IRWXU
#    define S_IRWXU 0700
#  endif
#  define my_mkdir(c) mkdir(c,S_IRWXU)

#  if defined(SYS_UNIX)
#    if defined(HAVE_GETCWD)
#      define GETCWD getcwd
#    else
#      define GETCWD mygetcwd
#    endif
#    define CHPATH chdir
#    define SHELL "SHELL"
#    define UNIXCMDS 1
#    define HAVE_LINK 1
#    define HAVE_UTIME 1
#    define USE_FORK 1
#    define SEP '/'
#    include <sys/ioctl.h>
#    if !defined(HAVE_UTIME_H)
	struct utimbuf { time_t actime; time_t modtime ; } ;
#    endif
void save_terminal( void );
void restore_terminal( void);
#  endif
#endif

/* define LATIN1 if your terminal supports ISO character set */

#ifdef __linux__
#define VT220 1
#define LATIN1 1
#endif

#ifdef SIMP 
#define NEEDREADLINE 1 /* not using Salz/Tumlee editline lib */
#define add_history(x) /* null x */
#endif

#ifdef _OS2
# include <io.h>
# include <process.h>
# ifndef __EMX__
    typedef int pid_t ;
#   define GETCWD getcwd
#   define CHPATH chdir
#   define unlink(X) remove(X)
#   define SEP '\\'
# else
#   define SEP '/'
# endif
# define SHELL "COMSPEC"
#endif

#ifdef MSDOS
#ifndef __MSDOS__
#define __MSDOS__ 1
#endif
#endif

#ifdef WIN32
typedef int pid_t ;
#define CHPATH chdir
#define ADDRIVE(X) addrive(X)
#define my_mkdir(c) mkdir(c)
#define sleep(t) _sleep(t)
#define GETCWD getcwd
#define SHELL "COMSPEC"   /* environment variable used for command shell */
#define SEP '\\'
#endif

#ifdef __MSDOS__
#define MAXPATHS 128
#define SHELL "COMSPEC"   /* environment variable used for command shell */
#define SEP '\\'
#define ADDRIVE(X) addrive(X)
extern void addrive(char *path);
#define CHPATH DosChPath
extern atp_ERROR_T DosChPath(const char *path);
#if defined(SPAWNO)
# ifdef _CDECL
#   define _Cdecl _CDECL
# endif 
# include <spawno.h>
#endif /* msdos only */

#ifdef _MSC_
extern char *getcwd( char *p, int len );
extern void sleep(unsigned sec); 
#define GETCWD getcwd
#include <sys/utime.h>
#define __ATP_LOOSE__ 1
#define DOSTIME 1
#define NEEDISKS 1
#define HAVE_GETCWD 1
#define HAVE_PERROR 1
#define HAVE_STRNICMP 1
#define HAVE_STRSTR 1
#define HAVE_STRUPR 1
#define HAVE_TMPNAM 1
#define HAVE_UTIME 1
#define findfirst(A,B,C) _dos_findfirst(A,C,B)
#define findnext _dos_findnext
#define ffblk find_t
#define ff_name name 
#endif

#ifndef DJGPP 
#define GETCWD getcwd
/* extern int getch( void );*/
/* #define unlink(X) remove(X) */
#define my_mkdir(X) mkdir(X)
#endif 

#ifdef __TURBOC__
#include <conio.h>
#ifndef __LINT__
#define HAVE_GETCWD 1
#define HAVE_PERROR 1
#define HAVE_STRNICMP 1
#define HAVE_STRSTR 1
#define HAVE_STRUPR 1
#define HAVE_TMPNAM 1
#else 
#undef GETCWD
#define GETCWD mygetcwd
char *strdup(const char *s);
extern long timezone ;
extern void tzset(void);
extern int putenv(const char *name);
#endif /* __LINT__ */
#define DOSTIME 1
#define HAVE_FTIME /* getftime() and setftime() library functions */
#endif /* __TURBOC__ */

#else /* NOT MSDOS */
#ifndef ADDRIVE
#define ADDRIVE(X) /* nothing X */
#endif
#define MAXPATHS 1024
#endif  /* if __MSDOS__ */

#if defined(SYS_UNIX)  
#  if defined(HAVE_TERMIOS_H)
#    include <termios.h>
#    define ATPSRTERM termios
#    define ATPGETERM tcgetattr(STDIN_FILENO, &savedterm )	
#    define ATPSETERM tcsetattr(STDIN_FILENO, TCSANOW, &savedterm )	
#  elif defined(HAVE_TERMIO_H)
#    include <sys/ioctl.h>
#    include <termio.h>
#    define ATPSRTERM termio
#    define ATPGETERM ioctl(STDIN_FILENO, TCGETA, &savedterm )	
#    define ATPSETERM ioctl(STDIN_FILENO, TCSETA, &savedterm )	
#  elif defined(HAVE_SGTTY_H)
#    include <sgtty.h>
#    include <sys/ioctl.h>
#    define ATPSRTERM sgttyb
#    define ATPGETERM ioctl(STDIN_FILENO, TIOCGETP, &savedterm )	
#    define ATPSETERM ioctl(STDIN_FILENO, TIOCSETP, &savedterm )	
#   endif
#else
#     define ATPGETERM ; 
#     define ATPSETERM ; 
#     define save_terminal() ;
#     define restore_terminal() ;
#endif

#ifndef CLSCRN
#define CLSCRN cls
#endif 

#ifdef __ATP_LOOSE__
#define CONSPTR *
#else
#define CONSPTR *const
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2
#endif


#ifdef __cplusplus
#define EXTERNC extern "C"
#else 
#define EXTERNC extern
#endif

#if !defined(HAVE_STRNICMP)
#if defined(HAVE_STRNCASECMP)
#define strnicmp strncasecmp
#define stricmp strcasecmp
#define HAVE_STRNICMP
EXTERNC int strncasecmp(const char *s1, const char *s2, size_t n);
EXTERNC int strcasecmp(const char *s1, const char *s2);
#else
int strnicmp(const char *s1, const char *s2, size_t n);
int stricmp(const char *s1, const char *s2);
#endif
#endif


#ifdef NEEDISKS  /* brain dead dos stuff */
#ifdef __MSDOS__
int getdisk(void);
int setdisk(int driv);
#endif
#endif

#ifndef R_OK
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* define ATP tagline */

#define ATPVER " 1.50 " /* DON'T CHANGE THIS it will mess up formatting */

/* #define ATPSYST  "your_system" */ /* you shouldn't  have to define this */

#ifndef ATPSYST
#ifdef __linux__
#define ATPSYST "Linux"
#elif defined(hurd)
#define ATPSYST "Hurd"
#elif defined(esix)
#define ATPSYST "Esix"
#elif defined(NeXT)
#define ATPSYST "NeXT"
#elif defined(__IBCS2__)
#define ATPSYST "ibcs2"
#elif defined(WIN32)
#define ATPSYST "Win32"
#elif defined(__EMX__)
#define ATPSYST "OS2"
#elif defined(_OS2)
#define ATPSYST "OS2"
#elif defined(xenix)
#define ATPSYST "Xenix"
#elif defined(usl)
#define ATPSYST "USL"
#elif defined(DJGPP)
#define ATPSYST "djgpp"
#elif defined _MSC_
#define ATPSYST "msc"
#elif defined(__MSDOS__)
#define ATPSYST "qwk"
#elif defined(amiga)
#define ATPSYST "Amiga"
#elif defined(mach)
#define ATPSYST "Mach"
#elif defined(sun)
#define ATPSYST "Sun"
#elif defined(BSD)
#define ATPSYST "bsd"
#elif defined(ATPUNAM)
#define ATPSYST ATPUNAM
#elif defined(unix)
#define ATPSYST "Unix"
#else
#define ATPSYST "QWK"
#endif
#endif

#define NTAG  "\n---\n \376 ATP/" ATPSYST ATPVER "\376 "
#define FTAG  "\n...\n * ATP/" ATPSYST ATPVER "* " 
#ifdef __EMX__
#define NTAX  "\n---\n \376 ATP/emx" ATPVER "\376 "
#define FTAX  "\n...\n * ATP/emx" ATPVER "* " 
#endif

#ifdef BADLWR     /* some versions of GCC 1.40 and before */
#undef tolower(c)
#undef toupper(c)
#define tolower(x) (((x)>0x40)&&((x)<0x5b)?((x)|0x20):(x))
#define toupper(x) (((x)>0x60)&&((x)<0x7b)?((x)&0xdf):(x))
#endif

/* provide the posix tmpnam() if it is missing; define it in system.c */
#ifndef L_tmpnam
#define L_tmpnam 30
#define NEEDTMPNAM
#endif

#if !defined(HAVE_TMPNAM) || defined(NEEDTMPNAM)
char *mytmprnam(/*@out@*/ char *buf );
#define tmpnam(s) mytmprnam(s)
#endif

#if !defined(HAVE_STRSTR)
char *strstr(const char *haystack, const char *needle); 
#endif

#if !defined(HAVE_PERROR)
#define perror(X) fprintf(stderr,"%s %s: %d\n", X , __FILE__ , __LINE__ )
#endif

#if !defined(CHPATH)
#define CHPATH chdir
#endif

#if !defined(GETCWD)
#define GETCWD getcwd
#endif

#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif

/* define DESQVP to make program DESQview aware */

#if defined(DESQVP) && defined(__MSDOS__)
#define A_DVPAUZ dv_pause();
#define FCPYBUF (size_t)1024
void dv_pause( void );
#else
#define A_DVPAUZ ;
#define FCPYBUF (size_t)4096
#endif

#ifndef ROW_OFFSET
#define ROW_OFFSET 3
#endif

#ifdef DOSTIME
#define ADJUST_DOS_TIME t -= timezone;
extern void msdos_time_init( void );
#else
#define ADJUST_DOS_TIME /* nothing */
#endif

#endif /* _ATP_SYSTEM_H */

