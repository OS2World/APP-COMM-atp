/*
**
**  Main editing routines for editline library.
*/
#include <ctype.h>
#include "editline.h"
#include "edlproto.h"
#include "reader.h"
#include "readlib.h"

/*
   **  Manifest constants.
 */

#if 0
#define SCREEN_WIDTH    80
#define SCREEN_ROWS     24
#else
#define SCREEN_WIDTH SCREEN_COLS
#endif

#define NO_ARG          (-1)
#define DEL             127
#define CTL(x)          ((x) & 0x1F)
#define ISCTL(x)        ((x) && (x) < ' ')
#define UNCTL(x)        ((x) + 64)
#define META(x)         ((x) | 0x80)

#ifdef NOMETA
#define ISMETA(x)       ((x) & 0x80)
#else
#define ISMETA(x)       (0)
#endif

#define UNMETA(x)       ((x) & 0x7F)

#if !defined(HIST_SIZE)
#define HIST_SIZE 20
#endif

#ifndef ALVOID
#define ALVOID void
#endif

/*
   **  Key to command mapping.
 */
typedef struct _KEYMAP {
    char Key;
     STATUS(*Func) (ALVOID);
} KEYMAP;

STATIC KEYMAP MetaMap[16];
STATIC KEYMAP Map[33];

/*
   **  Command history structure.
 */
typedef struct _HISTORY {
    int Size;
    int Pos;
    char *Lines[HIST_SIZE];
} HISTORY;

/*
 * Globals.
 */
static int rl_eof  = 0x04 ; /* ^D  */
static int rl_erase= 0x7f ; /* DEL */ 
static int rl_intr = 0x03 ; /* ^C  */
static int rl_kill = 0x15 ; /* ^U  */
static int rl_quit = 0x1c ; /* ^\  */

STATIC char NilStr[] = "";
STATIC char *Input = NilStr;
STATIC char *Line = NULL;
STATIC const char *Prompt = NULL;
STATIC char *Yanked = NULL;
STATIC char *Screen;
STATIC char NEWLINE[] = CRLF;
STATIC HISTORY H;
STATIC int Repeat;
STATIC int End;
STATIC int Mark;
STATIC int OldPoint;
STATIC int Point;
STATIC int PushBack;
STATIC int Pushed;
STATIC size_t Length;
STATIC size_t ScreenCount;
STATIC size_t ScreenSize;
static char *backspace = NULL;
/* NOTE: these MUST be NilStr and *not* NULL */
#if defined(HAVE_LIBTERMCAP)
char *rev_on = NilStr;
char *rev_off = NilStr;
char *blink_on = NilStr;
char *clear_attr = NilStr;
char *bold_on = NilStr;
char *clear_scr = NilStr;
static char *home = NilStr;
char *cur_right = NilStr;
char *del_eol = NilStr;
#endif

#if 0
STATIC int TTYwidth;
STATIC int TTYrows;
#endif

/* Display print 8-bit chars as `M-x' or as the actual 8-bit char? */
#ifdef NOMETA
STATIC int rl_meta_chars = 0;
#endif

/*
   **  Declarations.
 */
#if defined(_OS2) || defined(__MSDOS__)
STATIC int getakey(char *chr);
#endif

STATIC char *editinput(ALVOID);

/*
   **  TTY input/output functions.
 */

static void
TTYflush(void)
{
    if (ScreenCount) {
	(void) write(STDOUT_FILENO, Screen, ScreenCount);
	ScreenCount = 0;
    }
}

STATIC void
TTYput(const int c)
{
    Screen[ScreenCount] = (char)c;
    if (++ScreenCount >= ScreenSize - 1) {
	ScreenSize += SCREEN_INC;
	Screen = (char *) realloc(Screen, (sizeof(char) * ScreenSize));
    }
}

STATIC void
TTYputs(const char *p)
{
    fflush(stdout);
    while (*p)
	TTYput(*p++);
}

STATIC void
TTYshow(int c)
{
    if (c == DEL) {
	TTYput('^');
	TTYput('?');
    } else if (ISCTL(c)) {
	TTYput('^');
	TTYput(UNCTL(c));
    }
#ifdef NOMETA
    else if (rl_meta_chars && ISMETA(c)) {
	TTYput('M');
	TTYput('-');
	TTYput(UNMETA(c));
    }
#endif
    else
	TTYput(c);
}

STATIC void
TTYstring(char *p)
{
    while (*p)
	TTYshow(*p++);
}

STATIC UNSI
TTYget(void)
{
    char c;

    TTYflush();
    if (Pushed) {
	Pushed = 0;
	return PushBack;
    }
    if (*Input)
	return *Input++;
#if defined(_OS2) || defined(__MSDOS__)
    return getakey(&c) == 1 ? c : EOF;
#else
    return read(0, &c, (size_t) 1) == 1 ? c : EOF;
#endif
}

#define TTYback() ((backspace != NULL) ? TTYputs((char *)backspace) : TTYput('\b'))

STATIC void
TTYbackn(int n)
{
    while (--n >= 0)
	TTYback();
}

#ifdef HAVE_LIBTERMCAP
static char attr_buf[1024];
#endif

