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

// @(#) $Revision: 4.10 $ $Source: /judy/src/JudyCommon/JudySearchLeafOdd.c $
//
// This source file contains the body of a __JudySearchLeaf*() function for an
// "odd" index size (BPI):  3, 5, 6, or 7 bytes.  You can think of this file
// as a huge macro for generating efficient code for different sized indexes
// but always following the same template.  Unlike a real macro, this separate
// file can contain comments and is easier to read and maintain.
//
// This code must be #include'd by another source file, possibly multiple
// times, each time with the standard __JudySearchLeaf*() parameters present:
//
//	Pjll_t Pjll
//	Word_t LeafPop1,
//	Word_t Index
//
// and the following macros redefined appropriately for the including function:
//
//	BPI	 bytes per index
//	IPC	 indexes per cache line
//	IPG	 indexes per index group
//	WPG	 words per index group
//	MASK	 for BPI LSB of a word
//	GET_*	 macros for getting an aligned index
//	IPG_BIG	 set if IPG = 8; otherwise unset
//	JU_COPY_PINDEX_TO_LONG  one of JU_COPY*_PINDEX_TO_LONG

{
#ifndef	BRUTE_FORCE // ========================================================

	PWord_t Pword  = (PWord_t) Pjll;	// first word of index group.
	Word_t  offset = 0;			// offset of index in leaf.
	Word_t  testindex;		// aligned copy to check against Index.

	assert((LeafPop1 * BPI) <= (cJU_BYTESPERCL * 2));  // <= 2 cache lines.
	DBGCODE(JudyCheckSorted(Pjll, LeafPop1, (long) BPI);)


// HANDLE LEAF POPULATION GREATER THAN ONE CACHE LINE:
//
// Extract a BPI-byte Index near the end of the first cache line to compare.
// If it's too low, move to the following Index, which must be left-aligned in
// a word, which establishes which previous Index to use for comparison.  In
// other words, the comparison point is not actually the cache line boundary,
// but the last word at or prior to it where an index ends at a word boundary.
//
// TBD:  In rare cases this moves to the next "virtual" cache line when in fact
// the leaf fits entirely in a single real cache line.  Is this a problem?

	Index &= MASK;			// look for last BPI bytes only.

	if (LeafPop1 > IPC)
	{
	    // first virtual cache line, last index group, first word:
	    Pword += ((IPC * BPI) / cJU_BYTESPERWORD) - WPG;
	    testindex = GET_LIG;	// last index in group.

	    if (Index <= testindex)	// match, if any, in first cache line:
	    {
		Pword = (PWord_t) Pjll;	// reset to start.
		LeafPop1 = IPC;		// truncate.
	    }
	    else			// match, if any, in second cache line:
	    {
		Pword	 += WPG;	// first word of next index group.
		LeafPop1 -= IPC;	// skip first virtual cache line.
		offset	 += IPC;
	    }
	}


// SEARCH FOR INDEX GROUP IN ONE CACHE LINE:
//
// Test and skip one index group (WPG words) at a time, using the (aligned)
// last of IPG indexes in each group.  This is a serial search, but because it
// checks IPG indexes at a time, it compares well with a binary search for this
// small of an object.

	while (LeafPop1 >= IPG)		// whole index group remains.
	{
	    testindex = GET_LIG;	// check last index in group.

	    if (Index <= testindex)	// in this group, if present.
	    {
		if (Index == testindex)		// found desired Index...
		    return(offset + IPG - 1);	// is last in this group.

		break;			// match, if any, is in this group.
	    }

	    Pword    += WPG;		// next index group:
	    LeafPop1 -= IPG;
	    offset   += IPG;
	}


// SEARCH ONE INDEX GROUP:
//
// Now the search is narrowed to a small group of indexes with a known
// alignment (hidden in the GET_*() macros), where the last index in the group
// is known to be too high or does not exist.  Pword points to the first word
// of the group, and offset is the base-0 ordinal of the first index in the
// group.  Note that LeafPop1 might be < IPG, that is, this might not be a full
// index group.
//
// TBD:  This section "knows" (embodies) the value of IPG for 32/64-bit.  In
// other words, using the cases below for speed, rather than a loop, depends on
// specific values of IPG.
//
// Shorthand macro for testing the index at Offset2 in an index group using a
// specified GET_* macro.  If an absent index (Index > testindex) or a matching
// index (Index == testindex) is not found at this Offset2, each case falls
// through into the next one (lower Offset2).

#define	CHECKINDEX(GET,Offset2)	\
	testindex = GET;	\
	if (Index >  testindex) return(~(offset + (Offset2) + 1)); \
	if (Index == testindex) return(  offset + (Offset2))

	switch (LeafPop1)
	{
	    default:	// case 4 [8]: fall through to check previous index:
#ifdef IPG_BIG
	    case 7: CHECKINDEX(GET_6, 6);
	    case 6: CHECKINDEX(GET_5, 5);
	    case 5: CHECKINDEX(GET_4, 4);
	    case 4: CHECKINDEX(GET_3, 3);
#endif
	    case 3: CHECKINDEX(GET_2, 2);
	    case 2: CHECKINDEX(GET_1, 1);
	    case 1: CHECKINDEX(GET_0, 0);
	    case 0: break;			// none left => not found.
	}

	return(~offset);			// Index not found.

#else // BRUTE_FORCE ==========================================================

// BRUTE FORCE = LINEAR SEARCH:
//
// Scanning every index, including copying it to an aligned word for comparison
// with Index, is slower than the grouping method used above.

	uint8_t * Pleaf = (uint8_t *) Pjll;	// first byte of leaf.
	Word_t    testindex;		// aligned copy to check against Index.
	Word_t	  LeafIndexes = LeafPop1;

DBGCODE(JudyCheckSorted(Pjll, LeafPop1, (long) BPI);)

	Index &= MASK;			// look for last BPI bytes only.

	do {
	    JU_COPY_PINDEX_TO_LONG(testindex, Pleaf);

	    if (Index <= testindex)
	    {
		if (Index == testindex) return(LeafIndexes - LeafPop1);
		break;				// no match.
	    }

	    Pleaf += BPI;
	}
	while (--LeafPop1);

	return(~(LeafIndexes - LeafPop1));	// Index not found.

#endif // BRUTE_FORCE

} // JudySearchLeafOdd.c function body
