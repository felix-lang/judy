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

// @(#) $Revision: 4.43 $ $Source: /judy/src/JudyCommon/JudyGet.c $
//
// Judy1Test() and JudyLGet() functions for Judy1 and JudyL.
// Compile with one of -DJUDY1 or -DJUDYL.

#if (! (JUDY1 || JUDYL))
    Error:  One of -DJUDY1 or -DJUDYL must be specified.
#endif

#ifdef JUDY1
#include "Judy1.h"
#else
#include "JudyL.h"
#endif

#include "JudyPrivate1L.h"

#ifdef TRACEJPR			// different macro name, for "retrieval" only.
#include "JudyPrintJP.c"
#endif


// ****************************************************************************
// J U D Y   1   T E S T
// J U D Y   L   G E T
//
// See the manual entry for details.  Note support for "shortcut" entries to
// trees known to start with a JPM.

#ifdef JUDY1

#ifdef JUDYGETINLINE
FUNCTION int __Judy1Test(
#else
FUNCTION int Judy1Test(
#endif

#else  // JUDYL

#ifdef JUDYGETINLINE
FUNCTION PPvoid_t __JudyLGet(
#else
FUNCTION PPvoid_t JudyLGet(
#endif

#endif // JUDYL

#ifdef JUDYGETINLINE
	Pvoid_t   Pjpmvoid,	// Pjpm in array with pop1 > 31.
	Word_t	  Index)	// to retrieve.
#else
	Pcvoid_t  PArray,	// from which to retrieve.
	Word_t	  Index,	// to retrieve.
	PJError_t PJError)	// optional, for returning error info.
#endif
{
	Pjp_t	  Pjp;		// current JP while walking the tree.
	Pjpm_t	  Pjpm;		// for global accounting.
#ifndef JUDYGETINLINE
	uint8_t	  JAPtype;	// type of JAP (root pointer).
#endif
	uint8_t	  Digit;	// byte just decoded from Index.
	Word_t	  Pop1;		// leaf population (number of indexes).
	Pjll_t	  Pjll;		// pointer to LeafL.
	DBGCODE(uint8_t ParentJPType;)

#ifdef JUDYGETINLINE

	Pjpm = (Pjpm_t) Pjpmvoid;	// cc optimizes this out.
	Pjp  = &(Pjpm->jpm_JP);		// JP to first node (always a branch).

// Most frequent subcase is a BranchU below the JPM; otherwise it must be a
// BranchB or BranchL:

	if (Pjp->jp_Type == cJU_JPBRANCH_U) goto JudyBranchU;
	goto ContinueWalk;

#else // ! JUDYGETINLINE


// ****************************************************************************
// PROCESS TOP LEVEL "JAP" BRANCHES AND LEAVES:

	JAPtype = JAPTYPE(PArray);

// Use "if" statements instead of a switch to "force" the order of the "switch"
// so the frequent cases are faster:
//
// Most common case first:

	if (JAPtype == cJU_JAPBRANCH)
	{
	    Pjpm = P_JPM(PArray);
	    Pjp = &(Pjpm->jpm_JP);	// top branch is below JPM.

// Most frequent subcase is a BranchU below the JPM; otherwise it must be a
// BranchB or BranchL:

	    if (Pjp->jp_Type == cJU_JPBRANCH_U) goto JudyBranchU;
	    goto ContinueWalk;
	}

#if (LOW_POP && JUDYL)
	else if (JAPtype == cJL_JAPLEAF_POPU1)
	{
	    Pjlw_t Pjlw = P_JLW(PArray);	// first word of leaf.

	    if (Index == Pjlw[0]) return((PPvoid_t) (Pjlw + 1 + 0));
	    return((PPvoid_t) NULL);
	}
	else if (JAPtype == cJL_JAPLEAF_POPU2)
	{
	    if (Pjlw[0] == Index)  return((PPvoid_t) (Pjlw + 2 + 0));
	    if (Pjlw[1] == Index)  return((PPvoid_t) (Pjlw + 2 + 1));
	    return((PPvoid_t) NULL);
	}
#endif // (LOW_POP && JUDYL)

	else if (JAPtype == cJU_JAPLEAF)
	{
	    Pjlw_t Pjlw = P_JLW(PArray);	// first word of leaf.
	    int    posidx;			// signed offset in leaf.

	    Pop1   = Pjlw[0] + 1;
	    posidx = __JudySearchLeafW(Pjlw + 1, Pop1, Index);

	    if (posidx >= 0)
	    {
      JUDY1CODE(return(1);)
      JUDYLCODE(return((PPvoid_t) (JL_LEAFWVALUEAREA(Pjlw, Pop1) + posidx));)
	    }

  JUDY1CODE(return(0);)
  JUDYLCODE(return((PPvoid_t) NULL);)
	}
	else if (PArray == (Pcvoid_t) NULL)	// empty array.
	{
  JUDY1CODE(return(0);)
  JUDYLCODE(return((PPvoid_t) NULL);)
	}

// Any other case is an invalid root pointer (including cJU_JAPNULL with
// non-null address part):
//
// Note:  Here, and in other places, do not fold these macros into a generic
// JU_ERRNO_NOTJUDY, because the following is more explicit, AND yields a
// different je_ErrID (line number) for each case.

	JUDY1CODE(JU_SET_ERRNO(PJError, JU_ERRNO_NOTJUDY1); return(JERRI );)
	JUDYLCODE(JU_SET_ERRNO(PJError, JU_ERRNO_NOTJUDYL); return(PPJERR);)

#endif // ! JUDYGETINLINE


// ****************************************************************************
// WALK THE JUDY TREE USING A STATE MACHINE:

ContinueWalk:		// for going down one level; come here with Pjp set.

#ifdef TRACEJPR
	JudyPrintJP(Pjp, "g", __LINE__);
#endif
	switch (Pjp->jp_Type)
	{

// Ensure the switch table starts at 0 for speed; otherwise more code is
// executed:

	case 0: goto ReturnCorrupt;	// save a little code.


// ****************************************************************************
// JPNULL*:
//
// Note:  These are legitimate in a BranchU (only) and do not constitute a
// fault.

	case cJU_JPNULL1:
	case cJU_JPNULL2:
	case cJU_JPNULL3:
#ifdef JU_64BIT
	case cJU_JPNULL4:
	case cJU_JPNULL5:
	case cJU_JPNULL6:
	case cJU_JPNULL7:
#endif
	    assert(ParentJPType >= cJU_JPBRANCH_U2);
	    assert(ParentJPType <= cJU_JPBRANCH_U);
      JUDY1CODE(return(0);)
      JUDYLCODE(return((PPvoid_t) NULL);)


// ****************************************************************************
// JPBRANCH_L*:
//
// Note:  The use of JU_DCDNOTMATCHINDEX() in branches is not strictly
// required,since this can be done at leaf level, but it costs nothing to do it
// sooner, and it aborts an unnecessary traversal sooner.

	case cJU_JPBRANCH_L2:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 2)) break;
	    Digit = JU_DIGITATSTATE(Index, 2);
	    goto JudyBranchL;

	case cJU_JPBRANCH_L3:

#ifdef JU_64BIT // otherwise it's a no-op:
	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 3)) break;