void
TTYinfo(void)
{
    static int init = FALSE;

#if defined(HAVE_LIBTERMCAP)
    char *tttmp, *stand_out, *stand_end;
    const char *term;
    static char ebuff[2048];	/* !!!!! */
    char *bp;
    const int old_row = get_ScrnRows();
    const int old_col = get_ScrnCols();
    int tmp_row = 0, tmp_col = 0;
#endif

    if (init)
	return;
    init = TRUE;
#if defined(TIOCGWINSZ) && defined(SIGWINCH)
    getwinders(0);
#endif

#if defined(HAVE_LIBTERMCAP)
    bp = attr_buf;
    if ((term = getenv("TERM")) == NULL)
	term = "dumb";

    if (0 < tgetent(ebuff, term)) {
	tmp_row = tgetnum("li");
	tmp_col = tgetnum("co");
	blink_on = tgetstr("mb", &bp);
	rev_on = tgetstr("mr", &bp);
	clear_attr = tgetstr("me", &bp);
	bold_on = tgetstr("md", &bp);
	clear_scr = tgetstr("cl", &bp);
	home = tgetstr("ho", &bp);
	backspace = tgetstr("le", &bp);
	cur_right = tgetstr("nd", &bp);
	del_eol = tgetstr("ce", &bp);
	tttmp = tgetstr("pc", &bp);
	stand_out = tgetstr("so", &bp);
	stand_end = tgetstr("se", &bp);
	PC = tttmp ? *tttmp : 0;
	rev_off = clear_attr;
	if (rev_on == NULL && stand_out != NULL) {
	    rev_on = stand_out;
	    rev_off = stand_end;
	}
    }
#ifdef TIOCGWINSZ
    if (get_ScrnCols() < 1 || get_ScrnRows() < 1)	/* invalid call to getwinders() */
#endif
    {
	set_ScrnCols((tmp_col < 2 ? old_col : tmp_col), ss_TTYinfo);
	set_ScrnRows((tmp_row < 2 ? old_row : tmp_row), ss_TTYinfo);
    }
#endif
/* default behavior for everyone */
    if (get_ScrnCols() < 1 || get_ScrnRows() < 1) {
	set_ScrnCols(SCREEN_COLS, ss_TTYinfo);
	set_ScrnRows(SCREEN_ROWS, ss_TTYinfo);
    }
}

/*
   **  Print an array of words in columns.
 */
static void
columns(int ac, char **av)
{
    char *p;
    int i;
    int j;
    int k;
    int len;
    int skip;
    int longest;
    int cols;

    /* Find longest name, determine column count from that. */
    for (longest = 0, i = 0; i < ac; i++)
	if ((j = strlen((char *) av[i])) > longest)
	    longest = j;
#if 0
    cols = TTYwidth / (longest + 3);
#endif
    cols = get_ScrnCols() / (longest + 3);
    TTYputs((char *) NEWLINE);
    for (skip = ac / cols + 1, i = 0; i < skip; i++) {
	for (j = i; j < ac; j += skip) {
	    for (p = av[j], len = strlen((char *) p), k = len - 1; k >= 0; k--, p++)
		TTYput(*p);
	    if (j + skip < ac)
		while (++len < longest + 3)
		    TTYput(' ');
	}
	TTYputs((char *) NEWLINE);
    }
}

STATIC void
reposition(void)
{
    int i;
    char *p;

    TTYput('\r');
    TTYputs(Prompt);
    for (i = Point - 1, p = Line; i >= 0; i--, p++)
	TTYshow(*p);
}

STATIC void
left(STATUS Change)
{
    TTYback();
    if (Point) {
	if (ISCTL(Line[Point - 1]))
	    TTYback();
#ifdef NOMETA
	else if (rl_meta_chars && ISMETA(Line[Point - 1])) {
	    TTYback();
	    TTYback();
	}
#endif
    }
    if (Change == CSmove)
	Point--;
}

STATIC void
right(STATUS Change)
{
    TTYshow(Line[Point]);
    if (Change == CSmove)
	Point++;
}

STATIC STATUS
ring_bell(void)
{
/* extern int silent ; */

    if (!silent) {
	TTYput('\07');
	TTYflush();
    }
    return CSstay;
}

STATIC STATUS
do_macro(int c)
{
    char name[4];

    name[0] = '_';
    name[1] = (char) c;
    name[2] = '_';
    name[3] = '\0';

    if ((Input = (char *) getenv((char *) name)) == NULL) {
	Input = NilStr;
	return ring_bell();
    }
    return CSstay;
}

STATIC STATUS
do_forward(STATUS move)
{
    int i;
    char *p;

    i = 0;
    do {
	p = &Line[Point];
	for (; Point < End && (*p == ' ' || !isalnum(*p)); Point++, p++)
	    if (move == CSmove)
		right(CSstay);

	for (; Point < End && isalnum(*p); Point++, p++)
	    if (move == CSmove)
		right(CSstay);

	if (Point == End)
	    break;
    } while (++i < Repeat);

    return CSstay;
}

