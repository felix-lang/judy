dnl Process this file with autoconf to produce a configure script.
dnl
dnl Comments beginning with 'dnl' will be stripped out by autoconf.
dnl
dnl This file is a shell script containing GNU M4 macros.  All shell
dnl code should be compatible with ancient releases of the Bourne
dnl shell (which is all we can count on being available).  Please try
dnl to avoid Bashisms.  (No such restriction on using GNU M4 features exist,
dnl because GNU M4 is only needed by developers regenerating configure,
dnl and autoconf will self-destruct if run under anything other than GNU M4.)
dnl
dnl When debugging or enhancing this script, you may find it helpful to
dnl know all of the #if conditionals used in the source.  For a list,
dnl type:
dnl
dnl   ifnames `find . -name \*.h -o -name \*.c`
dnl
dnl For more information on autoconf, please type 'info autoconf'.


#==========================================================================
# Initialization
#==========================================================================
# Autoconf tests for the presence or absence of individual system features,
# and sets up Makefile directives and preprocessor macros appropriately.
#
# Note that we don't use all the information discovered by autoconf.
# Some of the tests in this file were inserted automatically by autoscan,
# and the Judy source code doesn't pay attention to the results yet.

# The arguments to AC_INIT are the package name, the package version, and
# the e-mail address for reporting bugs.
AC_INIT(judy, 0.0.4, doug@sourcejudy.com)

# Autoconf will look for tool/jhton.c before running, to make sure it
# has been invoked in the right directory.
AC_CONFIG_SRCDIR([tool/jhton.c])

# Tell the autotools to keep their support scripts in tool/ instead of
# in the top-level source directory.
AC_CONFIG_AUX_DIR([tool])

# Turn on automake, and pass it the PACKAGE_NAME and PACKAGE_VERSION, too.
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

# Tell autoconf we want to keep our preprocessor defines in a header named
# config.h.  This keeps automake from passing a zillion -D directives to
# the C compiler.
AM_CONFIG_HEADER([config.h])


#==========================================================================
# WARNING - WARNING - Shared Library Versioning - WARNING - WARNING
#==========================================================================
# This is the most dangerous part of this file--making a mistake here can
# cause massively painful chaos for libJudy developers, and potentially
# even end users.  So PLEASE pay attention, and read up on the theory of
# shared library versioning.  Tens of thousands of Linux users (and several
# QA departments) may thank you someday.
#
# There are two major concerns:
#
#   1) When changing the libJudy ABI (application binary interface),
#      VERSION_INFO *must* be updated according to libtool's rules.  Failure
#      to do this will make applications using libJudy dump core, typically
#      under obscure conditions on user systems.  I won't attempt to
#      explain these rules here; please see 'info libtool' for details.
#
#   2) When changing the libJudy ABI, it is also desirable to make libJudy
#      "parallel installable".  This means that it should be possible to
#      install development headers and libraries for more than one version
#      of libJudy at once.  Failure to do this may cause problems for
#      Linux distributions which include libJudy.  (For example, it's
#      impossible to switch between libpng2-dev and libpng3-dev on a
#      Debian system without uninstalling and reinstalling both the Gnome
#      and KDE SDKs.)  For more information, do a Google search for
#      "parallel installable".
#
# Right now, this package only provides the mechanisms to handle concern
# (1).  Concern (2) is slightly more complicated, and will require some
# careful thinking.  Fortunately, concern (2) doesn't become important
# until other SDKs rely on the libJudy SDK.
#
# Of course, it's safe to avoid changing the libJudy ABI. :-)

VERSION_INFO="-version-info 0:0:0"
AC_SUBST(VERSION_INFO)


#==========================================================================
# Flavors
#==========================================================================
# Judy can be compiled in one of three flavors: "product" (the default),
# "debug", or "cov".  We allow the user to select flavors using
# --enable-debug and --enable-ccover arguments to automake, which is
# the typical way of doing things.
# 
# Note how we perform string comparison:
#
#   if test "x$enable_debug" = xyes; then
#
# We do several odd things here:
#
#   1) We use 'test' instead of '[ ]' for shell portability.
#   2) We prefix strings with 'x' when comparing them, to protect against
#      empty strings.
#   3) We ALWAYS quote user-supplied shell variables, to protect against
#      embedded spaces.
#
# The results of this test aren't used anywhere yet.

# Keep the user entertained.
AC_MSG_CHECKING(which flavor to build)

# Process our --enable-debug argument.
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [enable debugging features]),
              , enable_debug=no)
if test "x$enable_debug" != xyes -a "x$enable_debug" != xno; then
   AC_MSG_ERROR(You may not pass an argument to --enable-debug)
fi

# Process our --enable-ccover argument.
AC_ARG_ENABLE(ccover,
              AC_HELP_STRING([--enable-ccover],
                             [enable use of ccover code coverage tools]),
              , enable_ccover=no)
if test "x$enable_ccover" != xyes -a "x$enable_ccover" != xno; then
   AC_MSG_ERROR(You may not pass an argument to --enable-ccover)
fi

# Determine our flavor.
if test "x$enable_debug" = xyes -a "x$enable_ccover" = xyes; then
   AC_MSG_ERROR(You may not use --enable-debug and --enable-ccover together)
elif test "x$enable_debug" = xyes; then
   FLAVOR=debug
elif test "x$enable_ccover" = xyes; then
   FLAVOR=cov
else
   FLAVOR=product
fi

# Define FLAVOR in our makefiles.
AC_SUBST(FLAVOR)

# Tell the user what flavor we've decided to build.
AC_MSG_RESULT($FLAVOR)


#==========================================================================
# Checks for Programs
#==========================================================================

