// Copyright (C) 2000 - 2002 Hewlett-Packard Company
//
// This program is free software; you can redistribute it and/or modify it
// under the term of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// _________________

// @(#) $Revision: 4.38 $ $Source: /judy/src/JudySL/JudySL.c $
//
// JUDY FUNCTIONS FOR STRING INDEXES, where associated values are longs.  One
// JudySL*() corresponds to each JudyNL or JudyL*() function (with exceptions).
//
// See the manual entry for details.
//
// Keep this code consistent with JudyNL.c.
//
// METHOD:  Break up each null-terminated Index (string) into chunks of W
// bytes, where W is the machine's word size, with null-padding in the last
// word if necessary.  Store strings as a tree of JudyL arrays, that is, array
// of array of array...  where each level consumes W bytes (one word) as an
// index to the JudyL array at that level.  Since strings can begin on
// arbitrary byte boundaries, copy each chunk of W bytes from Index into a
// word-aligned object before using it as a Judy index.
//
// The JudySL tree also supports "single-index shortcut leaves".  A simple
// JudySL array (tree of JudyL arrays) would go as many levels deep as the
// Index (string) is long, which wastes time and memory when an Index is unique
// beyond a certain point.  When there's just one Index under a pointer, given
// a reliable way to tell that the pointer is not a root pointer to another
// JudyL array, it should save a lot of time to instead point to a "leaf"
// object, similar to leaves in JudyL arrays.
//
// TBD:  Multi-index leaves, like those in JudyL, are also worth considering,
// but their payback for JudySL is less certain.  Likewise, shortcut branches
// are worth considering too.
//
// This code uses the Judy.h definitions and Doug Baskins' convention of a "P"
// prefix for pointers, except no "P" for the first level of char * (strings).


// IMPORTS:

#ifdef JU_FLAVOR_COV
#pragma C-Cover off	// exclude inline functions in public header files.
#endif
#include <string.h>
#include <stdlib.h>	// for malloc().
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#include "JudySL.h"

// __BSWAP* or equivalent can be defined for little-endian machines, but this
// is not mandatory:

#ifdef JU_LINUX
#ifdef JU_FLAVOR_COV
#pragma C-Cover off	// exclude inline functions in public header files.
#endif
#include <byteswap.h>	// for bswap_32/64.
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#define __BSWAP32(WORD) bswap_32(WORD)
#define __BSWAP64(WORD) bswap_64(WORD)
#endif

#ifdef JU_WIN
// To fix error below without knowing the answer, just remove line below (dlb):

#define __BSWAP32(WORD)  (TBD: Do not know what to put here yet.)
#endif


// Note:  Use JAP_INVALID, which is non-zero, to mark pointers to shortcut
// leaves, so a null pointer (empty array) does not appear to be a Pscl.  See
// below about SCLs.

#define	IS_PSCL(Pscl)    ((((Word_t) (Pscl)) &   JAP_MASK) == JAP_INVALID)
#define	CLEAR_PSCL(Pscl)  (((Word_t) (Pscl)) & (~JAP_MASK))
#define	SET_PSCL(Pscl)   (CLEAR_PSCL(Pscl) | JAP_INVALID)


// MISCELLANEOUS GLOBALS:

#define	PCNULL	((char *)   NULL)
#define	PVNULL	((Pvoid_t)  NULL)
#define	PPVNULL	((PPvoid_t) NULL)

// Get the Index (string) length in bytes, including the trailing '\0', which
// is an integral part of the string:
//
// Because strlen() should be optimized, and at least on HP-UX it's written in
// assembly code to use some instructions not available through the compiler,
// it's faster to precompute the Index length in bytes this way than to watch
// for a '\0' in each word.

#define	INDEXLEN(Index)  (strlen(Index) + 1)

// SUPPORT FOR HANDLING WORDS:
//
// In WORDS(), trust the compiler to issue efficient instructions when WORDSIZE
// is a constant power of 2, as it should be.

#define	WORDSIZE     (sizeof (Word_t))		// bytes in word = JudyL index.
#define	WORDS(Bytes) (((Bytes) + WORDSIZE - 1) / WORDSIZE)	// round up.

// A string is "in the last word" if a previously-set byte count is at or below
// the system word size, or in some cases if the last byte in the (null-padded)
// word is null (assume big-endian, including in a register on a little-endian
// machine):

#define	LASTWORD_BY_LEN(len)	((len) <= WORDSIZE)
#define	LASTWORD_BY_VALUE(word) (! ((word) & 0xffL))

// Macros for copying up to sizeof(Word_t) bytes between Word_t and string.
// Notice a byte of '\0' means terminate copy to a word and zero fill the rest,
// and a copy to a string terminates with a zero byte in a word.  (The zero
// byte is included in the copy.)  Also notice it does not matter whether or
// not the string pointer is aligned to a word boundary.

typedef union
{
	uint8_t	cw_uint8[sizeof(Word_t)];
	Word_t  cw_word;
} Jcw_t;


#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Who invented little-endian anyhow?

#ifndef JU_64BIT

#define	C0 3
#define	C1 2
#define	C2 1
#define	C3 0

#else // JU_64_BIT

#define	C0 7
#define	C1 6
#define	C2 5
#define	C3 4
#define	C4 3
#define	C5 2
#define	C6 1
#define	C7 0

#endif // JU_64_BIT
#endif // JU_LITTLE_ENDIAN


#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Byte swapping is not needed for big-endian machines:

#define	__BSWAP32(Word) (Word)
#ifdef JU_64BIT
#define	__BSWAP64(Word) (Word)
#endif

#define	C0 0
#define	C1 1
#define	C2 2
#define	C3 3
#define	C4 4
#define	C5 5
#define	C6 6
#define	C7 7

#endif // JU_BIG_ENDIAN