STATIC STATUS
do_case(CASE type)
{
    int i;
    int end;
    int count;
    char *p;

    (void) do_forward(CSstay);
    if (OldPoint != Point) {
	if ((count = Point - OldPoint) < 0)
	    count = -count;
	Point = OldPoint;
	if ((end = Point + count) > End)
	    end = End;
	for (i = Point, p = &Line[i]; i < end; i++, p++) {
	    if (type == TOupper) {
		if (islower(*p))
		    *p = toupper(*p);
	    } else if (isupper(*p))
		*p = tolower(*p);
	    right(CSmove);
	}
    }
    return CSstay;
}

STATIC STATUS
case_down_word(void)
{
    return do_case(TOlower);
}

STATIC STATUS
case_up_word(void)
{
    return do_case(TOupper);
}

STATIC void
ceol(void)
{
    int extras;
    int i;
    char *p;

    for (extras = 0, i = Point, p = &Line[i]; i <= End; i++, p++) {
	TTYput(' ');
	if (ISCTL(*p)) {
	    TTYput(' ');
	    extras++;
	}
#ifdef NOMETA
	else if (rl_meta_chars && ISMETA(*p)) {
	    TTYput(' ');
	    TTYput(' ');
	    extras += 2;
	}
#endif
    }

    for (i += extras; i > Point; i--)
	TTYback();
}

STATIC void
clear_line(void)
{
    Point = (0 - strlen(Prompt));
    TTYput('\r');
    ceol();
    Point = 0;
    End = 0;
    Line[0] = '\0';
}

STATIC STATUS
insert_string(char *p)
{
    size_t len;
    int i;
    char *snew;
    char *q;

    len = strlen((char *) p);
    if (End + len >= Length) {
	if ((snew = (char *) malloc((Length + len + MEM_INC))) == NULL)
	    return CSstay;
	if (Length) {
	    COPYFROMTO(snew, Line, Length);
	    free(Line);
	}
	Line = snew;
	Length += len + MEM_INC;
    }
    for (q = &Line[Point], i = (End - Point) - 1; i >= 0; i--)
	q[len + i] = q[i];
    COPYFROMTO(&Line[Point], p, len);
    End += len;
    Line[End] = '\0';
    TTYstring(&Line[Point]);
    Point += len;

    if (Point == End)
	return CSstay;
    else
	return CSmove;
}


STATIC char *
next_hist(void)
{
    return H.Pos >= H.Size - 1 ? (char *) 0 : H.Lines[++H.Pos];
}

STATIC char *
prev_hist(void)
{
    return H.Pos == 0 ? (char *) 0 : H.Lines[--H.Pos];
}

STATIC STATUS
do_insert_hist(char *p)
{
    if (p == NULL)
	return ring_bell();
    Point = 0;
    reposition();
    ceol();
    End = 0;
    return insert_string(p);
}

STATIC STATUS
do_hist(char *(*move) (ALVOID))
{
    char *p;
    int i;

    i = 0;
    do {
	if ((p = (*move) ()) == NULL)
	    return ring_bell();
    } while (++i < Repeat);
    return do_insert_hist(p);
}

STATIC STATUS
h_next(void)
{
    return do_hist(next_hist);
}

STATIC STATUS
h_prev(void)
{
    return do_hist(prev_hist);
}

STATIC STATUS
h_first(void)
{
    return do_insert_hist(H.Lines[H.Pos = 0]);
}

STATIC STATUS
h_last(void)
{
    return do_insert_hist(H.Lines[H.Pos = H.Size - 1]);
}

/*
   **  Return zero if pat appears as a substring in text.
 */
STATIC int
substrcmp(const char *text, const char *pat, size_t len)
{
    char c;

    if ((c = *pat) == '\0')
	return *text == '\0';
    for (; *text; text++)
	if ((char) *text == c && strncmp(text, pat, len) == 0)
	    return 0;
    return 1;
}

STATIC char *
search_hist(char *search, char *(*move) (ALVOID))
{
    static char *old_search = NULL;
    size_t len;
    int pos;
#ifdef no_proto
    int (*match) ();
#else
    int (*match) (const char *, const char *, size_t);
#endif
    char *pat;

    /* Save or get remembered search pattern. */
    if (search && *search) {
	if (old_search)
	    free(old_search);
	old_search = (char *) strdup(search);
    } else {
	if (old_search == NULL || *old_search == '\0')
	    return NULL;
	search = old_search;
    }

    /* Set up pattern-finder. */
    if (*search == '^') {
	match = strncmp;
	pat = (char *) (search + 1);
    } else {
	match = substrcmp;
	pat = (char *) search;
    }
    len = strlen(pat);

    for (pos = H.Pos; (*move) () != NULL;)
	if ((*match) ((char *) H.Lines[H.Pos], pat, len) == 0)
	    return H.Lines[H.Pos];
    H.Pos = pos;
    return NULL;
}

