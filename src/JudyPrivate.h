#ifndef _JUDYPRIVATE_INCLUDED
#define	_JUDYPRIVATE_INCLUDED
// _________________
//
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

// @(#) $Revision: 4.77 $ $Source: /judy/src/JudyCommon/JudyPrivate.h $
//
// Header file for all Judy sources, for global but private (non-exported)
// declarations.


// INCLUDE EXPORTED HEADER FILE:
//
// Assume all Judy internal sources need the exported header file.

#include "Judy.h"

// Define some types missing on Windows (not available in a system header
// file):

#ifdef JU_WIN
typedef unsigned char	 uint8_t;	//  8-bit unsigned.
typedef unsigned short	 uint16_t;	// 16-bit unsigned.
typedef unsigned long	 uint32_t;	// 32-bit unsigned.
#ifdef JU_WIN_IPF
typedef unsigned __int64 uint64_t;	// appears __int64 is a native type.
#endif
#endif


// ****************************************************************************
// A VERY BRIEF EXPLANATION OF A JUDY ARRAY
//
// A Judy array is, effectively, a digital tree (or Trie) with 256 element
// branches (nodes), and with "compression tricks" applied to low-population
// branches or leaves to save a lot of memory at the cost of relatively little
// CPU time or cache fills.
//
// In the actual implementation, a Judy array is level-less, and traversing the
// "tree" actually means following the states in a state machine (SM) as
// directed by the Index.  A Judy array is referred to here as an "SM", rather
// than as a "tree"; having "states", rather than "levels".
//
// Each branch or leaf in the SM decodes a portion ("digit") of the original
// Index; with 256-way branches there are 8 bits per digit.  There are 3 kinds
// of branches, called:  Linear, Bitmap and Uncompressed, of which the first 2
// are compressed to contain no NULL entries.
//
// An Uncompressed branch has a 1.0 cache line fill cost to decode 8 bits of
// (digit, part of an Index), but it might contain many NULL entries, and is
// therefore inefficient with memory if lightly populated.
//
// A Linear branch has a ~1.75 cache line fill cost when at maximum population.
// A Bitmap branch has ~2.0 cache line fills.  Linear and Bitmap branches are
// converted to Uncompressed branches when the additional memory can be
// amortized with larger populations.  Higher-state branches have higher
// priority to be converted.
//
// Linear branches can hold 28 elements (based on detailed analysis) -- thus 28
// expanses.  A Linear branch is converted to a Bitmap branch when the 29th
// expanse is required.
//
// A Bitmap branch could hold 256 expanses, but is forced to convert to an
// Uncompressed branch when 185 expanses are required.  Hopefully, it is
// converted before that because of population growth (again, based on detailed
// analysis and heuristics in the code).
//
// A path through the SM terminates to a leaf when the Index (or key)
// population in the expanse below a pointer will fit into 1 or 2 cache lines
// (~31..255 Indexes).  A maximum-population Leaf has ~1.5 cache line fill
// cost.
//
// Leaves are sorted arrays of Indexes, where the Index Sizes (IS) are:  0, 1,
// 8, 16, 24, 32, [40, 48, 56, 64] bits.  The IS depends on the "density"
// (population/expanse) of the values in the Leaf.  Zero bits are possible if
// population == expanse in the SM (that is, a full small expanse).
//
// Elements of a branches are called Judy Pointers (JPs).  Each JP object
// points to the next object in the SM, plus, a JP can decode an additional
// 2[6] bytes of an Index, but at the cost of "narrowing" the expanse
// represented by the next object in the SM.  A "narrow" JP (one which has
// decode bytes/digits) is a way of skipping states in the SM.
//
// Although counterintuitive, we think a Judy SM is optimal when the Leaves are
// stored at MINIMUM compression (narrowing, or use of Decode bytes).  If more
// aggressive compression was used, decompression of a leaf be required to
// insert an index.  Additional compression would save a little memory but not
// help performance significantly.


#ifdef A_PICTURE_IS_WORTH_1000_WORDS
*******************************************************************************

JUDY 32-BIT STATE MACHINE (SM) EXAMPLE, FOR INDEX = 0x02040103

The Index used in this example is purposely chosen to allow small, simple
examples below; each 1-byte "digit" from the Index has a small numeric value
that fits in one column.  In the drawing below:

   JAP  == Judy Array Pointer; least 3 bits encode Branch or Leaf type; L or B
	   appears in drawing as described below.

    C   == 1 byte of a 1..3 byte Population (count of Indexes) below this
	   pointer.  Since this is shared with the Decode field, the combined
	   sizes must be 3[7], that is, 1 word less 1 byte for the JP Type.

   The 1-byte field jp_Type is represented as:

   1..3 == Number of bytes in the population (Pop0) word of the Branch or Leaf
	   below the pointer (note:  1..7 on 64-bit); indicates:
	   - number of bytes in Decode field == 3 - this number;
	   - number of bytes remaining to decode.
	   Note:  The maximum is 3, not 4, because the 1st byte of the Index is
	   always decoded digitally in the top branch.
   -B-	== JP points to a Branch (there are many kinds of Branches).
   -L-	== JP points to a Leaf (there are many kinds of Leaves).

   (2)  == Digit of Index decoded by position offset in branch (really
	   0..0xff).

    4*  == Digit of Index necessary for decoding a "narrow" pointer, in a
	   Decode field; replaces 1 missing branch (really 0..0xff).

    4+  == Digit of Index NOT necessary for decoding a "narrow" pointer, but
	   used for fast traversal of the SM by Judy1Test() and JudyLGet()
	   (see the code) (really 0..0xff).

    0   == Byte in a JP''s Pop0 field that is always ignored, because a leaf
	   can never contain more than 256 Indexes (Pop0 <= 255).

    +-----  == A Branch or Leaf; drawn open-ended to remind you that it could
    |          have up to 256 columns.
    +-----

    |
    |   == Pointer to next Branch or Leaf.
    V

    |
    O   == A state is skipped by using a "narrow" pointer.
    |

    < 1 > == Digit (Index) shown as an example is not necessarily in the
	     position shown; is sorted in order with neighbor Indexes.
	     (Really 0..0xff.)

Note that this example shows every possibly topology to reach a leaf in a
32-bit Judy SM, although this is a very subtle point!

                                                                          STATE/
                                                                          LEVEL
     +---+    +---+    +---+    +---+    +---+    +---+    +---+    +---+
     |JAP|    |JAP|    |JAP|    |JAP|    |JAP|    |JAP|    |JAP|    |JAP|
     L---+    B---+    B---+    B---+    B---+    B---+    B---+    B---+
     |        |        |        |        |        |        |        |
     |        |        |        |        |        |        |        |
     V        V (2)    V (2)    V (2)    V (2)    V (2)    V (2)    V (2)
     +------  +------  +------  +------  +------  +------  +------  +------
Four |< 2 >   |  0     |  4*    |  C     |  4*    |  4*    |  C     |  C
byte |< 4 >   |  0     |  0     |  C     |  1*    |  C     |  C     |  C     4
Index|< 1 >   |  C     |  C     |  C     |  C     |  C     |  C     |  C
Leaf |< 3 >   |  3     |  2     |  3     |  1     |  2     |  3     |  3
     +------  +--L---  +--L---  +--B---  +--L---  +--B---  +--B---  +--B---
                 |        |        |        |        |        |        |
                /         |       /         |        |       /        /
               /          |      /          |        |      /        /
              |           |     |           |        |     |        |
              V           |     V   (4)     |        |     V   (4)  V   (4)
              +------     |     +------     |        |     +------  +------
    Three     |< 4 >      |     |    4+     |        |     |    4+  |    4+
    byte Index|< 1 >      O     |    0      O        O     |    1*  |    C   3
    Leaf      |< 3 >      |     |    C      |        |     |    C   |    C
              +------     |     |    2      |        |     |    1   |    2
                         /      +----L-     |        |     +----L-  +----B-
                        /            |      |        |          |        |
                       |            /       |       /          /        /
                       |           /        |      /          /        /
                       |          /         |     |          /        /
                       |         /          |     |         /        /
                       |        |           |     |        |        |
                       V        V           |     V(1)     |        V(1)
                       +------  +------     |     +------  |        +------
          Two byte     |< 1 >   |< 1 >      |     | 4+     |        | 4+
          Index Leaf   |< 3 >   |< 3 >      O     | 1+     O        | 1+     2
                       +------  +------    /      | C      |        | C
                                          /       | 1      |        | 1
                                         |        +-L----  |        +-L----
                                         |          |      |          |
                                         |         /       |         /
                                         |        |        |        |
                                         V        V        V        V
                                         +------  +------  +------  +------
                    One byte Index Leaf  |< 3 >   |< 3 >   |< 3 >   |< 3 >   1
                                         +------  +------  +------  +------


#endif // A_PICTURE_IS_WORTH_1000_WORDS


// ****************************************************************************
// MISCELLANEOUS GLOBALS:
//
// PLATFORM-SPECIFIC CONVENIENCE MACROS:
//
// These are derived from context (set by cc or in system header files) or
// based on JU_<PLATFORM> macros from make_includes/platform.*.mk.  We decided
// on 011018 that any macro reliably derivable from context (cc or headers) for
// ALL platforms supported by Judy is based on that derivation, but ANY
// exception means to stop using the external macro completely and derive from
// JU_<PLATFORM> instead.
//
// Note:  On win_ipf the data model is 32-bit data, 64-bit pointers, so the
// makefile must set JU_64BIT explicitly because __LP64__ is not set.

