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

// @(#) $Revision: 4.35 $ $Source: /judy/src/JudyCommon/JudySearchLeaf.c $

// At least in some contexts, this source file is #include'd by others for
// inlining, so ensure it only happens once:

#ifndef _JUDYSEARCHLEAF
#define _JUDYSEARCHLEAF

// JUDY LINEAR LEAF SEARCH AND BIT COUNTING FUNCTIONS.
//
// TBD:  Despite the names of some of these functions, they only search linear
// leaves, not bitmap leaves; and they are also used to search the index list
// portion of linear branches (Branch2..7), and one-byte immediates (Immed1*).
// Consider renaming them to __JudySearchLin*().
//
// Search function return values:
//
// All the search functions below return the position (offset, base 0) in the
// source array of the index passed in.  If the index is not found, the return
// value is the the ones-complement of the sought index's ideal position.  For
// example:
//
//   array: 10 20 30 40 50
//
//      Index to search for        returned value
//      -------------------        --------------
//              10                       0
//              20                       1
//              30                       2
//               5                      ~0  (that is, -1)
//              15                      ~1
//             100                      ~5
//
// The caller should check for result >= 0 for a match.  To get the position
// (offset) in which an absent index belongs, just take the ones-complement
// again.


#include "JudyPrivate.h"
#include "JudyPrivateBranch.h"

DBGCODE(void JudyCheckSorted(Pjll_t Pjll, Word_t Pop1, long IndexSize);)

// Leaf search function return types here depend on compilation context:

#ifdef COMPILE_INLINE
#define RETURN_TYPE static int
#else
#define RETURN_TYPE int
#endif

#ifdef JU_HPUX

// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   1
//
// Search a 1-byte-Index linear leaf (cJ*_JPLEAF1) for the specified Index.
// The return value is described in file header comments.
//
// Note:  Although Judy1 on 64-bit systems does not have Leaf1's, this function
// is also used to search BranchL's and Immed1*'s, as noted in file header
// comments.

FUNCTION RETURN_TYPE __JudySearchLeaf1(
	Pjll_t  Pjll,		// to linear Leaf1 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
	Index &= MASK1;		// look for last BPI1 bytes only.

#undef	PLEAF_T			// in case previously set.
#define	PLEAF_T uint8_t *	// for JudySearchLeafEven.c.

#undef	BPI
#define	BPI BPI1

#undef	IPC
#define	IPC IPC1

#include "JudySearchLeafEven.c"

} // __JudySearchLeaf1()


// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   2
//
// Search a 2-byte-Index linear leaf (cJ*_JPLEAF2, or the index list in a
// cJ*_JPBRANCH_L2) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf2(
	Pjll_t Pjll,		// to linear Leaf2 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
	Index &= MASK2;		// look for last BPI2 bytes only.

#undef	PLEAF_T			// in case previously set.
#define	PLEAF_T uint16_t *	// for JudySearchLeafEven.c.

#undef	BPI
#define	BPI BPI2

#undef	IPC
#define	IPC IPC2

#include "JudySearchLeafEven.c"

} // __JudySearchLeaf2()


#endif // JU_HPUX

// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   3
//
// Search a 3-byte-Index linear leaf (cJ*_JPLEAF3, or the index list in a
// cJ*_JPBRANCH_L3) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf3(
	Pjll_t Pjll,		// to linear Leaf3 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
#undef	BPI			// in case previously set.
#undef	IPC
#undef	IPG
#undef	WPG
#undef	MASK
#define	BPI  BPI3		// for JudySearchLeafOdd.c.
#define	IPC  IPC3
#define	IPG  IPG3
#define	WPG  WPG3
#define	MASK MASK3

#undef	GET_0
#undef	GET_1
#undef	GET_2
#undef	GET_3
#undef	GET_4
#undef	GET_5
#undef	GET_6
#undef	GET_LIG
#define	GET_0	GET3_0
#define	GET_1	GET3_1
#define	GET_2	GET3_2
#ifdef JU_64BIT
#define	GET_3	GET3_3
#define	GET_4	GET3_4
#define	GET_5	GET3_5
#define	GET_6	GET3_6
#endif
#define	GET_LIG	GET3_LIG

// Unfortunately IPG itself is an expression the compiler can't seem to handle,
// so define IPG_BIG here explicitly, though this contains knowledge of IPG
// values:

