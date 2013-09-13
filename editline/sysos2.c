/*  
**
**  OS/2 system-dependant routines for editline library.
*/
#include "editline.h"

void
rl_ttyset(int Reset)
{
}


void
rl_add_slash( char *path, char *p )
{
    struct stat	Sb;

    if (stat(path, &Sb) >= 0)
	(void)strcat(p, S_ISDIR(Sb.st_mode) ? SEPST : " ");
}