#endif
	    Digit = JU_DIGITATSTATE(Index, 3);
	    goto JudyBranchL;

#ifdef JU_64BIT
	case cJU_JPBRANCH_L4:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 4)) break;
	    Digit = JU_DIGITATSTATE(Index, 4);
	    goto JudyBranchL;

	case cJU_JPBRANCH_L5:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 5)) break;
	    Digit = JU_DIGITATSTATE(Index, 5);
	    goto JudyBranchL;

	case cJU_JPBRANCH_L6:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 6)) break;
	    Digit = JU_DIGITATSTATE(Index, 6);
	    goto JudyBranchL;

	case cJU_JPBRANCH_L7:

	    // JU_DCDNOTMATCHINDEX() would be a no-op.
	    Digit = JU_DIGITATSTATE(Index, 7);
	    goto JudyBranchL;

#endif // JU_64BIT

	case cJU_JPBRANCH_L:
	{
	    Pjbl_t Pjbl;
	    int    posidx;

	    Digit = JU_DIGITATSTATE(Index, cJU_ROOTSTATE);

// Common code for all BranchL's; come here with Digit set:

JudyBranchL:
	    Pjbl = P_JBL(Pjp->jp_Addr);

	    posidx = 0;

	    do {
		if (Pjbl->jbl_Expanse[posidx] == Digit)
		{			// found Digit; continue traversal:
		    DBGCODE(ParentJPType = Pjp->jp_Type;)
		    Pjp = (Pjbl->jbl_jp) + posidx;
		    goto ContinueWalk;
		}
	    } while (++posidx != Pjbl->jbl_NumJPs);

	    break;
	}


// ****************************************************************************
// JPBRANCH_B*:

	case cJU_JPBRANCH_B2:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 2)) break;
	    Digit = JU_DIGITATSTATE(Index, 2);
	    goto JudyBranchB;

	case cJU_JPBRANCH_B3:

#ifdef JU_64BIT // otherwise it's a no-op:
	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 3)) break;
#endif
	    Digit = JU_DIGITATSTATE(Index, 3);
	    goto JudyBranchB;