STATIC STATUS
h_search(void)
{
    static int Searching = FALSE;
    const char *old_prompt;
    char *(*move) (ALVOID);
    char *p;

    if (Searching)
	return ring_bell();
    Searching = TRUE;

    clear_line();
    old_prompt = Prompt;
    Prompt = "Search: ";
    TTYputs(Prompt);
#ifdef no_proto
    move = Repeat == NO_ARG ? prev_hist : next_hist;
#else
    move = Repeat == NO_ARG ? (char *(*)(void)) prev_hist : (char *(*)(void)) next_hist;
#endif
    p = search_hist(editinput(), move);
    clear_line();
    Prompt = old_prompt;
    TTYputs(Prompt);

    Searching = 0;
    return do_insert_hist(p);
}

STATIC STATUS
fd_char(void)
{
    int i;

    i = 0;
    do {
	if (Point >= End)
	    break;
	right(CSmove);
    } while (++i < Repeat);
    return CSstay;
}

STATIC void
save_yank(int begin, int i)
{
    if (Yanked) {
	free(Yanked);
	Yanked = NULL;
    }
    if (i < 1)
	return;

    if ((Yanked = (char *) malloc((size_t) (i + 1))) != NULL) {
	COPYFROMTO(Yanked, &Line[begin], (size_t) i);
	Yanked[i] = '\0';
    }
}

STATIC STATUS
delete_string(int count)
{
    int i;
    char *p;

    if (count <= 0 || End == Point)
	return ring_bell();

    if (count == 1 && Point == End - 1) {
	/* Optimize common case of delete at end of line. */
	End--;
	p = &Line[Point];
	i = 1;
	TTYput(' ');
	if (ISCTL(*p)) {
	    i = 2;
	    TTYput(' ');
	}
#ifdef NOMETA
	else if (rl_meta_chars && ISMETA(*p)) {
	    i = 3;
	    TTYput(' ');
	    TTYput(' ');
	}
#endif
	TTYbackn(i);
	*p = '\0';
	return CSmove;
    }
    if (Point + count > End && (count = End - Point) <= 0)
	return CSstay;

    if (count > 1)
	save_yank(Point, count);

    for (p = &Line[Point], i = End - (Point + count); i >= 0; i--, p++)
	p[0] = p[count];
    ceol();
    End -= count;
    TTYstring(&Line[Point]);
    return CSmove;
}

STATIC STATUS
bk_char(void)
{
    int i;

    i = 0;
    do {
	if (Point == 0)
	    break;
	left(CSmove);
    } while (++i < Repeat);

    return CSstay;
}

STATIC STATUS
bk_del_char(void)
{
    int i;

    i = 0;
    do {
	if (Point == 0)
	    break;
	left(CSmove);
    } while (++i < Repeat);

    return delete_string(i);
}

STATIC STATUS
del_char(void)
{
    return delete_string(Repeat == NO_ARG ? 1 : Repeat);
}

STATIC STATUS
redisplay(void)
{
    TTYputs((char *) NEWLINE);
    TTYputs(Prompt);
    TTYstring(Line);
    return CSmove;
}

STATIC STATUS
kill_line(void)
{
    int i;

    if (Repeat != NO_ARG) {
	if (Repeat < Point) {
	    i = Point;
	    Point = Repeat;
	    reposition();
	    (void) delete_string(i - Point);
	} else if (Repeat > Point) {
	    right(CSmove);
	    (void) delete_string(Repeat - Point - 1);
	}
	return CSmove;
    }
    save_yank(Point, End - Point);
    Line[Point] = '\0';
    ceol();
    End = Point;
    return CSstay;
}

STATIC STATUS
insert_char(int c)
{
    STATUS s;
    static char buff[2];
    char *p;
    char *q;
    int i;

    if (Repeat == NO_ARG || Repeat < 2) {
	buff[0] = (char) c;
	buff[1] = '\0';
	return insert_string(buff);
    }
    if ((p = (char *) malloc((size_t) (Repeat + 1))) == NULL)
	return CSstay;
    for (i = Repeat - 1, q = p; i >= 0; i--)
	*q++ = (char) c;
    *q = '\0';
    Repeat = 0;
    s = insert_string(p);
    free(p);
    return s;
}