#ifdef	__LP64__		// works most everywhere -- but not win_ipf.
#define	JU_64BIT 1		// for 64-bit systems; must be 1 for "#if".
#endif

#define	JU_BIG_ENDIAN    4321
#define	JU_LITTLE_ENDIAN 1234
#define	JU_PDP_ENDIAN    3412

// Note:  The LITTLE_ENDIAN macro is set on Linux but not on Win32, so derive
// JU_BYTEORDER here internally:

#if (JU_LINUX || JU_WIN)
#define	JU_BYTEORDER JU_LITTLE_ENDIAN
#else
#define	JU_BYTEORDER JU_BIG_ENDIAN
#endif

// Other miscellaneous stuff:

#ifndef _BOOL_T
#define	_BOOL_T
typedef int bool_t;
#endif

// Convert some (newer) external names (renamed from "JAP" to avoid negative
// connotations) to (older) internal names:

#define	JAP_MASK    JLAP_MASK
#define	JAP_INVALID JLAP_INVALID

#define	FUNCTION		// null; easy to find functions.

#ifndef	TRUE
#define	TRUE 1
#endif

#ifndef	FALSE
#define	FALSE 0
#endif

#ifdef TRACE		// turn on all other tracing in the code:
#define	TRACEJP	 1	// JP traversals in JudyIns.c and JudyDel.c.
#define	TRACEJPR 1	// JP traversals in retrieval code, JudyGet.c.
#define	TRACECF	 1	// cache fills in JudyGet.c.
#define	TRACEMI	 1	// malloc calls in JudyMallocIF.c.
#define	TRACEMF	 1	// malloc calls at a lower level in JudyMalloc.c.
#endif


// SUPPORT FOR DEBUG-ONLY CODE:
//
// By convention, use -DDEBUG to enable both debug-only code AND assertions in
// the Judy sources.
//
// Invert the sense of assertions, so they are off unless explicitly requested,
// in a uniform way.
//
// Note:  It is NOT appropriate to put this in Judy.h; it would mess up
// application code.

#ifndef DEBUG
#define	NDEBUG 1		// must be 1 for "#if".
#endif

// Shorthand notations to avoid #ifdefs for single-line conditional statements:
//
// Warning:  These cannot be used around compiler directives, such as
// "#include", nor in the case where Code contains a comma other than nested
// within parentheses or quotes.

#ifndef DEBUG
#define	DBGCODE(Code) // null.
#else
#define	DBGCODE(Code) Code
#endif

#ifdef JUDY1
#define	JUDY1CODE(Code) Code
#define	JUDYLCODE(Code) // null.
#endif
#ifdef JUDYL
#define	JUDY1CODE(Code) // null.
#define	JUDYLCODE(Code) Code
#endif

#ifdef JU_FLAVOR_COV
#pragma C-Cover off	// exclude inline functions in public header files.
#endif
#include <assert.h>
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif


// ****************************************************************************
// FUNDAMENTAL CONSTANTS FOR MACHINE
// ****************************************************************************

// Machine (CPU) cache line size:
//
// NOTE:  A leaf size of 2 cache lines maximum is the target (optimal) for
// Judy.  It's hard to obtain a machine's cache line size at compile time, but
// if the machine has an unexpected cache line size, it's not devastating if
// the following constants end up causing leaves that are 1 cache line in size,
// or even 4 cache lines in size.  The assumed 32-bit system has 16-word =
// 64-byte cache lines, and the assumed 64-bit system has 16-word = 128-byte
// cache lines.

#ifndef JU_64BIT
#define	cJU_BYTESPERCL  64		// cache line size in bytes.
#else
#define	cJU_BYTESPERCL 128		// cache line size in bytes.
#endif

// Bits Per Byte:

#define	cJU_BITSPERBYTE	0x8

// Bytes Per Word and Bits Per Word, latter assuming sizeof(byte) is 8 bits:
//
// Expect 32 [64] bits per word.

#define	cJU_BYTESPERWORD (sizeof(Word_t))
#define	cJU_BITSPERWORD  (sizeof(Word_t) * cJU_BITSPERBYTE)

#define	JU_BYTESTOWORDS(Bytes) \
	(((Bytes) + cJU_BYTESPERWORD - 1) / cJU_BYTESPERWORD)

// A word that is all-one's, normally equal to -1UL, but safer with ~0:

#define	cJU_ALLONES  (~0UL)

// Note, these are forward references, but that's OK:

#define	cJU_FULLBITMAPB	((BITMAPB_t) cJU_ALLONES)
#define	cJU_FULLBITMAPL	((BITMAPL_t) cJU_ALLONES)


// ****************************************************************************
// MISCELLANEOUS JUDY-SPECIFIC DECLARATIONS
// ****************************************************************************

// ROOT STATE:
//
// State at the start of the Judy SM, based on 1 byte decoded per state; equal
// to the number of bytes per Index to decode.

#define	cJU_ROOTSTATE (sizeof(Word_t))


// SUBEXPANSES PER STATE:
//
// Number of subexpanses per state traversed, which is the number of JPs in a
// branch (actual or theoretical) and the number of bits in a bitmap.

#define	cJU_SUBEXPPERSTATE  256


// LEAF AND VALUE POINTERS:
//
// Some other basic object types are in declared in JudyPrivateBranch.h
// (Pjbl_t, Pjbb_t, Pjbu_t, Pjp_t) or are Judy1/L-specific (Pjlb_t).  The
// few remaining types are declared below.
//
// Note:  Leaf pointers are cast to different-sized objects depending on the
// leaf's level, but are at least addresses (not just numbers), so use void *
// (Pvoid_t), not PWord_t or Word_t for them, except use Pjlw_t for whole-word
// (top-level, root-level) leaves.  Value areas, however, are always whole
// words.
//
// Furthermore, use Pjll_t only for generic leaf pointers (for various size
// LeafL's).  Use Pjlw_t for LeafW's.  Use Pleaf (with type uint8_t *, uint16_t
// *, etc) when the leaf index size is known.

typedef	PWord_t	Pjlw_t;	 // pointer to root-level leaf (whole-word indexes).
typedef	Pvoid_t	Pjll_t;	 // pointer to lower-level linear leaf.
#ifdef JUDYL
typedef PWord_t	Pjv_t;	 // pointer to JudyL value area.
#endif


// POINTER PREPARATION MACROS:
//
// These macros are used to strip malloc-namespace-type bits from a pointer +
// malloc-type word (which references any Judy malloc'd object that might be
// obtained from other than a direct call of malloc()), prior to dereferencing
// the pointer as an address.  The malloc-type bits allow Judy malloc'd objects
// to come from different "malloc() namespaces".
//
// Note:  Non-null Judy array root pointers (JAP) point to root-level
// (word-sized) leaves (JLW) or to top-level Judy population/memory (JPM)
// nodes, both of which come directly from malloc() and which are not capable
// of alternate name-spacing.  However, for consistency and for easier future
// changes, there are macros for accessing pointers to these objects too, which
// DO strip JAP_MASK bits.
//
// In other words, JLW and JPM pointers contain JAP type bits, and other
// pointers may contain malloc-namespace-type bits.  These pointers (addresses)
// include these fields in Judy objects:
//
//    (root pointer)	(JAP, see above)
//    jp.jp_Addr	generic pointer to next-level node, except when used
//			as a JudyL Immed01 value area
//    JU_JBB_PJP	macro hides jbbs_Pjp (pointer to JP subarray)
//    JL_JLB_PVALUE	macro hides jLlbs_PValue (pointer to value subarray)
//
// When setting one of these fields or passing an address to __JudyFree*(), the
// "raw" memory address is used; otherwise the memory address must be passed
// through one of the macros below before it's dereferenced.
//
// Note:  After much study, the typecasts below appear in the macros rather
// than at the point of use, which is both simpler and allows the compiler to
// do type-checking.

#define	__numbits  3		// number of malloc-type bits in pointer field.
#define	__mask	   (-(1UL << __numbits))

#define	JAPTYPE(Addr)		(((Word_t) (Addr)) &  JAP_MASK)	 // root type.
#define	P_JLW(  Addr) ((Pjlw_t) (((Word_t) (Addr)) & ~JAP_MASK)) // root leaf.
#define	P_JPM(  Addr) ((Pjpm_t) (((Word_t) (Addr)) & ~JAP_MASK)) // root JPM.
#define	P_JBL(  Addr) ((Pjbl_t) (((Word_t) (Addr)) & __mask))	 // BranchL.
#define	P_JBB(  Addr) ((Pjbb_t) (((Word_t) (Addr)) & __mask))	 // BranchB.
#define	P_JBU(  Addr) ((Pjbu_t) (((Word_t) (Addr)) & __mask))	 // BranchU.
#define	P_JLL(  Addr) ((Pjll_t) (((Word_t) (Addr)) & __mask))	 // LeafL.
#define	P_JLB(  Addr) ((Pjlb_t) (((Word_t) (Addr)) & __mask))	 // LeafB1.
#define	P_JP(   Addr) ((Pjp_t)  (((Word_t) (Addr)) & __mask))	 // JP.
#ifdef JUDYL
#define	P_JV(   Addr) ((Pjv_t)  (((Word_t) (Addr)) & __mask))	 // &value.
#endif


// LEAST BYTES:
//
// Mask for least bytes of a word, and a macro to perform this mask on an
// Index.
//
// Note:  This macro has been problematic in the past to get right and to make
// portable.  It's not OK on all systems to shift by the full word size.  This
// macro should allow shifting by 1..N bytes, where N is the word size, but
// should produce a compiler warning if the macro is called with Bytes == 0.
//
// Warning:  JU_LEASTBYTESMASK() is not a constant macro unless Bytes is a
// constant; otherwise it is a variable shift, which is expensive on some
// processors.

