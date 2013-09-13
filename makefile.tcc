# -------------------------------------------------------------------------
# Turbo C/C++ Makefile for MS-DOS version of ATP December 10, 1996
# tested with Borland's MAKE versions 3.0 and 3.7
# NOTE: Unix and D.J.Delore GCC for DOS use *other* makefiles!!!
#
# Read all this stuff first running make!
#
# -------------------------------------------------------------------------
# This makefile has been used and tested under Borland's Turbo C++ version
# 1.01 2nd edition, and with Borland C++ 4.02. They should compile ATP
# easily. A couple of cautionary warnings will be generated but these are
# harmless. The TCC version of ATP is identical to the Unix version except
# for somewhat smaller limits because of MS_DOS memory segmentation
# considerations.  Limits are smaller but in practice this should not be
# much of a problem since even the reduced limits of the TCC version are
# about an order of magnitude more generous than most QWK readers for MS-DOS.
#
# Make sure that you have a good number of files and buffers set in your
# CONFIG.SYS before compiling. At least 20 for each is a reasonable choice.
# It is suggested that the large memory model be used for compilation
# under Turbo C++ for DOS. If you have Ralph Brown's spawno libraries
# installed, uncomment SPAWNO and SPAWNL in the compiler and linker flags
# section below. This will allow ATP to use the spawno version of system().
# The advantage is that ATP will swap itself out when launching sub-programs,
# thus freeing-up memory.
# 
# Note that it is possible that your compiler's header files could contain
# errors. For example, in Turbo C++ 1.01 the stat() declaration is in
# error. The proper prototype for stat() is:
#
#          int  _Cdecl stat  (const char *__path, struct stat *__statbuf);
#
# Unless you correct this by editing your tc\include\sys\stat.h to match
# the above, you will encounter errors trying to compile ATP with C++. 
# C++ is much more picky about prototypes. However ATP is actually written
# in ansi C, so the only reason to compile it with C++ is for the stricter 
# type checking. 
#
# When using 16 bit compilers, it is suggested that the large memory
# model be used for compilation. Additonally, when using a 16 bit
# compiler you must define -D_ATP16_ below. That is how this makefile is
# already configured. If you modify this makefile for use with a 32 bit
# compiler, don't forget to remove -D_ATP16_ !
# -------------------------------------------------------------------------
# borland make likes these,
@ = $.
.AUTODEPEND

# -------------------------------------------------------------------------
BSHELL	= command.com /c
!if $d(BCC)
CC	= bcc
!else
CC	= tcc
!endif
LD	= tlink
AR	= tlib
RM	= $(BSHELL) RMV.BAT
CD	= $(BSHELL) CDIR.BAT
MV	= $(BSHELL) ren
CP	= $(BSHELL) copy
!if !$d(MAKE)
MAKE	= make
!endif
MK_FILE	= makefile.tcc

# release build definitions
!if $(ATPREL)
RELEASE = ATPREL
SPAWNO	= -DSPAWNO
SPAWNL	= spawnl.lib
CPLUS_=-P
!else
RELEASE = nothing
!endif
FORCEMAKE = FO.RCE

# -------------------------------------------------------------------------
# Here are the flags to enable or suppress warning messges
#
WARN	= -w -w-inl

# -------------------------------------------------------------------------
# Attention les francophones!
# Si vous voulez, effacez -DENGLISH dessous et substituez -DFRENCH 
LANG	= -DENGLISH
#LANG	= -DFRENCH
#LANG	= -DGERMAN

#---------------------------------------------------------------------------
# compiler and linker flags
#

!if $d(ATPDBG)
MAP	= -k -M -y -ls
DEBUG	= -N $(MAP) -DATPDBG
!endif

!if $d(BCC)
OFLAGS	= -Obeiglmptv -G -a -d -k- -f- -ml -1- -Z -vi -RT-
!else
OFLAGS	= -O -G -a -d -k- -f- -ml -1- -vi
!endif
MK_CMD	= $(MAKE) -f$(MK_FILE) -D$(RELEASE)
DEFS	= $(LANG) $(SPAWNO) -DDESQVP -D_ATP16_ 
CFLAGS	= $(CPLUS_) $(OFLAGS) $(DEBUG) $(WARN) $(DEFS)
LDFLAG	= $(OFLAGS) $(MAP) -ld -lc @link.atp
LIBS	= $(SPAWNL) .\editline\libedit.lib

# -------------------------------------------------------------------------
# lint configuration
LINT	= tlint
LINTFG	= $(LANG) $(SPAWNO) -D__LINT__ -D__MSDOS__ -D__TURBOC__
LINTLB	= -l./editline/libedit.ln

# -------------------------------------------------------------------------

SR1	= read.c readlib.c makemail.c system.c ansi.c qlib.c text.c reply.c
SR2	= chosetag.c atpmain.c rdconfig.c purge.c display.c mkindex.c

OB1	= read.obj readlib.obj makemail.obj system.obj ansi.obj qlib.obj text.obj reply.obj
OB2	= chosetag.obj atpmain.obj rdconfig.obj purge.obj display.obj mkindex.obj

SRC	= $(SR1) $(SR2)
OBJ	= $(OB1) $(OB2)

.c.obj:
	$(CC) $(CFLAGS) -c { $*.c }

all : atp.exe utility

atp.exe:  $(OBJ) eline link.atp
	$(CC) -eatp.exe $(LDFLAG) $(LIBS)
	$(BSHELL) dir atp.exe
	
link.atp:
	@if exist $(@) del $(@)
	@for %F in ($(OB1)) do echo %F >> $(@) 
	@for %F in ($(OB2)) do echo %F >> $(@) 
	@for %F in ($(OB3)) do echo %F >> $(@) 