#if (defined(__BSWAP32) || defined(__BSWAP64))

// Trust the compiler to optimize "%" to a mask operation when WORDSIZE is a
// constant power of 2, as it should be:

#define	WORDALIGNED(POBJ)  (! (((Word_t) (POBJ)) % WORDSIZE))
#else
#define	WORDALIGNED(POBJ)  0		// avoid "if" clause code.
#endif


#ifndef JU_64BIT

// Test if Word has any of the 4 bytes == '\0':
//
// This arcane code takes advantage of the CPU having a fast barrel shifter and
// a carry bit.  Doug stole this code from elsewhere, and we know it works, but
// we really can't explain why.

#define	NOZEROBYTE(Word) \
	((((((Word) - 0x01010101) | (Word)) ^ (Word)) & 0x80808080) ? 0 : 1)

#define	COPYSTRINGtoWORD(Word,Str)			        \
    {							        \
	if (WORDALIGNED(Str) && NOZEROBYTE(*((PWord_t) (Str))))\
	{							\
	    (Word) = __BSWAP32(*((PWord_t) (Str)));		\
	}							\
	else							\
	{							\
	    Jcw_t JCW;					        \
	    JCW.cw_word = 0;				        \
	    do {						\
	        if (! (JCW.cw_uint8[C0] = (Str)[0])) break;	\
	        if (! (JCW.cw_uint8[C1] = (Str)[1])) break;	\
	        if (! (JCW.cw_uint8[C2] = (Str)[2])) break;	\
		       JCW.cw_uint8[C3] = (Str)[3];             \
	    } while(0);					        \
	    (Word) = JCW.cw_word;				\
        }                                                       \
    }

#define	COPYWORDtoSTRING(Str,Word)				\
    {								\
	if ((! (LASTWORD_BY_VALUE(Word))) && WORDALIGNED(Str))	\
	{							\
	    *((PWord_t) (Str)) = __BSWAP32(Word);		\
	}							\
	else							\
	{							\
	    Jcw_t JCW;						\
	    JCW.cw_word = (Word);				\
	    do {						\
		if (! ((Str)[0] = JCW.cw_uint8[C0])) break;	\
		if (! ((Str)[1] = JCW.cw_uint8[C1])) break;	\
		if (! ((Str)[2] = JCW.cw_uint8[C2])) break;	\
		       (Str)[3] = JCW.cw_uint8[C3];		\
	    } while(0);						\
	}							\
    }

#else // JU_64_BIT

#define	NOZEROBYTE(Word)				        \
	((((((Word) - 0x0101010101010101) | (Word)) ^ (Word))   \
	& 0x8080808080808080) ? 0 : 1)

#define	COPYSTRINGtoWORD(Word,Str)			        \
    {								\
	if (WORDALIGNED(Str) && NOZEROBYTE(*((PWord_t) (Str))))	\
	{							\
	    (Word) = __BSWAP64(*((PWord_t) (Str)));		\
	}							\
	else							\
	{							\
	    Jcw_t JCW;						\
	    JCW.cw_word = 0;					\
	    do {						\
		if (! (JCW.cw_uint8[C0] = (Str)[0])) break;	\
		if (! (JCW.cw_uint8[C1] = (Str)[1])) break;	\
		if (! (JCW.cw_uint8[C2] = (Str)[2])) break;	\
		if (! (JCW.cw_uint8[C3] = (Str)[3])) break;	\
		if (! (JCW.cw_uint8[C4] = (Str)[4])) break;	\
		if (! (JCW.cw_uint8[C5] = (Str)[5])) break;	\
		if (! (JCW.cw_uint8[C6] = (Str)[6])) break;	\
		       JCW.cw_uint8[C7] = (Str)[7];		\
	    } while(0);						\
	    (Word) = JCW.cw_word;				\
	}							\
    }

#define	COPYWORDtoSTRING(Str,Word)				\
    {								\
	if ((! (LASTWORD_BY_VALUE(Word))) && WORDALIGNED(Str))	\
	{							\
	    *((PWord_t) (Str)) = __BSWAP64(Word);		\
	}							\
	else							\
	{							\
	    Jcw_t JCW;						\
	    JCW.cw_word = (Word);				\
	    do {						\
		if (! ((Str)[0] = JCW.cw_uint8[C0])) break;	\
		if (! ((Str)[1] = JCW.cw_uint8[C1])) break;	\
		if (! ((Str)[2] = JCW.cw_uint8[C2])) break;	\
		if (! ((Str)[3] = JCW.cw_uint8[C3])) break;	\
		if (! ((Str)[4] = JCW.cw_uint8[C4])) break;	\
		if (! ((Str)[5] = JCW.cw_uint8[C5])) break;	\
		if (! ((Str)[6] = JCW.cw_uint8[C6])) break;	\
		       (Str)[7] = JCW.cw_uint8[C7];		\
	    } while(0);						\
	}							\
    }

#endif // JU_64_BIT


// SUPPORT FOR SINGLE-INDEX SHORTCUT LEAVES:

typedef struct scl {
	Pvoid_t	scl_Pvalue;	// caller's value area.
	char	scl_index[1];	// base of undecoded remainder of Index string.
} * Pscl_t;

#define	PSCLNULL ((Pscl_t) NULL)

// How big to malloc a shortcut leaf; stringlen should already include the
// trailing null char:

#define	SCLSIZE(stringlen) ((size_t) (sizeof(Pvoid_t) + (stringlen)))

// Index and value area for a shortcut leaf, depending on how it matches the
// undecoded remainder of the Index, given a Pscl_t that includes type bits
// that must be cleared:
//
// PSCLINDEX() and PSCLVALUE() are also useful when Pscl contains uncleared
// TYPE bits.
//
// Note:  SCLCMP() cannot take advantage of knowing the Index length because
// the scl_index length is not pre-known when these macros are used.

