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

// @(#) $Revision: 4.11 $ $Source: /judy/src/JudyCommon/JudySearchLeafEven.c $
//
// This source file contains the body of a __JudySearchLeaf*() function for an
// "even" index size (BPI):  1, 2, 4, or 8 bytes.  You can think of this file
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
// except Index must already be masked if the index size is less than a Word_t;
// and the following macros redefined appropriately for the including function:
//
//	PLEAF_T	pointer to even index type, without parentheses.
//	BPI	bytes per index
//	IPC	indexes per cache line

{
#ifndef BRUTE_FORCE

// BINARY SEARCH:  A linear search preceded by two binary divisions (to
// restrict the search area to a quarter cache line or less):

	PLEAF_T	Pleaf = (PLEAF_T) Pjll;	// start of part of leaf.
	Word_t	halfleaf;		// offset to half of leaf.
	Word_t	offset = 0;		// offset of Index in leaf.

	assert((cJU_BYTESPERCL % sizeof(Word_t)) == 0);	  // whole words per CL.
	assert((LeafPop1 * BPI) <= (cJU_BYTESPERCL * 2)); // <= 2 cache lines.
	DBGCODE(JudyCheckSorted(Pjll, LeafPop1, (long) BPI);)


// BINARY SEARCH A LEAF LARGE ENOUGH TO WARRANT IT:
//
// First narrow to the correct cache line, if more than one in the leaf.

	if (LeafPop1 > IPC/4)		// large enough for binary search:
	{
	    if (LeafPop1 > IPC)		// even larger, pick a cache line:
	    {
		if (Index <= *(Pleaf + (IPC - 1)))
		{			// match, if any, in first cache line:
		    LeafPop1 = IPC;	// truncate.
		}
		else			// match, if any, in second cache line:
		{
		    LeafPop1 -= IPC;	// skip first cache line:
		    Pleaf    += IPC;
		    offset   += IPC;
		}
	    }

// Repeated code to binary search a leaf:
//
// TBD:  If the code above ended up in the second cache line with an odd
// LeafPop1 remaining, it's not clear how this old code (see 4.9) works right
// by simply subtracting halfleaf when Index is in the first half.  What about
// off-by-1 errors due to dividing an odd number by 2?  Even if this is
// correct, at least document it better here.

#undef	BINARYSEARCH
#define	BINARYSEARCH				\
	    halfleaf = LeafPop1 / 2;		\
						\
	    if (Index >= *(Pleaf + halfleaf))	\
	    {					\
		Pleaf  += halfleaf;		\
		offset += halfleaf;		\
	    }					\
						\
	    LeafPop1 -= halfleaf

// Narrow to 1/2 (sub)leaf, then to 1/4 (sub)leaf:
//
// TBD:  If previous code ended up in the second cache line, LeafPop1 could be
// a small number, < IPC/4, like 1,2,3, where this code is wasteful.

	    BINARYSEARCH;
	    BINARYSEARCH;

	} // if larger leaf.

// Now Pleaf and LeafPop1 are possibly adjusted to a small portion of the
// original leaf, not to exceed 1/4 cache line.


// CHECK IF INDEX IS BEYOND THE END OF THIS PORTION OF THE (SUB)LEAF:

	if (Index > Pleaf[LeafPop1 - 1])		// Index not found.
	    return(~(offset + LeafPop1));


// LINEAR SEARCH THE REMAINING (SUB)LEAF:

	while (TRUE)					// until return.
	{
	    if (Index <= *Pleaf)
	    {
	       if (Index == *Pleaf) return(offset);	// found Index.
	       return(~offset);				// Index not found.
	    }

	    ++Pleaf;
	    ++offset;
	}


#else // BRUTE_FORCE

// BRUTE FORCE = LINEAR SEARCH:

	PLEAF_T Pleaf = (PLEAF_T) Pjll;
	Word_t  pop1  = LeafPop1;

DBGCODE(JudyCheckSorted(Pjll, LeafPop1, (long) BPI);)

	do {
	    if (Index <= *Pleaf)
	    {
	       if (Index == *Pleaf) return(pop1 - LeafPop1);	// found Index.
	       break;					// Index not found.
	    }

	    ++Pleaf;
	}
	while (--LeafPop1);

	return(~(pop1 - LeafPop1));		// Index not found.

#endif // BRUTE_FORCE

} // JudySearchLeafEven.c function body
