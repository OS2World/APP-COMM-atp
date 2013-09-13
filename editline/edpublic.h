/* edpublic.h */
/* this file provides the public interface to the editline library */
#ifndef EDPUBLIC_H
#define EDPUBLIC_H

#ifdef no_proto   /* old non-ANSI C declaration */
size_t FindMatches() ;

#else  /* ANSI C */

#if (defined(__MSDOS__) && !defined(HAVE_DIRENT_H)) || defined(WIN32)
#define FINDFIRST
size_t FindMatches (const char *path, char ***avp);
#else
size_t FindMatches(const char *dir, const char *file, char ***avp) ;
#endif

#endif /* no_proto */

/* arbitrary constants used by readline().*/
typedef enum {
    no_scroll = -25, do_scroll
} scroll_command_t ;

extern const char *luxptr ;
char *readline( const char *prompt, scroll_command_t scrollflag);
void add_history(const char *p);
void TTYinfo(void);
void rl_cleanup(void);
void rl_initialize(void);

#endif /* EDPUBLIC_H */