#define	JU_LEASTBYTESMASK(Bytes) \
	((0x100UL << (cJU_BITSPERBYTE * ((Bytes) - 1))) - 1)

#define	JU_LEASTBYTES(Index,Bytes)  ((Index) & JU_LEASTBYTESMASK(Bytes))


// BITS IN EACH BITMAP SUBEXPANSE FOR BITMAP BRANCH AND LEAF:
//
// The bits per bitmap subexpanse times the number of subexpanses equals a
// constant (cJU_SUBEXPPERSTATE).  You can also think of this as a compile-time
// choice of "aspect ratio" for bitmap branches and leaves (which can be set
// independently for each).
//
// A default aspect ratio is hardwired here if not overridden at compile time,
// such as by "EXTCCOPTS=-DBITMAP_BRANCH16x16 make".

#if (! (defined(BITMAP_BRANCH8x32) || defined(BITMAP_BRANCH16x16) || defined(BITMAP_BRANCH32x8)))
#define	BITMAP_BRANCH32x8 1	// 32 bits per subexpanse, 8 subexpanses.
#endif

#ifdef BITMAP_BRANCH8x32
#define	BITMAPB_t uint8_t
#endif

#ifdef BITMAP_BRANCH16x16
#define	BITMAPB_t uint16_t
#endif

#ifdef BITMAP_BRANCH32x8
#define	BITMAPB_t uint32_t
#endif

// Note:  For bitmap leaves, BITMAP_LEAF64x4 is only valid for 64 bit:
//
// Note:  Choice of aspect ratio mostly matters for JudyL bitmap leaves.  For
// Judy1 the choice doesn't matter much -- the code generated for different
// BITMAP_LEAF* values choices varies, but correctness and performance are the
// same.

#ifndef JU_64BIT
#if (! (defined(BITMAP_LEAF8x32) || defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8)))
#define	BITMAP_LEAF32x8		// 32 bits per subexpanse, 8 subexpanses.
#endif

#else // JU_64BIT

#if (! (defined(BITMAP_LEAF8x32) || defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8) || defined(BITMAP_LEAF64x4)))
#define	BITMAP_LEAF64x4		// 64 bits per subexpanse, 4 subexpanses.
#endif
#endif // JU_64BIT

#ifdef BITMAP_LEAF8x32
#define	BITMAPL_t uint8_t
#endif

#ifdef BITMAP_LEAF16x16
#define	BITMAPL_t uint16_t
#endif

#ifdef BITMAP_LEAF32x8
#define	BITMAPL_t uint32_t
#endif

#ifdef BITMAP_LEAF64x4
#define	BITMAPL_t uint64_t
#endif


// EXPORTED DATA AND FUNCTIONS:

#ifdef JUDY1
extern const uint8_t __j1_BranchBJPPopToWords[];
#else
extern const uint8_t __jL_BranchBJPPopToWords[];
#endif

// Fast LeafL search routine used for inlined code:
//
// Note:  MaxPop1 is historical and unused.

#define SEARCHLEAFEVEN(LeafType,Addr,Pop1,Index,MaxPop1)		 \
	LeafType *_Pleaf = (LeafType *) (Addr);				 \
	LeafType  _Index = (Index);	/* with masking */		 \
	Word_t	  _Pop1  = (Pop1);					 \
									 \
	if (_Index > _Pleaf[_Pop1 - 1]) return(~(_Pop1)); /* past end */ \
	for(;;) { if (_Index <= *_Pleaf) break; ++_Pleaf; }		 \
	if (_Index == *_Pleaf) return(_Pleaf - (LeafType *) (Addr));	 \
	return(~(_Pleaf - (LeafType *) (Addr)))

// Fast way to count bits set in 8..32[64]-bit int:
//
// For performance, __JudyCountBits*() are written to take advantage of
// platform-specific features where available.
//
// Note:  On HPUX "static inline" is disallowed, meaning the only way to get
// the code inlined would be to have N copies, one per *.o file, so don't
// do that, although it means no inlining either.

#ifdef JU_HPUX_PA
extern BITMAPB_t __JudyCountBitsB(BITMAPB_t word);
extern BITMAPL_t __JudyCountBitsL(BITMAPL_t word);
#endif

#ifdef JU_HPUX_IPF
#define	__JudyCountBitsB(word)	_Asm_popcnt(word)
#define	__JudyCountBitsL(word)	_Asm_popcnt(word)
#endif

#if (JU_LINUX_IA32 || JU_WIN_IA32)

#if (JU_FLAVOR_COV && (! JU_COV_INLINES))
#pragma C-Cover off	// no metrics for inlines except when requested.
#endif

// The gcc compiler, and hopefully the Win32 compiler, allow static __inline__:
//
// TBD:  If Win32 does not allow __inline__, try _inline_ or even inline.

static __inline__ BITMAPB_t __JudyCountBitsB(BITMAPB_t word)
{
	word = (word & 0x55555555) + ((word & 0xAAAAAAAA) >>  1);
	word = (word & 0x33333333) + ((word & 0xCCCCCCCC) >>  2);
	word = (word & 0x0F0F0F0F) + ((word & 0xF0F0F0F0) >>  4);
#if defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8)
	word = (word & 0x00FF00FF) + ((word & 0xFF00FF00) >>  8);
#endif
#ifdef BITMAP_BRANCH32x8
	word = (word & 0x0000FFFF) + ((word & 0xFFFF0000) >> 16);
#endif
	return (word);

} // __JudyCountBitsB()

static __inline__ BITMAPL_t __JudyCountBitsL(BITMAPL_t word)
{
	word = (word & 0x55555555) + ((word & 0xAAAAAAAA) >>  1);
	word = (word & 0x33333333) + ((word & 0xCCCCCCCC) >>  2);
	word = (word & 0x0F0F0F0F) + ((word & 0xF0F0F0F0) >>  4);
#if defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8)
	word = (word & 0x00FF00FF) + ((word & 0xFF00FF00) >>  8);
#endif
#ifdef BITMAP_BRANCH32x8
	word = (word & 0x0000FFFF) + ((word & 0xFFFF0000) >> 16);
#endif
	return (word);

} // __JudyCountBitsL()

#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#endif // (JU_LINUX_IA32 || JU_WIN_IA32)

#ifdef JU_LINUX_IPF

#if (JU_FLAVOR_COV && (! JU_COV_INLINES))
#pragma C-Cover off	// no metrics for inlines except when requested.
#endif

static __inline__ BITMAPB_t __JudyCountBitsB(BITMAPB_t word)
{
        BITMAPB_t result;
        __asm__ ("popcnt %0=%1" : "=r" (result) : "r" (word));
        return(result);
}

static __inline__ BITMAPL_t __JudyCountBitsL(BITMAPL_t word)
{
        BITMAPL_t result;
        __asm__ ("popcnt %0=%1" : "=r" (result) : "r" (word));
        return(result);
}

#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#endif // JU_LINUX_IPF

#ifdef JU_WIN_IPF
// TBD:  Would prefer to use the IPF popcnt instruction, but must say
// "__inline", "__asm", and the __asm construct needs braces and straight
// assembly code, so for now do it the slower way:

extern BITMAPB_t __JudyCountBitsB(BITMAPB_t word);
extern BITMAPL_t __JudyCountBitsL(BITMAPL_t word);
#endif


// GET POP0:
//
// Get from jp_DcdPop0 the Pop0 for various JP Types.
//
// Notes:
//
// - Different macros require different parameters...
//
// - There are no simple macros for cJU_JAPBRANCH* Types because their
//   populations must be added up and don't reside in an already-calculated
//   place.  (TBD:  This is no longer true, now it's in the JPM.)
//
// - cJU_JPIMM_POP0() is not defined because it would be redundant because the
//   Pop1 is already encoded in each enum name.
//
// - A linear or bitmap leaf Pop0 cannot exceed cJU_SUBEXPPERSTATE - 1 (Pop0 =
//   0..255), so use a simpler, faster macro for it than for other JP Types.
//
// - Avoid any complex calculations that would slow down the compiled code.
//   Assume these macros are only called for the appropriate JP Types.
//   Unfortunately there's no way to trigger an assertion here if the JP type
//   is incorrect for the macro, because these are merely expressions, not
//   statements.

#define	 JU_JAPLEAF_POP0(Addr)		     (*((PWord_t) Addr))
//	 JU_JAPBRANCH_POP0()		     // see above.
//	 JU_JPBRANCH_POP0(DcdPop0,cPopBytes) // see JudyPrivateBranch.h.
#define	 JU_JPLEAF_POP0(DcdPop0)	     ((DcdPop0) & 0xFF)
//	cJU_JPIMM_POP0(cJPType,cJPTypeBase)  ((cJPType) - (cJPTypeBase))
#define	cJU_JPFULLPOPU1_POP0		     (cJU_SUBEXPPERSTATE - 1)


// NUMBER OF BITS IN A BRANCH OR LEAF BITMAP AND SUBEXPANSE:
//
// Note:  cJU_BITSPERBITMAP must be the same as the number of JPs in a branch.

#define	cJU_BITSPERBITMAP cJU_SUBEXPPERSTATE

// Bitmaps are accessed in units of "subexpanses":