#define	PSCLINDEX(Pscl)  (((Pscl_t) CLEAR_PSCL(Pscl))->scl_index)
#define	PSCLVALUE(Pscl)  (((Pscl_t) CLEAR_PSCL(Pscl))->scl_Pvalue)

#define	SCLCMP(Index,Pscl) strcmp(Index, PSCLINDEX(Pscl))

#define	PPSCLVALUE_EQ(Index,Pscl) \
		((SCLCMP(Index, Pscl) == 0) ? & PSCLVALUE(Pscl) : PPVNULL)
#define	PPSCLVALUE_LT(Index,Pscl) \
		((SCLCMP(Index, Pscl) <  0) ? & PSCLVALUE(Pscl) : PPVNULL)
#define	PPSCLVALUE_GT(Index,Pscl) \
		((SCLCMP(Index, Pscl) >  0) ? & PSCLVALUE(Pscl) : PPVNULL)

// Common in-lined code to append or free a shortcut leaf:
//
// See header comments about premature return().  Note that malloc() does not
// pre-zero the memory, so ensure scl_Pvalue is zeroed, just like a value area
// in a JudyL array.  Hope strcpy() is fast enough in this context.

#define	APPEND_SCL(Pscl,PPArray,Index,len)		\
	{						\
	    size_t bytes = SCLSIZE(len);		\
							\
	    if (((Pscl) = (Pscl_t) malloc(bytes)) == PSCLNULL) \
	    {					        \
	        JU_SET_ERRNO(PJError, JU_ERRNO_NOMEM);  \
	        return(PPJERR);			        \
	    }						\
	    *(PPArray) = (Pvoid_t) SET_PSCL(Pscl);	\
	    (Pscl->scl_Pvalue) = PVNULL;		\
	    (void) strcpy((Pscl)->scl_index, Index);	\
	}

#define	FREE_SCL(Pscl) { free((void *) (Pscl)); }


// "FORWARD" DECLARATIONS:

static void	JudySLModifyErrno(PJError_t PJError,
				  Pcvoid_t PArray, Pcvoid_t PArrayOrig);
static int	JudySLDelSub( PPvoid_t PPArray, PPvoid_t PPArrayOrig,
			const char * Index, Word_t len, PJError_t PJError);
static PPvoid_t JudySLPrevSub(Pcvoid_t  PArray, char * Index, bool_t original,
			Word_t len, PJError_t PJError);
static PPvoid_t JudySLNextSub(Pcvoid_t  PArray, char * Index, bool_t original,
			Word_t len, PJError_t PJError);


// ****************************************************************************
// J U D Y   S L   M O D I F Y   E R R N O
//
// Common code for error translation:  When a caller passes an invalid JAP
// ("not a JudyL pointer"), OR if the JudySL array is corrupted at a lower
// level, various JudyL*() calls return JU_ERRNO_NOTJUDYL.  If the caller wants
// detailed error info, convert this particular error to JU_ERRNO_NOTJUDYSL if
// at the top of the tree, otherwise convert it to JU_ERRNO_CORRUPT, meaning
// there was a corruption (the only one even detectable outside JudyL) in the
// JudySL tree; but pass through any other errors unaltered.

FUNCTION static void JudySLModifyErrno(
	PJError_t PJError,		// to modify if non-null.
	Pcvoid_t  PArray,		// current JudyL array.
	Pcvoid_t  PArrayOrig)		// top-of-tree JudyL array.
{
	if ((PJError != PJE0)			      // error info is desired.
	 && (JU_ERRNO(PJError) == JU_ERRNO_NOTJUDYL)) // map this Judy errno.
	{
	    if (PArray == PArrayOrig)			// caller's fault.
		JU_SET_ERRNO_NONNULL(PJError, JU_ERRNO_NOTJUDYSL)
	    else					// lower level.
		JU_SET_ERRNO_NONNULL(PJError, JU_ERRNO_CORRUPT)
	}

} // JudySLModifyErrno()


// ****************************************************************************
// J U D Y   S L   G E T
//
// See comments in file header and below.

FUNCTION PPvoid_t JudySLGet(
	Pcvoid_t     PArray,
	const char * Index,
	PJError_t    PJError)	// optional, for returning error info.
{
	Pcvoid_t     PArrayOrig = PArray;	// for error reporting.
	const char * pos = Index;		// place in Index.
	Word_t	     indexword;			// buffer for aligned copy.
	PPvoid_t     PPvalue;			// from JudyL array.


// CHECK FOR CALLER ERROR (NULL POINTER):

	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}


// SEARCH NEXT LEVEL JUDYL ARRAY IN TREE:
//
// Use or copy each word from the Index string and check for it in the next
// level JudyL array in the array tree, but first watch for shortcut leaves.
// Upon invalid Index or end of Index (string) in current word, return.

	while (TRUE)				// until return.
	{
	    if (IS_PSCL(PArray))		// a shortcut leaf.
		return(PPSCLVALUE_EQ(pos, PArray));

	    COPYSTRINGtoWORD(indexword, pos);	// copy next 4[8] bytes.

// Use JLG() for speed, at the cost of defining a local JUDYERROR():

#undef JUDYERROR
#define	JUDYERROR(ignore1, ignore2, ignore3, JudyErrno, JudyErrID)	\
	    if (PPvalue == PPJERR)					\
	    {								\
		JU_ERRNO(PJError) = (JudyErrno);			\
		JU_ERRID(PJError) = (JudyErrID);			\
		JudySLModifyErrno(PJError, PArray, PArrayOrig);		\
		return(PPJERR);						\
	    }

	    JLG(PPvalue, PArray, indexword);

	    if ((PPvalue == PPVNULL) || LASTWORD_BY_VALUE(indexword))
		return(PPvalue);


// CONTINUE TO NEXT LEVEL DOWN JUDYL ARRAY TREE:
//
// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated here as a dead-end (not a core
// dump or assertion; see version 1.25).

	    pos	  += WORDSIZE;
	    PArray = *PPvalue;		// each value -> next array.

	} // while

	/*NOTREACHED*/

} // JudySLGet()


