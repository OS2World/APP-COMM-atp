/*
**
**  Editline system header file for MSDOS 
*/

#ifndef __STDC__
#ifdef __BORLANDC__
#define __STDC__
#endif
#endif

#define CRLF		"\r\n"
#include <sys/types.h>
#include <sys/stat.h>
#if defined(DJGPP)
#include <pc.h>
#include <keys.h>
#endif

void dv_pause( void ) ;

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
typedef struct dirent	DIRENTRY;

#elif defined(_MSC_)
#define FINDFIRST
#include <direct.h>
#include <io.h>
#include <dos.h>
typedef struct _finddata DIRENTRY;

#else
#include <dir.h>
typedef struct ffblk DIRENTRY;
#endif	/* defined(HAVE_DIRENT_H) */

/* this just provides missing prototypes for certain Borland compilers */
#if defined(__TURBOC__) && defined(__STDC__)
char *strdup( const char *s );
char *strlwr( char *s );
#endif

#if	!defined(S_ISDIR)
#define S_ISDIR(m)		(((m) & S_IFMT) == S_IFDIR)
#endif	/* !defined(S_ISDIR) */

#ifdef BADLWR 
#ifdef tolower
#undef tolower
#undef toupper
#endif
#define tolower(x) (((x)>0x40)&&((x)<0x5b)?((x)|0x20):(x))
#define toupper(x) (((x)>0x60)&&((x)<0x7b)?((x)&0xdf):(x))
#endif