STATIC STATUS
meta(void)
{
    UNSI c;
    KEYMAP *kp;

    if ((c = TTYget()) == EOF)
	return CSeof;
#if     defined(ANSI_ARROWS)
    /* Also include VT-100 arrows. */
    if (c == '[' || c == 'O')
	switch (c = TTYget()) {
	default:
	    return ring_bell();
	case EOF:
	    return CSeof;
	case KUP:
	    return h_prev();	/* ansi arrow keys */
	case KDN:
	    return h_next();
	case KRT:
	    return fd_char();
	case KLT:
	    return bk_char();
	case KCE:
	    strcpy((char *) Line, "N");		/* center of keypad */
	    return CSdone;
	case KF1:
	    strcpy((char *) Line, "help");	/* vt100 function keys f1 */
	    return CSdone;
	case KF2:
	    strcpy((char *) Line, "tag help");	/* f2 */
	    return CSdone;
	case KF3:
	    strcpy((char *) Line, "tag list");	/* f3 */
	    return CSdone;
	case KF4:
	    strcpy((char *) Line, "qlist");	/* f4 */
	    return CSdone;
	case KF5:
	    strcpy((char *) Line, "show terms");	/* f5 */
	    return CSdone;
	case KF0:
	    strcpy((char *) Line, "next");	/* f10 */
	    return CSdone;
#ifndef SYS_UNIX  /* non-unix */
	case KPU:
	    strcpy((char *) Line, "-");		/* page up */
	    return CSdone;
	case KPD:
	    strcpy((char *) Line, "+");		/* page down */
	    return CSdone;
	case KHM:
	    strcpy((char *) Line, "1");		/* home */
	    return CSdone;
	case KEN:
	    strcpy((char *) Line, "last");	/* end  */
	    return CSdone;
	case KIN:
	    return ring_bell();
	case KDL:
	    return del_char();
#else	/* linux, ibcs2, some other unixes */
	case '2':
	    if ((c = TTYget()) != '1')
		break;
	    if ((c = TTYget()) != '~')
		break;
	    strcpy((char *) Line, "next");	/* f10 */
	    return CSdone;
	case '3':
	    if ((c = TTYget()) != '\176')	/* delete  */
		break;
	    return del_char();
	case '5':
	    if ((c = TTYget()) != '\176')	/* page up */
		break;
	    strcpy(Line, "-");
	    return CSdone;
	case '6':
	    if ((c = TTYget()) != '\176')	/* page down */
		break;
	    strcpy(Line, "+");
	    return CSdone;
	case '1':
	    if ((c = TTYget()) != '\176')	/* home */
		break;
	    strcpy(Line, "1");
	    return CSdone;
	case '4':
	    if ((c = TTYget()) != '\176')	/* end */
		break;
	    strcpy(Line, "last");
	    return CSdone;
	case '[':
	    switch (c = TTYget()) {	/* ansi function keys */
	    case 'A':
		strcpy(Line, "help");	/* f1 */
		return CSdone;
	    case 'B':
		strcpy(Line, "tag help");	/* f2 */
		return CSdone;
	    case 'C':
		strcpy(Line, "tag list");	/* f3 */
		return CSdone;
	    case 'D':
		strcpy(Line, "qlist");	/* f4 */
		return CSdone;
	    case 'E':
		strcpy(Line, "show terms");	/* f5 */
		return CSdone;
	    case 'J':
		strcpy(Line, "next");	/* f10 */
		return CSdone;
	    }
#endif
	}
#endif				/* defined(ANSI_ARROWS) */

    if (isdigit(c)) {
	for (Repeat = c - '0'; (c = TTYget()) != EOF && isdigit(c);)
	    Repeat = Repeat * 10 + c - '0';
	Pushed = 1;
	PushBack = c;
	return CSstay;
    }
    if (isupper(c))
	return do_macro(c);
    for (OldPoint = Point, kp = MetaMap; kp->Func; kp++)
	if (kp->Key == (char) c)
	    return (*kp->Func) ();

    return ring_bell();
}

STATIC STATUS
emacs(int c)
{
    STATUS s;
    KEYMAP *kp;
#ifdef NOMETA
    if (ISMETA(c)) {
	Pushed = 1;
	PushBack = UNMETA(c);
	return meta();
    }
#endif
    for (kp = Map; kp->Func; kp++)
	if (kp->Key == (char) c)
	    break;
    s = kp->Func ? (*kp->Func) () : insert_char(c);
    if (!Pushed)
	/* No pushback means no repeat count; hacky, but true. */
	Repeat = NO_ARG;
    return s;
}

STATIC STATUS
TTYspecial(int c)
{
#ifdef NOMETA
    if (ISMETA(c))
	return CSdispatch;
#endif
    if (c == rl_erase || c == DEL)
	return bk_del_char();
    if (c == rl_kill) {
	if (Point != 0) {
	    Point = 0;
	    reposition();
	}
	Repeat = NO_ARG;
	return kill_line();
    }
    if (c == rl_intr || c == rl_quit) {
	Point = End = 0;
	Line[0] = '\0';
	return redisplay();
    }
    if (c == rl_eof && Point == 0 && End == 0)
	return CSeof;

    return CSdispatch;
}

STATIC char *
editinput(void)
{
    /* extern const char *luxptr ; */
    UNSI c;

    Repeat = NO_ARG;
    OldPoint = Point = Mark = End = 0;
    Line[0] = '\0';
    if (luxptr != NULL) {	/* pre-load edit buffer with text */
	strcpy((char *) Line, luxptr);
	End = strlen((char *) Line);
	Point = End;
	reposition();
	Point = 0;
	reposition();
    }
    while ((c = TTYget()) != EOF)
	switch (TTYspecial(c)) {
	case CSdone:
	    return Line;
	case CSeof:
	    return NULL;
	case CSmove:
	    reposition();
	    break;
	case CSdispatch:
	    switch (emacs(c)) {
	    case CSdone:
		return Line;
	    case CSeof:
		return NULL;
	    case CSmove:
		reposition();
		break;
	    case CSdispatch:
	    case CSstay:
		break;
	    }
	    break;
	case CSstay:
	    break;
	}
    return NULL;
}