#undef	IPG_BIG
#ifdef JU_64BIT
#define	IPG_BIG 1			// set to 1 for "#if".
#endif

#undef	JU_COPY_PINDEX_TO_LONG
#define	JU_COPY_PINDEX_TO_LONG(LI,PI) JU_COPY3_PINDEX_TO_LONG(LI,PI)

#include "JudySearchLeafOdd.c"

} // __JudySearchLeaf3()

#ifdef JU_64BIT


#ifdef JU_HPUX

// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   4
//
// Search a 4-byte-Index linear leaf (cJ*_JPLEAF4, or the index list in a
// cJ*_JPBRANCH_L4) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf4(
	Pjll_t Pjll,		// to linear Leaf4 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
	Index &= MASK4;		// look for last BPI4 bytes only.

#undef	PLEAF_T			// in case previously set.
#define	PLEAF_T uint32_t *	// for JudySearchLeafEven.c.

#undef	BPI
#define	BPI BPI4

#undef	IPC
#define	IPC IPC4

#include "JudySearchLeafEven.c"

} // __JudySearchLeaf4()

#endif // JU_HPUX


// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   5
//
// Search a 5-byte-Index linear leaf (cJ*_JPLEAF5, or the index list in a
// cJ*_JPBRANCH_L5) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf5(
	Pjll_t Pjll,		// to linear Leaf5 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
#undef	BPI			// in case previously set.
#undef	IPC
#undef	IPG
#undef	WPG
#undef	MASK
#define	BPI  BPI5		// for JudySearchLeafOdd.c.
#define	IPC  IPC5
#define	IPG  IPG5
#define	WPG  WPG5
#define	MASK MASK5

#undef	GET_0
#undef	GET_1
#undef	GET_2
#undef	GET_3
#undef	GET_4
#undef	GET_5
#undef	GET_6
#undef	GET_LIG
#define	GET_0	GET5_0
#define	GET_1	GET5_1
#define	GET_2	GET5_2
#define	GET_3	GET5_3
#define	GET_4	GET5_4
#define	GET_5	GET5_5
#define	GET_6	GET5_6
#define	GET_LIG	GET5_LIG

// Unfortunately IPG itself is an expression the compiler can't seem to handle,
// so define IPG_BIG here explicitly, though this contains knowledge of IPG
// values:

#undef	IPG_BIG
#define	IPG_BIG 1			// set to 1 for "#if".

#undef	JU_COPY_PINDEX_TO_LONG
#define	JU_COPY_PINDEX_TO_LONG(LI,PI) JU_COPY5_PINDEX_TO_LONG(LI,PI)

#include "JudySearchLeafOdd.c"

} // __JudySearchLeaf5()


// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   6
//
// Search a 6-byte-Index linear leaf (cJ*_JPLEAF6, or the index list in a
// cJ*_JPBRANCH_L6) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf6(
	Pjll_t Pjll,		// to linear Leaf6 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
#undef	BPI			// in case previously set.
#undef	IPC
#undef	IPG
#undef	WPG
#undef	MASK
#define	BPI  BPI6		// for JudySearchLeafOdd.c.
#define	IPC  IPC6
#define	IPG  IPG6
#define	WPG  WPG6
#define	MASK MASK6

#undef	GET_0
#undef	GET_1
#undef	GET_2
#undef	GET_LIG
#define	GET_0	GET6_0
#define	GET_1	GET6_1
#define	GET_2	GET6_2
#define	GET_LIG	GET6_LIG

// Unfortunately IPG itself is an expression the compiler can't seem to handle,
// so unset IPG_BIG here explicitly, though this contains knowledge of IPG
// values:

#undef	IPG_BIG

#undef	JU_COPY_PINDEX_TO_LONG
#define	JU_COPY_PINDEX_TO_LONG(LI,PI) JU_COPY6_PINDEX_TO_LONG(LI,PI)

#include "JudySearchLeafOdd.c"

} // __JudySearchLeaf6()


// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   7
//
// Search a 7-byte-Index linear leaf (cJ*_JPLEAF7, or the index list in a
// cJ*_JPBRANCH_L7) for the specified Index.  The return value is described in
// file header comments.

