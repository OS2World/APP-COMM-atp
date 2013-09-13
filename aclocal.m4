# Local additions to Autoconf macros.
# Copyright (C) 1992, 1994 Free Software Foundation, Inc.
# Francois Pinard <pinard@iro.umontreal.ca>, 1992.
# ( modified slightly for ATP by tgm 11/16/94 )
# @defmac AC_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it defines @code{__STDC__} to 1 and
# handles function prototypes correctly.
# 
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{ac_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac

define(fp_PROG_CC_STDC,
[AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(ac_cv_prog_cc_stdc,
[ac_cv_prog_cc_stdc=no
ac_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
for ac_arg in "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" -Xc
do
  CFLAGS="$ac_save_CFLAGS $ac_arg"
  AC_TRY_COMPILE(
[#if !defined(__STDC__) || __STDC__ != 1
choke me
#endif	
], [int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};],
[ac_cv_prog_cc_stdc="$ac_arg"; break])
done
CFLAGS="$ac_save_CFLAGS"
])
AC_MSG_RESULT($ac_cv_prog_cc_stdc)
case "x$ac_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $ac_cv_prog_cc_stdc" ;;
esac
])

# Check for function prototypes.

AC_DEFUN(atp_C_PROTOTYPES,
[AC_REQUIRE([fp_PROG_CC_STDC])
AC_MSG_CHECKING([for working ansi prototypes])
if test "$ac_cv_prog_cc_stdc" != no; then
  AC_MSG_RESULT(yes)
  ATPCNVR=
else
  AC_MSG_RESULT(no)
  ATPCNVR=convert
  AC_DEFINE(no_proto)
fi
AC_SUBST(ATPCNVR)
])

# Check if --with-dmalloc was given.

AC_DEFUN(fp_WITH_DMALLOC,
[AC_MSG_CHECKING(if malloc debugging is wanted)
AC_ARG_WITH(dmalloc,
[  --with-dmalloc          use dmalloc, as in dmalloc.tar.gz from
                          @/ftp.antaire.com:antaire/src/dmalloc.],
[if test "$withval" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_DMALLOC)
  LIBS="$LIBS -ldmalloc"
  LDFLAGS="$LDFLAGS -g"
else
  AC_MSG_RESULT(no)
fi], [AC_MSG_RESULT(no)])])

dnl ### Checks for operating system services


AC_DEFUN(atp_SYS_INTERPRETER,
[# Pull the hash mark out of the macro call to avoid m4 problems.
AC_PATH_PROG(CATBPATH,cat,/bin/cat)
ac_msg="whether #! works in shell scripts"
AC_MSG_CHECKING($ac_msg)
AC_CACHE_VAL(ac_cv_sys_interpreter,
[echo "#!$CATBPATH
exit 69
" > conftest
chmod u+x conftest
(SHELL=/bin/sh; export SHELL; ./conftest >/dev/null 2>&1)
if test $? -ne 69; then
   ac_cv_sys_interpreter=yes
else
   ac_cv_sys_interpreter=no
fi
rm -f conftest])dnl
AC_MSG_RESULT($ac_cv_sys_interpreter)
])


# check for nroff program and proper output device for plain text
#
AC_DEFUN(atp_PROG_NROFF,
[
# see which nroff program is available (gnu groff preferred of course).
AC_CHECK_PROG(ROFF,groff,groff,nroff)

# set output device for nroff command
AC_CACHE_CHECK(for $ROFF output device,atp_cv_prog_nroff_device,[
cat>nroff.tst <<EOF
hello
EOF

if test "$ROFF" = groff ; then
  atp_cv_prog_nroff_device=-Tascii
elif  $ROFF -Tascii nroff.tst >/dev/null 2>&1 ; then
  atp_cv_prog_nroff_device=-Tascii
elif $ROFF -Tman  nroff.tst >/dev/null 2>&1 ; then
  atp_cv_prog_nroff_device=-Tman
elif $ROFF -Tlpr  nroff.tst >/dev/null 2>&1 ; then
  atp_cv_prog_nroff_device=-Tlpr
else 
  atp_cv_prog_nroff_device=""
fi

rm -f nroff.tst
])
A_DEVRF=$atp_cv_prog_nroff_device
AC_SUBST(A_DEVRF)
])


# check if mkdir accepts the -p switch
#
AC_DEFUN(atp_PROG_MKDIR_P,
[
AC_CACHE_CHECK(whether mkdir accepts -p,atp_cv_prog_mkdir_p,[
if mkdir -p ./sd9fdSf3/Td7sfvIl ; then
  rmdir ./sd9fdSf3/Td7sfvIl
  rmdir ./sd9fdSf3
  atp_cv_prog_mkdir_p=yes
else
  atp_cv_prog_mkdir_p=no
fi
])
if test "$atp_cv_prog_mkdir_p" = yes ; then
  MKDIR="mkdir -p"
else
  MKDIR=mkdir
fi
AC_SUBST(MKDIR)
])