#define	cJU_BITSPERSUBEXPB  (sizeof(BITMAPB_t) * cJU_BITSPERBYTE)
#define	cJU_NUMSUBEXPB	    (cJU_BITSPERBITMAP / cJU_BITSPERSUBEXPB)

#define	cJU_BITSPERSUBEXPL  (sizeof(BITMAPL_t) * cJU_BITSPERBYTE)
#define	cJU_NUMSUBEXPL	    (cJU_BITSPERBITMAP / cJU_BITSPERSUBEXPL)


// MASK FOR A SPECIFIED BIT IN A BITMAP:
//
// Warning:  If BitNum is a variable, this results in a variable shift that is
// expensive, at least on some processors.  Use with caution.
//
// Warning:  BitNum must be less than cJU_BITSPERWORD, that is, 0 ..
// cJU_BITSPERWORD - 1, to avoid a truncated shift on some machines.
//
// TBD:  Perhaps use an array[32] of masks instead of calculating them.

#define	JU_BITPOSMASKB(BitNum) (1L << ((BitNum) % cJU_BITSPERSUBEXPB))
#define	JU_BITPOSMASKL(BitNum) (1L << ((BitNum) % cJU_BITSPERSUBEXPL))


// TEST/SET/CLEAR A BIT IN A BITMAP LEAF:
//
// Test if a byte-sized Digit (portion of Index) has a corresponding bit set in
// a bitmap, or set a byte-sized Digit's bit into a bitmap, by looking up the
// correct subexpanse and then checking/setting the correct bit.
//
// Note:  Mask higher bits, if any, for the convenience of the user of this
// macro, in case they pass a full Index, not just a digit.  If the caller has
// a true 8-bit digit, make it of type uint8_t and the compiler should skip the
// unnecessary mask step.

#define	JU_SUBEXPL(DIGIT) (((DIGIT) / cJU_BITSPERSUBEXPL) & (cJU_NUMSUBEXPL-1))

#define	JU_BITMAPTESTL(PJLB, INDEX)  \
    (JU_JLB_BITMAP(PJLB, JU_SUBEXPL(INDEX)) &  JU_BITPOSMASKL(INDEX))

#define	JU_BITMAPSETL(PJLB, INDEX)   \
    (JU_JLB_BITMAP(PJLB, JU_SUBEXPL(INDEX)) |= JU_BITPOSMASKL(INDEX))

#define	JU_BITMAPCLEARL(PJLB, INDEX) \
    (JU_JLB_BITMAP(PJLB, JU_SUBEXPL(INDEX)) ^= JU_BITPOSMASKL(INDEX))


// MAP BITMAP BIT OFFSET TO DIGIT:
//
// Given a digit variable to set, a bitmap branch or leaf subexpanse (base 0),
// the bitmap (BITMAP*_t) for that subexpanse, and an offset (Nth set bit in
// the bitmap, base 0), compute the digit (also base 0) corresponding to the
// subexpanse and offset by counting all bits in the bitmap until offset+1 set
// bits are seen.  Avoid expensive variable shifts.  Offset should be less than
// the number of set bits in the bitmap; assert this.
//
// If there's a better way to do this, I don't know what it is.

#define	JU_BITMAPDIGITB(Digit,Subexp,Bitmap,Offset)		\
	{							\
	    BITMAPB_t bitmap = (Bitmap); int remain = (Offset);	\
	    (Digit) = (Subexp) * cJU_BITSPERSUBEXPB;		\
								\
	    while ((remain -= (bitmap & 1)) >= 0)		\
	    {							\
		bitmap >>= 1; ++(Digit);			\
		assert((Digit) < ((Subexp) + 1) * cJU_BITSPERSUBEXPB); \
	    }							\
	}

#define	JU_BITMAPDIGITL(Digit,Subexp,Bitmap,Offset)		\
	{							\
	    BITMAPL_t bitmap = (Bitmap); int remain = (Offset);	\
	    (Digit) = (Subexp) * cJU_BITSPERSUBEXPL;		\
								\
	    while ((remain -= (bitmap & 1)) >= 0)		\
	    {							\
		bitmap >>= 1; ++(Digit);			\
		assert((Digit) < ((Subexp) + 1) * cJU_BITSPERSUBEXPL); \
	    }							\
	}


// MASKS FOR PORTIONS OF 32-BIT WORDS:
//
// These are useful for bitmap subexpanses.
//
// "LOWER"/"HIGHER" means bits representing lower/higher-valued Indexes.  The
// exact order of bits in the word is explicit here but is hidden from the
// caller.
//
// "EXC" means exclusive of the specified bit; "INC" means inclusive.
//
// In each case, BitPos is either "JU_BITPOSMASK*(BitNum)", or a variable saved
// from an earlier call of that macro; either way, it must be a 32-bit word
// with a single bit set.  In the first case, assume the compiler is smart
// enough to optimize out common subexpressions.
//
// The expressions depend on unsigned decimal math that should be universal.

#define	JU_MASKLOWEREXC( BitPos)  ((BitPos) - 1)
#define	JU_MASKLOWERINC( BitPos)  (JU_MASKLOWEREXC(BitPos) | (BitPos))
#define	JU_MASKHIGHERINC(BitPos)  (-(BitPos))
#define	JU_MASKHIGHEREXC(BitPos)  (JU_MASKHIGHERINC(BitPos) ^ (BitPos))


// ****************************************************************************
// SUPPORT FOR NATIVE INDEX SIZES
// ****************************************************************************
//
// Copy a series of generic objects (uint8_t, uint16_t, uint32_t, Word_t) from
// one place to another.

#define	JU_COPYMEM(PDst,PSrc,Pop1)			\
    {							\
	Word_t __index = 0;				\
	assert((Pop1) > 0);				\
	do { (PDst)[__index] = (PSrc)[__index]; }	\
	while (++__index < (Pop1));			\
    }


// ****************************************************************************
// SUPPORT FOR ODD (NON-NATIVE) INDEX SIZES
// ****************************************************************************
//
// This is the best fake uint24_t I could design in C.  -- dlb
//
// Note about endian issues:  On a little-endian machine, the _t24_3byte field
// is apparently accessed through a register (always big-endian), while the
// _t24_3bytes field is in memory.  Therefore, on a little-endian machine, the
// 3 index bytes are in little-endian order in memory.  For example, if the
// index bytes are big-endian "cba", they are stored "abc" in memory.

typedef struct __FAKE_24BIT_UINT
	{
	    union
	    {
		Word_t  _t24_3byte:24;
		uint8_t _t24_3bytes[3];
	    } _;
	} uint24_t;

// Copy a 3-byte Index pointed by a uint8_t * to a Word_t:
//
// See above about endian issues.

#define	JU_COPY3_PINDEX_TO_LONG(DestLong,PIndex)	\
	{						\
	    uint24_t temp;				\
	    temp._._t24_3bytes[0] = (PIndex)[0];	\
	    temp._._t24_3bytes[1] = (PIndex)[1];	\
	    temp._._t24_3bytes[2] = (PIndex)[2];	\
	    (DestLong) = temp._._t24_3byte;		\
	}

// Copy a Word_t to a 3-byte Index pointed at by a uint8_t *:
//
// See above about endian issues.

#define	JU_COPY3_LONG_TO_PINDEX(PIndex,SourceLong)	\
	{						\
	    uint24_t temp;				\
	    temp._._t24_3byte = (SourceLong);		\
	    (PIndex)[0] = temp._._t24_3bytes[0];	\
	    (PIndex)[1] = temp._._t24_3bytes[1];	\
	    (PIndex)[2] = temp._._t24_3bytes[2];	\
	}


#ifdef JU_64BIT

// SIMILAR STRUCTS AND MACROS FOR ODD INDEX SIZES ON 64-BIT SYSTEMS:

typedef struct __FAKE_40BIT_UINT
	{
	    union
	    {
		Word_t  _t40_5byte:40;
		uint8_t _t40_5bytes[5];
	    } _;
	} uint40_t;

#define	JU_COPY5_PINDEX_TO_LONG(DestLong,PIndex)	\
	{						\
	    uint40_t temp;				\
	    temp._._t40_5bytes[0] = (PIndex)[0];	\
	    temp._._t40_5bytes[1] = (PIndex)[1];	\
	    temp._._t40_5bytes[2] = (PIndex)[2];	\
	    temp._._t40_5bytes[3] = (PIndex)[3];	\
	    temp._._t40_5bytes[4] = (PIndex)[4];	\
	    (DestLong) = temp._._t40_5byte;		\
	}

#define	JU_COPY5_LONG_TO_PINDEX(PIndex,SourceLong)	\
	{						\
	    uint40_t temp;				\
	    temp._._t40_5byte = (SourceLong);		\
	    (PIndex)[0] = temp._._t40_5bytes[0];	\
	    (PIndex)[1] = temp._._t40_5bytes[1];	\
	    (PIndex)[2] = temp._._t40_5bytes[2];	\
	    (PIndex)[3] = temp._._t40_5bytes[3];	\
	    (PIndex)[4] = temp._._t40_5bytes[4];	\
	}

typedef struct __FAKE_48BIT_UINT
	{
	    union
	    {
		Word_t  _t48_6byte:48;
		uint8_t _t48_6bytes[6];
	    } _;
	} uint48_t;

#define	JU_COPY6_PINDEX_TO_LONG(DestLong,PIndex)	\
	{						\
	    uint48_t temp;				\
	    temp._._t48_6bytes[0] = (PIndex)[0];	\
	    temp._._t48_6bytes[1] = (PIndex)[1];	\
	    temp._._t48_6bytes[2] = (PIndex)[2];	\
	    temp._._t48_6bytes[3] = (PIndex)[3];	\
	    temp._._t48_6bytes[4] = (PIndex)[4];	\
	    temp._._t48_6bytes[5] = (PIndex)[5];	\
	    (DestLong) = temp._._t48_6byte;		\
	}

