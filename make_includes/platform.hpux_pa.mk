# @(#) $Revision: 4.21 $ $Source: /judy/make_includes/platform.hpux_pa.mk $

# Makefile fragment for Judy* for platform "hpux_pa" (HP-UX on PA-RISC).

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

# Ensure $PLATFORM is set:

PLATFORM = hpux_pa

# Platform-specific library lists for HP-UX/PA for various "lib" targets:

LIB_LIST	= lib11_32a
LIBS_LIST	= lib11_32a    lib20_64a
LIBS_ALL_LIST	= lib11_32a    lib20_64a lib11_32sl lib20_32sl lib20_64sl
LIBS_PIC_LIST	= lib20_32PICa lib20_64PICa

# Platform-specific $CC options for different kinds of libraries:

CC_OPTS_LIB11_32 = +DA1.1 -DJU_32BITS
CC_OPTS_LIB20_32 = +DA2.0 -DJU_32BITS
CC_OPTS_LIB20_64 = +DD64  -DJU_64BITS

# Unfortunately there are CC_OPTS differences between both platforms AND
# flavors, so spell them out here by flavor, meaning this file cannot be
# flavor-ignorant:

CC_OPTS_product = -Wall -O2
CC_OPTS_cov	= -Wall -O -DJU_FLAVOR_COV
CC_OPTS_debug	= -Wall -g -DJU_FLAVOR_DEBUG

# When compiling a generic (once-only, not per-library) constructed tool
# (program) on a PA 2.0 box, a warning is issued unless this option is given:
#
# (On other platforms the macro is null.)

CC_OPTS_TOOL_GEN = +DA1.1

# Use normal tools for building internal tools (programs):
#
# Note:  The only tool actually needed for this purpose is cc.

CCPATH_TOOL = $(CCPATH)

# Waiver (hide) the following HP-UX-only, PIC-only warning from makelog by
# indenting it in cc output:
#
# On HP-UX, we do want to use +O3, although currently we do not (see
# make_includes/flavor.product.mk 4.1, although there's not much explanation in
# the delta logs, but see also later versions of that file), so keep this
# supporting code around although $PIC_SED is not used; and note, no one should
# redefine a Judy routine.  Note the tab in the sed script.

PIC_WARNING =	cc: warning 8006: Do not use optimization levels higher than 2 to generate a shared library if a user of that library may redefine a routine within that library (8006)

PIC_SED_VALUE =	| sed '/^$(PIC_WARNING)$$/s/^/	/'
