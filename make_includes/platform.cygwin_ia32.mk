# @(#) $Revision: 4.21 $ $Source: /judy/make_includes/platform.cygwin_ia32.mk $

# Makefile fragment for Judy* for platform "cygwin_ia32".
# See platform.hpux_pa.mk for more comments.

# Note that not all Cygwin systems have ksh.  Use the following shell instead
# because it should always be present and POSIX-compliant; avoid /bin/bash
# because it isn't; even if /bin/sh is a symlink to bash, the name matters.

SHELL	= /bin/sh

# For portability use the generic names for cc, ld, and ar, which are typically
# symlinks to more specific names, such as /usr/bin/cc -> /opt/ansic/bin/cc.

CCPATH	= cc
LDPATH	= cc
ARPATH	= ar
AR	= $(ARPATH)
AR_OPT1	= -r

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

# For Cygwin, $DELDIR_LIB_SUFFIX is simply null.

DELDIR_LIB =	$(DELDIR)/usr/lib$(DELDIR_LIB_SUFFIX)
DELDIR_INC =	$(DELDIR)/usr/include
DELDIR_DOC =	$(DELDIR)/usr/share/doc/Judy
DELDIR_DEMO =	$(DELDIR_DOC)/demo
DELDIR_MAN =	$(DELDIR)/usr/share/man/man3$(MANDIR_SUFFIX)

# Use normal tools for building internal tools (programs):

CCPATH_TOOL = $(CCPATH)

# How to echo with escaped chars expanded; Cygwin /bin/sh = bash requires -e:

ECHO	= echo -e
PWD	= /bin/pwd

# Suffix for $LIB_ID (same as what's passed in $LIB_SUFFIX for a recursive
# make):

LIB_ID_SUFFIX = a

# Linker option to build a shared library:
#
# Note:  The gcc manual entry contains this ominous prose about the -shared
# option:  "Only a few systems support this option."  We'll see...
#
# TBD:  In the long run we should probably append a version number suffix to
# the library and use the ld -soname option (see the manual entry), but I think
# we can accomplish this later (at "version 2").  Also per the howto file (see
# http://www.ibiblio.org/pub/Linux/docs/HOWTO/GCC-HOWTO), it's possible the
# building should be done with gcc instead of ld.

LD_OPT_SL = -shared

# Optionally include PIC libs in tarchives:

TARCHIVE_LIBS_PIC = libs_pic

# Cygwin manual entries are compressed a variety of ways, but bzip2 appears most
# common; they go in suffixless directories regardless of compression, and
# filenames have compression suffixes:

COMPRESSPATH	= gzip
MANDIR_SUFFIX	=
MANFILE_SUFFIX	= .gz

# A weird concession to Win32 VC++ "CL" compiler, which needs -Fo to specify
# the object file and -Fe to specify the executable file:
#
# WARNING!  There must be a blank after the "-o" on the following lines to
# satisfy the /opt/ccover/bin/covc command.

CCoo = -o 
CCoe = -o 

# Ensure $PLATFORM is set:

PLATFORM = cygwin_ia32

# Platform-specific library lists for Cygwin/IA32 for various "lib" targets:

LIB_LIST	= lib32a
LIBS_LIST	= lib32a
LIBS_ALL_LIST	= lib32a lib32so
LIBS_PIC_LIST	= lib32PICa

# Unfortunately there are CC_OPTS differences between both platforms AND
# flavors, so spell them out here by flavor, meaning this file cannot be
# flavor-ignorant:

CC_OPTS_product = -Wall -O2
CC_OPTS_cov	= -Wall -O -DJU_FLAVOR_COV
CC_OPTS_debug	= -Wall -g -DJU_FLAVOR_DEBUG

# Compiler option to build position independent code:

CC_OPTS_PIC	=

# At least on this platform, for make to recognize that a tool is already
# built, the filename + auto-added suffix must be used:

TOOL_SUFFIX = .exe

# Judy is normally built for Cygwin on an i686 (Pentium Pro) system (see uname
# -m output), which requires no special compiler options.  To build for a
# "lesser" Intel system such as i586 (Pentium), consider adding a flag like:
#
#	CC_OPTS = -mcpu=i586
#
# As written above this won't work, there are details to be resolved.
