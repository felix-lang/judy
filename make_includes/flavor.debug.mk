# @(#) $Revision: 4.11 $ $Source: /judy/make_includes/flavor.debug.mk $

# Makefile fragment for Judy* for "debug" flavor object files.
# See flavor.product.mk for more comments.

FLAVOR = debug

CC_OPTS =	$(CC_OPTS_LIB) $(CC_OPTS_debug)   -DDEBUG $(EXTCCOPTS)
CC_OPTS_TOOL =	               $(CC_OPTS_product)	  $(EXTCCOPTS)

CC =	   $(CCPRE) $(CCPATH)	   $(CC_OPTS)
CC_NOALL = $(CCPRE) $(CCPATH)	   $(CC_OPTS)
CC_TOOL =	    $(CCPATH_TOOL) $(CC_OPTS_TOOL)

LD =	   $(LDPATH)
