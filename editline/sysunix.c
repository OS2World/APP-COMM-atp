/*
**
**  Unix system-dependant routines for editline library.
*/
/* editline.h also includes unix.h */
#include "editline.h"
#include "edlproto.h"

#if defined(HAVE_TERMIOS_H) || defined(HAVE_TERMIO_H)

#define BITSON ( ~0 )

#if  defined(HAVE_LIBTERMCAP)
#if !defined(HAVE_CFGETOSPEED)
#if  defined(CBAUD)
static short 
edl_cfgetospeed( struct termios *s)
{	short tmp ;
	tmp = ( short ) ((s->c_cflag)&(CBAUD)) ;
	return  tmp ;
}
#define cfgetospeed edl_cfgetospeed
#else
#define cfgetospeed(s) 13 
#endif
#endif
#endif

void
rl_ttyset(int Reset)
{
    static struct termios old;
    struct termios tnew;

    if (Reset == 0) {
	(void) tcgetattr(STDIN_FILENO, &old);
#if defined(HAVE_LIBTERMCAP)
	ospeed = cfgetospeed(&old);
#endif
	rl_set_cntrl_char(old.c_cc[VQUIT], old.c_cc[VERASE], old.c_cc[VEOF], old.c_cc[VINTR], old.c_cc[VKILL]);
#if 0
	rl_erase = old.c_cc[VERASE];
	rl_kill = old.c_cc[VKILL];
	rl_eof = old.c_cc[VEOF];
	rl_intr = old.c_cc[VINTR];
	rl_quit = old.c_cc[VQUIT];
#endif
	tnew = old;
#ifdef OCRNL
	tnew.c_oflag &=  ~ OCRNL;
#endif
#ifdef OPOST
	tnew.c_oflag &=  ~ OPOST;
#endif
	tnew.c_cc[VINTR] = ( cc_t ) BITSON;
	tnew.c_cc[VQUIT] = ( cc_t ) BITSON;
	tnew.c_lflag &= ~(ECHO | ICANON);
	tnew.c_iflag &= ~(ISTRIP | INPCK);
	tnew.c_cc[VMIN] = 1;
	tnew.c_cc[VTIME] = 0;
	(void) tcsetattr(STDIN_FILENO, TCSANOW, &tnew);
    } else
	(void) tcsetattr(STDIN_FILENO, TCSANOW, &old);
}

#else /* HAVE_SGTTY_H */

void
rl_ttyset(int Reset)
{
    static struct sgttyb old_sgttyb;
    static struct tchars old_tchars;
    struct sgttyb tnew_sgttyb;
    struct tchars tnew_tchars;

    if (Reset == 0) {
	(void) ioctl(0, TIOCGETP, &old_sgttyb);
	(void) ioctl(0, TIOCGETC, &old_tchars);

	rl_set_cntrl_char(old_tchars.t_quitc, old_sgttyb.sg_erase, old_tchars.t_eofc, old_tchars.t_intrc, old_sgttyb.sg_kill);
#if 0
	rl_quit = old_tchars.t_quitc;
	rl_erase = old_sgttyb.sg_erase;
	rl_eof = old_tchars.t_eofc;
	rl_intr = old_tchars.t_intrc;
	rl_kill = old_sgttyb.sg_kill;
#endif

#if defined(HAVE_LIBTERMCAP)
	ospeed = old_sgttyb.sg_ospeed;
#endif
	tnew_sgttyb = old_sgttyb;
	tnew_sgttyb.sg_flags &= ~ECHO;
	tnew_sgttyb.sg_flags |= RAW;
#if	defined(PASS8)
	tnew_sgttyb.sg_flags |= PASS8;
#endif				/* defined(PASS8) */
	(void) ioctl(0, TIOCSETP, &tnew_sgttyb);

	tnew_tchars = old_tchars;
	tnew_tchars.t_intrc = -1;
	tnew_tchars.t_quitc = -1;
	(void) ioctl(0, TIOCSETC, &tnew_tchars);
    } else {
	(void) ioctl(0, TIOCSETP, &old_sgttyb);
	(void) ioctl(0, TIOCSETC, &old_tchars);
    }
}
#endif	/* defined(HAVE_TCGETATTR) */

void
rl_add_slash( char *path, char *p )
{
    struct stat	Sb;

    if (stat(path, &Sb) >= 0)
	strcat(p, S_ISDIR(Sb.st_mode) ? SEPST : " ");
}