// ****************************************************************************
// J U D Y   S L   I N S
//
// See also the comments in JudySLGet(), which is somewhat similar, though
// simpler.
//
// Theory of operation:
//
// Upon encountering a null pointer in the tree of JudyL arrays, insert a
// shortcut leaf -- including directly under a null root pointer for the first
// Index in the JudySL array.
//
// Upon encountering a pre-existing shortcut leaf, if the old Index is equal to
// the new one, return the old value area.  Otherwise, "carry down" the old
// Index until the old and new Indexes diverge, at which point each Index
// either terminates in the last JudyL array or a new shortcut leaf is inserted
// under it for the Index's remainder.
//
// TBD:  Running out of memory below the starting point causes a premature
// return below (in several places) and leaves a dead-end in the JudySL tree.
// Ideally the code here would back this out rather than wasting a little
// memory, but in lieu of that, the get, delete, and search functions
// understand dead-ends and handle them appropriately.

FUNCTION PPvoid_t JudySLIns(
	PPvoid_t     PPArray,
	const char * Index,
	PJError_t    PJError)	// optional, for returning error info.
{
	PPvoid_t     PPArrayOrig = PPArray;	// for error reporting.
	const char * pos  = Index;	// place in Index.
	const char * pos2 = PCNULL;	// for old Index (SCL being moved).
	Word_t	     len;		// bytes remaining.

// Note:  len2 is set when needed and only used when valid, but this is not
// clear to gcc -Wall, so initialize it here to avoid a warning:

	Word_t	     len2 = 0;		// for old Index (SCL being moved).
	Word_t	     indexword;		// buffer for aligned copy.
	Word_t	     indexword2;	// for old Index (SCL being moved).
	PPvoid_t     PPvalue;		// from JudyL array.
	PPvoid_t     PPvalue2;		// for old Index (SCL being moved).
	Pscl_t	     Pscl = PSCLNULL;	// shortcut leaf.
	Pscl_t	     Pscl2;		// for old Index (SCL being moved).


// CHECK FOR CALLER ERROR (NULL POINTER):

	if (PPArray == PPVNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPPARRAY);
	    return(PPJERR);
	}
	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}

	len = INDEXLEN(Index);		// bytes remaining.


// APPEND SHORTCUT LEAF:
//
// If PPArray, which is the root pointer to the first or next JudyL array in
// the tree, points to null (no next JudyL array), AND there is no shortcut
// leaf being carried down, append a shortcut leaf here for the new Index, no
// matter how much of the Index string remains (one or more bytes, including
// the trailing '\0').

	while (TRUE)				// until return.
	{
	    if (*PPArray == PVNULL)		// no next JudyL array.
	    {
		if (Pscl == PSCLNULL)		// no SCL being carried down.
		{
		    APPEND_SCL(Pscl, PPArray, pos, len);  // returns if error.
		    return(&(Pscl->scl_Pvalue));
		}
		// else do nothing here; see below.
	    }


// CARRY DOWN PRE-EXISTING SHORTCUT LEAF:
//
// When PPArray points to a pre-existing shortcut leaf, if its Index is equal
// to the Index to be inserted, meaning no insertion is required, return its
// value area; otherwise, "move it aside" and "carry it down" -- replace it
// (see below) with one or more levels of JudyL arrays.  Moving it aside
// initially just means setting Pscl non-null, both as a flag and for later
// use, and clearing the pointer to the SCL in the JudyL array.

	    else if (IS_PSCL(*PPArray))
	    {
		assert(Pscl == PSCLNULL);		// no nested SCLs.
		Pscl = (Pscl_t) CLEAR_PSCL(*PPArray);
		len2 = INDEXLEN(Pscl->scl_index);

		if ((len == len2) && (strcmp(pos, Pscl->scl_index) == 0))
		    return(&(Pscl->scl_Pvalue));

		*PPArray = PVNULL;		// disconnect SCL.
		pos2 = Pscl->scl_index;		// note: is word-aligned.

		// continue with *PPArray now clear, and Pscl, pos2, len2 set.
	    }


// CHECK IF OLD AND NEW INDEXES DIVERGE IN THE CURRENT INDEX WORD:
//
// If a shortcut leaf is being carried down and its remaining Index chars now
// diverge from the remaining chars of the Index being inserted, that is, if
// the next words of each Index differ, "plug in" the old Index here, in a new
// JudyL array, before proceeding.
//
// Note:  Call JudyLIns() for the SCL Index word before calling it for the new
// Index word, so PPvalue remains correct for the latter.  (JudyLIns() return
// values are not stable across multiple calls.)
//
// Note:  Although pos2 is word-aligned, and a Pscl_t is a whole number of
// words in size, pos2 is not certain to be null-padded through a whole word,
// so copy it first to an index word for later use.
//
// See header comments about premature return().

	    COPYSTRINGtoWORD(indexword, pos);	// copy next 4[8] bytes.

	    if (Pscl != PSCLNULL)
	    {
		COPYSTRINGtoWORD(indexword2, pos2);  // copy next 4[8] bytes.

		if (indexword != indexword2)	// SCL and new Indexes diverge.
		{
		    assert(*PPArray == PVNULL);	 // should be new JudyL array.

// Note:  If JudyLIns() returns JU_ERRNO_NOTJUDYL here, *PPArray should not be
// modified, so JudySLModifyErrno() can do the right thing.

		    if ((PPvalue2 = JudyLIns(PPArray, indexword2, PJError))
			== PPJERR)
		    {
			JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
			return(PPJERR);
		    }

		    assert(PPvalue2 != PPVNULL);

// If the old (SCL) Index terminates here, copy its value directly into the
// JudyL value area; otherwise create a new shortcut leaf for it, under
// *PPvalue2 (skipping the word just inserted), and copy its value to the new
// SCL:

		    if (LASTWORD_BY_LEN(len2))
		    {
			*((PWord_t) PPvalue2) = (Word_t) (Pscl->scl_Pvalue);
		    }
		    else
		    {
			APPEND_SCL(Pscl2, PPvalue2, pos2 + WORDSIZE,
						    len2 - WORDSIZE);
			(Pscl2->scl_Pvalue) = Pscl->scl_Pvalue;
		    }

		    FREE_SCL(Pscl);		// old SCL no longer needed.
		    Pscl = PSCLNULL;
		}
	    }


// APPEND NEXT LEVEL JUDYL ARRAY TO TREE:
//
// If a shortcut leaf was carried down and diverged at this level, the code
// above already appended the new JudyL array, but the next word of the new
// Index still must be inserted in it.
//
// See header comments about premature return().
//
// Note:  If JudyLIns() returns JU_ERRNO_NOTJUDYL here, *PPArray should not be
// modified, so JudySLModifyErrno() can do the right thing.

	    if ((PPvalue = JudyLIns(PPArray, indexword, PJError)) == PPJERR)
	    {
		JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
		return(PPJERR);
	    }

	    assert(PPvalue != PPVNULL);


// CHECK IF NEW INDEX TERMINATES:
//
// Note that if it does, and an old SCL was being carried down, it must have
// diverged by this point, and is already handled.

	    if (LASTWORD_BY_LEN(len))
	    {
		assert(Pscl == PSCLNULL);
		return(PPvalue);	// is value for whole Index string.
	    }

	    pos	 += WORDSIZE;
	    len	 -= WORDSIZE;
	    pos2 += WORDSIZE;			// useless unless Pscl is set.
	    len2 -= WORDSIZE;

	    PPArray = PPvalue;			// each value -> next array.

	} // while.

	/*NOTREACHED*/

} // JudySLIns()