#ifdef JU_64BIT
	case cJU_JPBRANCH_B4:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 4)) break;
	    Digit = JU_DIGITATSTATE(Index, 4);
	    goto JudyBranchB;

	case cJU_JPBRANCH_B5:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 5)) break;
	    Digit = JU_DIGITATSTATE(Index, 5);
	    goto JudyBranchB;

	case cJU_JPBRANCH_B6:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 6)) break;
	    Digit = JU_DIGITATSTATE(Index, 6);
	    goto JudyBranchB;

	case cJU_JPBRANCH_B7:

	    // JU_DCDNOTMATCHINDEX() would be a no-op.
	    Digit = JU_DIGITATSTATE(Index, 7);
	    goto JudyBranchB;

#endif // JU_64BIT

	case cJU_JPBRANCH_B:
	{
	    Pjbb_t    Pjbb;
	    Word_t    subexp;	// in bitmap, 0..7.
	    BITMAPB_t BitMap;	// for one subexpanse.
	    BITMAPB_t BitMask;	// bit in BitMap for Index's Digit.

	    Digit = JU_DIGITATSTATE(Index, cJU_ROOTSTATE);

// Common code for all BranchB's; come here with Digit set:

JudyBranchB:
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjbb   = P_JBB(Pjp->jp_Addr);
	    subexp = Digit / cJU_BITSPERSUBEXPB;

	    BitMap = JU_JBB_BITMAP(Pjbb, subexp);
	    Pjp	   = P_JP(JU_JBB_PJP(Pjbb, subexp));

	    BitMask = JU_BITPOSMASKB(Digit);

// No JP in subexpanse for Index => Index not found:

	    if (! (BitMap & BitMask)) break;

// Count JPs in the subexpanse below the one for Index:

	    Pjp += __JudyCountBitsB(BitMap & (BitMask - 1));

	    goto ContinueWalk;

	} // case cJU_JPBRANCH_B*


// ****************************************************************************
// JPBRANCH_U*:
//
// Notice the reverse order of the cases, and falling through to the next case,
// for performance.

#ifdef notdef			// unused, arrive here via shortcut goto:
	case cJU_JPBRANCH_U:
#endif

JudyBranchU:	// come here on entry to the state machine, with Pjp set.

	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, cJU_ROOTSTATE);

// If not a BranchU, traverse; otherwise fall into the next case, which makes
// this very fast code for a large Judy array (mainly BranchU's), especially
// when branches are already in the cache, such as for prev/next:

#ifndef JU_64BIT
	    if (Pjp->jp_Type != cJU_JPBRANCH_U3) goto ContinueWalk;
#else
	    if (Pjp->jp_Type != cJU_JPBRANCH_U7) goto ContinueWalk;
#endif

#ifdef JU_64BIT
	case cJU_JPBRANCH_U7:

	    // JU_DCDNOTMATCHINDEX() would be a no-op.
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 7);

	    if (Pjp->jp_Type != cJU_JPBRANCH_U6) goto ContinueWalk;
	    // and fall through.

	case cJU_JPBRANCH_U6:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 6)) break;
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 6);

	    if (Pjp->jp_Type != cJU_JPBRANCH_U5) goto ContinueWalk;
	    // and fall through.

	case cJU_JPBRANCH_U5:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 5)) break;
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 5);

	    if (Pjp->jp_Type != cJU_JPBRANCH_U4) goto ContinueWalk;
	    // and fall through.

	case cJU_JPBRANCH_U4:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 4)) break;
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 4);

	    if (Pjp->jp_Type != cJU_JPBRANCH_U3) goto ContinueWalk;
	    // and fall through.

#endif // JU_64BIT

	case cJU_JPBRANCH_U3:

#ifdef JU_64BIT // otherwise it's a no-op:
	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 3)) break;
#endif
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 3);

	    if (Pjp->jp_Type != cJU_JPBRANCH_U2) goto ContinueWalk;
	    // and fall through.

	case cJU_JPBRANCH_U2:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 2)) break;
	    DBGCODE(ParentJPType = Pjp->jp_Type;)
	    Pjp = JU_JBU_PJP(Pjp, Index, 2);

// Note:  BranchU2 is a special case that must continue traversal to a leaf,
// immed, full, or null type:

	    goto ContinueWalk;


// ****************************************************************************
// JPLEAF*:
//
// Note:  Here the calls of JU_DCDNOTMATCHINDEX() are necessary and check
// whether Index is out of the expanse of a narrow pointer.

#if (JUDYL || (! JU_64BIT))

	case cJU_JPLEAF1:
	{
	    int posidx;		// signed offset in leaf.

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 1)) break;

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf1(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF1VALUEAREA(Pjll, Pop1) + posidx));)
	}