STATIC void
hist_add(const char *p)
{
    int i;
    char *tpr;

    if ((tpr = strdup(p)) == NULL)
	return;
    if (H.Size < HIST_SIZE)
	H.Lines[H.Size++] = tpr;
    else {
	free(H.Lines[0]);
	for (i = 0; i < HIST_SIZE - 1; i++)
	    H.Lines[i] = H.Lines[i + 1];
	H.Lines[i] = tpr;
    }
    H.Pos = H.Size - 1;
}

/*
   **  For compatibility with FSF readline.
 */
#if 0
/* ARGSUSED0 */
void
rl_reset_terminal(char *p)
{
}

#endif

char *
readline(const char *prompt, scroll_command_t scrollflag)
{
    char *line;

    if (Line == NULL) {
	Length = MEM_INC;
	if ((Line = (char *) malloc(Length)) == NULL)
	    return NULL;
    }
    TTYinfo();
    rl_ttyset(0);
    hist_add(NilStr);
    ScreenSize = SCREEN_INC;
    Screen = (char *) malloc(ScreenSize);
    if (prompt != NULL)
	Prompt = prompt;
    else
	Prompt = (char *) NilStr;
    /*   Prompt = ((prompt != NULL) ? prompt : (char *) NilStr ) ; */
    TTYputs(Prompt);
    line = editinput();
    if (line != NULL) {
	line = (char *) strdup(line);	/*???? */
	if (scrollflag == do_scroll)
	    TTYputs((char *) NEWLINE);
	else if (scrollflag == no_scroll)
	    TTYputs("\r");
#ifdef ATPDBG
	else
	    assert(scrollflag == no_scroll || scrollflag == do_scroll);
#endif
	TTYflush();
    }
    rl_ttyset(1);
    free(Screen);
    free(H.Lines[--H.Size]);
    return (char *) line;
}

void
add_history(const char *p)
{
    if (p == NULL || *p == '\0')
	return;

#if     defined(UNIQUE_HISTORY)
    if (H.Pos && strcmp(p, (char *) (H.Lines[H.Pos - 1])) == 0)
	return;
#endif				/* defined(UNIQUE_HISTORY) */
    hist_add( /* (char *) */ p);
}


STATIC STATUS
beg_line(void)
{
    if (Point) {
	Point = 0;
	return CSmove;
    }
    return CSstay;
}

STATIC STATUS
end_line(void)
{
    if (Point != End) {
	Point = End;
	return CSmove;
    }
    return CSstay;
}

/*
   **  Move back to the beginning of the current word and return an
   **  allocated copy of it.
 */
STATIC char *
find_word(void)
{
    static char SEPS[] = "#;&|^$=`'{}()<>\n\t ";
    char *p;
    char *mnew;
    size_t len;

    for (p = &Line[Point]; p > Line && strchr(SEPS, (char) p[-1]) == NULL; p--)
	continue;
    len = (Point - (p - Line)) + 1;
    if ((mnew = (char *) malloc((len + 3))) == NULL)	/* we add 3 for wild cards used by ms-dos */
	return NULL;
    COPYFROMTO(mnew, p, len);
    mnew[len - 1] = '\0';
    return mnew;
}

STATIC STATUS
c_complete(void)
{
    char *p;
    char *word;
    int unique;
    STATUS s;

    word = find_word();
    p = (char *) rl_complete((char *) word, &unique);
    if (word)
	free(word);
    if (p && *p) {
	s = insert_string(p);
	if (!unique)
	    (void) ring_bell();
	free(p);
	return s;
    }
    return ring_bell();
}

STATIC STATUS
c_possible(void)
{
    char **av;
    char *word;
    int ac;

    word = find_word();
    ac = rl_list_possib((char *) word, (char ***) &av);
    if (word)
	free(word);
    if (ac) {
	columns(ac, av);
	while (--ac >= 0)
	    free(av[ac]);
	free(av);
	return CSmove;
    }
    return ring_bell();
}

STATIC STATUS
accept_line(void)
{
    Line[End] = '\0';
    return CSdone;
}

STATIC STATUS
transpose(void)
{
    char c;

    if (Point) {
	if (Point == End)
	    left(CSmove);
	c = Line[Point - 1];
	left(CSstay);
	Line[Point - 1] = Line[Point];
	TTYshow(Line[Point - 1]);
	Line[Point++] = c;
	TTYshow(c);
    }
    return CSstay;
}

STATIC STATUS
quote(void)
{
    UNSI c;

    c = TTYget();

    if (c == EOF)
	return CSeof;
    else
	return insert_char((int) c);
}

STATIC STATUS
wipe(void)
{
    int i;

    if (Mark > End)
	return ring_bell();

    if (Point > Mark) {
	i = Point;
	Point = Mark;
	Mark = i;
	reposition();
    }
    return delete_string(Mark - Point);
}

STATIC STATUS
mk_set(void)
{
    Mark = Point;
    return CSstay;
}

STATIC STATUS
exchange(void)
{
    UNSI c;

    if ((c = TTYget()) != CTL('X')) {
	if (c == EOF)
	    return CSeof;
	else
	    return ring_bell();
    }
    if ((c = Mark) <= End) {
	Mark = Point;
	Point = c;
	return CSmove;
    }
    return CSstay;
}

