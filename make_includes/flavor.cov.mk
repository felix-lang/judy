# @(#) $Revision: 4.18 $ $Source: /judy/make_includes/flavor.cov.mk $

# Makefile fragment for Judy* for "cov" flavor object files.  This is intended
# to be as close to "product" as possible while building with ccover.  See
# flavor.product.mk for more comments.

FLAVOR = cov

# $COVC is a prefix to $CC or $ACC.  Use the full path, rather than relying on
# $PATH, just as for other tools, so there's no ambiguity about what was run,
# even in logfiles.
#
# Explicitly pass $COVFILE so it's unambiguous, but build using a local (top of
# source tree) file so source file paths are shorter.  WARNING!  This means the
# top test.cov file is not flavor-specific.  Be careful to remove it and the
# corresponding build tree, if necessary, and rebuild all from scratch, so the
# $COVFILE has exactly what you want, no more, no less.
#
# Note:  Use default covc, which does not measure trinary statements involving
# strings (?).  See the -C option to covc.
#
# Note:  The covc command automatically sets -DCCOVER.

COVFILE =	test.cov_$(LIB_SUFFIX)
COVFILE_DEL =	$(DELDIR_LIB)/$(COVFILE)

# Set target name so covfiles are constructed:

COV_RECURSE_PREP =	$(COV_RECURSE_PREP_VALUE)
COV_RECURSE_FINISH =	$(COV_RECURSE_FINISH_VALUE)

COVC =		/opt/ccover/bin/covc -f"$(COVFILE)" --no-banner

CC_OPTS =	$(CC_OPTS_LIB) $(CC_OPTS_cov)	  $(EXTCCOPTS)
CC_OPTS_NOALL =	$(CC_OPTS_LIB) $(CC_OPTS_cov)	  $(EXTCCOPTS)
CC_OPTS_TOOL =	               $(CC_OPTS_product) $(EXTCCOPTS)

# For cov flavor, force $CCPRE to reference the ccpre tool, so cov builds use
# preprocessed (separated) *.c files, because otherwise C-Cover doesn't know to
# count the common code separately in multi-compiled functions.

CCPRE =		$(SRCDIR_TOOL)/ccpre

CC =		$(CCPRE) $(COVC) $(CCPATH)	$(CC_OPTS)
CC_NOALL =	$(CCPRE) $(COVC) $(CCPATH)	$(CC_OPTS_NOALL)
CC_TOOL =			 $(CCPATH_TOOL)	$(CC_OPTS_TOOL)

LD =		$(LDPATH)