// ****************************************************************************
// J U D Y   S L   D E L
//
// See the comments in JudySLGet(), which is somewhat similar.
//
// Unlike JudySLGet() and JudySLIns(), recurse downward through the tree of
// JudyL arrays to find and delete the given Index, if present, and then on the
// way back up, any of its parent arrays which ends up empty.
//
// TECHNICAL NOTES:
//
// Recursion seems bad, but this allows for an arbitrary-length Index.  Also, a
// more clever iterative solution that used JudyLCount() (see below) would
// still require a function call per tree level, so why not just recurse?
//
// An earlier version (1.20) used a fixed-size stack, which limited the Index
// size.  We were going to replace this with using JudyLCount(), in order to
// note and return to (read this carefully) the highest level JudyL array with
// a count of 1, all of whose descendant JudyL arrays also have a count of 1,
// and delete from that point downwards.  This solution would traverse the
// array tree downward looking to see if the given Index is in the tree, then
// if so, delete layers downwards starting below the last one that contains
// other Indexes than the one being deleted.
//
// TBD:  To save time coding, and to very likely save time overall during
// execution, this function does "lazy deletions", or putting it more nicely,
// it allows "hysteresis" in the JudySL tree, when shortcut leafs are present.
// It only removes the specified Index, and recursively any empty JudyL arrays
// above it, without fully reversing the effects of JudySLIns().  This is
// probably OK because any application that calls JudySLDel() is likely to call
// JudySLIns() again with the same or a neighbor Index.

FUNCTION int JudySLDel (
	PPvoid_t     PPArray,
	const char * Index,
	PJError_t    PJError)	// optional, for returning error info.
{

// Check for caller error (null pointer):

	if (PPArray == PPVNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPPARRAY);
	    return(JERRI);
	}
	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(JERRI);
	}

// Do the deletion:

	return(JudySLDelSub(PPArray, PPArray, Index, INDEXLEN(Index), PJError));

} // JudySLDel()


// ****************************************************************************
// J U D Y   S L   D E L   S U B
//
// This is the "engine" for JudySLDel() that expects aligned and len to already
// be computed (only once).  See the header comments for JudySLDel().

FUNCTION static int JudySLDelSub(
	PPvoid_t     PPArray,		// in which to delete.
	PPvoid_t     PPArrayOrig,	// for error reporting.
	const char * Index,		// to delete.
	Word_t       len,		// bytes remaining.
	PJError_t    PJError)		// optional, for returning error info.
{
	Word_t	     indexword;		// next word to find.
	PPvoid_t     PPvalue;		// from JudyL array.
	int	     retcode;		// from lower-level call.

	assert(PPArray != PPVNULL);
	assert(Index   != PCNULL);


// DELETE SHORTCUT LEAF:
//
// As described above, this can leave an empty JudyL array, or one containing
// only a single other Index word -- which could be, but is not, condensed into
// a higher-level shortcut leaf.  More precisely, at this level it leaves a
// temporary "dead end" in the JudySL tree, similar to when running out of
// memory during JudySLIns(), and this is somewhat cleaned up by higher
// recursions of the same function (see below); but remaining shortcut leaves
// for other Indexes are not coalesced.

	if (IS_PSCL(*PPArray))
	{
	    if (SCLCMP(Index, *PPArray)) return(0);	// incorrect index.

	    FREE_SCL((Pscl_t) CLEAR_PSCL(*PPArray));
	    *PPArray = PVNULL;
	    return(1);		// correct index deleted.
	}


// DELETE LAST INDEX WORD, FROM CURRENT JUDYL ARRAY:
//
// When at the end of the full Index, delete the last word, if present, from
// the current JudyL array, and return the result all the way up.

	COPYSTRINGtoWORD(indexword, Index);	// copy next 4[8] bytes.

	if (LASTWORD_BY_LEN(len))
	{
	    if ((retcode = JudyLDel(PPArray, indexword, PJError)) == JERRI)
	    {
		JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
		return(JERRI);
	    }
	    return(retcode);
	}


// DELETE BELOW NON-LAST INDEX WORD IN CURRENT JUDYL ARRAY:
//
// If a word before the end of the full Index is present in the current JudyL
// array, recurse through its value, which must be a pointer to another JudyL
// array, to continue the deletion at the next level.  Return the JudyLGet()
// return if the Index's current word is not in the JudyL array, or if no
// delete occurs below this level, both of which mean the whole Index is not
// currently valid.
//
// Use JLG() for speed, at the cost of defining a local JUDYERROR():

#undef JUDYERROR
#define	JUDYERROR(ignore1, ignore2, ignore3, JudyErrno, JudyErrID)	\
	if (PPvalue == PPJERR)						\
	{								\
	    JU_ERRNO(PJError) = (JudyErrno);				\
	    JU_ERRID(PJError) = (JudyErrID);				\
	    JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);		\
	    return(JERRI);						\
	}

	JLG(PPvalue, *PPArray, indexword);

	if (PPvalue == PPVNULL) return(0);	// Index not in JudySL array.

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25).

	if ((retcode = JudySLDelSub(PPvalue, PPArrayOrig, Index + WORDSIZE,
			   len - WORDSIZE, PJError)) != 1)
	{
	    return(retcode);	// no lower-level delete, or error.
	}


