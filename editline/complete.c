/*
**
**  History and file completion functions for editline library.
*/
#include "editline.h"
#include "edlproto.h"

#ifdef NEED_STRDUP
/*
**  Return an allocated copy of a string.
*/
char *
strdup( const char *p)
{
    char	*mnew;

    if ((mnew = (char *) malloc ( (strlen(p)+1) )) != NULL)
	(void)strcpy(mnew, p);
    return mnew;
}
#endif	/* defined(NEED_STRDUP) */

/*
**  strcmp-like sorting predicate for qsort.
*/

STATIC int
compare(const void *p1, const void *p2)
{
    return strcmp( (const char *) p1, (const char *) p2);
}

#ifdef FINDFIRST 
size_t 
FindMatches(const char *path, char ***avp)
{
    char	**av;
    char	**mnew;
    char	*p, *tpath;
    size_t	ac;
    int 	i=0 ;
 
#if defined(WIN32)
     struct _finddata_t	merv;
     long    filegrouphandle;
#elif defined(_MSC_)
#   define findfirst(A,B,C) _dos_findfirst(A,C,B)
#   define findnext _dos_findnext
#   define ff_name name
#   define ffblk find_t
#endif
 
    struct ffblk merv ;
 
    av = NULL;
    ac = 0;
    tpath = (char *)malloc( strlen(path) + 5 );
    strcpy( tpath, path );
    p  = tpath ;

   while(*p) p++, i++ ;  /* find proper wild card to append */
   if( i > 4) 
       i = 4 ;
   if( i==0 )
   	i = 1 ;
   else
	p-- ;	 
   if( *p != '*')
       strcat(tpath, "*") ; 
   for( ; i > 0 ; i-- ){
      if( *p == '.' ) 
         i = -1 ;
      else  if ( i == 1 ) 
        strcat(tpath, ".*") ;
      else
        p-- ;
    }
#ifdef WIN32
   if((filegrouphandle = _findfirst( (char *) tpath, &merv)) != 0){
    p = merv.name;
#else
   if(!findfirst( (char *) tpath, &merv, 0 )){
	p = merv.ff_name ;
#endif	
	strlwr( p ) ;
	if ((mnew = (char **)malloc ( sizeof(char*) * ( ac + MEM_INC))) == NULL)
		goto myend ;
	*avp = mnew;
        av = mnew ;
	if ((av[ac] = strdup(p)) == NULL) {
		free (av);
		goto myend ;
	}
	ac++;
#ifdef WIN32
    while( !_findnext( filegrouphandle, &merv ) ){
    p = merv.name;
#else
	while( !findnext( &merv )){
	p = merv.ff_name ;
#endif	
	strlwr(p);
	if ((ac % MEM_INC) == 0) { /* allocate bigger array */
	    if ((mnew = (char **)malloc ( sizeof(char*) * ( ac + MEM_INC))) == NULL)
		goto myend ;
	    if (ac) {
		COPYFROMTO(mnew, av, ac * sizeof (char **));
		free (av);
	    }
	    *avp = mnew;
            av = mnew ;
	}
	if ((av[ac] = strdup(p)) == NULL) 
            goto myend ;
	ac++;
    	}
	}
myend:
#ifdef WIN32
    _findclose (filegrouphandle);
#endif
    if (ac)
	qsort( av, ac, sizeof (char **), compare );
    free(tpath);
    return ac;
}
#else /* POSIX DIRENT */
/*
**  Fill in *avp with an array of names that match file, up to its length.
**  Ignore . and .. .
*/
size_t 
FindMatches(const char *dir, const char *file, char ***avp)
{
    char	**av;
    char	**fnew;
    char	*p;
    DIR		*dp;
    DIRENTRY	*ep;
    size_t	ac;
    size_t	len;

    if ((dp = opendir(dir)) == NULL)
	return 0;

    av = NULL;
    ac = 0;
    len = strlen(file);
    while ((ep = readdir(dp)) != NULL) {
	p = ep->d_name;
	if (p[0] == '.' && (p[1] == '\0' || (p[1] == '.' && p[2] == '\0')))
	    continue;
	if (len && strncmp(p, file, len) != 0)
	    continue;

	if ((ac % MEM_INC) == 0) {
	    if ((fnew = (char **) malloc (sizeof(char*) * (ac + MEM_INC))) == NULL)
		break;
	    if (ac) {
		COPYFROMTO(fnew, av, ac * sizeof (char **));
		free (av);
	    }
	    *avp = av = fnew;
	}

	if ((av[ac] = (char *) strdup(p)) == NULL) {
	    if (ac == 0)
		free (av);
	    break;
	}
	ac++;
    }

    /* Clean up and return. */
    (void)closedir(dp);
    if (ac)
	qsort(av, ac, sizeof (char **), compare);
    return ac;
}

#endif
/*
**  Split a pathname into allocated directory and trailing filename parts.
*/
STATIC int
SplitPath( const char *path, char** const dirpart, char** const filepart )
{
    static char	DOT[] = ".";
    char	*dpart;
    char	*fpart;
    const char	*tpart;

    if ((tpart = strrchr(path, SEPCH )) == NULL) {
	if ((dpart = strdup(DOT)) == NULL)
	    return -1;
	if ((fpart = strdup(path)) == NULL) {
	    free (dpart);
	    return -1;
	}
    }
    else {
	if ((dpart = strdup(path)) == NULL)
	    return -1;
	dpart[tpart - path] = '\0';
	tpart++;
	if ((fpart = strdup(tpart)) == NULL) {
	    free (dpart);
	    return -1;
	}
    }
    *dirpart = dpart;
    *filepart = fpart;
    return 0;
}

/*
**  Attempt to complete the pathname, returning an allocated copy.
**  Fill in *unique if we completed it, or set it to 0 if ambiguous.
*/
char *
rl_complete( const char *pathname, int *unique)
{
    char **av;
    char *dir;
    char *file;
    char *p;
    size_t ac;
    size_t end;
    size_t i;
    size_t j;
    size_t len;

    if (SplitPath(pathname, &dir, &file) < 0)
	return NULL;
#ifndef FINDFIRST
    if ((ac = FindMatches(dir, file, &av)) == 0)
#else
    if ((ac = FindMatches(pathname, &av)) == 0)
#endif
    {
	free(dir);
	free(file);
	return NULL;
    }
    p = NULL;
    len = strlen(file);
    if (ac == 1) {
	/* Exactly one match -- finish it off. */
	*unique = 1;
	j = (strlen(av[0]) - len) + 1;
	if ((p = (char *) malloc(j + 1)) != NULL) {
	    char *tcpath;
	    const size_t mlen = strlen(dir) + strlen(av[0]) + (size_t) 3;	/* 3 ?? */
	    COPYFROMTO(p, av[0] + len, j);
	    if ((tcpath = (char *) malloc(mlen)) != NULL) {
		strcpy(tcpath, dir);
		strcat(tcpath, SEPST);
		strcat(tcpath, av[0]);
		rl_add_slash(tcpath, p);
		free(tcpath);
	    }
	}
    } else {
	*unique = 0;
	if (len) {
	    /* Find largest matching substring. */
	    for (i = len, end = strlen(av[0]); i < end; i++)
		for (j = 1; j < ac; j++)
		    if (av[0][i] != av[j][i])
			goto breakout;
	  breakout:
	    if (i > len) {
		j = i - len + 1;
		if ((p = (char *) malloc(j)) != NULL) {
		    COPYFROMTO(p, av[0] + len, j);
		    p[j - 1] = '\0';
		}
	    }
	}
    }

    /* Clean up and return. */
    free(dir);
    free(file);
    for (i = 0; i < ac; i++)
	free(av[i]);
    free(av);
    return p;
}

/*
**  Return all possible completions.
*/
int
rl_list_possib(char *pathname, char ***avp)
{
    int		ac;
#ifndef FINDFIRST
    char	*dir;
    char	*file;

    if (SplitPath(pathname, &dir, &file) < 0)
	return 0;
    ac = FindMatches(dir, file, avp);
    free (dir);
    free (file);
#else
    ac = FindMatches(pathname, avp);
#endif
    return ac;
}