FUNCTION RETURN_TYPE __JudySearchLeaf7(
	Pjll_t Pjll,		// to linear Leaf7 to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
#undef	BPI			// in case previously set.
#undef	IPC
#undef	IPG
#undef	WPG
#undef	MASK
#define	BPI  BPI7		// for JudySearchLeafOdd.c.
#define	IPC  IPC7
#define	IPG  IPG7
#define	WPG  WPG7
#define	MASK MASK7

#undef	GET_0
#undef	GET_1
#undef	GET_2
#undef	GET_3
#undef	GET_4
#undef	GET_5
#undef	GET_6
#undef	GET_LIG
#define	GET_0	GET7_0
#define	GET_1	GET7_1
#define	GET_2	GET7_2
#define	GET_3	GET7_3
#define	GET_4	GET7_4
#define	GET_5	GET7_5
#define	GET_6	GET7_6
#define	GET_LIG	GET7_LIG

// Unfortunately IPG itself is an expression the compiler can't seem to handle,
// so define IPG_BIG here explicitly, though this contains knowledge of IPG
// values:

#undef	IPG_BIG
#define	IPG_BIG 1			// set to 1 for "#if".

#undef	JU_COPY_PINDEX_TO_LONG
#define	JU_COPY_PINDEX_TO_LONG(LI,PI) JU_COPY7_PINDEX_TO_LONG(LI,PI)

#include "JudySearchLeafOdd.c"

} // __JudySearchLeaf7()

#endif // JU_64BIT


#ifdef JU_HPUX

// ****************************************************************************
// __ J U D Y   S E A R C H   L E A F   W
//
// Search a root-level leaf (cJ*_JAPLEAF, index size = Word_t) for the
// specified Index.  The return value is described in file header comments.

FUNCTION RETURN_TYPE __JudySearchLeafW(
	Pjlw_t Pjlw,		// to linear LeafW to search.
	Word_t LeafPop1,	// number of valid indexes.
	Word_t Index)		// to find.
{
#undef	PLEAF_T			// in case previously set.
#define	PLEAF_T Pjlw_t		// for JudySearchLeafEven.c.

#undef	BPI
#define	BPI BPIL

#undef	IPC
#define	IPC IPCL

	Pjll_t Pjll = (Pjll_t) Pjlw;

#include "JudySearchLeafEven.c"

} // __JudySearchLeafW()

#endif // JU_HPUX


#if (JU_HPUX_PA || JU_WIN_IPF)

// Note:  only hpux_pa needs this; see JudyPrivate.h.
//
// TBD:  Does aCC need this too, or is it smarter?  Perhaps if we build with
// aCC on hpux_pa, we can get rid of this.


// ****************************************************************************
// __ J U D Y   C O U N T   B I T S   B
//
// Return the number of bits set in "Word", for a bitmap branch.
//
// Note:  Bitmap branches have maximum bitmap size = 32 bits.

FUNCTION BITMAPB_t __JudyCountBitsB(BITMAPB_t word)
{
	word = (word & 0x55555555) + ((word & 0xAAAAAAAA) >>  1);
	word = (word & 0x33333333) + ((word & 0xCCCCCCCC) >>  2);
	word = (word & 0x0F0F0F0F) + ((word & 0xF0F0F0F0) >>  4); // >= 8 bits.
#if defined(BITMAP_BRANCH16x16) || defined(BITMAP_BRANCH32x8)
	word = (word & 0x00FF00FF) + ((word & 0xFF00FF00) >>  8); // >= 16 bits.
#endif
#ifdef BITMAP_BRANCH32x8
	word = (word & 0x0000FFFF) + ((word & 0xFFFF0000) >> 16); // >= 32 bits.
#endif
	return(word);

} // __JudyCountBitsB()


// ****************************************************************************
// __ J U D Y   C O U N T   B I T S   L
//
// Return the number of bits set in "Word", for a bitmap leaf.
//
// Note:  Bitmap branches have maximum bitmap size = 32 bits.

// Note:  Need both 32-bit and 64-bit versions of __JudyCountBitsL() because
// bitmap leaves can have 64-bit bitmaps.

