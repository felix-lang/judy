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

// @(#) $Revision: 4.38 $ $Source: /judy/src/JudyCommon/JudyCascade.c $

#ifdef JUDY1
#include "Judy1.h"
#else
#include "JudyL.h"
#endif

#include "JudyPrivate1L.h"

extern int __JudyCreateBranchL(Pjp_t, Pjp_t, uint8_t *, Word_t, Pvoid_t);
extern int __JudyCreateBranchB(Pjp_t, Pjp_t, uint8_t *, Word_t, Pvoid_t);

DBGCODE(extern void JudyCheckSorted(Pjll_t Pjll, Word_t Pop1, long IndexSize);)

const jbb_t StageJBBZero;	// zeroed versions of namesake struct.

// TBD:  There are multiple copies of (some of) these CopyWto3, Copy3toW,
// CopyWto7 and Copy7toW functions in Judy1Cascade.c, JudyLCascade.c, and
// JudyDecascade.c.  These static functions should probably be moved to a
// common place, made macros, or something to avoid having four copies.


// ****************************************************************************
// __ J U D Y   C O P Y   X   T O   W


FUNCTION static void __JudyCopy3toW(
	PWord_t	  PDest,
	uint8_t * PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY3_PINDEX_TO_LONG(*PDest, PSrc);
		PSrc	+= 3;
		PDest	+= 1;

	} while(--LeafIndexes);

} //__JudyCopy3toW()


#ifdef JU_64BIT

FUNCTION static void __JudyCopy4toW(
	PWord_t	   PDest,
	uint32_t * PSrc,
	Word_t	   LeafIndexes)
{
	do { *PDest++ = *PSrc++;
	} while(--LeafIndexes);

} // __JudyCopy4toW()


FUNCTION static void __JudyCopy5toW(
	PWord_t	  PDest,
	uint8_t	* PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY5_PINDEX_TO_LONG(*PDest, PSrc);
		PSrc	+= 5;
		PDest	+= 1;

	} while(--LeafIndexes);

} // __JudyCopy5toW()


FUNCTION static void __JudyCopy6toW(
	PWord_t	  PDest,
	uint8_t	* PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY6_PINDEX_TO_LONG(*PDest, PSrc);
		PSrc	+= 6;
		PDest	+= 1;

	} while(--LeafIndexes);

} // __JudyCopy6toW()


FUNCTION static void __JudyCopy7toW(
	PWord_t	  PDest,
	uint8_t	* PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY7_PINDEX_TO_LONG(*PDest, PSrc);
		PSrc	+= 7;
		PDest	+= 1;

	} while(--LeafIndexes);

} // __JudyCopy7toW()

#endif // JU_64BIT


// ****************************************************************************
// __ J U D Y   C O P Y   W   T O   X


FUNCTION static void __JudyCopyWto3(
	uint8_t	* PDest,
	PWord_t	  PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY3_LONG_TO_PINDEX(PDest, *PSrc);
		PSrc	+= 1;
		PDest	+= 3;

	} while(--LeafIndexes);

} // __JudyCopyWto3()


#ifdef JU_64BIT

FUNCTION static void __JudyCopyWto4(
	uint8_t	* PDest,
	PWord_t	  PSrc,
	Word_t	  LeafIndexes)
{
	uint32_t *PDest32 = (uint32_t *)PDest;

	do
	{
		*PDest32 = *PSrc;
		PSrc	+= 1;
		PDest32	+= 1;
	} while(--LeafIndexes);

} // __JudyCopyWto4()


FUNCTION static void __JudyCopyWto5(
	uint8_t	* PDest,
	PWord_t	  PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY5_LONG_TO_PINDEX(PDest, *PSrc);
		PSrc	+= 1;
		PDest	+= 5;

	} while(--LeafIndexes);

} // __JudyCopyWto5()


FUNCTION static void __JudyCopyWto6(
	uint8_t	* PDest,
	PWord_t	  PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY6_LONG_TO_PINDEX(PDest, *PSrc);
		PSrc	+= 1;
		PDest	+= 6;

	} while(--LeafIndexes);

} // __JudyCopyWto6()


FUNCTION static void __JudyCopyWto7(
	uint8_t	* PDest,
	PWord_t	  PSrc,
	Word_t	  LeafIndexes)
{
	do
	{
		JU_COPY7_LONG_TO_PINDEX(PDest, *PSrc);
		PSrc	+= 1;
		PDest	+= 7;

	} while(--LeafIndexes);

} // __JudyCopyWto7()

#endif // JU_64BIT


// ****************************************************************************
// COMMON CODE (MACROS):
//
// Free objects in an array of valid JPs, StageJP[ExpCnt] == last one may
// include Immeds, which are ignored.

#define FREEALLEXIT(ExpCnt,StageJP,Pjpm)				\
	{								\
	    Word_t _expct = (ExpCnt);					\
	    while (_expct--) __JudyFreeSM(&((StageJP)[_expct]), Pjpm);  \
	    return(-1);                                                 \
	}

// Clear the array that keeps track of the number of JPs in a subexpanse:

#define ZEROJP(SubJPCount)                                              \
	{								\
		int ii;							\
		for (ii = 0; ii < cJU_NUMSUBEXPB; ii++) (SubJPCount[ii]) = 0; \
	}


#define FORMNEWJP(PjpNew,PJP,CIndex,Pop1,Level,AddrRaw,Type)	        \
									\
	(PjpNew)->jp_DcdPop0 = ((PJP)->jp_DcdPop0 & cJU_DCDMASK(Level))	\
			     | ((CIndex) & cJU_DCDMASK((Level)-1))	\
			     | ((Pop1) - 1);			        \
	(PjpNew)->jp_Addr = (Word_t)(AddrRaw);			        \
	(PjpNew)->jp_Type = (Type)


#define FORMIMMJP(PjpNew,Pjp,CIndex,Level)			        \
									\
	(PjpNew)->jp_DcdPop0 = ((Pjp)->jp_DcdPop0 & cJU_DCDMASK(Level))	\
			     | (CIndex);				\
	(PjpNew)->jp_Type = cJU_JPIMMED_1_01 - 1 + (Level)



// ****************************************************************************
// __ J U D Y   S T A G E   J B B   T O   J B B
//
// Create a malloc'd BranchB (jbb_t) from a staged BranchB while "splaying" a
// single old leaf.  Return -1 if out of memory, otherwise 1.