# check if col accepts the -x switch
#
AC_DEFUN(atp_PROG_COL_X,
[
AC_CHECK_PROG(A_COL,col,UCOL,SCOL)
A_UCOL="col -b"
if test "$A_COL" = UCOL ; then
AC_CACHE_CHECK(whether col accepts -x,atp_cv_prog_col_x,[
cat>col.tst <<EOF
hello
EOF

   if col -x < col.tst >/dev/null 2>&1 ; then
     atp_cv_prog_col_x=yes
    else
     atp_cv_prog_col_x=no
    fi
    rm -f col.tst
])
    if test "$atp_cv_prog_col_x" = yes ; then
      A_UCOL="col -bx"
    fi
fi
AC_SUBST(A_UCOL)
])


# Check for termcap global variables in libraries.
#
#AC_DEFUN(atp_VAR_TERMCAP,
#[
#AC_MSG_CHECKING([for termcap global variables])
#AC_CACHE_VAL(atp_cv_var_termcap,
#[ AC_TRY_LINK( [#define SYS_UNIX 1],
#extern char *BC  ;
#extern char *UP ;
#extern char PC ;
#extern short ospeed ;
#int atp_ospeed ; int atp_PC; char *atp_BC; char *atp_UP ;
#atp_ospeed = ospeed ; atp_PC = PC ; atp_BC = BC ; atp_UP = UP ; ,
#atp_cv_var_termcap=yes, atp_cv_var_termcap=no)])
#if test "$atp_cv_var_termcap" = no; then
#AC_DEFINE(NEED_TCVARS)
#fi
#AC_MSG_RESULT($atp_cv_var_termcap)
#])

## check for man page owner and group

# helper function
AC_DEFUN(atp_GROUP_MAN_TPATH,
[
AC_PROVIDE([$0])
atp_t_file=man/man1/ls.1

if   test -f /usr/$atp_t_file* ; then
  atp_t_path=/usr/$atp_t_file
elif test -f /usr/share/$atp_t_file* ; then
  atp_t_path=/usr/share/$atp_t_file
else
  >t_tmp_t.test
  atp_t_path=./t_tmp_t.test
fi
])

# get man owner
AC_DEFUN(atp_GROUP_MAN_OWN,
[
 AC_REQUIRE([atp_GROUP_MAN_TPATH])
 atp_cv_group_man_own=`ls -l $atp_t_path* | $ac_cv_prog_AWK '{ print [$[]3] }'`
])

# get bin owner
AC_DEFUN(atp_GROUP_BIN_OWN,
[
  AC_REQUIRE([AC_PROG_AWK])
  >t_tmp_b.test
  atp_b_path=./t_tmp_b.test
 atp_cv_group_bin_own=`ls -l $atp_b_path* | $ac_cv_prog_AWK '{ print [$[]3] }'`
 rm -f t_tmp_b.test
])

dnl AC_DEFUN(atp_GROUP_MAN_GRP,
dnl [
dnl dnl AC_REQUIRE([atp_GROUP_MAN_TPATH])
dnl 
dnl atp_t_grp=`ls -l $atp_t_path* | $ac_cv_prog_AWK '{ print [$[]4] }'`
dnl 
dnl if echo $atp_t_grp | grep -e '[A-Za-z]' >/dev/null 2>&1 ; then
dnl   :
dnl else
dnl   atp_t_grp=`ls -lg $atp_t_path* | $ac_cv_prog_AWK '{ print [$[]4] }'`
dnl fi
dnl 
dnl rm -f ./t_tmp_t.test
dnl atp_cv_group_man_grp=$atp_t_grp
dnl ])

dnl AC_DEFUN(atp_GROUP_MAN,
dnl [
dnl  AC_CACHE_CHECK(for man owner,atp_cv_group_man_own,
dnl   atp_GROUP_MAN_OWN
dnl  )
dnl  AC_CACHE_CHECK(for man group,atp_cv_group_man_grp,
dnl   atp_GROUP_MAN_GRP
dnl  )
dnl  rm -f ./t_tmp_t.test
dnl  man_own=$atp_cv_group_man_own
dnl  AC_SUBST(man_own)
dnl  man_grp=$atp_cv_group_man_grp
dnl  AC_SUBST(man_grp)
dnl ])

AC_DEFUN(atp_C_VOLATILE,
[
AC_CACHE_CHECK(for working volatile,atp_cv_c_volatile,[
AC_TRY_COMPILE( ,
static volatile int mcmahon ; ,
atp_cv_c_volatile=yes,
atp_cv_c_volatile=no)])
# define volatile to empty string if test fails
if test "$atp_cv_c_volatile" = no ; then
  AC_DEFINE(volatile, )
fi 
])