AC_PROG_CC
AC_PROG_LIBTOOL
AC_CHECK_TOOL([STRIP],[strip])


#==========================================================================
# Checks for Libraries
#==========================================================================


#==========================================================================
# Checks for Header Files
#==========================================================================

# Do the simple, standard header checks.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h malloc.h stddef.h stdlib.h string.h strings.h sys/param.h sys/time.h unistd.h values.h])

# These two are more complicated, because we need to prepare for running
# ifundef over Judy.h before installing it.  (Notice that there's no
# semi-colon after AC_DEFINE; see 'info autoconf' for an explanation.)
AC_CHECK_HEADER(inttypes.h,
[AC_DEFINE(HAVE_INTTYPES_H) D_HAVE_INTTYPES_H=-DHAVE_INTTYPES_H],
[D_HAVE_INTTYPES_H=-UHAVE_INTTYPES_H])
AC_SUBST(D_HAVE_INTTYPES_H)
AC_CHECK_HEADER(stdint.h,
[AC_DEFINE(HAVE_STDINT_H) D_HAVE_STDINT_H=-DHAVE_STDINT_H],
[D_HAVE_STDINT_H=-UHAVE_STDINT_H])
AC_SUBST(D_HAVE_STDINT_H)


#==========================================================================
# Checks for Typedefs, Structures, and Compiler Characteristics
#==========================================================================

# Standard, boring stuff.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_TYPES([ptrdiff_t])

# If we're compiling for a little-endian system, define JU_LITTLE_ENDIAN.
# If we can't tell what kind of system we're compling for, alert the
# user as described in 'info autoconf'.
AC_C_BIGENDIAN(, AC_DEFINE(JU_LITTLE_ENDIAN, 1,
			   [Define to 1 on little-endian systems.]))

# Determine whether or not we're compiling for a 64-bit system by looking
# at the size of a 'long'.  This will define SIZEOF_LONG in config.h.  We
# append some text to the bottom of config.h to set JU_64BIT appropriately.
AC_CHECK_SIZEOF(long)
AH_BOTTOM([/* Define JU_64BIT to 1 if we're on a 64-bit system. */
#if SIZEOF_LONG == 8
# define JU_64BIT 1
#endif])


#==========================================================================
# Checks for Library Functions
#==========================================================================

AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([getpagesize gettimeofday memset munmap pow strchr strcspn strerror strstr strtoul])


#==========================================================================
# Platform-Specific Actions
#==========================================================================
# Autoconf is perfectly capable of examining the platform name and setting
# up some manual parameters, but this is discouraged.  Looking at the
# platform name is fragile; testing for individual features is robust.
#
# However, Judy's old configuration system used platform names, so I've
# included this section to ease the transition.
#
# Note that automake supports cross-compilation: the "build" OS runs the
# compilers, and the "host" OS will run Judy.  This won't actually work for
# Judy (yet), because unifdef and Judy?TableGen need to be run on the build
# machine.  But it's better to get things correct here, anyway.

# Determine the system we're compiling on.  Sets $build{,_cpu,_vendor,_os}.
# AC_CANONICAL_BUILD - Already run by libtool.

# Determine the system we're compiling for.  Sets $host{,_cpu,_vendor,_os}.
# AC_CANONICAL_HOST - Already run by libtool.


#==========================================================================
# Post-Processing of Man Pages
#==========================================================================
# Figure out how to process manual pages after installation.  See
# doc/ext/Makefile.am for the other half of this logic.  Note that
# AM_CONDITIONAL must appear at the top-level of this file.

# Should we try to get fancy, or should we leave automake alone?
AC_MSG_CHECKING(whether to munge manual pages)
AC_ARG_ENABLE(manpage-munging,
              AC_HELP_STRING([--disable-manpage-munging],
                             [don't hardlink or compress manual pages]),
              , enable_manpage_munging=yes)
if test "x$enable_manpage_munging" != xyes -a "x$enable_manpage_munging" != xno
then
   AC_MSG_ERROR(You may not pass an argument to --enable-manpage-munging)
fi
AM_CONDITIONAL(POST_PROCESS_MAN_PAGES, test "x$enable_manpage_munging" = xyes)
AC_MSG_RESULT($enable_manpage_munging)

# Do we know how to compress man pages on this platform?
AC_MSG_CHECKING(whether to compress manual pages)
COMPRESS_MAN=no
COMPRESS_PATH=
MANDIR_SUFFIX=
MANFILE_SUFFIX=
case $host_os in
linux*)
        COMPRESS_MAN=yes
	COMPRESS_PATH=gzip
	MANFILE_SUFFIX=.gz
	;;
hpux*)
	COMPRESS_MAN=yes
	COMPRESS_PATH=/usr/bin/compress
	MANDIR_SUFFIX=.Z
	;;
esac
AM_CONDITIONAL(COMPRESS_MAN_PAGES, test x$COMPRESS_MAN = xyes)
AC_SUBST(COMPRESS_PATH)
AC_SUBST(MANDIR_SUFFIX)
AC_SUBST(MANFILE_SUFFIX)
if test "x$enable_manpage_munging" = xyes; then
   AC_MSG_RESULT($COMPRESS_MAN)
else
   AC_MSG_RESULT(no)
fi


#==========================================================================
# Setting Up the Build Environment
#==========================================================================

AC_CONFIG_FILES([
Makefile
src/Makefile
src/JudyCommon/Makefile
src/Judy1/Makefile
src/JudyL/Makefile
src/JudySL/Makefile
src/apps/Makefile
src/apps/demo/Makefile
tool/Makefile
doc/Makefile
doc/ext/Makefile
doc/int/Makefile
test/Makefile
test/manual/Makefile
make_includes/Makefile
])

AC_OUTPUT
