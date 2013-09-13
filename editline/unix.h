/*
**
**  Editline system header file for Unix.
*/

#define CRLF		"\r\n"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
typedef struct dirent	DIRENTRY;
#elif defined(SYSDIR)
#include <sys/dir.h>
typedef struct direct	DIRENTRY;
#elif defined(SYSNDIR)
#include <sys/ndir.h>
typedef struct direct	DIRENTRY;
#elif defined(NDIR)
#include <ndir.h>
typedef struct direct	DIRENTRY;
#else
#include <dir.h>
typedef struct direct	DIRENTRY;
#endif

#if	!defined(S_ISDIR)
#define S_ISDIR(m)		(((m) & S_IFMT) == S_IFDIR)
#endif	/* !defined(S_ISDIR) */

#ifdef BSD   /* 386BSD has bad toupper() tolower()  functions */
#ifdef i386
#undef tolower(c)
#undef toupper(c)
#define tolower(x) (((x)>0x40)&&((x)<0x5b)?((x)|0x20):(x))
#define toupper(x) (((x)>0x60)&&((x)<0x7b)?((x)&0xdf):(x))
#endif
#endif
