# @(#) $Revision: 4.11 $ $Source: /judy/make_includes/flavor.product.mk $

# Makefile fragment for Judy* for "product" flavor object files.

# Ensure $FLAVOR is set:

FLAVOR = product

# $CCPRE is a means to externally pass in a command to accept and modify the
# entire compilation command line.
#
# $CC_OPTS_NOALL is a special case to support building sources that do not
# behave correctly with +Oall...  Specifically, JudyMalloc.c has problems with
# the pointer arithmetic that (only) it does, when large memory spaces cross
# into the third or fourth memory quadrant.  Doug thinks this is a bug in the
# compiler, but we have to work around it indefinitely.  For now, compile
# without inlining; see makefile 4.7.  For now, all compilations are not +Oall,
# to match +O2.  See 4.0 of this file.
#
# $CC_OPTS_TOOL is a special set for building Judy-specific tools (programs)
# needed to build other Judy components.

CC_OPTS =	$(CC_OPTS_LIB) $(CC_OPTS_product) $(EXTCCOPTS)
CC_OPTS_NOALL =	$(CC_OPTS_LIB) $(CC_OPTS_product) $(EXTCCOPTS)
CC_OPTS_TOOL =	                                  $(EXTCCOPTS)

CC =		$(CCPRE) $(CCPATH)	$(CC_OPTS)
CC_NOALL =	$(CCPRE) $(CCPATH)	$(CC_OPTS_NOALL)
CC_TOOL =		 $(CCPATH_TOOL)	$(CC_OPTS_TOOL)

LD =		$(LDPATH)