static int __JudyStageJBBtoJBB(
	Pjp_t     PjpLeaf,	// JP of leaf being splayed.
	Pjbb_t    PStageJBB,	// temp jbb_t on stack.
	Pjp_t     PjpArray,	// array of JPs to splayed new leaves.
	uint8_t * PSubCount,	// count of JPs for each subexpanse.
	Pjpm_t    Pjpm)		// the jpm_t for JudyAlloc*().
{
	Pjbb_t    PjbbRaw;	// pointer to new bitmap branch.
	Pjbb_t    Pjbb;
	Word_t    subexp;

// Get memory for new BranchB:

	if ((PjbbRaw = __JudyAllocJBB(Pjpm)) == (Pjbb_t) NULL) return(-1);
	Pjbb = P_JBB(PjbbRaw);

// Copy staged BranchB into just-allocated BranchB:

	*Pjbb = *PStageJBB;

// Allocate the JP subarrays (BJP) for the new BranchB:

	for (subexp = 0; subexp < cJU_NUMSUBEXPB; subexp++)
	{
	    Pjp_t  PjpRaw;
	    Pjp_t  Pjp;
	    Word_t NumJP;       // number of JPs in each subexpanse.

	    if ((NumJP = PSubCount[subexp]) == 0) continue;	// empty.

// Out of memory, back out previous allocations:

	    if ((PjpRaw = __JudyAllocJBBJP(NumJP, Pjpm)) == (Pjp_t) NULL)
	    {
		while(subexp--)
		{
		    if ((NumJP = PSubCount[subexp]) == 0) continue;

		    PjpRaw = JU_JBB_PJP(Pjbb, subexp);
		    __JudyFreeJBBJP(PjpRaw, NumJP, Pjpm);
		}
		__JudyFreeJBB(PjbbRaw, Pjpm);
		return(-1);	// out of memory.
	    }
	    Pjp = P_JP(PjpRaw);

// Place the JP subarray pointer in the new BranchB, copy subarray JPs, and
// advance to the next subexpanse:

	    JU_JBB_PJP(Pjbb, subexp) = PjpRaw;
	    JU_COPYMEM(Pjp, PjpArray, NumJP);
	    PjpArray += NumJP;

	} // for each subexpanse.

// Change the PjpLeaf from Leaf to BranchB:

	PjpLeaf->jp_Addr  = (Word_t) PjbbRaw;
	PjpLeaf->jp_Type += cJU_JPBRANCH_B2 - cJU_JPLEAF2;  // Leaf to BranchB.

	return(1);

} // __JudyStageJBBtoJBB()


// ****************************************************************************
// __ J U D Y   J L L 2   T O   J L B 1
//
// Create a LeafB1 (jlb_t = JLB1) from a Leaf2 (2-byte Indexes and for JudyL,
// Word_t Values).  Return NULL if out of memory, else a pointer to the new
// LeafB1.
//
// NOTE:  Caller must release the Leaf2 that was passed in.

FUNCTION static Pjlb_t __JudyJLL2toJLB1(
	uint16_t * Pjll,	// array of 16-bit indexes.
#ifdef JUDYL
	Pjv_t      Pjv,		// array of associated values.
#endif
	Word_t     LeafPop1,	// number of indexes/values.
	Pvoid_t    Pjpm)	// jpm_t for JudyAlloc*()/JudyFree*().
{
	Pjlb_t     PjlbRaw;
	Pjlb_t     Pjlb;
	int	   offset;
JUDYLCODE(int	   subexp;)

// Allocate the LeafB1:

	if ((PjlbRaw = __JudyAllocJLB1(Pjpm)) == (Pjlb_t) NULL)
	    return((Pjlb_t) NULL);
	Pjlb = P_JLB(PjlbRaw);

// Copy Leaf2 indexes to LeafB1:

	for (offset = 0; offset < LeafPop1; ++offset)
	    JU_BITMAPSETL(Pjlb, Pjll[offset]);

#ifdef JUDYL

// Build LeafV's from bitmap:

	for (subexp = 0; subexp < cJU_NUMSUBEXPL; ++subexp)
	{
	    struct _POINTER_VALUES
	    {
		Word_t pv_Pop1;		// size of value area.
		Pjv_t  pv_Pjv;		// raw pointer to value area.
	    } pv[cJU_NUMSUBEXPL];

// Get the population of the subexpanse, and if any, allocate a LeafV:

	    pv[subexp].pv_Pop1 = __JudyCountBitsL(JU_JLB_BITMAP(Pjlb, subexp));

	    if (pv[subexp].pv_Pop1)
	    {
		Pjv_t Pjvnew;

// TBD:  There is an opportunity to put pop == 1 value in pointer:

		pv[subexp].pv_Pjv = __JudyLAllocJV(pv[subexp].pv_Pop1, Pjpm);

// Upon out of memory, free all previously allocated:

		if (pv[subexp].pv_Pjv == (Pjv_t) NULL)
		{
		    while(subexp--)
		    {
			if (pv[subexp].pv_Pop1)
			{
			    __JudyLFreeJV(pv[subexp].pv_Pjv, pv[subexp].pv_Pop1,
					  Pjpm);
			}
		    }
		    __JudyFreeJLB1(PjlbRaw, Pjpm);
		    return((Pjlb_t) NULL);
		}

		Pjvnew = P_JV(pv[subexp].pv_Pjv);
		JU_COPYMEM(Pjvnew, Pjv, pv[subexp].pv_Pop1);
		Pjv += pv[subexp].pv_Pop1;	// advance value pointer.

// Place raw pointer to value array in bitmap subexpanse:

		JL_JLB_PVALUE(Pjlb, subexp) = pv[subexp].pv_Pjv;

	    } // populated subexpanse.
	} // each subexpanse.

#endif // JUDYL

	return(PjlbRaw);	// pointer to LeafB1.

} // __JudyJLL2toJLB1()


// ****************************************************************************
// __ J U D Y   C A S C A D E 1
//
// Create bitmap leaf from 1-byte Indexes and Word_t Values.
//
// TBD:  There must be a better way.
//
// Only for JudyL 32 bit:  (note, unifdef disallows comment on next line)

#if (JUDYL || (! JU_64BIT))

