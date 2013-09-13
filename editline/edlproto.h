/* editline.c  prototypes */

#ifndef EDLPROTO_H
#define EDLPROTO_H

#include "atptypes.h"
#include "edpublic.h"

#if defined(__LINT__) && defined(__LINTLIB__)
#include <stdio.h>
#endif

/*
**  Command status codes.
*/
#define CASE enum TOULR
CASE { TOupper, TOlower } ;
#define STATUS enum RSTAT
STATUS { CSdone, CSeof, CSmove, CSdispatch, CSstay } ;

extern char    *rl_complete(const char *a, int *b);
extern int      rl_list_possib(char *a, char ***b);
extern void	rl_ttyset(int a);
extern void	rl_add_slash(char *a, char *b);
extern void     rl_reset_terminal(char *p);
extern void     rl_initialize(void);
extern void     rl_set_cntrl_char(int cquit, int cerase, int ceof, int cintr, int ckill);

#ifdef NEED_STRDUP
extern char *strdup( const char *p );
#endif

#ifdef no_proto
#define ALVOID
#else
#define ALVOID void
#endif

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif


#endif /* edlproto.h */

