# @(#) $Revision: 4.24 $ $Source: /judy/make_includes/platform.hpux_ipf.mk $

# Makefile fragment for Judy* for platform "hpux_ipf" (HP-UX on IPF).
# See platform.hpux_pa.mk for more comments.
#
# Note:  To build Judy non-chrooted (as a developer) on an hpux_ipf system with
# native compilers (not cross-compilers), such as mercedes.fc, you must support
# this makefile by creating fake "relocated native PA compilers" symlinks, used
# when building internal tools:
#
#   ! mkdir -p /CLO/BUILD_ENV/Exports
#   ! ln -s /opt/ansic/bin/cc /CLO/BUILD_ENV/Exports/cc
#   ! ln -s /opt/ansic/bin/cc /CLO/BUILD_ENV/Exports/cc64

SHELL	= /usr/bin/ksh
#
# For $CCPATH, carefully point to the correct location (per Dennis Handly,
# 011214) and hope it's on all boxes where developers build, even though
# /usr/bin/cc is typically (but not necessarily) a symlink to it:

CCPATH	= /opt/ansic/bin/cc
LDPATH	= /usr/bin/ld
ARPATH	= /usr/bin/ar
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

# For Linux, $DELDIR_LIB_SUFFIX is simply null.

DELDIR_LIB =	$(DELDIR)/usr/lib$(DELDIR_LIB_SUFFIX)
DELDIR_INC =	$(DELDIR)/usr/include
DELDIR_DOC =	$(DELDIR)/usr/share/doc/Judy
DELDIR_DEMO =	$(DELDIR_DOC)/demo
DELDIR_MAN =	$(DELDIR)/usr/share/man/man3$(MANDIR_SUFFIX)

# Use normal tools for building internal tools (programs):

ECHO	= echo
PWD	= /bin/pwd

# Optionally include PIC libs in tarchives:

TARCHIVE_LIBS_PIC = libs_pic

# Suffix for $LIB_ID (same as what's passed in $LIB_SUFFIX for a recursive
# make):

LIB_ID_SUFFIX = a

# Linker option to build a shared library:

LD_OPT_SL = -b

# On HP-UX, compressed manual entries are compressed with compress(1), go in
# directories whose names end with a suffix, and filenames have no special
# suffix:

COMPRESSPATH	= /usr/bin/compress
MANDIR_SUFFIX	= .Z
MANFILE_SUFFIX	=

# A weird concession to Win32 VC++ "CL" compiler, which needs -Fo to specify
# the object file and -Fe to specify the executable file:

CCoo = -o
CCoe = -o

PLATFORM	= hpux_ipf

LIB_LIST	= lib_hpux_ipf_32a
LIBS_LIST	= lib_hpux_ipf_32a    lib_hpux_ipf_64a
LIBS_ALL_LIST	= lib_hpux_ipf_32a    lib_hpux_ipf_64a \
		  lib_hpux_ipf_32sl   lib_hpux_ipf_64sl
LIBS_PIC_LIST	= lib_hpux_ipf_32PICa lib_hpux_ipf_64PICa

LIB_ID_PATH	= /hpux32

# Note:  +DD32 is the default on IPF and takes less space and time for programs
# that do not need the larger address space:

CC_OPTS_LIB_HPUX_IPF_32 = +DD32 -DJU_32BITS
CC_OPTS_LIB_HPUX_IPF_64 = +DD64 -DJU_64BITS

# Note:  For cov flavor only, ignore warning 67 about unrecognized pragmas
# until/unless covc finds a way to deal with this.  Newer IPF compilers no
# longer send cc -E processing through a separate cpp.ansi, meaning the pragmas
# cause many warnings during the cc -E phase invoked by covc.  Turning off all
# pragma warnings is not ideal, but much simpler than passing every $CC
# invocation through sed to hide them, and preferable to using +legacy_hpc.

CC_OPTS_product = -O -DJU_HPUX_IPF
CC_OPTS_cov	= -O -DJU_HPUX_IPF -DJU_FLAVOR_COV     +W67
CC_OPTS_debug	= -g -DJU_HPUX_IPF -DJU_FLAVOR_DEBUG

# Use special native tools in IPF cross-compile BE for building internal tools
# (programs):
#
# Note:  The only tool actually needed for this purpose is cc.
#
# Note:  At least as of 020319, the Exports/ wrappers are not quite smart
# enough.  To do a 64-bit link requires naming cc64 explicitly, hence
# $CC_TOOL_SUFFIX below.

CCPATH_TOOL = /CLO/BUILD_ENV/Exports/cc$(CC_TOOL_SUFFIX)