// DELETE EMPTY JUDYL ARRAY:
//
// A delete occurred below in the tree.  If the child JudyL array became empty,
// delete the current Index word from the current JudyL array, which could
// empty the current array and null out *PPArray in turn (or pass through an
// error).  Otherwise simply indicate that a deletion did occur.

	if (*PPvalue == PVNULL)
	{
	    if ((retcode = JudyLDel(PPArray, indexword, PJError)) == JERRI)
	    {
		JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
		return(JERRI);
	    }

	    return(retcode);
	}

	return(1);

} // JudySLDelSub()


// ****************************************************************************
// J U D Y   S L   P R E V
//
// Recursively traverse the JudySL tree downward using JudyLGet() to look for
// each successive index word from Index in the JudyL array at each level.  At
// the last level for the Index (LASTWORD_BY_LEN()), use JudyLPrev() instead of
// JudyLGet(), to exclude the initial Index.  If this doesn't result in finding
// a previous Index, work back up the tree using JudyLPrev() at each higher
// level to search for a previous index word.  Upon finding a previous index
// word, descend again if/as necessary, this time inclusively, to find and
// return the full previous Index.
//
// Also support shortcut leaves.
//
// Since this function is recursive and it also needs to know if it's still
// looking for the original Index (to exclude it at the LASTWORD_BY_LEN()
// level) or for the remaining words of the previous Index (inclusive),
// actually call a subroutine that takes an additional parameter.
//
// See also the technical notes in JudySLDel() regarding the use of recursion
// rather than iteration.

FUNCTION PPvoid_t JudySLPrev(
	Pcvoid_t  PArray,
	char *	  Index,
	PJError_t PJError)	// optional, for returning error info.
{
// Check for caller error (null pointer), or empty JudySL array:

	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}

	if (PArray == PVNULL) return(PPVNULL);

// Do the search:

	return(JudySLPrevSub(PArray, Index, /* original = */ TRUE,
			     INDEXLEN(Index), PJError));

} // JudySLPrev()


// ****************************************************************************
// J U D Y   S L   P R E V   S U B
//
// This is the "engine" for JudySLPrev() that knows whether it's still looking
// for the original Index (exclusive) or a neighbor index (inclusive), and that
// expects aligned and len to already be computed (only once).  See the header
// comments for JudySLPrev().