FUNCTION int __JudyCascade1(
	Pjp_t	   Pjp,
	Pvoid_t    Pjpm)
{
	uint8_t	 * PLeaf;
	Pjlb_t	   PjlbRaw;
	Pjlb_t	   Pjlb;
	Word_t     Pop1;
	Word_t     ii;		// temp for loop counter
	jp_t       jp;
JUDYLCODE(Pjv_t	   Pjv;)

	assert(Pjp->jp_Type == cJU_JPLEAF1);
	assert((Pjp->jp_DcdPop0 & 0xFF) == (cJU_LEAF1_MAXPOP1-1));

	PjlbRaw = __JudyAllocJLB1(Pjpm);
	if (PjlbRaw == (Pjlb_t) NULL) return(-1);

	Pjlb  = P_JLB(PjlbRaw);
	PLeaf = (uint8_t *) P_JLL(Pjp->jp_Addr);
	Pop1  = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;

	JUDYLCODE(Pjv = JL_LEAF1VALUEAREA(PLeaf, Pop1);)

//	Copy 1 byte index Leaf to bitmap Leaf
	for (ii = 0; ii < Pop1; ii++) JU_BITMAPSETL(Pjlb, PLeaf[ii]);

#ifdef JUDYL
//	Build 8 subexpanse Value leaves from bitmap
	for (ii = 0; ii < cJU_NUMSUBEXPL; ii++)
	{
//	    Get number of Indexes in subexpanse
	    if ((Pop1 = __JudyCountBitsL(JU_JLB_BITMAP(Pjlb, ii))))
	    {
		Pjv_t PjvnewRaw;	// value area of new leaf.
		Pjv_t Pjvnew;

		PjvnewRaw = __JudyLAllocJV(Pop1, Pjpm);
		if (PjvnewRaw == (Pjv_t) NULL)	// out of memory.
		{
//                  Free prevously allocated LeafVs:
		    while(ii--)
		    {
			if ((Pop1 = __JudyCountBitsL(JU_JLB_BITMAP(Pjlb, ii))))
			{
			    PjvnewRaw = JL_JLB_PVALUE(Pjlb, ii);
			    __JudyLFreeJV(PjvnewRaw, Pop1, Pjpm);
			}
		    }
//                  Free the bitmap leaf
		    __JudyLFreeJLB1(PjlbRaw,Pjpm);
		    return(-1);
		}
		Pjvnew    = P_JV(PjvnewRaw);
		JU_COPYMEM(Pjvnew, Pjv, Pop1);

		Pjv += Pop1;
		JL_JLB_PVALUE(Pjlb, ii) = PjvnewRaw;
	    }
	}
#endif // JUDYL

//	Add in another Dcd byte because compressing
	jp.jp_DcdPop0 = Pjp->jp_DcdPop0 | (PLeaf[0] & cJU_DCDMASK(1));
//
//	Set JP to new compressed leaf values
	jp.jp_Type   = cJU_JPLEAF_B1;	// A bitmap Leaf
	jp.jp_Addr   = (Word_t) PjlbRaw;

	*Pjp = jp;      // may or not be atomic

	return(1);	// return success

} // __JudyCascade1()

#endif // (!(JUDY1 && JU_64BIT))


// ****************************************************************************
// __ J U D Y   C A S C A D E 2
//
// Entry PLeaf of size LeafPop1 is either compressed or splayed with pointer
// returned in Pjp.  Entry Levels sizeof(Word_t) down to level 2.
//
// Splay or compress the 2-byte Index Leaf that Pjp point to.  Return *Pjp as a
// (compressed) cJU_LEAFB1 or a cJU_BRANCH_*2