#endif // (JUDYL || (! JU_64BIT))

	case cJU_JPLEAF2:
	{
	    int posidx;		// signed offset in leaf.

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 2)) break;

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf2(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF2VALUEAREA(Pjll, Pop1) + posidx));)
	}
	case cJU_JPLEAF3:
	{
	    int posidx;		// signed offset in leaf.

#ifdef JU_64BIT // otherwise it's a no-op:
	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 3)) break;
#endif

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf3(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF3VALUEAREA(Pjll, Pop1) + posidx));)
	}
#ifdef JU_64BIT
	case cJU_JPLEAF4:
	{
	    int posidx;		// signed offset in leaf.

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 4)) break;

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf4(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF4VALUEAREA(Pjll, Pop1) + posidx));)
	}
	case cJU_JPLEAF5:
	{
	    int posidx;		// signed offset in leaf.

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 5)) break;

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf5(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF5VALUEAREA(Pjll, Pop1) + posidx));)
	}

	case cJU_JPLEAF6:
	{
	    int posidx;		// signed offset in leaf.

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 6)) break;

	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf6(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF6VALUEAREA(Pjll, Pop1) + posidx));)
	}
	case cJU_JPLEAF7:
	{
	    int posidx;		// signed offset in leaf.

	    // JU_DCDNOTMATCHINDEX() would be a no-op.
	    Pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;
	    Pjll = P_JLL(Pjp->jp_Addr);

	    if ((posidx = __JudySearchLeaf7(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (JL_LEAF7VALUEAREA(Pjll, Pop1) + posidx));)
	}
#endif // JU_64BIT


// ****************************************************************************
// JPLEAF_B1:

	case cJU_JPLEAF_B1:
	{
	    Pjlb_t    Pjlb;
#ifdef JUDYL
	    int       posidx;
	    Word_t    subexp;	// in bitmap, 0..7.
	    BITMAPL_t BitMap;	// for one subexpanse.
	    BITMAPL_t BitMask;	// bit in BitMap for Index's Digit.
	    Pjv_t     Pjv;
#endif
	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 1)) break;

	    Pjlb = P_JLB(Pjp->jp_Addr);

#ifdef JUDY1

// Simply check if Index's bit is set in the bitmap:

	    if (JU_BITMAPTESTL(Pjlb, Index)) return(1);
	    break;

#else // JUDYL

// JudyL is much more complicated because of value area subarrays:

	    Digit   = JU_DIGITATSTATE(Index, 1);
	    subexp  = Digit / cJU_BITSPERSUBEXPL;
	    BitMap  = JU_JLB_BITMAP(Pjlb, subexp);
	    BitMask = JU_BITPOSMASKL(Digit);

// No value in subexpanse for Index => Index not found:

	    if (! (BitMap & BitMask)) break;

// Count value areas in the subexpanse below the one for Index:

	    Pjv = P_JV(JL_JLB_PVALUE(Pjlb, subexp));
	    assert(Pjv != (Pjv_t) NULL);
	    posidx = __JudyCountBitsL(BitMap & (BitMask - 1));

	    return((PPvoid_t) (Pjv + posidx));

#endif // JUDYL

	} // case cJU_JPLEAF_B1

#ifdef JUDY1

// ****************************************************************************
// JPFULLPOPU1:
//
// If the Index is in the expanse, it is necessarily valid (found).

	case cJ1_JPFULLPOPU1:

	    if (JU_DCDNOTMATCHINDEX(Index, Pjp->jp_DcdPop0, 1)) break;
	    return(1);

#endif // JUDY1

// Macro used for even (1, 2, 4) sized IMMED_*_02 and up:

#define SEARCHIMMEDEVEN(LeafType,Pjp,Index,JPType)			\
    {									\
	LeafType _Index = (Index);  /* necessary for masking Index */	\
	JUDY1CODE(LeafType *_PLeaf = (LeafType *)((Pjp)->jp_1Index);)	\
	JUDYLCODE(LeafType *_PLeaf = (LeafType *)((Pjp)->jp_LIndex);)	\
     /* if beyond end of leaf */					\
	if ((_Index) > _PLeaf[(Pjp)->jp_Type - (JPType) + 1]) break;	\
     /* find it */							\
	for(;;) { if (_Index <= *_PLeaf) break; _PLeaf++; }		\
	if ((_Index) == *_PLeaf) /* found ? */				\
	{								\
	    JUDY1CODE(return(1);)					\
	    JUDYLCODE(return((PPvoid_t)(P_JV((Pjp)->jp_Addr) +		\
			     (_PLeaf - (LeafType *)((Pjp)->jp_LIndex))));) \
	}								\
	break; /* exit not found */					\
    }