FUNCTION static PPvoid_t JudySLPrevSub(
	Pcvoid_t  PArray,
	char *	  Index,
	bool_t	  original,	// still working on original Index.
	Word_t    len,		// bytes remaining.
	PJError_t PJError)	// optional, for returning error info.
{
	Word_t    indexword;	// next word to find.
	PPvoid_t  PPvalue;	// from JudyL array.


// ORIGINAL SEARCH:
//
// When at a shortcut leaf, copy its remaining Index (string) chars into Index
// and return its value area if the current Index is after (greater than) the
// SCL's index; otherwise return null.

	if (original)
	{
	    if (IS_PSCL(PArray))
	    {
		if ((PPvalue = PPSCLVALUE_GT(Index, PArray)) != PPVNULL)
		    (void) strcpy(Index, PSCLINDEX(PArray));

		return(PPvalue);
	    }

// If the current Index word:
// - is not the last word in Index (end of string),
// - exists in the current JudyL array, and,
// - a previous Index is found below it, return that Index's value area.

	    COPYSTRINGtoWORD(indexword, Index);	// copy next 4[8] bytes.

	    if (! LASTWORD_BY_LEN(len))		// not at end of Index.
	    {

// Use JLG() for speed, at the cost of defining a local JUDYERROR():

#undef JUDYERROR
#define	JUDYERROR(ignore1, ignore2, ignore3, JudyErrno, JudyErrID)	\
		if (PPvalue == PPJERR)					\
		{							\
		    JU_ERRNO(PJError) = (JudyErrno);			\
		    JU_ERRID(PJError) = (JudyErrID);			\
		    JudySLModifyErrno(PJError, PArray,			\
				      original ? PArray : PVNULL);	\
		    return(PPJERR);					\
		}

		JLG(PPvalue, PArray, indexword);

		if (PPvalue != PPVNULL)
		{

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

		    PPvalue = JudySLPrevSub(*PPvalue, Index + WORDSIZE,
					    /* original = */ TRUE,
					    len - WORDSIZE, PJError);

		    if (PPvalue == PPJERR)  return(PPJERR);  // propagate error.
		    if (PPvalue != PPVNULL) return(PPvalue); // see above.
		}
	    }

// Search for previous index word:
//
// One of the above conditions is false.  Search the current JudyL array for
// the Index word, if any, prior to the current index word.  If none is found,
// return null; otherwise fall through to common later code.

	    if ((PPvalue = JudyLPrev(PArray, &indexword, PJError)) == PPJERR)
	    {
		JudySLModifyErrno(PJError, PArray, original ? PArray : PVNULL);
		return(PPJERR);
	    }

	    if (PPvalue == PPVNULL) return(PPVNULL);  // no previous index word.

	} // if.


// SUBSEQUENT SEARCH:
//
// A higher level search already excluded the initial Index, then found a
// previous index word, and is now traversing down to determine the rest of the
// Index and to obtain its value area.  If at a shortcut leaf, return its value
// area.  Otherwise search the current JudyL array backward from the upper
// limit for its last index word.  If no index word is found, return null --
// should never happen unless the JudySL tree is corrupt; otherwise fall
// through to common later code.

	else
	{
	    if (IS_PSCL(PArray))		// at shortcut leaf.
	    {
		(void) strcpy(Index, PSCLINDEX(PArray));
		return (&PSCLVALUE(PArray));
	    }

	    indexword = cJU_ALLONES;

	    if ((PPvalue = JudyLLast(PArray, &indexword, PJError))
		== PPJERR)
	    {
		JudySLModifyErrno(PJError, PArray, original ? PArray : PVNULL);
		return(PPJERR);
	    }

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

	    if (PPvalue == PPVNULL) return(PPVNULL);  // no previous index word.
	}


// FOUND PREVIOUS INDEX WORD:
//
// A previous (if original) or last (if subsequent) index word was located in
// the current JudyL array.  Store it into the caller's Index (string).  Then
// if the found (previous) Index ends here, return its value area; otherwise do
// a subsequent search below this point, which should never fail unless the
// JudySL tree is corrupt, but this is detected at a lower level by the above
// assertion.
//
// Note:  Treat Index as unaligned, even if it is aligned, to avoid writing
// past the end of allocated memory (in case it's less than a whole number of
// words).

	COPYWORDtoSTRING(Index, indexword);	// copy next 4[8] bytes.

	if (LASTWORD_BY_VALUE(indexword)) return(PPvalue);

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

	return(JudySLPrevSub(*PPvalue, Index + WORDSIZE, /* original = */ FALSE,
			     len - WORDSIZE, PJError));

} // JudySLPrevSub()


// ****************************************************************************
// J U D Y   S L   N E X T
//
// See the comments in JudySLPrev(), which is very similar.
//
// TBD:  Could the two functions call a common engine function with various
// subfunctions and other constants specified?

FUNCTION PPvoid_t JudySLNext(
	Pcvoid_t  PArray,
	char *	  Index,
	PJError_t PJError)	// optional, for returning error info.
{
// Check for caller error (null pointer), or empty JudySL array:

	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}

	if (PArray == PVNULL) return(PPVNULL);

// Do the search:

	return(JudySLNextSub(PArray, Index, /* original = */ TRUE,
			     INDEXLEN(Index), PJError));

} // JudySLNext()


// ****************************************************************************
// J U D Y   S L   N E X T   S U B
//
// See the comments in JudySLPrevSub(), which is very similar.

FUNCTION static PPvoid_t JudySLNextSub(
	Pcvoid_t  PArray,
	char *	  Index,
	bool_t	  original,		// still working on original Index.
	Word_t    len,			// bytes remaining.
	PJError_t PJError)		// optional, for returning error info.
{
	Word_t    indexword;		// next word to find.
	PPvoid_t  PPvalue;		// from JudyL array.

	if (original)
	{
	    if (IS_PSCL(PArray))
	    {
		if ((PPvalue = PPSCLVALUE_LT(Index, PArray)) != PPVNULL)
		    (void) strcpy(Index, PSCLINDEX(PArray));

		return(PPvalue);
	    }

	    COPYSTRINGtoWORD(indexword, Index);	 // copy next 4[8] bytes.

	    if (! LASTWORD_BY_LEN(len))		// not at end of Index.
	    {

// Use JLG() for speed, at the cost of defining a local JUDYERROR():

#undef JUDYERROR
#define	JUDYERROR(ignore1, ignore2, ignore3, JudyErrno, JudyErrID)	\
		if (PPvalue == PPJERR)					\
		{							\
		    JU_ERRNO(PJError) = (JudyErrno);			\
		    JU_ERRID(PJError) = (JudyErrID);			\
		    JudySLModifyErrno(PJError, PArray,			\
				      original ? PArray : PVNULL);	\
		    return(PPJERR);					\
		}

		JLG(PPvalue, PArray, indexword);

		if (PPvalue != PPVNULL)
		{

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

		    PPvalue = JudySLNextSub(*PPvalue, Index + WORDSIZE,
					    /* original = */ TRUE,
					    len - WORDSIZE, PJError);

		    if (PPvalue == PPJERR)  return(PPJERR);  // propagate error.
		    if (PPvalue != PPVNULL) return(PPvalue); // see above.
		}
	    }

	    if ((PPvalue = JudyLNext(PArray, &indexword, PJError)) == PPJERR)
	    {
		JudySLModifyErrno(PJError, PArray, original ? PArray : PVNULL);
		return(PPJERR);
	    }

	    if (PPvalue == PPVNULL) return(PPVNULL);	// no next index word.
	}
	else
	{
	    if (IS_PSCL(PArray))		// at shortcut leaf.
	    {
		(void) strcpy(Index, PSCLINDEX(PArray));
		return(&PSCLVALUE(PArray));
	    }

	    indexword = 0;

	    if ((PPvalue = JudyLFirst(PArray, &indexword, PJError)) == PPJERR)
	    {
		JudySLModifyErrno(PJError, PArray, original ? PArray : PVNULL);
		return(PPJERR);
	    }

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

	    if (PPvalue == PPVNULL) return(PPVNULL);	// no next index word.
	}

	COPYWORDtoSTRING(Index, indexword);	// copy next 4[8] bytes

	if (LASTWORD_BY_VALUE(indexword)) return(PPvalue);

// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue; this is automatically treated as a dead-end (not a core dump
// or assertion; see version 1.25):

	return(JudySLNextSub(*PPvalue, Index + WORDSIZE, /* original = */ FALSE,
				 len - WORDSIZE, PJError));

} // JudySLNextSub()