STATIC STATUS
yank(void)
{
    if (Yanked && *Yanked)
	return insert_string(Yanked);
    return CSstay;
}

STATIC STATUS
copy_region(void)
{
    if (Mark > End)
	return ring_bell();

    if (Point > Mark)
	save_yank(Mark, Point - Mark);
    else
	save_yank(Point, Mark - Point);

    return CSstay;
}

STATIC STATUS
move_to_char(void)
{
    UNSI c;
    int i;
    char *p;

    if ((c = TTYget()) == EOF)
	return CSeof;
    for (i = Point + 1, p = &Line[i]; i < End; i++, p++)
	if (*p == (char) c) {
	    Point = i;
	    return CSmove;
	}
    return CSstay;
}

STATIC STATUS
fd_word(void)
{
    return do_forward(CSmove);
}

STATIC STATUS
fd_kill_word(void)
{
    int i;

    (void) do_forward(CSstay);
    if (OldPoint != Point) {
	i = Point - OldPoint;
	Point = OldPoint;
	return delete_string(i);
    }
    return CSstay;
}

STATIC STATUS
bk_word(void)
{
    int i;
    char *p;

    i = 0;
    do {
	for (p = &Line[Point]; p > Line && !isalnum(p[-1]); p--)
	    left(CSmove);

	for (; p > Line && p[-1] != ' ' && isalnum(p[-1]); p--)
	    left(CSmove);

	if (Point == 0)
	    break;
    } while (++i < Repeat);

    return CSstay;
}

STATIC STATUS
bk_kill_word(void)
{
    (void) bk_word();
    if (OldPoint != Point)
	return delete_string(OldPoint - Point);
    return CSstay;
}

STATIC int
argify(char *line, char ***avp)
{
    char *c;
    char **p;
    char **snew;
    int ac;
    int i;

    i = MEM_INC;
    /* if ((*avp = p = NEW(char*, i))== NULL) */
    if ((*avp = p = (char **) malloc((i * sizeof(c)))) == NULL)
	return 0;

    for (c = line; isspace(*c); c++)
	continue;
    if (*c == '\n' || *c == '\0')
	return 0;

    for (ac = 0, p[ac++] = c; *c && *c != '\n';) {
	if (isspace(*c)) {
	    *c++ = '\0';
	    if (*c && *c != '\n') {
		if (ac + 1 == i) {
		    /* snew = NEW(char*, i + MEM_INC); */
		    snew = (char **) malloc((i + MEM_INC) * sizeof(c));
		    if (snew == NULL) {
			p[ac] = NULL;
			return ac;
		    }
		    COPYFROMTO(snew, p, i * sizeof(char **));
		    i += MEM_INC;
		    free(p);
		    *avp = p = snew;
		}
		p[ac++] = c;
	    }
	} else
	    c++;
    }
    *c = '\0';
    p[ac] = NULL;
    return ac;
}

STATIC STATUS
last_argument(void)
{
    char **av;
    char *p;
    STATUS s;
    int ac;

    if (H.Size == 1)
	return ring_bell();

    p = H.Lines[H.Size - 2];
    if (p == NULL)
	return ring_bell();

    if ((p = (char *) strdup((char *) p)) == NULL)
	return CSstay;
    ac = argify(p, &av);

    if (Repeat != NO_ARG)
	s = Repeat < ac ? insert_string(av[Repeat]) : ring_bell();
    else
	s = ac ? insert_string(av[ac - 1]) : CSstay;

    if (ac)
	free(av);
    free(p);
    return s;
}

/*

 * getakey() is primarily for reading the keyboard on non-unix systems
 * that don't understand termios or sgtty for setting raw key strokes mode.
 * Additionally, getakey() gets the raw keycode and builds the ANSI escape
 * sequences for special keys such as the arrow keys.
 *
 */

#if defined(_OS2) || defined(__MSDOS__)

STATIC int
getakey(char *chr)
{

    static int pushed = 0;
    static char c = 0;
    int t;

    if (pushed > 1) {
	pushed--;
	*chr = '[';
	return 1;
    }
    if (pushed) {
	pushed = 0;
	*chr = c;
	return 1;
    }
#if defined(DJGPP)
    t = getkey();
    if (t < 256) {
	*chr = (char) t;
	return 1;
    }
    t &= 0xFF;			/* mask out high bits */
#else
#ifndef __EMX__
    while (!kbhit())
	A_DVPAUZ;
#endif
    t = getch();
    if (t) {
	*chr = (char) t;
	return 1;
    }
    /* else extended character */
    t = getch();
#endif

    switch (t) {
    case 0x3b:
	c = KF1;
	break;
    case 0x3c:
	c = KF2;
	break;
    case 0x3d:
	c = KF3;
	break;
    case 0x3e:
	c = KF4;
	break;
    case 0x3f:
	c = KF5;
	break;
    case 0x40:
	c = KF6;
	break;
    case 0x41:
	c = KF7;
	break;
    case 0x42:
	c = KF8;
	break;
    case 0x43:
	c = KF9;
	break;
    case 0x44:
	c = KF0;
	break;
    case 0x47:
	c = KHM;
	break;
    case 0x48:
	c = KUP;
	break;
    case 0x49:
	c = KPU;
	break;
    case 0x4b:
	c = KLT;
	break;
    case 0x4c:
	c = KCE;
	break;
    case 0x4d:
	c = KRT;
	break;
    case 0x4f:
	c = KEN;
	break;
    case 0x50:
	c = KDN;
	break;
    case 0x51:
	c = KPD;
	break;
    case 0x52:
	c = KIN;
	break;
    case 0x53:
	c = KDL;
	break;
    default:
	c = 0;
    }
    if (c) {
	pushed = 2;
	*chr = ESC;
	return 1;
    } else
	return 0;
}
#endif