#define	JU_COPY6_LONG_TO_PINDEX(PIndex,SourceLong)	\
	{						\
	    uint48_t temp;				\
	    temp._._t48_6byte = (SourceLong);		\
	    (PIndex)[0] = temp._._t48_6bytes[0];	\
	    (PIndex)[1] = temp._._t48_6bytes[1];	\
	    (PIndex)[2] = temp._._t48_6bytes[2];	\
	    (PIndex)[3] = temp._._t48_6bytes[3];	\
	    (PIndex)[4] = temp._._t48_6bytes[4];	\
	    (PIndex)[5] = temp._._t48_6bytes[5];	\
	}

typedef struct __FAKE_56BIT_UINT
	{
	    union
	    {
		Word_t  _t56_7byte:56;
		uint8_t _t56_7bytes[7];
	    } _;
	} uint56_t;

#define	JU_COPY7_PINDEX_TO_LONG(DestLong,PIndex)	\
	{						\
	    uint56_t temp;				\
	    temp._._t56_7bytes[0] = (PIndex)[0];	\
	    temp._._t56_7bytes[1] = (PIndex)[1];	\
	    temp._._t56_7bytes[2] = (PIndex)[2];	\
	    temp._._t56_7bytes[3] = (PIndex)[3];	\
	    temp._._t56_7bytes[4] = (PIndex)[4];	\
	    temp._._t56_7bytes[5] = (PIndex)[5];	\
	    temp._._t56_7bytes[6] = (PIndex)[6];	\
	    (DestLong) = temp._._t56_7byte;		\
	}

#define	JU_COPY7_LONG_TO_PINDEX(PIndex,SourceLong)	\
	{						\
	    uint56_t temp;				\
	    temp._._t56_7byte = (SourceLong);		\
	    (PIndex)[0] = temp._._t56_7bytes[0];	\
	    (PIndex)[1] = temp._._t56_7bytes[1];	\
	    (PIndex)[2] = temp._._t56_7bytes[2];	\
	    (PIndex)[3] = temp._._t56_7bytes[3];	\
	    (PIndex)[4] = temp._._t56_7bytes[4];	\
	    (PIndex)[5] = temp._._t56_7bytes[5];	\
	    (PIndex)[6] = temp._._t56_7bytes[6];	\
	}

#endif // JU_64BIT


// ****************************************************************************
// COMMON CODE FRAGMENTS (MACROS)
// ****************************************************************************
//
// These code chunks are shared between various source files.


// SET (REPLACE) ONE DIGIT IN AN INDEX:
//
// To avoid endian issues, use masking and ORing, which operates in a
// big-endian register, rather than treating the Index as an array of bytes,
// though that would be simpler, but would operate in endian-specific memory.
//
// TBD:  This contains two variable shifts, is that bad?

#define	JU_SETDIGIT(Index,Digit,State)			\
	(Index) = ((Index) & (~cJU_MASKATSTATE(State)))	\
			   | (((Word_t) (Digit))	\
			      << (((State) - 1) * cJU_BITSPERBYTE))

// Fast version for single LSB:

#define	JU_SETDIGIT1(Index,Digit) (Index) = ((Index) & ~0xff) | (Digit)


// SET (REPLACE) "N" LEAST DIGITS IN AN INDEX:

#define	JU_SETDIGITS(Index,Index2,cState) \
	(Index) = ((Index ) & (~JU_LEASTBYTESMASK(cState))) \
		| ((Index2) & ( JU_LEASTBYTESMASK(cState)))


// COPY DECODE BYTES FROM JP TO INDEX:
//
// Modify Index digit(s) to match the bytes in jp_DcdPop0 in case one or more
// branches are skipped and the digits are significant.  It's probably faster
// to just do this unconditionally than to check if it's necessary.
//
// To avoid endian issues, use masking and ORing, which operates in a
// big-endian register, rather than treating the Index as an array of bytes,
// though that would be simpler, but would operate in endian-specific memory.
//
// WARNING:  Must not call JU_LEASTBYTESMASK (via cJU_DCDMASK) with Bytes =
// cJU_ROOTSTATE or a bad mask is generated, but there are no Dcd bytes to copy
// in this case anyway.  In fact there are no Dcd bytes unless State <
// cJU_ROOTSTATE - 1, so don't call this macro except in those cases.
//
// TBD:  It would be nice to validate jp_DcdPop0 against known digits to ensure
// no corruption, but this is non-trivial.

#define	JU_SETDCD(Index,Dcd,cState) \
	(Index) = ((Index) & ~cJU_DCDMASK(cState)) \
		| ((Dcd)   &  cJU_DCDMASK(cState))


// INSERT/DELETE AN INDEX IN-PLACE IN MEMORY:
//
// Given a pointer to an array of "even" (native), same-sized objects
// (indexes), the current population of the array, an offset in the array, and
// a new Index to insert, "shift up" the array elements (Indexes) above the
// insertion point and insert the new Index.  Assume there is sufficient memory
// to do this.
//
// In these macros, "_ioffset" is an index offset, and "_boffset" is a byte
// offset for odd Index sizes.
//
// Note:  Endian issues only arise fro insertion, not deletion, and even for
// insertion, they are transparent when native (even) objects are used, and
// handled explicitly for odd (non-native) Index sizes.
//
// Note:  The following macros are tricky enough that there is some test code
// for them appended to this file.

#define	JU_INSERTINPLACE(PArray,Pop1,Offset,Index)		\
	assert((long) (Pop1) > 0);				\
	assert((Word_t) (Offset) <= (Word_t) (Pop1));		\
	{							\
	    Word_t _ioffset = (Pop1);				\
								\
	    while (_ioffset-- > (Offset))			\
		(PArray)[_ioffset + 1] = (PArray)[_ioffset];	\
								\
	    (PArray)[Offset] = (Index);				\
	}

// Variation for odd-byte-sized (non-native) Indexes, where cIS = Index Size
// and PByte must point to a uint8_t (byte); shift byte-by-byte:
//
// Note:  If cIS == 1, JU_INSERTINPLACE_ODD == JU_INSERTINPLACE, at least in
// concept.
//
// Note:  JU_INSERTINPLACE_ODD() is hidden common code; use
// JU_INSERTINPLACE3(), etc.
//
// Note:  To handle endian issues, these macros pass Indexes through odd-index
// structs defined elsewhere.
//
// TBD:  Would it be more efficient for big-endian systems to use JU_BYTEORDER
// to avoid going through a struct?

#define	JU_INSERTINPLACE_ODD(PByte,Pop1,Offset,Index,			\
			     cIS, Struct_t, Field_bytes, Field_int)	\
	assert((long) (Pop1) > 0);					\
	assert((Word_t) (Offset) <= (Word_t) (Pop1));			\
	{								\
	    Struct_t  _temp;						\
	    uint8_t * _Pindex  = &(_temp._.Field_bytes[0]);		\
	    Word_t    _boffset = (((Pop1) + 1) * (cIS)) - 1;		\
									\
	    _temp._.Field_int = (Index);				\
									\
	    while (_boffset-- > (((Offset) + 1) * (cIS)) - 1)		\
		(PByte)[_boffset + 1] = (PByte)[_boffset + 1 - (cIS)];	\
									\
	    for (_boffset = (Offset) * (cIS);				\
		 _boffset < ((Offset) + 1) * (cIS);			\
		 ++_boffset)						\
	    {								\
		(PByte)[_boffset] = *_Pindex++;				\
	    }								\
	}

#define	JU_INSERTINPLACE3(   PByte,Pop1,Offset,Index) \
	JU_INSERTINPLACE_ODD(PByte,Pop1,Offset,Index, \
			     3, uint24_t, _t24_3bytes, _t24_3byte)

#ifdef JU_64BIT
#define	JU_INSERTINPLACE5(   PByte,Pop1,Offset,Index) \
	JU_INSERTINPLACE_ODD(PByte,Pop1,Offset,Index, \
			     5, uint40_t, _t40_5bytes, _t40_5byte)

#define	JU_INSERTINPLACE6(   PByte,Pop1,Offset,Index) \
	JU_INSERTINPLACE_ODD(PByte,Pop1,Offset,Index, \
			     6, uint48_t, _t48_6bytes, _t48_6byte)

#define	JU_INSERTINPLACE7(   PByte,Pop1,Offset,Index) \
	JU_INSERTINPLACE_ODD(PByte,Pop1,Offset,Index, \
			     7, uint56_t, _t56_7bytes, _t56_7byte)
#endif

// Counterparts to the above for deleting an Index:
//
// "Shift down" the array elements starting at the Index to be deleted.

#define	JU_DELETEINPLACE(PArray,Pop1,Offset,ignore)		\
	assert((long) (Pop1) > 0);				\
	assert((Word_t) (Offset) < (Word_t) (Pop1));		\
	{							\
	    Word_t _ioffset = (Offset);				\
								\
	    while (++_ioffset < (Pop1))				\
		(PArray)[_ioffset - 1] = (PArray)[_ioffset];	\
	}

// Variation for odd-byte-sized (non-native) Indexes, where cIS = Index Size
// and PByte must point to a uint8_t (byte); copy byte-by-byte:
//
// Note:  If cIS == 1, JU_DELETEINPLACE_ODD == JU_DELETEINPLACE.
//
// Note:  There are no endian issues here because bytes are just shifted as-is,
// not converted to/from an Index.