// ****************************************************************************
// JPIMMED*:
//
// Note that the contents of jp_DcdPop0 are different for cJU_JPIMMED_*_01:

	case cJU_JPIMMED_1_01:
	case cJU_JPIMMED_2_01:
	case cJU_JPIMMED_3_01:
#ifdef JU_64BIT
	case cJU_JPIMMED_4_01:
	case cJU_JPIMMED_5_01:
	case cJU_JPIMMED_6_01:
	case cJU_JPIMMED_7_01:
#endif
	    if (Pjp->jp_DcdPop0 != JU_TRIMTODCDSIZE(Index)) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) &(Pjp->jp_Addr));)  // immediate value area.

	case cJU_JPIMMED_1_02:
	case cJU_JPIMMED_1_03:
#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_1_04:
	case cJU_JPIMMED_1_05:
	case cJU_JPIMMED_1_06:
	case cJU_JPIMMED_1_07:
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_1_08:
	case cJ1_JPIMMED_1_09:
	case cJ1_JPIMMED_1_10:
	case cJ1_JPIMMED_1_11:
	case cJ1_JPIMMED_1_12:
	case cJ1_JPIMMED_1_13:
	case cJ1_JPIMMED_1_14:
	case cJ1_JPIMMED_1_15:
#endif
	    SEARCHIMMEDEVEN(uint8_t, Pjp, Index, cJU_JPIMMED_1_02);

#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_2_02:
	case cJU_JPIMMED_2_03:
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_2_04:
	case cJ1_JPIMMED_2_05:
	case cJ1_JPIMMED_2_06:
	case cJ1_JPIMMED_2_07:
#endif

#if (JUDY1 || JU_64BIT)
	    SEARCHIMMEDEVEN(uint16_t, Pjp, Index, cJU_JPIMMED_2_02);
#endif

#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_3_02:
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_3_03:
	case cJ1_JPIMMED_3_04:
	case cJ1_JPIMMED_3_05:
#endif
#if (JUDY1 || JU_64BIT)
	{
	    int posidx;
#ifdef JUDY1
	    Pjll = (Pjll_t)(Pjp->jp_1Index);
#else
	    Pjv_t Pjv;
	    Pjll = (Pjll_t) (Pjp->jp_LIndex);
	    Pjv	 = P_JV(Pjp->jp_Addr);
#endif
	    Pop1 = Pjp->jp_Type - cJU_JPIMMED_3_02 + 2;

	    if ((posidx = __JudySearchLeaf3(Pjll, Pop1, Index)) < 0) break;

  JUDY1CODE(return(1);)
  JUDYLCODE(return((PPvoid_t) (Pjv + posidx));)
	}
#endif

#if (JUDY1 && JU_64BIT)

	case cJ1_JPIMMED_4_02:
	case cJ1_JPIMMED_4_03:
	    SEARCHIMMEDEVEN(uint32_t, Pjp, Index, cJ1_JPIMMED_4_02);

	case cJ1_JPIMMED_5_02:
	case cJ1_JPIMMED_5_03:

	    Pop1 = Pjp->jp_Type - cJ1_JPIMMED_5_02 + 2;
	    Pjll = (Pjll_t) (Pjp->jp_1Index);

	    if (__JudySearchLeaf5(Pjll, Pop1, Index) < 0) break;
	    return(1);

	case cJ1_JPIMMED_6_02:

	    Pjll = (Pjll_t) (Pjp->jp_1Index);

	    if (__JudySearchLeaf6(Pjll, 2, Index) < 0) break;
	    return(1);

	case cJ1_JPIMMED_7_02:

	    Pjll = (Pjll_t) (Pjp->jp_1Index);

	    if (__JudySearchLeaf7(Pjll, 2, Index) < 0) break;
	    return(1);

#endif // (JUDY1 && JU_64BIT)


// ****************************************************************************
// INVALID JP TYPE:

	default:

ReturnCorrupt:

#ifdef JUDYGETINLINE	// Pjpm is known to be non-null:
	    JU_SET_ERRNO_NONNULL(Pjpm, JU_ERRNO_CORRUPT);
#else
	    JU_SET_ERRNO(PJError, JU_ERRNO_CORRUPT);
#endif
	    JUDY1CODE(return(JERRI );)
	    JUDYLCODE(return(PPJERR);)

	} // switch on JP type

JUDY1CODE(return(0);)
JUDYLCODE(return((PPvoid_t) NULL);)

} // Judy1Test() / JudyLGet()


#ifndef JUDYGETINLINE	// only compile the following function once:
#ifdef DEBUG

