/* 
**
**  A "micro-shell" to test editline library.
**  If given any arguments, commands aren't executed.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_UNISTD_H)
#include <sys/types.h>
#include <unistd.h>
#endif

#include "editline.h"
#include "edlproto.h"

#ifdef MEMDEBUG
#include <memdebug.h>
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif 

#if !defined(HAVE_PERROR)
#define perror(p) fprintf(stderr,"%s\n",(p))
#endif

int silent = 0 ;
const char *luxptr ;
int ScrnCols = 0 ;
int ScrnRows = 0 ;

int
get_ScrnRows(void)
{
    return 25;
}

void
get_ScrnCols(int dum1, int dum2)
{
}

void
set_ScrnRows(int dum1, int dum2)
{
}

int
set_ScrnCols(void)
{
    return 80;
}

/* ARGSUSED1 */
int
main( int ac, char *av[] )
{
    char	*p;
    int		doit;

    doit = ac == 1;
	rl_initialize();
    while ((p = readline("testit> ", TRUE )) != NULL) {
	if( strncmp( p, "quit", 4) == 0 )
		break; 	
	if( strncmp( p, "exit", 4) == 0 )
		break;
	printf("\t\t\t|%s|\n", p);
	if (doit)
	    if (strncmp(p, "cd ", 3) == 0) {
		if (chdir(&p[3]) < 0)
		    perror(&p[3]);
	    }
	    else if (system(p) != 0)
		perror(p);
	add_history(p);
	free(p);
	p = NULL ;
    }
    /* clean-up before exit */
    free (p);
    rl_cleanup();
    return 0 ;
    /* NOTREACHED */
}

#if defined(SYS_UNIX)
/* signal handler for SIGWINCH */

RETSIGTYPE
getwinders (int dummy)
{

}
#endif