// ****************************************************************************
// J U D Y   S L   F I R S T
//
// Like JudyLFirst(), do a JudySLGet(), then if necessary a JudySLNext().

FUNCTION PPvoid_t JudySLFirst(
	Pcvoid_t  PArray,
	char *	  Index,
	PJError_t PJError)	// optional, for returning error info.
{
	PPvoid_t  PPvalue;	// from JudyL array.

	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}

	if ((PPvalue = JudySLGet(PArray, Index, PJError)) == PPJERR)
	    return(PPJERR);		// propagate serious error.

	if ((PPvalue == PPVNULL)	// first try failed.
	 && ((PPvalue = JudySLNext(PArray, Index, PJError)) == PPJERR))
	{
	    return(PPJERR);		// propagate serious error.
	}

	return(PPvalue);

} // JudySLFirst()


// ****************************************************************************
// J U D Y   S L   L A S T
//
// Like JudyLLast(), do a JudySLGet(), then if necessary a JudySLPrev().

FUNCTION PPvoid_t JudySLLast(
	Pcvoid_t  PArray,
	char *	  Index,
	PJError_t PJError)	// optional, for returning error info.
{
	PPvoid_t  PPvalue;	// from JudyL array.

	if (Index == PCNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPINDEX);
	    return(PPJERR);
	}

	if ((PPvalue = JudySLGet(PArray, Index, PJError)) == PPJERR)
	    return(PPJERR);		// propagate serious error.

	if ((PPvalue == PPVNULL)	// first try failed.
	 && ((PPvalue = JudySLPrev(PArray, Index, PJError)) == PPJERR))
	{
	    return(PPJERR);		// propagate serious error.
	}

	return(PPvalue);

} // JudySLLast()


// ****************************************************************************
// J U D Y   S L   F R E E   A R R A Y
//
// Walk the JudySL tree of JudyL arrays to free each JudyL array, depth-first.
// During the walk, ignore indexes (strings) that end in the current JudyL
// array to be freed.  Just recurse through those indexes which do not end,
// that is, those whose associated value areas point to subsidiary JudyL
// arrays, except for those which point to shortcut leaves.  Return the total
// bytes freed in all of the JudyL arrays at or below the current level.
//
// Like the JudyLFreeArray() and Judy1FreeArray() code, this is written
// recursively, which is probably fast enough, to allow indexes (strings) of
// arbitrary size.  If recursion turns out to be a problem, consider instead
// doing some large, fixed number of iterative descents (like 100) using a
// fixed-size "stack" (array), then recursing upon overflow (relatively
// rarely).

FUNCTION Word_t JudySLFreeArray(
	PPvoid_t  PPArray,
	PJError_t PJError)	// optional, for returning error info.
{
	PPvoid_t  PPArrayOrig = PPArray;	// for error reporting.
	Word_t    indexword = 0;		// word just found.
	PPvoid_t  PPvalue;			// from Judy array.
	Word_t	  bytes_freed = 0;		// bytes freed at this level.
	Word_t	  bytes_total = 0;		// bytes freed at all levels.

	if (PPArray == PPVNULL)
	{
	    JU_SET_ERRNO(PJError, JU_ERRNO_NULLPPARRAY);
	    return(JERR);
	}


// FREE SHORTCUT LEAF:

	if (IS_PSCL(*PPArray))
	{
	    bytes_freed = SCLSIZE(strlen(PSCLINDEX(*PPArray)));
	    FREE_SCL((Pscl_t) CLEAR_PSCL(*PPArray));
	    *PPArray = PVNULL;

	    return(bytes_freed);
	}


// FREE EACH SUB-ARRAY (DEPTH-FIRST):
//
// If a previous JudySLIns() ran out of memory partway down the tree, it left a
// null *PPvalue.  This is automatically treated correctly here as a dead-end.
//
// An Index (string) ends in the current word iff the last byte of the
// (null-padded) word is null.

	for ( PPvalue  = JudyLFirst(*PPArray, &indexword, PJError);
	     (PPvalue != PPVNULL) && (PPvalue != PPJERR);
	      PPvalue  = JudyLNext( *PPArray, &indexword, PJError))
	{
	    if (! LASTWORD_BY_VALUE(indexword))
	    {
		if ((bytes_freed = JudySLFreeArray(PPvalue, PJError)) == JERR)
		    return(JERR);		// propagate serious error.

		bytes_total += bytes_freed;
	    }
	}

// Check for a serious error in a JudyL*() call:

	if (PPvalue == PPJERR)
	{
	    JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
	    return(JERR);
	}

// Now free the current array, which also nulls the pointer:
//
// Note:  *PPArray can be null here for a totally null JudySL array =>
// JudyLFreeArray() returns zero.

	if ((bytes_freed = JudyLFreeArray(PPArray, PJError)) == JERR)
	{
	    JudySLModifyErrno(PJError, *PPArray, *PPArrayOrig);
	    return(JERR);
	}
	return(bytes_total + bytes_freed);

} // JudySLFreeArray()