// ****************************************************************************
// J U D Y   C H E C K   P O P
//
// Given a pointer to a Judy array, traverse the entire array to ensure
// population counts add up correctly.  This can catch various coding errors.
//
// Since walking the entire tree is probably time-consuming, enable this
// function by setting env parameter $CHECKPOP to first call at which to start
// checking.  Note:  This function is called both from insert and delete code.
//
// Note:  Even though this function does nothing useful for JAP leaves, it's
// good practice to call it anyway, and cheap too.
//
// TBD:  This is a debug-only check function similar to JudyCheckSorted(), but
// since it walks the tree it is Judy1/JudyL-specific and must live in a source
// file that is built both ways.
//
// TBD:  As feared, enabling this code for every insert/delete makes Judy
// deathly slow, even for a small tree (10K indexes).  It's not so bad if
// present but disabled (<1% slowdown measured).  Still, should it be ifdef'd
// other than DEBUG and/or called less often?
//
// TBD:  Should this "population checker" be expanded to a comprehensive tree
// checker?  It currently detects invalid JAP/JP types as well as inconsistent
// pop1's.  Other possible checks, all based on essentially redundant data in
// the Judy tree, include:
//
// - Zero LS bits in jp_Addr field.
//
// - Correct Dcd bits.
//
// - Consistent JP types (always descending down the tree).
//
// - Sorted linear lists in BranchL's and leaves (using JudyCheckSorted(), but
//   ideally that function is already called wherever appropriate after any
//   linear list is modified).
//
// - Any others possible?

#include <stdlib.h>		// for getenv() and atol().

static Word_t JudyCheckPopSM(Pjp_t Pjp, Word_t RootPop1);

FUNCTION void JudyCheckPop(
	Pvoid_t PArray)
{
static	bool_t	checked	= FALSE;	// already checked env parameter.
static	bool_t	enabled	= FALSE;	// env parameter set.
static	bool_t	active	= FALSE;	// calls >= callsmin.
static	Word_t	callsmin;		// start point from $CHECKPOP.
static	Word_t	calls = 0;		// times called so far.

	Word_t	JAPtype;		// JAP type part of PArray.


// CHECK FOR EXTERNAL ENABLING:

	if (! checked)			// only check once.
	{
	    char * value;		// for getenv().

	    checked = TRUE;

	    if ((value = getenv("CHECKPOP")) == (char *) NULL)
	    {
#ifdef notdef
// Take this out because nightly tests want to be flavor-independent; it's not
// OK to emit special non-error output from the debug flavor:

		(void) puts("JudyCheckPop() present but not enabled by "
			    "$CHECKPOP env parameter; set it to the number of "
			    "calls at which to begin checking");
#endif
		return;
	    }

	    callsmin = atol(value);	// note: non-number evaluates to 0.
	    enabled  = TRUE;

	    (void) printf("JudyCheckPop() present and enabled; callsmin = "
			  "%lu\n", callsmin);
	}
	else if (! enabled) return;

// Previously or just now enabled; check if non-active or newly active:

	if (! active)
	{
	    if (++calls < callsmin) return;

	    (void) printf("JudyCheckPop() activated at call %lu\n", calls);
	    active = TRUE;
	}


// START AT TOP OF TREE:

	JAPtype = JAPTYPE(PArray);

	switch (JAPtype)
	{
	case cJU_JAPNULL:
	    {
		Pjlw_t Pjlw = P_JLW(PArray);	// first word of leaf.
		assert(Pjlw == (Pjlw_t) NULL);	// rest of word must be null.
		return;				// valid null JAP.
	    }

#if (LOW_POP && JUDYL)

// Little or no pop checking is possible for these types, but see function
// header comments:

	  case cJL_JAPLEAF_POPU1: return;
	  case cJL_JAPLEAF_POPU2: return;
#endif

	case cJU_JAPLEAF:
	    {
		Pjlw_t Pjlw = P_JLW(PArray);	// first word of leaf.
		assert(*Pjlw + 1 <= cJU_JAPLEAF_MAXPOP1);
		return;
	    }

// Check JPM pop0 against tree, recursively:
//
// Note:  The traversal code in JudyCheckPopSM() is simplest when the case
// statement for each JP type compares the pop1 for that JP to its subtree (if
// any) after traversing the subtree (that's the hard part) and adding up
// actual pop1's.  A top branch's JP in the JPM does not have room for a
// full-word pop1, so pass it in as a special case.

	case cJU_JAPBRANCH:
	    {
		Pjpm_t Pjpm = P_JPM(PArray);
		(void) JudyCheckPopSM(&(Pjpm->jpm_JP), Pjpm->jpm_Pop0 + 1);
		return;
	    }
	}

	assert(FALSE);		// unrecognized JAP type => corruption.

} // JudyCheckPop()