#define	JU_DELETEINPLACE_ODD(PByte,Pop1,Offset,cIS)		\
	assert((long) (Pop1) > 0);				\
	assert((Word_t) (Offset) < (Word_t) (Pop1));		\
	{							\
	    Word_t _boffset = (((Offset) + 1) * (cIS)) - 1;	\
								\
	    while (++_boffset < ((Pop1) * (cIS)))		\
		(PByte)[_boffset - (cIS)] = (PByte)[_boffset];	\
	}


// INSERT/DELETE AN INDEX WHILE COPYING OTHERS:
//
// Copy PSource[] to PDest[], where PSource[] has Pop1 elements (Indexes),
// inserting Index at PDest[Offset].  Unlike JU_*INPLACE*() above, these macros
// are used when moving Indexes from one memory object to another.
//
// Note:  See above about endian issues.

#define	JU_INSERTCOPY(PDest,PSource,Pop1,Offset,Index)		\
	assert((long) (Pop1) > 0);				\
	assert((Word_t) (Offset) <= (Word_t) (Pop1));		\
	{							\
	    Word_t _ioffset;					\
								\
	    for (_ioffset = 0; _ioffset < (Offset); ++_ioffset)	\
		(PDest)[_ioffset] = (PSource)[_ioffset];	\
								\
	    (PDest)[_ioffset] = (Index);			\
								\
	    for (/* null */; _ioffset < (Pop1); ++_ioffset)	\
		(PDest)[_ioffset + 1] = (PSource)[_ioffset];	\
	}

// Variation for odd-byte-sized (non-native) Indexes, where cIS = Index Size;
// copy byte-by-byte:
//
// Note:  If cIS == 1, JU_INSERTCOPY_ODD == JU_INSERTCOPY, at least in concept.
//
// Note:  JU_INSERTCOPY_ODD() is hidden common code; use JU_INSERTCOPY3(), etc.
//
// Note:  To handle endian issues, these macros pass Indexes through odd-index
// structs defined elsewhere.
//
// TBD:  Would it be more efficient for big-endian systems to use JU_BYTEORDER
// to avoid going through a struct?

#define	JU_INSERTCOPY_ODD(PDest,PSource,Pop1,Offset,Index,		\
			  cIS, Struct_t, Field_bytes, Field_int)	\
	assert((long) (Pop1) > 0);					\
	assert((Word_t) (Offset) <= (Word_t) (Pop1));			\
	{								\
	    Struct_t  _temp;						\
	    uint8_t * _Pdest   = (uint8_t *) (PDest);			\
	    uint8_t * _Psource = (uint8_t *) (PSource);			\
	    uint8_t * _Pindex  = &(_temp._.Field_bytes[0]);		\
	    Word_t    _boffset;						\
	    Word_t    _boffset2;					\
									\
	    _temp._.Field_int = (Index);				\
									\
	    for (_boffset = 0; _boffset < ((Offset) * (cIS)); ++_boffset) \
		*_Pdest++ = *_Psource++;				\
									\
	    for (_boffset2 = 0; _boffset2 < (cIS); ++_boffset2)		\
		*_Pdest++ = *_Pindex++;					\
									\
	    for (/* null */; _boffset < ((Pop1) * (cIS)); ++_boffset)	\
		*_Pdest++ = *_Psource++;				\
	}

#define	JU_INSERTCOPY3(   PDest,PSource,Pop1,Offset,Index)	\
	JU_INSERTCOPY_ODD(PDest,PSource,Pop1,Offset,Index,	\
			  3, uint24_t, _t24_3bytes, _t24_3byte)

#ifdef JU_64BIT
#define	JU_INSERTCOPY5(   PDest,PSource,Pop1,Offset,Index)	\
	JU_INSERTCOPY_ODD(PDest,PSource,Pop1,Offset,Index,	\
			  5, uint40_t, _t40_5bytes, _t40_5byte)

#define	JU_INSERTCOPY6(   PDest,PSource,Pop1,Offset,Index)	\
	JU_INSERTCOPY_ODD(PDest,PSource,Pop1,Offset,Index,	\
			  6, uint48_t, _t48_6bytes, _t48_6byte)

#define	JU_INSERTCOPY7(   PDest,PSource,Pop1,Offset,Index)	\
	JU_INSERTCOPY_ODD(PDest,PSource,Pop1,Offset,Index,	\
			  7, uint56_t, _t56_7bytes, _t56_7byte)
#endif

// Counterparts to the above for deleting an Index:

#define	JU_DELETECOPY(PDest,PSource,Pop1,Offset,ignore)		\
	assert((long) (Pop1) > 0);				\
	assert((Word_t) (Offset) < (Word_t) (Pop1));		\
	{							\
	    Word_t _ioffset;					\
								\
	    for (_ioffset = 0; _ioffset < (Offset); ++_ioffset)	\
		(PDest)[_ioffset] = (PSource)[_ioffset];	\
								\
	    for (++_ioffset; _ioffset < (Pop1); ++_ioffset)	\
		(PDest)[_ioffset - 1] = (PSource)[_ioffset];	\
	}

// Variation for odd-byte-sized (non-native) Indexes, where cIS = Index Size;
// copy byte-by-byte:
//
// Note:  There are no endian issues here because bytes are just shifted as-is,
// not converted to/from an Index.
//
// Note:  If cIS == 1, JU_DELETECOPY_ODD == JU_DELETECOPY, at least in concept.

#define	JU_DELETECOPY_ODD(PDest,PSource,Pop1,Offset,cIS)		\
	assert((long) (Pop1) > 0);					\
	assert((Word_t) (Offset) < (Word_t) (Pop1));			\
	{								\
	    uint8_t *_Pdest   = (uint8_t *) (PDest);			\
	    uint8_t *_Psource = (uint8_t *) (PSource);			\
	    Word_t   _boffset;						\
									\
	    for (_boffset = 0; _boffset < ((Offset) * (cIS)); ++_boffset) \
		*_Pdest++ = *_Psource++;				\
									\
	    _Psource += (cIS);						\
									\
	    for (_boffset += (cIS); _boffset < ((Pop1) * (cIS)); ++_boffset) \
		*_Pdest++ = *_Psource++;				\
	}


// GENERIC RETURN CODE HANDLING FOR JUDY1 (NO VALUE AREAS) AND JUDYL (VALUE
// AREAS):
//
// This common code hides Judy1 versus JudyL details of how to return various
// conditions, including a pointer to a value area for JudyL.
//
// First, define an internal variation of JERR called JERRI (I = int) to make
// lint happy.  We accidentally shipped to 11.11 OEUR with all functions that
// return int or Word_t using JERR, which is type Word_t, for errors.  Lint
// complains about this for functions that return int.  So, internally use
// JERRI for error returns from the int functions.  Experiments show that
// callers which compare int Foo() to (Word_t) JERR (~0UL) are OK, since JERRI
// sign-extends to match JERR.

#define	JERRI ((int) ~0)		// see above.

#ifdef JUDY1

#define	JU_RET_FOUND	return(1)
#define	JU_RET_NOTFOUND	return(0)

// For Judy1, these all "fall through" to simply JU_RET_FOUND, since there is no
// value area pointer to return:

#define	JU_RET_FOUND_JAPLEAF(Pjlw,Pop1,Offset)	JU_RET_FOUND

#define	JU_RET_FOUND_JPM(Pjpm)			JU_RET_FOUND
#define	JU_RET_FOUND_PVALUE(Pjv,Offset)		JU_RET_FOUND
#ifndef JU_64BIT
#define	JU_RET_FOUND_LEAF1(Pjll,Pop1,Offset)	JU_RET_FOUND
#endif
#define	JU_RET_FOUND_LEAF2(Pjll,Pop1,Offset)	JU_RET_FOUND
#define	JU_RET_FOUND_LEAF3(Pjll,Pop1,Offset)	JU_RET_FOUND
#ifdef JU_64BIT
#define	JU_RET_FOUND_LEAF4(Pjll,Pop1,Offset)	JU_RET_FOUND
#define	JU_RET_FOUND_LEAF5(Pjll,Pop1,Offset)	JU_RET_FOUND
#define	JU_RET_FOUND_LEAF6(Pjll,Pop1,Offset)	JU_RET_FOUND
#define	JU_RET_FOUND_LEAF7(Pjll,Pop1,Offset)	JU_RET_FOUND
#endif
#define	JU_RET_FOUND_IMM_01(Pjp)		JU_RET_FOUND
#define	JU_RET_FOUND_IMM(Pjp,Offset)		JU_RET_FOUND

// Note:  No JudyL equivalent:

#define	JU_RET_FOUND_FULLPOPU1			 JU_RET_FOUND
#define	JU_RET_FOUND_LEAF_B1(Pjlb,Subexp,Offset) JU_RET_FOUND

#else // JUDYL

//	JU_RET_FOUND		// see below; must NOT be defined for JudyL.
#define	JU_RET_NOTFOUND	return((PPvoid_t) NULL)

// For JudyL, the location of the value area depends on the JP type and other
// factors:
//
// TBD:  The value areas should be accessed via data structures, here and in
// Doug's code, not by hard-coded address calculations.
//
// This is useful in insert/delete code when the value area is returned from
// lower levels in the JPM:

#define	JU_RET_FOUND_JPM(Pjpm)	return((PPvoid_t) ((Pjpm) -> jpm_PValue))

// This is useful in insert/delete code when the value area location is already
// computed:

#define	JU_RET_FOUND_PVALUE(Pjv,Offset) return((PPvoid_t) ((Pjv) + Offset))

#define	JU_RET_FOUND_JAPLEAF(Pjlw,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAFWVALUEAREA(Pjlw, Pop1) + (Offset)))

#define	JU_RET_FOUND_LEAF1(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF1VALUEAREA(Pjll, Pop1) + (Offset)))
#define	JU_RET_FOUND_LEAF2(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF2VALUEAREA(Pjll, Pop1) + (Offset)))
#define	JU_RET_FOUND_LEAF3(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF3VALUEAREA(Pjll, Pop1) + (Offset)))
#ifdef JU_64BIT
#define	JU_RET_FOUND_LEAF4(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF4VALUEAREA(Pjll, Pop1) + (Offset)))
#define	JU_RET_FOUND_LEAF5(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF5VALUEAREA(Pjll, Pop1) + (Offset)))
#define	JU_RET_FOUND_LEAF6(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF6VALUEAREA(Pjll, Pop1) + (Offset)))
#define	JU_RET_FOUND_LEAF7(Pjll,Pop1,Offset) \
		return((PPvoid_t) (JL_LEAF7VALUEAREA(Pjll, Pop1) + (Offset)))
#endif

// Note:  Here jp_Addr is a value area itself and not an address, so P_JV() is
// not needed:

#define	JU_RET_FOUND_IMM_01(Pjp)  return((PPvoid_t) (&((Pjp) -> jp_Addr)))

// Note:  Here jp_Addr is a pointer to a separately-malloc'd value area, so
// P_JV() is required; likewise for JL_JLB_PVALUE:

#define	JU_RET_FOUND_IMM(Pjp,Offset) \
	    return((PPvoid_t) (P_JV((Pjp)->jp_Addr) + (Offset)))

#define	JU_RET_FOUND_LEAF_B1(Pjlb,Subexp,Offset) \
	    return((PPvoid_t) (P_JV(JL_JLB_PVALUE(Pjlb, Subexp)) + (Offset)))

#endif // JUDYL


// GENERIC ERROR HANDLING:
//
// This is complicated by variations in the needs of the callers of these
// macros.  Only use JU_SET_ERRNO() for PJError, because it can be null; use
// JU_SET_ERRNO_NONNULL() for Pjpm, which is never null, and also in other
// cases where the pointer is known not to be null (to save dead branches).
//
// Note:  Most cases of JU_ERRNO_OVERRUN or JU_ERRNO_CORRUPT should result in
// an assertion failure in debug code, so they are more likely to be caught, so
// do that here in each macro.

#define	JU_SET_ERRNO(PJError, JErrno)			\
	{						\
	    assert((JErrno) != JU_ERRNO_OVERRUN);	\
	    assert((JErrno) != JU_ERRNO_CORRUPT);	\
							\
	    if (PJError != (PJError_t) NULL)		\
	    {						\
		JU_ERRNO(PJError) = (JErrno);		\
		JU_ERRID(PJError) = __LINE__;		\
	    }						\
	}

// Variation for callers who know already that PJError is non-null; and, it can
// also be Pjpm (both PJError_t and Pjpm_t have je_* fields), so only assert it
// for null, not cast to any specific pointer type:

#define	JU_SET_ERRNO_NONNULL(PJError, JErrno)		\
	{						\
	    assert((JErrno) != JU_ERRNO_OVERRUN);	\
	    assert((JErrno) != JU_ERRNO_CORRUPT);	\
	    assert(PJError);				\
							\
	    JU_ERRNO(PJError) = (JErrno);		\
	    JU_ERRID(PJError) = __LINE__;		\
	}

// Variation to copy error info from a (required) JPM to an (optional)
// PJError_t:
//
// Note:  The assertions above about JU_ERRNO_OVERRUN and JU_ERRNO_CORRUPT
// should have already popped, so they are not needed here.

#define	JU_COPY_ERRNO(PJError, Pjpm)			\
	{						\
	    if (PJError)				\
	    {						\
		JU_ERRNO(PJError) = JU_ERRNO(Pjpm);	\
		JU_ERRID(PJError) = JU_ERRID(Pjpm);	\
	    }						\
	}

// For JErrno parameter to previous macros upon return from Judy*Alloc*():
//
// The memory allocator returns an address of 0 for out of memory,
// 1..sizeof(Word_t)-1 for corruption (an invalid pointer), otherwise a valid
// pointer.

#define	JU_ALLOC_ERRNO(Addr) \
	(((void *) (Addr) != (void *) NULL) ? JU_ERRNO_OVERRUN : JU_ERRNO_NOMEM)

#define	JU_CHECKALLOC(Type,Ptr,Retval)			\
	if ((Ptr) < (Type) sizeof(Word_t))		\
	{						\
	    JU_SET_ERRNO(PJError, JU_ALLOC_ERRNO(Ptr));	\
	    return(Retval);				\
	}


// ****************************************************************************
// SUPPORT FOR LEAF SEARCHING:
// ****************************************************************************
//
// These macros are shared by JudySearchLeaf.c and JudyPrevNextEmpty.c only so
// they lack noisy JU_* prefixes.

// BYTES PER INDEX (BPI), just for clarity:

#define	BPI1  1
#define	BPI2  2
#define	BPI3  3
#ifdef JU_64BIT
#define	BPI4  4
#define	BPI5  5
#define	BPI6  6
#define	BPI7  7
#endif
#define	BPIL  (sizeof(Word_t))


// SHORTHAND NAMES FOR MASKS:

#define	MASK1  JU_LEASTBYTESMASK(BPI1)
#define	MASK2  JU_LEASTBYTESMASK(BPI2)
#define	MASK3  JU_LEASTBYTESMASK(BPI3)
#ifdef JU_64BIT
#define	MASK4  JU_LEASTBYTESMASK(BPI4)
#define	MASK5  JU_LEASTBYTESMASK(BPI5)
#define	MASK6  JU_LEASTBYTESMASK(BPI6)
#define	MASK7  JU_LEASTBYTESMASK(BPI7)
#endif
// No need for MASKL.


// SUPPORT FOR ODD-SIZED INDEXES:
//
// An index size of 3 bytes, and for 64-bit only, 5, 6, and 7 bytes, are "odd
// sized" because they do not have corresponding native C data types and do not
// fit neatly into a single word.  For efficiency, the leaf search code scans
// these odd indexes in "index groups" that fit in whole words.

// INDEXES PER INDEX GROUP (IPG) for odd-sized indexes:  The simple way to
// calculate the number of indexes per index group is BPI(N) * cJU_BYTESPERWORD
// / BPI(N), or essentially just cJU_BYTESPERWORD.  However, for BPI6, 6 * 8 =
// 48 bytes (6 words, 8 indexes), but there is a common prime factor of 2, so
// 24 bytes (3 words, 4 indexes) is better because it's smaller.

#ifndef JU_64BIT
#define	IPG3  4		// indexes per group (fit in whole words).
#else
#define	IPG3  8
#define	IPG5  8
#define	IPG6  4		// see above.
#define	IPG7  8
#endif

// WORDS PER INDEX GROUP (WPG) for odd-sized indexes:

#define	WPG3  ((IPG3 * BPI3) / cJU_BYTESPERWORD)	// 3 [3].
#ifdef JU_64BIT
#define	WPG5  ((IPG5 * BPI5) / cJU_BYTESPERWORD)	//   [5].
#define	WPG6  ((IPG6 * BPI6) / cJU_BYTESPERWORD)	//   [3].
#define	WPG7  ((IPG7 * BPI7) / cJU_BYTESPERWORD)	//   [7].
#endif

// INDEXES PER CACHE LINE (IPC) for odd-sized indexes, really the number of
// indexes within the maximum number of whole (word-aligned) index groups per
// "virtual" cache line, which is less than or equal in size to a real cache
// line size:
//
// Note:  For odd-sized indexes these IPC's are less than the actual indexes
// per cache line, sometimes less even the number of whole indexes per cache
// line, due to word-aligned grouping.

#define	IPC1   (cJU_BYTESPERCL / 1)			// 64 [128].
#define	IPC2   (cJU_BYTESPERCL / 2)			// 32  [64].
#define	IPC3  ((cJU_BYTESPERCL / (IPG3 * BPI3)) * IPG3)	// 20  [40].
#ifdef JU_64BIT
#define	IPC4   (cJU_BYTESPERCL / 4)			//     [32].
#define	IPC5  ((cJU_BYTESPERCL / (IPG5 * BPI5)) * IPG5)	//     [24].
#define	IPC6  ((cJU_BYTESPERCL / (IPG6 * BPI6)) * IPG6)	//     [20].
#define	IPC7  ((cJU_BYTESPERCL / (IPG7 * BPI7)) * IPG7)	//     [16].
#endif
#define	IPCL   (cJU_BYTESPERCL / sizeof(Word_t))	// 16  [16].


// MACROS FOR EFFICIENTLY EXTRACTING ALIGNED ODD-SIZE INDEXES FROM A LEAF:
//
// Doing shifts, ORs, and masks in registers is presumably faster than
// rebuilding odd-sized indexes one byte at a time.  However, it makes the code
// unavoidably torturous as it works through all the 32/64-bit and
// little/big-endian permutations efficiently.  (Trading longer coding time and
// code size for faster execution.)
//
// The GET3_*() macros hide the details of how to extract bytes "cba"
// (big-endian order) for index 0..3 [0..7] in an index group, given Pword
// pointing to the first (unsigned long) word of a group.

