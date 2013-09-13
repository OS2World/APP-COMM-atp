/* rot13.c -- rotate or de-rotate a file.

 * Original Author: Marc Unangst <mju@mudos.ann-arbor.mi.us>
 *
 * Copyright (c) 1990 Marc Unangst.  All Rights Reserved.
 * Copyright (c) 1994, 1996 Thomas McWilliams.  All Rights Reserved.
 *
 * Distribution permitted in accordance with the GNU General Public License
 * version 2. Obtain copy of GPL from: 
 *
 *      Free Software Foundation, Inc.
 *      675 Mass Ave.
 *      Cambridge, MA 02139
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE  !FALSE
#endif
const char version[] = "rot13 version 1.2, March 1996\n";

#define FAST

#if defined(__cplusplus)
# define ROT_INLINE static inline
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__) && (__GNUC__ > 1 )
# define ROT_INLINE static inline
#else
# define ROT_INLINE static
#endif

#ifdef FAST
const char rot[] = "@NOPQRSTUVWXYZABCDEFGHIJKLM[\\]^_\140nopqrstuvwxyzabcdefghijklm";
#else
/* ALPHALEN is the length of norm[] and rot[] */
#define ALPHALEN    53
const char rot[]  = "@NOPQRSTUVWXYZABCDEFGHIJKLMnopqrstuvwxyzabcdefghijklm";
const char norm[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
#endif

ROT_INLINE int
rotate(int c)
{
#ifdef FAST
    if ((int)'A' <= c && c <= (int)'z') {
	c &= 0x3f;
	c = (int)rot[c];
    }
#else				/* original slower code below */
    int i, flag = FALSE;
    for (i = 1; i < ALPHALEN; i++)
	if (c == (int) norm[i]) {
	    flag = TRUE;
	    break;
	}
    if (flag == TRUE)
	return ((int) rot[i]);
    else
#endif
	return (c);
}

static void
usage(void)
{
    fprintf(stderr, "%s", version);
    fprintf(stderr, "usage: rot13 [input [output]]\n");
}

int
main(int argc, char **argv)
{
    FILE *input = stdin, *output =stdout ;	/* Input, output buffers */

    if (argc > 3) {		/* Bad args */
	usage();
	exit(EXIT_FAILURE);
    } else if (argc != 1) {			/* input = file; output = stdout or file */
	if ((input = fopen((char *) argv[1], "r")) == NULL) {
	    perror(argv[1]);
	    fprintf(stderr, "rot13: couldn't open %s for input\n", (char *) argv[1]);
	    usage();
	    exit(EXIT_FAILURE);
	}
	if (argc == 3) {	/* output = file */
	    if ((output = fopen((char *) argv[2], "w")) == NULL) {
		perror(argv[2]);
		fprintf(stderr, "rot13: couldn't open %s for output\n",
			(char *) argv[2]);
		usage();
		exit(EXIT_FAILURE);
	    }
	}
    }

    /* Main loop -- read a char, rotate it, output it */
    {
	int a, b = 1;
	while ((a = fgetc(input)) != EOF && b != EOF)
	    b = putc(rotate(a), output);
    }
    return 0;
}