FUNCTION BITMAPL_t __JudyCountBitsL(BITMAPL_t word)
{
#ifndef JU_64BIT

	word = (word & 0x55555555) + ((word & 0xAAAAAAAA) >>  1);
	word = (word & 0x33333333) + ((word & 0xCCCCCCCC) >>  2);
	word = (word & 0x0F0F0F0F) + ((word & 0xF0F0F0F0) >>  4); // >= 8 bits.
#if defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8)
	word = (word & 0x00FF00FF) + ((word & 0xFF00FF00) >>  8); // >= 16 bits.
#endif
#ifdef BITMAP_LEAF32x8
	word = (word & 0x0000FFFF) + ((word & 0xFFFF0000) >> 16); // >= 32 bits.
#endif

#else // JU_64BIT

	word = (word & 0x5555555555555555) + ((word & 0xAAAAAAAAAAAAAAAA) >> 1);
	word = (word & 0x3333333333333333) + ((word & 0xCCCCCCCCCCCCCCCC) >> 2);
	word = (word & 0x0F0F0F0F0F0F0F0F) + ((word & 0xF0F0F0F0F0F0F0F0) >> 4);
#if defined(BITMAP_LEAF16x16) || defined(BITMAP_LEAF32x8) || defined(BITMAP_LEAF64x4)
	word = (word & 0x00FF00FF00FF00FF) + ((word & 0xFF00FF00FF00FF00) >> 8);
#endif
#if defined(BITMAP_LEAF32x8) || defined(BITMAP_LEAF64x4)
	word = (word & 0x0000FFFF0000FFFF) + ((word & 0xFFFF0000FFFF0000) >>16);
#endif
#ifdef BITMAP_LEAF64x4
	word = (word & 0x00000000FFFFFFFF) + ((word & 0xFFFFFFFF00000000) >>32);
#endif
#endif // JU_64BIT

	return(word);

} // __JudyCountBitsL()

#endif // (JU_HPUX_PA || JU_WIN_IPF)


#ifdef DEBUG

// ****************************************************************************
// J U D Y   C H E C K   S O R T E D
//
// Given a pointer to a packed list of Pop1 N-byte indexes (N = IndexSize),
// assert that the list is in order.

FUNCTION void JudyCheckSorted(
	Pjll_t Pjll,		// leaf or list to check.
	Word_t Pop1,		// number of indexes to check.
	long   IndexSize)	// bytes per index in list.
{

// Macros for common code:
//
// Check a list of "even"-sized indexes, counting backwards from last to first:
//
// Pjll and Pop1 are in the context.

#define	JU_CHECKSORTED(PType)	\
	while (--Pop1 > 0)	\
	    assert(((PType) Pjll)[Pop1 - 1] < ((PType) Pjll)[Pop1]); \
	break

// Check a list of "odd"-sized indexes, which requires first copying each one
// to an aligned word:
//
// Pjll, Pop1, and IndexSize are in the context.

#define	JU_CHECKSORTED_ODD(CopyIndex)			\
	{						\
	    Word_t indexlow;				\
	    Word_t indexhigh;				\
	    long   offset;				\
							\
	    CopyIndex(indexlow, (uint8_t *) Pjll);	\
							\
	    for (offset = 1; offset < Pop1; ++offset)	\
	    {						\
		CopyIndex(indexhigh,			\
			  ((uint8_t *) Pjll) + (offset * IndexSize)); \
		assert(indexlow < indexhigh);		\
		indexlow = indexhigh;			\
	    }						\
	    break;					\
	}

	switch (IndexSize)
	{
	case 1:	JU_CHECKSORTED(uint8_t *);
	case 2:	JU_CHECKSORTED(uint16_t *);
	case 3:	JU_CHECKSORTED_ODD(JU_COPY3_PINDEX_TO_LONG);
	case 4:	JU_CHECKSORTED(uint32_t *);
#ifdef JU_64BIT
	case 5:	JU_CHECKSORTED_ODD(JU_COPY5_PINDEX_TO_LONG);
	case 6:	JU_CHECKSORTED_ODD(JU_COPY6_PINDEX_TO_LONG);
	case 7:	JU_CHECKSORTED_ODD(JU_COPY7_PINDEX_TO_LONG);
	case 8:	JU_CHECKSORTED(Pjlw_t);
#endif
	default:  assert(FALSE);	// invalid IndexSize.
	}

} // JudyCheckSorted()

#endif // DEBUG

#endif // _JUDYSEARCHLEAF