#ifndef JU_64BIT
#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Index bytes for indexes 0-3 look like this (big-endian):
//
//	0001 1122 2333			// in memory or registers.
//	cbac bacb acba

#define	GET3_0	  (Pword[0] >>  8)
#define	GET3_1	(((Pword[0] << 16) | (Pword[1] >> 16)) & MASK3)
#define	GET3_2	(((Pword[1] <<  8) | (Pword[2] >> 24)) & MASK3)
#define	GET3_3	  (Pword[2]			       & MASK3)

#endif // JU_BIG_ENDIAN

#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Index bytes for indexes 0-3 look like this (little-endian):
//
//	0001 1122 2333			// in memory.
//	abca bcab cabc
//
//	1000 2211 3332			// in registers.
//	acba bacb cbac			// big-endian Indexes.

#define	GET3_0	  (Pword[0]			       & MASK3)
#define	GET3_1	(((Pword[0] >> 24) | (Pword[1] <<  8)) & MASK3)
#define	GET3_2	(((Pword[1] >> 16) | (Pword[2] << 16)) & MASK3)
#define	GET3_3	  (Pword[2] >>  8)

#endif // JU_LITTLE_ENDIAN

#define	GET3_LIG GET3_3			// last index in index group.

#else  // JU_64BIT

#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Index bytes for indexes 0-7 look like this (big-endian):
//
//	00011122 23334445 55666777	// in memory or registers.
//	cbacbacb acbacbac bacbacba

#define	GET3_0	  (Pword[0] >> 40)
#define	GET3_1	 ((Pword[0] >> 16)		       & MASK3)
#define	GET3_2	(((Pword[0] <<  8) | (Pword[1] >> 56)) & MASK3)
#define	GET3_3	 ((Pword[1] >> 32)		       & MASK3)
#define	GET3_4	 ((Pword[1] >>  8)		       & MASK3)
#define	GET3_5	(((Pword[1] << 16) | (Pword[2] >> 48)) & MASK3)
#define	GET3_6	 ((Pword[2] >> 24)		       & MASK3)
#define	GET3_7	  (Pword[2]			       & MASK3)

#endif // JU_BIG_ENDIAN

#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Index bytes for indexes 0-7 look like this (little-endian):
//
//	00011122 23334445 55666777	// in memory.
//	abcabcab cabcabca bcabcabc
//
//	22111000 54443332 77766655	// in registers.
//	bacbacba acbacbac cbacbacb	// big-endian Indexes.

#define	GET3_0	  (Pword[0]			       & MASK3)
#define	GET3_1	 ((Pword[0] >> 24)		       & MASK3)
#define	GET3_2	(((Pword[0] >> 48) | (Pword[1] << 16)) & MASK3)
#define	GET3_3	 ((Pword[1] >>  8)		       & MASK3)
#define	GET3_4	 ((Pword[1] >> 32)		       & MASK3)
#define	GET3_5	(((Pword[1] >> 56) | (Pword[2] <<  8)) & MASK3)
#define	GET3_6	 ((Pword[2] >> 16)		       & MASK3)
#define	GET3_7	  (Pword[2] >> 40)

#endif // JU_LITTLE_ENDIAN

#define	GET3_LIG GET3_7			// last index in index group.

#endif // JU_64BIT


#ifdef JU_64BIT

// The GET5_* macros are similar to GET3_*() for larger index sizes:

#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Index bytes for indexes 0-7 look like this (big-endian):
//
//	00000111 11222223 33334444 45555566 66677777  // in memory or registers.
//	edcbaedc baedcbae dcbaedcb aedcbaed cbaedcba

#define	GET5_0	  (Pword[0] >> 24)
#define	GET5_1	(((Pword[0] << 16) | (Pword[1] >> 48)) & MASK5)
#define	GET5_2	 ((Pword[1] >>  8)		       & MASK5)
#define	GET5_3	(((Pword[1] << 32) | (Pword[2] >> 32)) & MASK5)
#define	GET5_4	(((Pword[2] <<  8) | (Pword[3] >> 56)) & MASK5)
#define	GET5_5	 ((Pword[3] >> 16)		       & MASK5)
#define	GET5_6	(((Pword[3] << 24) | (Pword[4] >> 40)) & MASK5)
#define	GET5_7	  (Pword[4]			       & MASK5)

#endif // JU_BIG_ENDIAN

#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Index bytes for indexes 0-7 look like this (little-endian):
//
//	00000111 11222223 33334444 45555566 66677777  // in memory.
//	abcdeabc deabcdea bcdeabcd eabcdeab cdeabcde
//
//	11100000 32222211 44443333 66555554 77777666  // in registers.
//	cbaedcba aedcbaed dcbaedcb daedcbae edcbaedc  // big-endian Indexes.

#define	GET5_0	  (Pword[0]			       & MASK5)
#define	GET5_1	(((Pword[0] >> 40) | (Pword[1] << 24)) & MASK5)
#define	GET5_2	 ((Pword[1] >> 16)		       & MASK5)
#define	GET5_3	(((Pword[1] >> 56) | (Pword[2] <<  8)) & MASK5)
#define	GET5_4	(((Pword[2] >> 32) | (Pword[3] << 32)) & MASK5)
#define	GET5_5	 ((Pword[3] >>  8)		       & MASK5)
#define	GET5_6	(((Pword[3] >> 48) | (Pword[4] << 16)) & MASK5)
#define	GET5_7	  (Pword[4] >> 24)

#endif // JU_LITTLE_ENDIAN

#define	GET5_LIG GET5_7			// last index in index group.


// The GET6_* macros are similar to GET3_*() for larger index sizes:

#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Index bytes for indexes 0-7 look like this (big-endian):
//
//	00000011 11112222 22333333	// in memory or registers.
//	fedcbafe dcbafedc bafedcba

#define	GET6_0	  (Pword[0] >> 16)
#define	GET6_1	(((Pword[0] << 32) | (Pword[1] >> 32)) & MASK6)
#define	GET6_2	(((Pword[1] << 16) | (Pword[2] >> 48)) & MASK6)
#define	GET6_3	  (Pword[2]			       & MASK6)

#endif // JU_BIG_ENDIAN

#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Index bytes for indexes 0-7 look like this (little-endian):
//
//	00000011 11112222 22333333	// in memory.
//	abcdefab cdefabcd efabcdef
//
//	11000000 22221111 33333322	// in registers.
//	bafedcba dcbafedc fedcbafe	// big-endian Indexes.

#define	GET6_0	  (Pword[0]			       & MASK6)
#define	GET6_1	(((Pword[0] >> 48) | (Pword[1] << 16)) & MASK6)
#define	GET6_2	(((Pword[1] >> 32) | (Pword[2] << 32)) & MASK6)
#define	GET6_3	  (Pword[2] >> 16)

#endif // JU_LITTLE_ENDIAN

#define	GET6_LIG GET6_3			// last index in index group.


// The GET7_* macros are similar to GET3_*() for larger index sizes:

#if (JU_BYTEORDER == JU_BIG_ENDIAN)

// Index bytes for indexes 0-7 look like this (big-endian) (in memory or
// registers):
//
//	00000001 11111122 22222333 33334444 44455555 55666666 67777777
//	gfedcbag fedcbagf edcbagfe dcbagfed cbagfedc bagfedcb agfedcba

#define	GET7_0	  (Pword[0] >>  8)
#define	GET7_1	(((Pword[0] << 48) | (Pword[1] >> 16)) & MASK7)
#define	GET7_2	(((Pword[1] << 40) | (Pword[2] >> 24)) & MASK7)
#define	GET7_3	(((Pword[2] << 32) | (Pword[3] >> 32)) & MASK7)
#define	GET7_4	(((Pword[3] << 24) | (Pword[4] >> 40)) & MASK7)
#define	GET7_5	(((Pword[4] << 16) | (Pword[5] >> 48)) & MASK7)
#define	GET7_6	(((Pword[5] <<  8) | (Pword[6] >> 56)) & MASK7)
#define	GET7_7	  (Pword[6]			       & MASK7)

#endif // JU_BIG_ENDIAN

#if (JU_BYTEORDER == JU_LITTLE_ENDIAN)

// Index bytes for indexes 0-7 look like this (little-endian) (in memory, then
// in registers, with big-endian Indexes):
//
//	00000001 11111122 22222333 33334444 44455555 55666666 67777777
//	abcdefga bcdefgab cdefgabc defgabcd efgabcde fgabcdef gabcdefg
//
//	10000000 22111111 33322222 44443333 55555444 66666655 77777776
//	agfedcba bagfedcb cbagfedc dcbagfed edcbagfe fedcbagf gfedcbag

#define	GET7_0	  (Pword[0]			       & MASK7)
#define	GET7_1	(((Pword[0] >> 56) | (Pword[1] <<  8)) & MASK7)
#define	GET7_2	(((Pword[1] >> 48) | (Pword[2] << 16)) & MASK7)
#define	GET7_3	(((Pword[2] >> 40) | (Pword[3] << 24)) & MASK7)
#define	GET7_4	(((Pword[3] >> 32) | (Pword[4] << 32)) & MASK7)
#define	GET7_5	(((Pword[4] >> 24) | (Pword[5] << 40)) & MASK7)
#define	GET7_6	(((Pword[5] >> 16) | (Pword[6] << 48)) & MASK7)
#define	GET7_7	  (Pword[6] >>  8)

#endif // JU_LITTLE_ENDIAN

#define	GET7_LIG GET7_7			// last index in index group.

#endif // JU_64BIT

#endif // ! _JUDYPRIVATE_INCLUDED
