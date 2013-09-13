/*
**
**  MS-DOS system-dependant routines for editline library.
*/
#include "editline.h"
#include "dos.h"

#ifdef __TURBOC__
#pragma argsused
#endif

/* Reset is a dummy argument */

void
rl_ttyset(int Reset)
{
}


void
rl_add_slash( char *path, char *p )
{
    struct stat	Sb;

    if (stat (path, &Sb) >= 0 )
    (void)strcat( p, S_ISDIR (Sb.st_mode) ? SEPST : " ");
}

#if defined(DESQVP)
/* DESQview routine for giving up time slice */ 

static  union REGS pregsi ;
static  union REGS pregso ;
static internum ;
static dv_flag = 0 ;

static void
dv_init( void )
{
  /* test general capability */ 
   internum = 0x2F ;
   pregsi.x.ax = 0x1680 ;
   int86( internum, &pregsi, &pregso); 
   if ( pregso.h.al == 0 ) 
   { 
	dv_flag = 1 ;
	return ;
   }

  /* test desqview capability */ 
   internum = 0x21 ;
   pregsi.x.ax = 0x2b01 ;
   int86( internum, &pregsi, &pregso); 
   if ( pregso.h.al != 0xff ) 
   { 
	dv_flag = 1 ;
        pregsi.x.ax = 0x1000 ;
        internum = 0x15 ;
	return ;
   }
   dv_flag = -1 ;  
   return ;
}

void
dv_pause( void )
{
  switch ( dv_flag )
  {
  case 1 :
	int86( internum, &pregsi, &pregso );
	break ;
  case 0 :
	dv_init();
  default:
	return ;	
  }
}
/* DESQview and multi-tasking aware sleep() for MSDOS */

#define DOS_TICK_CNTR 0x046c 

void
sleep( unsigned sec )
{
 time_t *ticker = ( time_t * ) DOS_TICK_CNTR ; 
 time_t tick, tock ;

 tock = ( time_t) ((181 * sec)+5) / 10 ; /* fixed point math with round up */
 tick = *ticker ;
 tock += tick ;
 do
 {
   dv_pause();  
   tick = *ticker ;
 } while ( tick < tock );
}
#endif
