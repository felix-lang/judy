# @(#) $Revision: 4.5 $ $Source: /judy/make_includes/platform.win_ipf.mk $

# Makefile fragment for Judy* for platform "win_ipf" (using MKS Toolkit and
# Visual Studio / VC++).  See platform.win_ia32.mk for more comments.

PLATFORM = win_ipf

# Commands to run, using full paths where appropriate to avoid ambiguity:
#
# Note:  MKS uses unusual path names.  Giving the "user" versions of these
# names to make(1), even with quote marks to hide spaces, seems to fail, so
# instead use "real" names (either way without *.exe suffixes).

SHELL	 = /mks/mksnt/sh

# Use VC++ compile tools rather than MKS compile tools:

# CCPATH = /PROGRA~1/MKSTOO~1/mksnt/cc
CCPATH	 = /PROGRA~1/MICROS~3/Bin/Win64/CL
# LDPATH = /PROGRA~1/MKSTOO~1/mksnt/cc
LDPATH	 = /PROGRA~1/MICROS~3/Bin/Win64/LINK

# Must use the "lib" command:

ARPATH	 = /PROGRA~1/MICROS~3/Bin/Win64/LIB
AR	 = $(ARPATH)
AR_OPT2	 = -out:

# Object file directories:
#
# $INTDIR is the location for intermediate (non-deliverable) constructed files,
# optionally including a suffix component for recursive make calls.
#
# $DELDIR is the CPF-standard "deliver/" followed by various paths for
# different files so they are in an appropriate place for delivery to an HPUX
# system.  Yes, this leads to annoyingly long full paths such as
# hpux_pa/product/deliver/usr/share/doc/Judy, but Judy-cious use of symlinks in
# the top of a personal sandbox can mitigate this.
#
# Note:  Through version 4.56, files were placed in deliver/opt/Judy, but in
# preparation for 11.11 OEUR delivery we decided they belong in various "core
# HPUX" locations under /usr on the filesystem (see below).
#
# Note:  For open source delivery, Judy files belong back in /opt/Judy; see the
# install target.
#
# Note:  For safety the values of $OBJDIR_OPT and the suffix of $DELDIR_DOC are
# hard-coded into the install and uninstall targets.

OBJDIR_OPT =	/opt/Judy

# For Linux, $DELDIR_LIB_SUFFIX is simply null.

DELDIR_LIB =	$(DELDIR)/usr/lib$(DELDIR_LIB_SUFFIX)
DELDIR_INC =	$(DELDIR)/usr/include
DELDIR_DOC =	$(DELDIR)/usr/share/doc/Judy
DELDIR_DEMO =	$(DELDIR_DOC)/demo
DELDIR_MAN =	$(DELDIR)/usr/share/man/man3$(MANDIR_SUFFIX)

# Use normal tools for building internal tools (programs):

CCPATH_TOOL = $(CCPATH)

ECHO	 = echo
PWD	 = /mks/mksnt/pwd

# Platform-specific library lists for win_ipf for various "lib" targets:

LIB_LIST      = lib64lib
LIBS_LIST     = lib64lib
LIBS_ALL_LIST = lib64lib
LIBS_PIC_LIST = TBD

# Suffix for $LIB_ID (same as what's passed in $LIB_SUFFIX for a recursive
# make):

LIB_ID_SUFFIX = lib

# Linker option to build a shared library:
#
# TBD:  Fill this in when appropriate.

LD_OPT_SL = ?

TARCHIVE_LIBS_PIC =

# Unfortunately there are CC_OPTS differences between both platforms AND
# flavors, so spell them out here by flavor, meaning this file cannot be
# flavor-ignorant:
#
# CL -nologo hides the banner.
# CL -W4 is like -Wall, and "4" is the highest number it seems to accept.
# This platform does not define __LP64__ because it's not a 64-bit data model,
# but Judy wants JU_64BIT anyway because it uses 64-bit words to match pointer
# sizes.

CC_OPTS_product = -O2 -W4                      -nologo -DJU_64BIT
CC_OPTS_cov	= -O2 -W4  -DJU_FLAVOR_COV     -nologo -DJU_64BIT
CC_OPTS_debug	= -GZ -W4  -DJU_FLAVOR_DEBUG   -nologo -DJU_64BIT

# At least on this platform, for make to recognize that a tool is already
# built, the filename + auto-added suffix must be used:

TOOL_SUFFIX = .exe

# TBD:  MKS has compress, but directory paths look like:
#
#   C:/Program Files/MKS Toolkit/etc/cat1
#
# Unclear where to put manual entries or whether to compress them.  Also, MKS
# compress complains:  "Output in LZW compress format is not supported."  For
# now just drop them uncompressed.

COMPRESSPATH   = cat
MANDIR_SUFFIX  =
MANFILE_SUFFIX =

# Platform win_ipf VC++ "CL" compiler needs -Fo to specify the object file and
# -Fe to specify the executable file:

CCoo = -Fo
CCoe = -Fe