// ****************************************************************************
// J U D Y   C H E C K   P O P   S M
//
// Recursive state machine (subroutine) for JudyCheckPop():  Given a Pjp (other
// than JPNULL*; caller should shortcut) and the root population for top-level
// branches, check the subtree's actual pop1 against its nominal value, and
// return the total pop1 for the subtree.
//
// Note:  Expect RootPop1 to be ignored at lower levels, so pass down 0, which
// should pop an assertion if this expectation is violated.

FUNCTION static Word_t JudyCheckPopSM(
	Pjp_t  Pjp,		// top of subtree.
	Word_t RootPop1)	// whole array, for top-level branches only.
{
	Word_t pop1_jp;		// nominal population from the JP.
	Word_t pop1 = 0;	// actual population at this level.
	Word_t offset;		// in a branch.

#define	PREPBRANCH(cPopBytes,Next) \
	pop1_jp = JU_JPBRANCH_POP0(Pjp->jp_DcdPop0, cPopBytes) + 1; goto Next

assert((((Word_t) (Pjp->jp_Addr)) & 7) == 3);
	switch (Pjp->jp_Type)
	{

	case cJU_JPBRANCH_L2: PREPBRANCH(2, BranchL);
	case cJU_JPBRANCH_L3: PREPBRANCH(3, BranchL);
#ifdef JU_64BIT
	case cJU_JPBRANCH_L4: PREPBRANCH(4, BranchL);
	case cJU_JPBRANCH_L5: PREPBRANCH(5, BranchL);
	case cJU_JPBRANCH_L6: PREPBRANCH(6, BranchL);
	case cJU_JPBRANCH_L7: PREPBRANCH(7, BranchL);
#endif
	case cJU_JPBRANCH_L:  pop1_jp = RootPop1;
	{
	    Pjbl_t Pjbl;
BranchL:
	    Pjbl = P_JBL(Pjp->jp_Addr);

	    for (offset = 0; offset < (Pjbl->jbl_NumJPs); ++offset)
	        pop1 += JudyCheckPopSM((Pjbl->jbl_jp) + offset, 0);

	    assert(pop1_jp == pop1);
	    return(pop1);
	}

	case cJU_JPBRANCH_B2: PREPBRANCH(2, BranchB);
	case cJU_JPBRANCH_B3: PREPBRANCH(3, BranchB);
#ifdef JU_64BIT
	case cJU_JPBRANCH_B4: PREPBRANCH(4, BranchB);
	case cJU_JPBRANCH_B5: PREPBRANCH(5, BranchB);
	case cJU_JPBRANCH_B6: PREPBRANCH(6, BranchB);
	case cJU_JPBRANCH_B7: PREPBRANCH(7, BranchB);
#endif
	case cJU_JPBRANCH_B:  pop1_jp = RootPop1;
	{
	    Word_t subexp;
	    Word_t jpcount;
	    Pjbb_t Pjbb;
BranchB:
	    Pjbb = P_JBB(Pjp->jp_Addr);

	    for (subexp = 0; subexp < cJU_NUMSUBEXPB; ++subexp)
	    {
	        jpcount = __JudyCountBitsB(JU_JBB_BITMAP(Pjbb, subexp));

		for (offset = 0; offset < jpcount; ++offset)
		{
		    pop1 += JudyCheckPopSM(P_JP(JU_JBB_PJP(Pjbb, subexp))
					 + offset, 0);
		}
	    }

	    assert(pop1_jp == pop1);
	    return(pop1);
	}

	case cJU_JPBRANCH_U2: PREPBRANCH(2, BranchU);
	case cJU_JPBRANCH_U3: PREPBRANCH(3, BranchU);
#ifdef JU_64BIT
	case cJU_JPBRANCH_U4: PREPBRANCH(4, BranchU);
	case cJU_JPBRANCH_U5: PREPBRANCH(5, BranchU);
	case cJU_JPBRANCH_U6: PREPBRANCH(6, BranchU);
	case cJU_JPBRANCH_U7: PREPBRANCH(7, BranchU);
#endif
	case cJU_JPBRANCH_U:  pop1_jp = RootPop1;
	{
	    Pjbu_t Pjbu;
BranchU:
	    Pjbu = P_JBU(Pjp->jp_Addr);

	    for (offset = 0; offset < cJU_BRANCHUNUMJPS; ++offset)
	    {
		if (((Pjbu->jbu_jp[offset].jp_Type) >= cJU_JPNULL1)
		 && ((Pjbu->jbu_jp[offset].jp_Type) <= cJU_JPNULLMAX))
		{
		    continue;		// skip null JP to save time.
		}

	        pop1 += JudyCheckPopSM((Pjbu->jbu_jp) + offset, 0);
	    }

	    assert(pop1_jp == pop1);
	    return(pop1);
	}


// -- Cases below here terminate and do not recurse. --
//
// For all of these cases except JPLEAF_B1, there is no way to check the JP's
// pop1 against the object itself; just return the pop1; but for linear leaves,
// a bounds check is possible.

#define	CHECKLEAF(MaxPop1)				\
	pop1 = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;	\
	assert(pop1 >= 1);				\
	assert(pop1 <= (MaxPop1));			\
	return(pop1)

#if (JUDYL || (! JU_64BIT ))
	case cJU_JPLEAF1:  CHECKLEAF(cJU_LEAF1_MAXPOP1);
#endif
	case cJU_JPLEAF2:  CHECKLEAF(cJU_LEAF2_MAXPOP1);
	case cJU_JPLEAF3:  CHECKLEAF(cJU_LEAF3_MAXPOP1);
#ifdef JU_64BIT
	case cJU_JPLEAF4:  CHECKLEAF(cJU_LEAF4_MAXPOP1);
	case cJU_JPLEAF5:  CHECKLEAF(cJU_LEAF5_MAXPOP1);
	case cJU_JPLEAF6:  CHECKLEAF(cJU_LEAF6_MAXPOP1);
	case cJU_JPLEAF7:  CHECKLEAF(cJU_LEAF7_MAXPOP1);
#endif

	case cJU_JPLEAF_B1:
	{
	    Word_t subexp;
	    Pjlb_t Pjlb;

	    pop1_jp = JU_JPLEAF_POP0(Pjp->jp_DcdPop0) + 1;

	    Pjlb = P_JLB(Pjp->jp_Addr);

	    for (subexp = 0; subexp < cJU_NUMSUBEXPL; ++subexp)
	        pop1 += __JudyCountBitsL(JU_JLB_BITMAP(Pjlb, subexp));

	    assert(pop1_jp == pop1);
	    return(pop1);
	}

	JUDY1CODE(case cJ1_JPFULLPOPU1: return(cJU_JPFULLPOPU1_POP0);)

	case cJU_JPIMMED_1_01:	return(1);
	case cJU_JPIMMED_2_01:	return(1);
	case cJU_JPIMMED_3_01:	return(1);
#ifdef JU_64BIT
	case cJU_JPIMMED_4_01:	return(1);
	case cJU_JPIMMED_5_01:	return(1);
	case cJU_JPIMMED_6_01:	return(1);
	case cJU_JPIMMED_7_01:	return(1);
#endif

	case cJU_JPIMMED_1_02:	return(2);
	case cJU_JPIMMED_1_03:	return(3);
#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_1_04:	return(4);
	case cJU_JPIMMED_1_05:	return(5);
	case cJU_JPIMMED_1_06:	return(6);
	case cJU_JPIMMED_1_07:	return(7);
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_1_08:	return(8);
	case cJ1_JPIMMED_1_09:	return(9);
	case cJ1_JPIMMED_1_10:	return(10);
	case cJ1_JPIMMED_1_11:	return(11);
	case cJ1_JPIMMED_1_12:	return(12);
	case cJ1_JPIMMED_1_13:	return(13);
	case cJ1_JPIMMED_1_14:	return(14);
	case cJ1_JPIMMED_1_15:	return(15);
#endif

#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_2_02:	return(2);
	case cJU_JPIMMED_2_03:	return(3);
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_2_04:	return(4);
	case cJ1_JPIMMED_2_05:	return(5);
	case cJ1_JPIMMED_2_06:	return(6);
	case cJ1_JPIMMED_2_07:	return(7);
#endif

#if (JUDY1 || JU_64BIT)
	case cJU_JPIMMED_3_02:	return(2);
#endif
#if (JUDY1 && JU_64BIT)
	case cJ1_JPIMMED_3_03:	return(3);
	case cJ1_JPIMMED_3_04:	return(4);
	case cJ1_JPIMMED_3_05:	return(5);

	case cJ1_JPIMMED_4_02:	return(2);
	case cJ1_JPIMMED_4_03:	return(3);
	case cJ1_JPIMMED_5_02:	return(2);
	case cJ1_JPIMMED_5_03:	return(3);
	case cJ1_JPIMMED_6_02:	return(2);
	case cJ1_JPIMMED_7_02:	return(2);
#endif

	} // switch (Pjp->jp_Type)

	assert(FALSE);		// unrecognized JP type => corruption.
	return(0);		// to make some compilers happy.

} // JudyCheckPopSM()

#endif // DEBUG
#endif // ! JUDYGETINLINE