#---------------------------------------------------------------------------
# create tcc makefiles in subdirectories

# utils
utils\$(MK_FILE) : utils\makefile.dos
	@echo CC        = $(CC) $(OFLAGS) >trb.tmp
	@$(CP) utils\makefile.dos makefile.tmp
	edlin makefile.tmp <<X
24t trb.tmp
3,23d
e
X
	@$(CP) makefile.tmp utils\$(MK_FILE)
	@del makefile.tmp
	@del trb.tmp

# editline
editline\$(MK_FILE) : editline\makefile.dos
	@$(CP) editline\stub.tcc trb.tmp
	@echo CC = $(CC)>>trb.tmp
	@echo AR = $(AR)>>trb.tmp
	@echo ARFLAG = /c>>trb.tmp
	@echo LD = $(LD)>>trb.tmp
	@echo LDFLAG = /c /d>>trb.tmp
	@echo DEFS = -DDESQVP>>trb.tmp
	@echo OFLAGS = $(OFLAGS)>>trb.tmp
	@echo WARN = -w>>trb.tmp
	@$(CP) editline\makefile.dos makefile.tmp
	edlin makefile.tmp <<X
34t trb.tmp
3,33d
e
X
	@$(CP) makefile.tmp editline\$(MK_FILE)
	@del makefile.tmp
	@del trb.tmp

#---------------------------------------------------------------------------
# utility builds the atp support and diagnostic programs

utility : utils\$(MK_FILE) knowname $(FORCEMAKE)
	@$(CD) utils $(MK_CMD) all

# --------------------------------------------------------------------------
# eline builds libedit.lib used for command-line editing and history recall.

eline : knowname $(FORCEMAKE)
	$(CD) editline $(MK_CMD) libedit.lib

# -------------------------------------------------------------------------
# lint and doc targets

lint :	elint
	$(LINT) $(LINTFG) $(LINTLB) $(SRC) >$(@)

elint : knowname 
	@$(CD) editline $(MK_CMD) libedit.ln
	
atp.doc :
	groff -man docs\atp.1 | sed -e "s/.//g" >$(@)

manifest : $(FORCEMAKE)
	ls -lR > Manifest	

# -------------------------------------------------------------------------
# cleaning and housekeeping targets

clean : rmv.bat $(FORCEMAKE)
	@$(RM) atp.exe atp.lib $(FORCEMAKE) *.obj *.map *.lst
	
mclean : knowname clean 
	@$(RM) atplint
	@$(CD) editline $(MK_CMD) $(@)
	@$(CD) utils    $(MK_CMD) $(@)

clobber : knowname clean 
	@$(RM) link.atp ansi2knr.exe rot13.exe atpdiag.bat link.atp atplint
	@$(RM) *.a *.o *.bak *.cof *.lib *.lst
	@$(CD) editline $(MK_CMD) $(@)
	@$(CD) utils    $(MK_CMD) $(@)
	@$(RM) tstmymak.bat cdir.bat
	@$(BSHELL) del rmv.bat

mostlyclean : mclean 

# -------------------------------------------------------------------------
# support targets and batch files used by make

# does our version of MAKE know it's own name? 
knowname : batch utils\$(MK_FILE) editline\$(MK_FILE) $(FORCEMAKE)
	@$(BSHELL) TSTMYMAK.BAT $(MK_FILE) $(MAKE) 

batch : tstmymak.bat cdir.bat rmv.bat

tstmymak.bat : utils\batch\tstmymak.in
	$(CP) utils\batch\tstmymak.in $(@) 

rmv.bat : utils\batch\rmv.in
	$(CP) utils\batch\rmv.in $(@) 

cdir.bat : utils\batch\cdir.in
	$(CP) utils\batch\cdir.in $(@) 

# phony rule is used to FORCE unconditional build of targets 
$(FORCEMAKE) :
	@$(BSHELL) rem

memchk: knowname $(FORCEMAKE)
	@$(CD) editline $(MK_CMD) $(@)

wipe: $(FORCEMAKE)
	@$(RM) editline\makefile.tcc utils\makefile.tcc
	
# -------------------------------------------------------------------------
# object file depenencies
#
atpmain.obj : atpmain.c system.h atptypes.h globals.h reader.h readlib.h makemail.h qlib.h ansi.h
read.obj : read.c system.h atptypes.h globals.h reader.h readlib.h makemail.h qlib.h ansi.h
readlib.obj : readlib.c system.h atptypes.h globals.h reader.h readlib.h ansi.h 
makemail.obj : makemail.c system.h atptypes.h globals.h reader.h readlib.h 
system.obj : system.c system.h atptypes.h globals.h reader.h readlib.h ansi.h qlib.h
ansi.obj : ansi.c system.h atptypes.h globals.h reader.h readlib.h ansi.h
qlib.obj : qlib.c system.h atptypes.h globals.h reader.h readlib.h ansi.h qlib.h  makemail.h
mkindex.obj : mkindex.c system.h atptypes.h globals.h reader.h readlib.h ansi.h 
text.obj : text.c system.h atptypes.h globals.h reader.h readlib.h ansi.h
chosetag.obj : chosetag.c system.h atptypes.h globals.h reader.h readlib.h qlib.h  ansi.h makemail.h
purge.obj : purge.c system.h atptypes.h globals.h reader.h readlib.h qlib.h ansi.h makemail.h
display.obj : display.c system.h atptypes.h globals.h reader.h readlib.h makemail.h qlib.h ansi.h
reply.obj : reply.c system.h atptypes.h globals.h reader.h readlib.h ansi.h makemail.h qlib.h
rdconfig.obj : rdconfig.c system.h atptypes.h globals.h reader.h readlib.h makemail.h qlib.h ansi.h
#
# end of makefile ---------------------------------------------------------