FUNCTION int __JudyCascade2(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint16_t * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF2_MAXPOP1];  // JPs of new leaves
	uint8_t	   StageExp  [cJU_LEAF2_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF2);
	assert((Pjp->jp_DcdPop0 & 0xFFFF) == (cJU_LEAF2_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint16_t *) P_JLL(Pjp->jp_Addr);

//	And its Value area
	JUDYLCODE(Pjv = JL_LEAF2VALUEAREA(PLeaf, cJU_LEAF2_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it to a Bitmap Leaf

	CIndex = PLeaf[0];
	if (!JU_DIGITATSTATE(CIndex ^ PLeaf[cJU_LEAF2_MAXPOP1-1], 2))
	{
//	cJU_JPLEAF_B1
		Pjlb_t PjlbRaw;
		PjlbRaw = __JudyJLL2toJLB1(PLeaf,
#ifdef JUDYL
				     Pjv,
#endif
				     cJU_LEAF2_MAXPOP1, Pjpm);
		if (PjlbRaw == (Pjlb_t)NULL) return(-1);  // out of memory

//		Set JP to new compressed leaf values
		Pjp->jp_Addr = (Word_t) PjlbRaw;
		Pjp->jp_Type = cJU_JPLEAF_B1;	// A bitmap Leaf

//		Merge in another Dcd byte because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(1));

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 2 byte index Leaf to 1 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF2_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ PLeaf[End], 2))
		   )
		{
//			Build a leaf below the previous expanse
//
			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 2);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 2);

			if (Pop1 == 1)	// cJU_JPIMMED_1_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 1);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
			else if (Pop1 <= cJU_IMMED1_MAXPOP1) // bigger
			{
//		cJL_JPIMMED_1_02..3:  JudyL 32
//		cJ1_JPIMMED_1_02..7:  Judy1 32
//		cJL_JPIMMED_1_02..7:  JudyL 64
//		cJ1_JPIMMED_1_02..15: Judy1 64
#ifdef JUDYL
				Pjv_t  PjvnewRaw;	// value area of leaf.
				Pjv_t  Pjvnew;

//				Allocate Value area for Immediate Leaf
				PjvnewRaw = __JudyLAllocJV(Pop1, Pjpm);
				if (PjvnewRaw == (Pjv_t) NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjvnew = P_JV(PjvnewRaw);

//				Copy to Values to Value Leaf
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
				PjpJP->jp_Addr = (Word_t) PjvnewRaw;

//				Copy to JP as an immediate Leaf
				JU_COPYMEM(PjpJP->jp_LIndex, PLeaf + Start,
					   Pop1);
#else
				JU_COPYMEM(PjpJP->jp_1Index, PLeaf + Start,
					   Pop1);
#endif
//				Set Type, Population and Index size
				PjpJP->jp_Type = cJU_JPIMMED_1_02 + Pop1 - 2;
			}

// 64Bit Judy1 does not have Leaf1:  (note, unifdef disallows comment on next
// line)

#if (! (JUDY1 && JU_64BIT))
			else if (Pop1 <= cJU_LEAF1_MAXPOP1) // still bigger
			{
//		cJU_JPLEAF1
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//				Get a new Leaf
				PjllRaw = __JudyAllocJLL1(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjll = P_JLL(PjllRaw);
#ifdef JUDYL
//				Copy to Values to new Leaf
				Pjvnew = JL_LEAF1VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
//				Copy Indexes to new Leaf
				JU_COPYMEM((uint8_t *)Pjll, PLeaf+Start, Pop1);

				DBGCODE(JudyCheckSorted(Pjll, Pop1, 1);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 2, PjllRaw,
					  cJU_JPLEAF1);
			}
#endif //  (!(JUDY1 && JU_64BIT)) // Not 64Bit Judy1

			else				// biggest
			{
//		cJU_JPLEAF_B1
				Pjlb_t PjlbRaw;
				PjlbRaw = __JudyJLL2toJLB1(
						PLeaf + Start,
#ifdef JUDYL
						Pjv + Start,
#endif
						Pop1, Pjpm);
				if (PjlbRaw == (Pjlb_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 2, PjlbRaw,
					  cJU_JPLEAF_B1);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF2_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = PLeaf[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L2;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

} // __JudyCascade2()


// ****************************************************************************
// __ J U D Y   C A S C A D E 3
//
// Return *Pjp as a (compressed) cJU_LEAF2, cJU_BRANCH_L3, cJU_BRANCH_B3.

FUNCTION int __JudyCascade3(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint8_t  * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF3_MAXPOP1];  // JPs of new leaves
	Word_t	   StageA    [cJU_LEAF3_MAXPOP1];
	uint8_t	   StageExp  [cJU_LEAF3_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF3);
	assert((Pjp->jp_DcdPop0 & 0xFFFFFF) == (cJU_LEAF3_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint8_t *) P_JLL(Pjp->jp_Addr);

//	Extract leaf to Word_t and insert-sort Index into it
	__JudyCopy3toW(StageA, PLeaf, cJU_LEAF3_MAXPOP1);

//	Get the address of the Leaf and Value area
	JUDYLCODE(Pjv = JL_LEAF3VALUEAREA(PLeaf, cJU_LEAF3_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it (compare 1st, last & Index)

	CIndex = StageA[0];
	if (!JU_DIGITATSTATE(CIndex ^ StageA[cJU_LEAF3_MAXPOP1-1], 3))
	{
		Pjll_t PjllRaw;	 // pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//		Alloc a 2 byte Index Leaf
		PjllRaw	= __JudyAllocJLL2(cJU_LEAF3_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy just 2 bytes Indexes to new Leaf
//		__JudyCopyWto2((uint16_t *) Pjll, StageA, cJU_LEAF3_MAXPOP1);
		JU_COPYMEM    ((uint16_t *) Pjll, StageA, cJU_LEAF3_MAXPOP1);
#ifdef JUDYL
//		Copy Value area into new Leaf
		Pjvnew = JL_LEAF2VALUEAREA(Pjll, cJU_LEAF3_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_LEAF3_MAXPOP1);
#endif
		DBGCODE(JudyCheckSorted(Pjll, cJU_LEAF3_MAXPOP1, 2);)

//		Form new JP, Pop0 field is unchanged
		Pjp->jp_Addr = (Word_t) PjllRaw;
		Pjp->jp_Type = cJU_JPLEAF2;

//		Add in another Dcd byte because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(2));

		return(1); // Success
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 3 byte index Leaf to 2 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF3_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ StageA[End], 3))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 3);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 3);

			if (Pop1 == 1)	// cJU_JPIMMED_2_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 2);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
#if (JUDY1 || JU_64BIT)
			else if (Pop1 <= cJU_IMMED2_MAXPOP1)
			{
//		cJ1_JPIMMED_2_02..3:  Judy1 32
//		cJL_JPIMMED_2_02..3:  JudyL 64
//		cJ1_JPIMMED_2_02..7:  Judy1 64
#ifdef JUDYL
//				Alloc is 1st in case of malloc fail
				Pjv_t PjvnewRaw;  // value area of new leaf.
				Pjv_t Pjvnew;

//				Allocate Value area for Immediate Leaf
				PjvnewRaw = __JudyLAllocJV(Pop1, Pjpm);
				if (PjvnewRaw == (Pjv_t) NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjvnew = P_JV(PjvnewRaw);

//				Copy to Values to Value Leaf
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);

				PjpJP->jp_Addr = (Word_t) PjvnewRaw;

//				Copy to Index to JP as an immediate Leaf
				JU_COPYMEM((uint16_t *) (PjpJP->jp_LIndex),
					   StageA + Start, Pop1);
#else // JUDY1
				JU_COPYMEM((uint16_t *) (PjpJP->jp_1Index),
					   StageA + Start, Pop1);
#endif // JUDY1
//				Set Type, Population and Index size
				PjpJP->jp_Type = cJU_JPIMMED_2_02 + Pop1 - 2;
			}
#endif // (JUDY1 || JU_64BIT)

			else	// Make a linear leaf2
			{
//		cJU_JPLEAF2
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

				PjllRaw = __JudyAllocJLL2(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t) NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjll = P_JLL(PjllRaw);
#ifdef JUDYL
//				Copy to Values to new Leaf
				Pjvnew = JL_LEAF2VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
//				Copy least 2 bytes per Index of Leaf to new Leaf
				JU_COPYMEM((uint16_t *) Pjll, StageA+Start,
					   Pop1);

				DBGCODE(JudyCheckSorted(Pjll, Pop1, 2);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 3, PjllRaw,
					  cJU_JPLEAF2);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF3_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = StageA[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L3;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

} // __JudyCascade3()


#ifdef JU_64BIT   // JudyCascade[4567]

// ****************************************************************************
// __ J U D Y   C A S C A D E 4
//
// Cascade from a cJU_JPLEAF4 to one of the following:
//  1. if leaf is in 1 expanse:
//        compress it into a JPLEAF3
//  2. if leaf contains multiple expanses:
//        create linear or bitmap branch containing
//        each new expanse is either a:
//               JPIMMED_3_01  branch
//               JPIMMED_3_02  branch
//               JPLEAF3

FUNCTION int __JudyCascade4(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint32_t * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF4_MAXPOP1];  // JPs of new leaves
	Word_t	   StageA    [cJU_LEAF4_MAXPOP1];
	uint8_t	   StageExp  [cJU_LEAF4_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF4);
	assert((Pjp->jp_DcdPop0 & 0xFFFFFFFF) == (cJU_LEAF4_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint32_t *) P_JLL(Pjp->jp_Addr);

//	Extract 4 byte index Leaf to Word_t
	__JudyCopy4toW(StageA, PLeaf, cJU_LEAF4_MAXPOP1);

//	Get the address of the Leaf and Value area
	JUDYLCODE(Pjv = JL_LEAF4VALUEAREA(PLeaf, cJU_LEAF4_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it (compare 1st, last & Index)

	CIndex = StageA[0];
	if (!JU_DIGITATSTATE(CIndex ^ StageA[cJU_LEAF4_MAXPOP1-1], 4))
	{
		Pjll_t PjllRaw;	 // pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new Leaf.

//		Alloc a 3 byte Index Leaf
		PjllRaw = __JudyAllocJLL3(cJU_LEAF4_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy Index area into new Leaf
		__JudyCopyWto3((uint8_t *) Pjll, StageA, cJU_LEAF4_MAXPOP1);
#ifdef JUDYL
//		Copy Value area into new Leaf
		Pjvnew = JL_LEAF3VALUEAREA(Pjll, cJU_LEAF4_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_LEAF4_MAXPOP1);
#endif
		DBGCODE(JudyCheckSorted(Pjll, cJU_LEAF4_MAXPOP1, 3);)

//		Set JP to new compressed leaf values
		Pjp->jp_Addr = (Word_t) PjllRaw;
		Pjp->jp_Type = cJU_JPLEAF3;

//		Add in more Dcd bytes because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(3));

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 4 byte index Leaf to 3 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF4_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ StageA[End], 4))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 4);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 4);

			if (Pop1 == 1)	// cJU_JPIMMED_3_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 3);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
			else if (Pop1 <= cJU_IMMED3_MAXPOP1)
			{
//		cJ1_JPIMMED_3_02   :  Judy1 32
//		cJL_JPIMMED_3_02   :  JudyL 64
//		cJ1_JPIMMED_3_02..5:  Judy1 64

#ifdef JUDYL
//				Alloc is 1st in case of malloc fail
				Pjv_t PjvnewRaw;  // value area of new leaf.
				Pjv_t Pjvnew;

//				Allocate Value area for Immediate Leaf
				PjvnewRaw = __JudyLAllocJV(Pop1, Pjpm);
				if (PjvnewRaw == (Pjv_t) NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjvnew = P_JV(PjvnewRaw);

//				Copy to Values to Value Leaf
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
				PjpJP->jp_Addr = (Word_t) PjvnewRaw;

//				Copy to Index to JP as an immediate Leaf
				__JudyCopyWto3(PjpJP->jp_LIndex,
					       StageA + Start, Pop1);
#else
				__JudyCopyWto3(PjpJP->jp_1Index,
					       StageA + Start, Pop1);
#endif
//				Set type, population and Index size
				PjpJP->jp_Type = cJU_JPIMMED_3_02 + Pop1 - 2;
			}
			else
			{
//		cJU_JPLEAF3
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

				PjllRaw = __JudyAllocJLL3(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjll = P_JLL(PjllRaw);

//				Copy Indexes to new Leaf
				__JudyCopyWto3((uint8_t *) Pjll, StageA + Start,
					       Pop1);
#ifdef JUDYL
//				Copy to Values to new Leaf
				Pjvnew = JL_LEAF3VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 3);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 4, PjllRaw,
					  cJU_JPLEAF3);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF4_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = StageA[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L4;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

}  // __JudyCascade4()


// ****************************************************************************
// __ J U D Y   C A S C A D E 5
//
// Cascade from a cJU_JPLEAF5 to one of the following:
//  1. if leaf is in 1 expanse:
//        compress it into a JPLEAF4
//  2. if leaf contains multiple expanses:
//        create linear or bitmap branch containing
//        each new expanse is either a:
//               JPIMMED_4_01  branch
//               JPLEAF4

FUNCTION int __JudyCascade5(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint8_t  * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF5_MAXPOP1];  // JPs of new leaves
	Word_t	   StageA    [cJU_LEAF5_MAXPOP1];
	uint8_t	   StageExp  [cJU_LEAF5_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF5);
	assert((Pjp->jp_DcdPop0 & 0xFFFFFFFFFF) == (cJU_LEAF5_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint8_t *) P_JLL(Pjp->jp_Addr);

//	Extract 5 byte index Leaf to Word_t
	__JudyCopy5toW(StageA, PLeaf, cJU_LEAF5_MAXPOP1);

//	Get the address of the Leaf and Value area
	JUDYLCODE(Pjv = JL_LEAF5VALUEAREA(PLeaf, cJU_LEAF5_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it (compare 1st, last & Index)

	CIndex = StageA[0];
	if (!JU_DIGITATSTATE(CIndex ^ StageA[cJU_LEAF5_MAXPOP1-1], 5))
	{
		Pjll_t PjllRaw;	 // pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//		Alloc a 4 byte Index Leaf
		PjllRaw = __JudyAllocJLL4(cJU_LEAF5_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy Index area into new Leaf
		__JudyCopyWto4((uint8_t *) Pjll, StageA, cJU_LEAF5_MAXPOP1);
#ifdef JUDYL
//		Copy Value area into new Leaf
		Pjvnew = JL_LEAF4VALUEAREA(Pjll, cJU_LEAF5_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_LEAF5_MAXPOP1);
#endif
		DBGCODE(JudyCheckSorted(Pjll, cJU_LEAF5_MAXPOP1, 4);)

//		Set JP to new compressed leaf values
		Pjp->jp_Addr = (Word_t) PjllRaw;
		Pjp->jp_Type = cJU_JPLEAF4;

//		Add in more Dcd bytes because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(4));

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 5 byte index Leaf to 4 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF5_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ StageA[End], 5))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 5);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 5);

			if (Pop1 == 1)	// cJU_JPIMMED_4_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 4);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
#ifdef JUDY1
			else if (Pop1 <= cJ1_IMMED4_MAXPOP1)
			{
//		cJ1_JPIMMED_4_02..3: Judy1 64

//                              Copy to Index to JP as an immediate Leaf
				__JudyCopyWto4(PjpJP->jp_1Index,
					       StageA + Start, Pop1);

//                              Set pointer, type, population and Index size
				PjpJP->jp_Type = cJ1_JPIMMED_4_02 + Pop1 - 2;
			}
#endif
			else
			{
//		cJU_JPLEAF4
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//				Get a new Leaf
				PjllRaw = __JudyAllocJLL4(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjll = P_JLL(PjllRaw);

//				Copy Indexes to new Leaf
				__JudyCopyWto4((uint8_t *) Pjll, StageA + Start,
					       Pop1);
#ifdef JUDYL
//				Copy to Values to new Leaf
				Pjvnew = JL_LEAF4VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 4);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 5, PjllRaw,
					  cJU_JPLEAF4);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF5_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = StageA[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L5;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

}  // __JudyCascade5()


// ****************************************************************************
// __ J U D Y   C A S C A D E 6
//
// Cascade from a cJU_JPLEAF6 to one of the following:
//  1. if leaf is in 1 expanse:
//        compress it into a JPLEAF5
//  2. if leaf contains multiple expanses:
//        create linear or bitmap branch containing
//        each new expanse is either a:
//               JPIMMED_5_01 ... JPIMMED_5_03  branch
//               JPIMMED_5_01  branch
//               JPLEAF5

FUNCTION int __JudyCascade6(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint8_t  * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF6_MAXPOP1];  // JPs of new leaves
	Word_t	   StageA    [cJU_LEAF6_MAXPOP1];
	uint8_t	   StageExp  [cJU_LEAF6_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF6);
	assert((Pjp->jp_DcdPop0 & 0xFFFFFFFFFFFF) == (cJU_LEAF6_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint8_t *) P_JLL(Pjp->jp_Addr);

//	Extract 6 byte index Leaf to Word_t
	__JudyCopy6toW(StageA, PLeaf, cJU_LEAF6_MAXPOP1);

//	Get the address of the Leaf and Value area
	JUDYLCODE(Pjv = JL_LEAF6VALUEAREA(PLeaf, cJU_LEAF6_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it (compare 1st, last & Index)

	CIndex = StageA[0];
	if (!JU_DIGITATSTATE(CIndex ^ StageA[cJU_LEAF6_MAXPOP1-1], 6))
	{
		Pjll_t PjllRaw;	 // pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//		Alloc a 5 byte Index Leaf
		PjllRaw = __JudyAllocJLL5(cJU_LEAF6_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy Index area into new Leaf
		__JudyCopyWto5((uint8_t *) Pjll, StageA, cJU_LEAF6_MAXPOP1);
#ifdef JUDYL
//		Copy Value area into new Leaf
		Pjvnew = JL_LEAF5VALUEAREA(Pjll, cJU_LEAF6_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_LEAF6_MAXPOP1);
#endif
		DBGCODE(JudyCheckSorted(Pjll, cJU_LEAF6_MAXPOP1, 5);)

//		Set JP to new compressed leaf values
		Pjp->jp_Addr = (Word_t) PjllRaw;
		Pjp->jp_Type = cJU_JPLEAF5;

//		Add in more Dcd bytes because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(5));

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 6 byte index Leaf to 5 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF6_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ StageA[End], 6))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 6);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 6);

			if (Pop1 == 1)	// cJU_JPIMMED_5_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 5);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
#ifdef JUDY1
			else if (Pop1 <= cJ1_IMMED5_MAXPOP1)
			{
//		cJ1_JPIMMED_5_02..3: Judy1 64

//                              Copy to Index to JP as an immediate Leaf
				__JudyCopyWto5(PjpJP->jp_1Index,
					       StageA + Start, Pop1);

//                              Set pointer, type, population and Index size
				PjpJP->jp_Type = cJ1_JPIMMED_5_02 + Pop1 - 2;
			}
#endif
			else
			{
//		cJU_JPLEAF5
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//				Get a new Leaf
				PjllRaw = __JudyAllocJLL5(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);

				Pjll = P_JLL(PjllRaw);

//				Copy Indexes to new Leaf
				__JudyCopyWto5((uint8_t *) Pjll, StageA + Start,
					       Pop1);

//				Copy to Values to new Leaf
#ifdef JUDYL
				Pjvnew = JL_LEAF5VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 5);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 6, PjllRaw,
					  cJU_JPLEAF5);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF6_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = StageA[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L6;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

}  // __JudyCascade6()


// ****************************************************************************
// __ J U D Y   C A S C A D E 7
//
// Cascade from a cJU_JPLEAF7 to one of the following:
//  1. if leaf is in 1 expanse:
//        compress it into a JPLEAF6
//  2. if leaf contains multiple expanses:
//        create linear or bitmap branch containing
//        each new expanse is either a:
//               JPIMMED_6_01 ... JPIMMED_6_02  branch
//               JPIMMED_6_01  branch
//               JPLEAF6

FUNCTION int __JudyCascade7(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	uint8_t  * PLeaf;	// pointer to leaf, explicit type.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t     CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	   StageJP   [cJU_LEAF7_MAXPOP1];  // JPs of new leaves
	Word_t	   StageA    [cJU_LEAF7_MAXPOP1];
	uint8_t	   StageExp  [cJU_LEAF7_MAXPOP1];  // Expanses of new leaves
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

	assert(Pjp->jp_Type == cJU_JPLEAF7);
	assert((Pjp->jp_DcdPop0) == (cJU_LEAF7_MAXPOP1-1));

//	Get the address of the Leaf
	PLeaf = (uint8_t *) P_JLL(Pjp->jp_Addr);

//	Extract 7 byte index Leaf to Word_t
	__JudyCopy7toW(StageA, PLeaf, cJU_LEAF7_MAXPOP1);

//	Get the address of the Leaf and Value area
	JUDYLCODE(Pjv = JL_LEAF7VALUEAREA(PLeaf, cJU_LEAF7_MAXPOP1);)

//  If Leaf is in 1 expanse -- just compress it (compare 1st, last & Index)

	CIndex = StageA[0];
	if (!JU_DIGITATSTATE(CIndex ^ StageA[cJU_LEAF7_MAXPOP1-1], 7))
	{
		Pjll_t PjllRaw;	 // pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//		Alloc a 6 byte Index Leaf
		PjllRaw = __JudyAllocJLL6(cJU_LEAF7_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy Index area into new Leaf
		__JudyCopyWto6((uint8_t *) Pjll, StageA, cJU_LEAF7_MAXPOP1);
#ifdef JUDYL
//		Copy Value area into new Leaf
		Pjvnew = JL_LEAF6VALUEAREA(Pjll, cJU_LEAF7_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_LEAF7_MAXPOP1);
#endif
		DBGCODE(JudyCheckSorted(Pjll, cJU_LEAF7_MAXPOP1, 6);)

//		Set JP to new compressed leaf values
		Pjp->jp_Addr = (Word_t) PjllRaw;
		Pjp->jp_Type = cJU_JPLEAF6;

//		Add in more Dcd bytes because compressing
		Pjp->jp_DcdPop0	|= (CIndex & cJU_DCDMASK(6));

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 7 byte index Leaf to 6 byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_LEAF7_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ StageA[End], 7))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, 7);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex, 7);

			if (Pop1 == 1)	// cJU_JPIMMED_6_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, 6);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
#ifdef JUDY1
			else if (Pop1 == cJ1_IMMED6_MAXPOP1)
			{
//		cJ1_JPIMMED_6_02:    Judy1 64

//                              Copy to Index to JP as an immediate Leaf
				__JudyCopyWto6(PjpJP->jp_1Index,
					       StageA + Start, 2);

//                              Set pointer, type, population and Index size
				PjpJP->jp_Type = cJ1_JPIMMED_6_02;
			}
#endif
			else
			{
//		cJU_JPLEAF6
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.

//				Get a new Leaf
				PjllRaw = __JudyAllocJLL6(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t)NULL)
					FREEALLEXIT(ExpCnt, StageJP, Pjpm);
				Pjll = P_JLL(PjllRaw);

//				Copy Indexes to new Leaf
				__JudyCopyWto6((uint8_t *) Pjll, StageA + Start,
					       Pop1);
#ifdef JUDYL
//				Copy to Values to new Leaf
				Pjvnew = JL_LEAF6VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 6);)

				FORMNEWJP(PjpJP, Pjp, CIndex, Pop1, 7, PjllRaw,
					  cJU_JPLEAF6);
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_LEAF7_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = StageA[End];
			Start  = End;
		}
	}

//      Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L7;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);
	}
	return(1);

}  // __JudyCascade7()

#endif // JU_64BIT


// ****************************************************************************
// __ J U D Y   C A S C A D E   L
//
// (Compressed) cJU_LEAF3[7], cJ1_JPBRANCH_L.
//
// Cascade from a JAPLeaf (under Pjp) to one of the following:
//  1. if JAPLeaf is in 1 expanse:
//        create linear branch with a JPLEAF3[7] under it
//  2. JAPLeaf contains multiple expanses:
//        create linear or bitmap branch containing new expanses
//        each new expanse is either a: 32   64
//               JPIMMED_3_01  branch    Y    N
//               JPIMMED_7_01  branch    N    Y
//               JPLEAF3                 Y    N
//               JPLEAF7                 N    Y

FUNCTION int __JudyCascadeL(
	Pjp_t	   Pjp,
	Pvoid_t	   Pjpm)
{
	Pjlw_t	   Pjlw;	// leaf to work on.
	Word_t	   End, Start;	// temporaries.
	Word_t	   ExpCnt;	// count of expanses of splay.
	Word_t	   CIndex;	// current Index word.
JUDYLCODE(Pjv_t	   Pjv;)	// value area of leaf.

//	Temp staging for parts(Leaves) of newly splayed leaf
	jp_t	StageJP [cJU_JAPLEAF_MAXPOP1];
	uint8_t	StageExp[cJU_JAPLEAF_MAXPOP1];
	uint8_t	   SubJPCount[cJU_NUMSUBEXPB];     // JPs in each subexpanse
	jbb_t      StageJBB;                       // staged bitmap branch

//	Get the address of the Leaf
	Pjlw = P_JLW(Pjp->jp_Addr);

	assert(Pjp->jp_Type == cJU_JAPLEAF);
	assert(Pjlw[0] == (cJU_JAPLEAF_MAXPOP1 - 1));

//	Get pointer to Value area of old Leaf
	JUDYLCODE(Pjv = JL_LEAFWVALUEAREA(Pjlw, cJU_JAPLEAF_MAXPOP1);)

	Pjlw++;		// Now point to Index area

// If Leaf is in 1 expanse -- first compress it (compare 1st, last & Index):

	CIndex = Pjlw[0];	// also used far below
	if (!JU_DIGITATSTATE(CIndex ^ Pjlw[cJU_JAPLEAF_MAXPOP1 - 1],
			     cJU_ROOTSTATE))
	{
		Pjll_t PjllRaw;		// pointer to new leaf.
		Pjll_t Pjll;
      JUDYLCODE(Pjv_t  Pjvnew;)		// value area of new leaf.

//		Get the common expanse to all elements in Leaf
		StageExp[0] = JU_DIGITATSTATE(CIndex, cJU_ROOTSTATE);

//		Alloc a 3[7] byte Index Leaf
#ifdef JU_64BIT
		PjllRaw	= __JudyAllocJLL7(cJU_JAPLEAF_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjlb_t)NULL) return(-1);  // out of memory

		Pjll = P_JLL(PjllRaw);

//		Copy JAPLeaf to a cJU_JPLEAF7
		__JudyCopyWto7((uint8_t *) Pjll, Pjlw, cJU_JAPLEAF_MAXPOP1);
#ifdef JUDYL
//		Get the Value area of new Leaf
		Pjvnew = JL_LEAF7VALUEAREA(Pjll, cJU_JAPLEAF_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_JAPLEAF_MAXPOP1);
#endif
		StageJP[0].jp_Type = cJU_JPLEAF7;

		DBGCODE(JudyCheckSorted(Pjll, cJU_JAPLEAF_MAXPOP1, 7);)
#else // JU_64BIT - 32 Bit
		PjllRaw	= __JudyAllocJLL3(cJU_JAPLEAF_MAXPOP1, Pjpm);
		if (PjllRaw == (Pjll_t) NULL) return(-1);

		Pjll = P_JLL(PjllRaw);

//		Copy JAPLeaf to a cJU_JPLEAF3
		__JudyCopyWto3((uint8_t *) Pjll, Pjlw, cJU_JAPLEAF_MAXPOP1);
#ifdef JUDYL
//		Get the Value area of new Leaf
		Pjvnew = JL_LEAF3VALUEAREA(Pjll, cJU_JAPLEAF_MAXPOP1);
		JU_COPYMEM(Pjvnew, Pjv, cJU_JAPLEAF_MAXPOP1);
#endif
		StageJP[0].jp_Type = cJU_JPLEAF3;

		DBGCODE(JudyCheckSorted(Pjll, cJU_JAPLEAF_MAXPOP1, 3);)
#endif // JU_64BIT

//		Stuff the new leaf in the Linear branches future JP
		StageJP[0].jp_Addr    = (Word_t) PjllRaw;
		StageJP[0].jp_DcdPop0 = cJU_JAPLEAF_MAXPOP1 - 1; // -1

//		Following not needed because cJU_DCDMASK(3[7]) is == 0
//////		StageJP[0].jp_DcdPop0	|= (CIndex & cJU_DCDMASK(3[7]));

//		Create a 1 element Linear branch
		if (__JudyCreateBranchL(Pjp, StageJP, StageExp, 1, Pjpm) == -1)
		    return(-1);

//		Change the type of callers JP
		Pjp->jp_Type = cJU_JPBRANCH_L;

		return(1);
	}

//  Else in 2+ expanses, splay Leaf into smaller leaves at higher compression

	StageJBB = StageJBBZero;       // zero staged bitmap branch
	ZEROJP(SubJPCount);

//	Splay the 4[8] byte Index Leaf to 3[7] byte Index Leaves
	for (ExpCnt = Start = 0, End = 1; ; End++)
	{
//		Check if new expanse or last one
		if (	(End == cJU_JAPLEAF_MAXPOP1)
				||
			(JU_DIGITATSTATE(CIndex ^ Pjlw[End], cJU_ROOTSTATE))
		   )
		{
//			Build a leaf below the previous expanse

			Pjp_t  PjpJP	= StageJP + ExpCnt;
			Word_t Pop1	= End - Start;
			Word_t expanse = JU_DIGITATSTATE(CIndex, cJU_ROOTSTATE);
			Word_t subexp  = expanse / cJU_BITSPERSUBEXPB;
//
//                      set the bit that is the current expanse
			JU_JBB_BITMAP(&StageJBB, subexp) |= JU_BITPOSMASKB(expanse);
#ifdef SUBEXPCOUNTS
			StageJBB.jbb_subPop1[subexp] += Pop1; // pop of subexpanse
#endif
//                      count number of expanses in each subexpanse
			SubJPCount[subexp]++;

//			Save byte expanse of leaf
			StageExp[ExpCnt] = JU_DIGITATSTATE(CIndex,
							   cJU_ROOTSTATE);

			if (Pop1 == 1)	// cJU_JPIMMED_3[7]_01
			{
				FORMIMMJP(PjpJP, Pjp, CIndex, cJU_ROOTSTATE-1);
				JUDY1CODE(PjpJP->jp_Addr = 0;)
				JUDYLCODE(PjpJP->jp_Addr = Pjv[Start];)
			}
#ifdef JUDY1
#ifdef  JU_64BIT
			else if (Pop1 <= cJ1_IMMED7_MAXPOP1)
#else
			else if (Pop1 <= cJ1_IMMED3_MAXPOP1)
#endif
			{
//		cJ1_JPIMMED_3_02   :  Judy1 32
//		cJ1_JPIMMED_7_02   :  Judy1 64
//                              Copy to JP as an immediate Leaf
#ifdef  JU_64BIT
				__JudyCopyWto7(PjpJP->jp_1Index, Pjlw+Start, 2);
				PjpJP->jp_Type = cJ1_JPIMMED_7_02;
#else
				__JudyCopyWto3(PjpJP->jp_1Index, Pjlw+Start, 2);
				PjpJP->jp_Type = cJ1_JPIMMED_3_02;
#endif // 32 Bit
			}
#endif // JUDY1
			else // Linear Leaf JPLEAF3[7]
			{
//		cJU_JPLEAF3[7]
				Pjll_t PjllRaw;	 // pointer to new leaf.
				Pjll_t Pjll;
		      JUDYLCODE(Pjv_t  Pjvnew;)	 // value area of new leaf.
#ifdef JU_64BIT
				PjllRaw = __JudyAllocJLL7(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t) NULL) return(-1);
				Pjll = P_JLL(PjllRaw);

				__JudyCopyWto7((uint8_t *) Pjll, Pjlw + Start,
					       Pop1);
				PjpJP->jp_Type = cJU_JPLEAF7;
#ifdef JUDYL
				Pjvnew = JL_LEAF7VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif // JUDYL
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 7);)
#else // JU_64BIT - 32 Bit
				PjllRaw = __JudyAllocJLL3(Pop1, Pjpm);
				if (PjllRaw == (Pjll_t) NULL) return(-1);
				Pjll = P_JLL(PjllRaw);

				__JudyCopyWto3((uint8_t *) Pjll, Pjlw + Start,
					       Pop1);
				PjpJP->jp_Type = cJU_JPLEAF3;
#ifdef JUDYL
				Pjvnew = JL_LEAF3VALUEAREA(Pjll, Pop1);
				JU_COPYMEM(Pjvnew, Pjv + Start, Pop1);
#endif // JUDYL
				DBGCODE(JudyCheckSorted(Pjll, Pop1, 3);)
#endif // JU_64BIT
//				Set up the JP - no DCD field at this level
				PjpJP->jp_DcdPop0 = Pop1 - 1;
				PjpJP->jp_Addr	  = (Word_t) PjllRaw;
			}
			ExpCnt++;
//                      Done?
			if (End == cJU_JAPLEAF_MAXPOP1) break;

//			New Expanse, Start and Count
			CIndex = Pjlw[End];
			Start  = End;
		}
	}

// Now put all the Leaves below a BranchL or BranchB:
	if (ExpCnt <= cJU_BRANCHLMAXJPS) // put the Leaves below a BranchL
	{
	    if (__JudyCreateBranchL(Pjp, StageJP, StageExp, ExpCnt,
			Pjpm) == -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_L;
	}
	else
	{
	    if (__JudyStageJBBtoJBB(Pjp, &StageJBB, StageJP, SubJPCount, Pjpm)
		== -1) FREEALLEXIT(ExpCnt, StageJP, Pjpm);

	    Pjp->jp_Type = cJU_JPBRANCH_B;  // cJU_JAPLEAF is out of sequence
	}
	return(1);

} // __JudyCascadeL()
