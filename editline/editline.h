/*
**
**  Internal header file for editline library.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#ifdef MEMDEBUG
#include <memdebug.h>
#endif

#if defined(GC_DEBUG) || defined(MYGC)
#undef malloc
#undef realloc
#undef calloc
#undef free 
#define malloc(s) GC_MALLOC((s))
#define realloc(n,s) GC_REALLOC((n),(s))
#define free(s) GC_FREE((s))
/* #define free(s) ;*/
#define calloc(n,s) GC_MALLOC((n)*(s))
#include <gc.h>
#endif

extern  void Erase(const char *PathName);

#if defined(GO32)
#define DJGPP 1
#endif

#ifdef DJGPP
#define HAVE_DIRENT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_PERROR 1
#define HAVE_STRDUP 1
#ifndef __MSDOS__
#define __MSDOS__ 1
#endif
#endif

#ifdef _MSC_  
#define HAVE_PERROR 1
#define HAVE_STRDUP 1
#endif

#ifdef __TURBOC__
#ifndef __LINT__
#define HAVE_PERROR 1
#define HAVE_STRDUP 1 
#endif
#endif

#if defined(__EMX__)
#define _OS2
#define HAVE_DIRENT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_PERROR 1
#define HAVE_STRDUP 1
#endif

#ifdef _OS2
#include <io.h>
#include <conio.h>
#include "msdos.h"
#define SEPST "\\"
#define SEPCH '\\'
#endif

#ifdef MSDOS
#ifndef __MSDOS__
#define __MSDOS__ 1
#endif
#endif

#if defined(__MSDOS__) || defined(WIN32)
#define UNIQUE_HISTORY 1
#define ANSI_ARROWS 1
#include <io.h>
#include "msdos.h"
#define SEPST "\\"
#define SEPCH '\\'
#ifndef WIN32
#include <conio.h>
#endif
#endif

#if defined(DESQVP) && defined(__MSDOS__)
# define A_DVPAUZ dv_pause();
void dv_pause( void ) ;
#else 
#define A_DVPAUZ ;
#endif

#if	defined(SYS_OS9)
#include "os9.h"
#endif	/* defined(SYS_OS9) */

#if defined(SYS_UNIX)
#include "unix.h"
#define SEPST "/"
#define SEPCH '/'
#endif	/* defined(SYS_UNIX) */

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#else
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#endif

/* the following test is generally relevant to Unix systems only */
#if defined(HAVE_TERMIOS_H)
#  include <termios.h>
#elif defined(HAVE_TERMIO_H)
#  include <sys/ioctl.h>
#  include <termio.h>
#  define termios termio
#  define cc_t unsigned char
#elif defined(HAVE_SGTTY_H)
#  include <sys/ioctl.h>
#  include <sgtty.h>
#endif

#if defined(HAVE_TERMIO_H) && !defined(HAVE_TCGETATTR)
#    define tcgetattr(x,z) ioctl(x,TCGETA,z)
#    define tcsetattr(x,y,z) ioctl(x,TCSETA,z)
#    if !defined(TCSANOW)
#      define TCSANOW TCSETA
#    endif
#endif

#if defined(HAVE_TERMCAP_H)
#include <termcap.h>
#elif defined(SYS_UNIX)
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif
extern char PC;
extern char *UP;
extern char *BC;
extern short ospeed;
EXTERNC int tgetnum (const char *name);
EXTERNC int tgetflag (const char *name);
EXTERNC char *tgetstr (const char *name, char **area);
EXTERNC int tgetent (char *buffer, const char *termtype);
EXTERNC char *tgoto (const char *cstring, int hpos, int vpos);
EXTERNC void tputs (const char *string, int nlines, int (*outfun)(int));
EXTERNC char *tparam (const char *ctlstring, char *buffer, int size, ...);
#endif

typedef int UNSI;
#define HIDE

#if defined(HIDE)
#define STATIC	static
#else
#define STATIC	/* NULL */
#endif

#define MEM_INC		64
#define SCREEN_INC	256

#if !defined(HAVE_STRDUP)
#define NEED_STRDUP 1
#endif

#define COPYFROMTO(cnew, p, len) (void)memcpy((char *)(cnew), (char *)(p), (len))

/* ms_dos key codes 
#define KLT 0x4b
#define KRT 0x4d
#define KUP 0x48
#define KDN 0x50
#define KHM 0x47
#define KPU 0x49
#define KPD 0x51
#define KEN 0x4f
#define KIN 0x52
#define KDL 0x53
#define KCE 0x4c
#define KF1 0x3b
#define KF2 0x3c
#define KF3 0x3d
#define KF4 0x3e
*/

/* ansi */

#define ESC 0x1b
#define KLT 'D' /* left */
#define KRT 'C' /* right */
#define KUP 'A' /* up */
#define KDN 'B' /* down */
#define KCE 'G' /* center */
#define KHM 'H' /* home */
#define KPU '5' /* page up */
#define KPD '6' /* page down */
#define KEN 'Y' /* end */
#define KIN '2' /* insert */
#define KDL '3' /* delete */
#define KF1 'P' /* f1 */
#define KF2 'Q' /* f2 */
#define KF3 'R' /* f3 */
#define KF4 'S' /* f4 */
#define KF5 'T' /* f5 */
#define KF6 'U' /* f6 */
#define KF7 'V' /* f7 */
#define KF8 'W' /* f8 */
#define KF9 'X' /* f9 */
#define KF0 'O' /* f10 */


/*
**  Variables and routines internal to this package.
*/
#if 0
extern int	rl_eof;
extern int	rl_erase;
extern int	rl_intr;
extern int	rl_kill;
extern int	rl_quit;
#endif 