void
rl_initialize(void)
{
    MetaMap[0].Key = CTL('H');
    MetaMap[0].Func = bk_kill_word;
    MetaMap[1].Key = DEL;
    MetaMap[1].Func = bk_kill_word;
    MetaMap[2].Key = ' ';
    MetaMap[2].Func = mk_set;
    MetaMap[3].Key = '.';
    MetaMap[3].Func = last_argument;
    MetaMap[4].Key = '<';
    MetaMap[4].Func = h_first;
    MetaMap[5].Key = '>';
    MetaMap[5].Func = h_last;
    MetaMap[6].Key = '?';
    MetaMap[6].Func = c_possible;
    MetaMap[7].Key = 'b';
    MetaMap[7].Func = bk_word;
    MetaMap[8].Key = 'd';
    MetaMap[8].Func = fd_kill_word;
    MetaMap[9].Key = 'f';
    MetaMap[9].Func = fd_word;
    MetaMap[10].Key = 'l';
    MetaMap[10].Func = case_down_word;
    MetaMap[11].Key = 'u';
    MetaMap[11].Func = case_up_word;
    MetaMap[12].Key = 'y';
    MetaMap[12].Func = yank;
    MetaMap[13].Key = 'w';
    MetaMap[13].Func = copy_region;
    MetaMap[14].Key = '\0';
    MetaMap[14].Func = 0; /* null pointer */

    Map[0].Key = CTL('@');
    Map[0].Func = ring_bell;
    Map[1].Key = CTL('A');
    Map[1].Func = beg_line;
    Map[2].Key = CTL('B');
    Map[2].Func = bk_char;
    Map[3].Key = CTL('D');
    Map[3].Func = del_char;
    Map[4].Key = CTL('E');
    Map[4].Func = end_line;
    Map[5].Key = CTL('F');
    Map[5].Func = fd_char;
    Map[6].Key = CTL('G');
    Map[6].Func = ring_bell;
    Map[7].Key = CTL('H');
    Map[7].Func = bk_del_char;
    Map[8].Key = CTL('I');
    Map[8].Func = c_complete;
    Map[9].Key = CTL('J');
    Map[9].Func = accept_line;
    Map[10].Key = CTL('K');
    Map[10].Func = kill_line;
    Map[11].Key = CTL('L');
    Map[11].Func = redisplay;
    Map[12].Key = CTL('M');
    Map[12].Func = accept_line;
    Map[13].Key = CTL('N');
    Map[13].Func = h_next;
    Map[14].Key = CTL('O');
    Map[14].Func = ring_bell;
    Map[15].Key = CTL('P');
    Map[15].Func = h_prev;
    Map[16].Key = CTL('Q');
    Map[16].Func = ring_bell;
    Map[17].Key = CTL('R');
    Map[17].Func = h_search;
    Map[18].Key = CTL('S');
    Map[18].Func = ring_bell;
    Map[19].Key = CTL('T');
    Map[19].Func = transpose;
    Map[20].Key = CTL('U');
    Map[20].Func = ring_bell;
    Map[21].Key = CTL('V');
    Map[21].Func = quote;
    Map[22].Key = CTL('W');
    Map[22].Func = wipe;
    Map[23].Key = CTL('X');
    Map[23].Func = exchange;
    Map[24].Key = CTL('Y');
    Map[24].Func = yank;
    Map[25].Key = CTL('Z');
    Map[25].Func = ring_bell;
    Map[26].Key = CTL('[');
    Map[26].Func = meta;
    Map[27].Key = CTL(']');
    Map[27].Func = move_to_char;
    Map[28].Key = CTL('^');
    Map[28].Func = ring_bell;
    Map[29].Key = CTL('_');
    Map[29].Func = ring_bell;
    Map[30].Key = '\0';
    Map[30].Func = 0;  /* null pointer */
}


/* call this before exit */
void
rl_cleanup()
{
    int i = 0;

    free(Line);
    while (i < H.Size) {
	free(H.Lines[i]);
	i++;
    }
    if (Yanked)
	free(Yanked);
}


#if !defined(_OS2) && !defined(__MSDOS__)
void 
rl_set_cntrl_char(int cquit, int cerase, int ceof, int cintr, int ckill)
{
    rl_quit = cquit;
    rl_erase = cerase;
    rl_eof = ceof;
    rl_intr = cintr;
    rl_kill = ckill;
}
#endif

