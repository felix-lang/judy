#ifndef _JUDYPRIVATE1L_INCLUDED
#define	_JUDYPRIVATE1L_INCLUDED
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

// @(#) $Revision: 4.31 $ $Source: /judy/src/JudyCommon/JudyPrivate1L.h $

// ****************************************************************************
// Declare common cJU_* names for JP Types that occur in both Judy1 and JudyL,
// for use by code that ifdef's JUDY1 and JUDYL.  Only JP Types common to both
// Judy1 and JudyL are #define'd here with equivalent cJU_* names.  JP Types
// unique to only Judy1 or JudyL are listed in comments, so the type lists
// match the Judy1.h and JudyL.h files.
//
// This file also defines cJU_* for other JP-related constants and functions
// that some shared JUDY1/JUDYL code finds handy.
//
// At least in principle this file should be included AFTER Judy1.h or JudyL.h.
//
// WARNING:  This file must be kept consistent with the enum's in Judy1.h and
// JudyL.h.
//
// TBD:  You might think, why not define common cJU_* enums in, say,
// JudyPrivate.h, and then inherit them into superset enums in Judy1.h and
// JudyL.h?  The problem is that the enum lists for each class (cJ1_* and
// cJL_*) must be numerically "packed" into the correct order, for two reasons:
// (1) allow the compiler to generate "tight" switch statements with no wasted
// slots (although this is not very big), and (2) allow calculations using the
// enum values, although this is also not an issue if the calculations are only
// within each cJ*_JPIMMED_*_* class and the members are packed within the
// class.

#ifdef JUDY1

#define	cJU_JAPNULL		cJ1_JAPNULL
#define	cJU_JAPLEAF		cJ1_JAPLEAF
#define	cJU_JAPBRANCH		cJ1_JAPBRANCH
#define	cJU_JPNULL1		cJ1_JPNULL1
#define	cJU_JPNULL2		cJ1_JPNULL2
#define	cJU_JPNULL3		cJ1_JPNULL3
#ifdef JU_64BIT
#define	cJU_JPNULL4		cJ1_JPNULL4
#define	cJU_JPNULL5		cJ1_JPNULL5
#define	cJU_JPNULL6		cJ1_JPNULL6
#define	cJU_JPNULL7		cJ1_JPNULL7
#endif
#define	cJU_JPNULLMAX		cJ1_JPNULLMAX
#define	cJU_JPBRANCH_L2		cJ1_JPBRANCH_L2
#define	cJU_JPBRANCH_L3		cJ1_JPBRANCH_L3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_L4		cJ1_JPBRANCH_L4
#define	cJU_JPBRANCH_L5		cJ1_JPBRANCH_L5
#define	cJU_JPBRANCH_L6		cJ1_JPBRANCH_L6
#define	cJU_JPBRANCH_L7		cJ1_JPBRANCH_L7
#endif
#define	cJU_JPBRANCH_L		cJ1_JPBRANCH_L
#define	__jU_BranchBJPPopToWords __j1_BranchBJPPopToWords
#define	cJU_JPBRANCH_B2		cJ1_JPBRANCH_B2
#define	cJU_JPBRANCH_B3		cJ1_JPBRANCH_B3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_B4		cJ1_JPBRANCH_B4
#define	cJU_JPBRANCH_B5		cJ1_JPBRANCH_B5
#define	cJU_JPBRANCH_B6		cJ1_JPBRANCH_B6
#define	cJU_JPBRANCH_B7		cJ1_JPBRANCH_B7
#endif
#define	cJU_JPBRANCH_B		cJ1_JPBRANCH_B
#define	cJU_JPBRANCH_U2		cJ1_JPBRANCH_U2
#define	cJU_JPBRANCH_U3		cJ1_JPBRANCH_U3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_U4		cJ1_JPBRANCH_U4
#define	cJU_JPBRANCH_U5		cJ1_JPBRANCH_U5
#define	cJU_JPBRANCH_U6		cJ1_JPBRANCH_U6
#define	cJU_JPBRANCH_U7		cJ1_JPBRANCH_U7
#endif
#define	cJU_JPBRANCH_U		cJ1_JPBRANCH_U
#ifndef JU_64BIT
#define	cJU_JPLEAF1		cJ1_JPLEAF1
#endif
#define	cJU_JPLEAF2		cJ1_JPLEAF2
#define	cJU_JPLEAF3		cJ1_JPLEAF3
#ifdef JU_64BIT
#define	cJU_JPLEAF4		cJ1_JPLEAF4
#define	cJU_JPLEAF5		cJ1_JPLEAF5
#define	cJU_JPLEAF6		cJ1_JPLEAF6
#define	cJU_JPLEAF7		cJ1_JPLEAF7
#endif
#define	cJU_JPLEAF_B1		cJ1_JPLEAF_B1
//				cJ1_JPFULLPOPU1
#define	cJU_JPIMMED_1_01	cJ1_JPIMMED_1_01
#define	cJU_JPIMMED_2_01	cJ1_JPIMMED_2_01
#define	cJU_JPIMMED_3_01	cJ1_JPIMMED_3_01
#ifdef JU_64BIT
#define	cJU_JPIMMED_4_01	cJ1_JPIMMED_4_01
#define	cJU_JPIMMED_5_01	cJ1_JPIMMED_5_01
#define	cJU_JPIMMED_6_01	cJ1_JPIMMED_6_01
#define	cJU_JPIMMED_7_01	cJ1_JPIMMED_7_01
#endif
#define	cJU_JPIMMED_1_02	cJ1_JPIMMED_1_02
#define	cJU_JPIMMED_1_03	cJ1_JPIMMED_1_03
#define	cJU_JPIMMED_1_04	cJ1_JPIMMED_1_04
#define	cJU_JPIMMED_1_05	cJ1_JPIMMED_1_05
#define	cJU_JPIMMED_1_06	cJ1_JPIMMED_1_06
#define	cJU_JPIMMED_1_07	cJ1_JPIMMED_1_07
#ifdef JU_64BIT
//				cJ1_JPIMMED_1_08
//				cJ1_JPIMMED_1_09
//				cJ1_JPIMMED_1_10
//				cJ1_JPIMMED_1_11
//				cJ1_JPIMMED_1_12
//				cJ1_JPIMMED_1_13
//				cJ1_JPIMMED_1_14
//				cJ1_JPIMMED_1_15
#endif
#define	cJU_JPIMMED_2_02	cJ1_JPIMMED_2_02
#define	cJU_JPIMMED_2_03	cJ1_JPIMMED_2_03
#ifdef JU_64BIT
//				cJ1_JPIMMED_2_04
//				cJ1_JPIMMED_2_05
//				cJ1_JPIMMED_2_06
//				cJ1_JPIMMED_2_07
#endif
#define	cJU_JPIMMED_3_02	cJ1_JPIMMED_3_02
#ifdef JU_64BIT
//				cJ1_JPIMMED_3_03
//				cJ1_JPIMMED_3_04
//				cJ1_JPIMMED_3_05
//				cJ1_JPIMMED_4_02
//				cJ1_JPIMMED_4_03
//				cJ1_JPIMMED_5_02
//				cJ1_JPIMMED_5_03
//				cJ1_JPIMMED_6_02
//				cJ1_JPIMMED_7_02
#endif
#define	cJU_JPIMMED_CAP		cJ1_JPIMMED_CAP

#else // JUDYL ****************************************************************

#define	cJU_JAPNULL		cJL_JAPNULL
#define	cJU_JAPLEAF		cJL_JAPLEAF
#define	cJU_JAPBRANCH		cJL_JAPBRANCH
//				cJL_JAPINVALID
#define	cJU_JPNULL1		cJL_JPNULL1
#define	cJU_JPNULL2		cJL_JPNULL2
#define	cJU_JPNULL3		cJL_JPNULL3
#ifdef JU_64BIT
#define	cJU_JPNULL4		cJL_JPNULL4
#define	cJU_JPNULL5		cJL_JPNULL5
#define	cJU_JPNULL6		cJL_JPNULL6
#define	cJU_JPNULL7		cJL_JPNULL7
#endif
#define	cJU_JPNULLMAX		cJL_JPNULLMAX
#define	cJU_JPBRANCH_L2		cJL_JPBRANCH_L2
#define	cJU_JPBRANCH_L3		cJL_JPBRANCH_L3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_L4		cJL_JPBRANCH_L4
#define	cJU_JPBRANCH_L5		cJL_JPBRANCH_L5
#define	cJU_JPBRANCH_L6		cJL_JPBRANCH_L6
#define	cJU_JPBRANCH_L7		cJL_JPBRANCH_L7
#endif
#define	cJU_JPBRANCH_L		cJL_JPBRANCH_L
#define	__jU_BranchBJPPopToWords __jL_BranchBJPPopToWords
#define	cJU_JPBRANCH_B2		cJL_JPBRANCH_B2
#define	cJU_JPBRANCH_B3		cJL_JPBRANCH_B3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_B4		cJL_JPBRANCH_B4
#define	cJU_JPBRANCH_B5		cJL_JPBRANCH_B5
#define	cJU_JPBRANCH_B6		cJL_JPBRANCH_B6
#define	cJU_JPBRANCH_B7		cJL_JPBRANCH_B7
#endif
#define	cJU_JPBRANCH_B		cJL_JPBRANCH_B
#define	cJU_JPBRANCH_U2		cJL_JPBRANCH_U2
#define	cJU_JPBRANCH_U3		cJL_JPBRANCH_U3
#ifdef JU_64BIT
#define	cJU_JPBRANCH_U4		cJL_JPBRANCH_U4
#define	cJU_JPBRANCH_U5		cJL_JPBRANCH_U5
#define	cJU_JPBRANCH_U6		cJL_JPBRANCH_U6
#define	cJU_JPBRANCH_U7		cJL_JPBRANCH_U7
#endif
#define	cJU_JPBRANCH_U		cJL_JPBRANCH_U
#define	cJU_JPLEAF1		cJL_JPLEAF1
#define	cJU_JPLEAF2		cJL_JPLEAF2
#define	cJU_JPLEAF3		cJL_JPLEAF3
#ifdef JU_64BIT
#define	cJU_JPLEAF4		cJL_JPLEAF4
#define	cJU_JPLEAF5		cJL_JPLEAF5
#define	cJU_JPLEAF6		cJL_JPLEAF6
#define	cJU_JPLEAF7		cJL_JPLEAF7
#endif
#define	cJU_JPLEAF_B1		cJL_JPLEAF_B1
#define	cJU_JPIMMED_1_01	cJL_JPIMMED_1_01
#define	cJU_JPIMMED_2_01	cJL_JPIMMED_2_01
#define	cJU_JPIMMED_3_01	cJL_JPIMMED_3_01
#ifdef JU_64BIT
#define	cJU_JPIMMED_4_01	cJL_JPIMMED_4_01
#define	cJU_JPIMMED_5_01	cJL_JPIMMED_5_01
#define	cJU_JPIMMED_6_01	cJL_JPIMMED_6_01
#define	cJU_JPIMMED_7_01	cJL_JPIMMED_7_01
#endif
#define	cJU_JPIMMED_1_02	cJL_JPIMMED_1_02
#define	cJU_JPIMMED_1_03	cJL_JPIMMED_1_03
#ifdef JU_64BIT
#define	cJU_JPIMMED_1_04	cJL_JPIMMED_1_04
#define	cJU_JPIMMED_1_05	cJL_JPIMMED_1_05
#define	cJU_JPIMMED_1_06	cJL_JPIMMED_1_06
#define	cJU_JPIMMED_1_07	cJL_JPIMMED_1_07
#define	cJU_JPIMMED_2_02	cJL_JPIMMED_2_02
#define	cJU_JPIMMED_2_03	cJL_JPIMMED_2_03
#define	cJU_JPIMMED_3_02	cJL_JPIMMED_3_02
#endif
#define	cJU_JPIMMED_CAP		cJL_JPIMMED_CAP

#endif // JUDYL


// ****************************************************************************
// cJU*_ other than JP types:

#ifdef JUDY1

#define	cJU_JAPLEAF_MAXPOP1	cJ1_JAPLEAF_MAXPOP1
#ifndef JU_64BIT
#define	cJU_LEAF1_MAXPOP1	cJ1_LEAF1_MAXPOP1
#endif
#define	cJU_LEAF2_MAXPOP1	cJ1_LEAF2_MAXPOP1
#define	cJU_LEAF3_MAXPOP1	cJ1_LEAF3_MAXPOP1
#ifdef JU_64BIT
#define	cJU_LEAF4_MAXPOP1	cJ1_LEAF4_MAXPOP1
#define	cJU_LEAF5_MAXPOP1	cJ1_LEAF5_MAXPOP1
#define	cJU_LEAF6_MAXPOP1	cJ1_LEAF6_MAXPOP1
#define	cJU_LEAF7_MAXPOP1	cJ1_LEAF7_MAXPOP1
#endif
#define	cJU_IMMED1_MAXPOP1	cJ1_IMMED1_MAXPOP1
#define	cJU_IMMED2_MAXPOP1	cJ1_IMMED2_MAXPOP1
#define	cJU_IMMED3_MAXPOP1	cJ1_IMMED3_MAXPOP1
#ifdef JU_64BIT
#define	cJU_IMMED4_MAXPOP1	cJ1_IMMED4_MAXPOP1
#define	cJU_IMMED5_MAXPOP1	cJ1_IMMED5_MAXPOP1
#define	cJU_IMMED6_MAXPOP1	cJ1_IMMED6_MAXPOP1
#define	cJU_IMMED7_MAXPOP1	cJ1_IMMED7_MAXPOP1
#endif

#define	JU_LEAF1POPTOWORDS(Pop1)	J1_LEAF1POPTOWORDS(Pop1)
#define	JU_LEAF2POPTOWORDS(Pop1)	J1_LEAF2POPTOWORDS(Pop1)
#define	JU_LEAF3POPTOWORDS(Pop1)	J1_LEAF3POPTOWORDS(Pop1)
#ifdef JU_64BIT
#define	JU_LEAF4POPTOWORDS(Pop1)	J1_LEAF4POPTOWORDS(Pop1)
#define	JU_LEAF5POPTOWORDS(Pop1)	J1_LEAF5POPTOWORDS(Pop1)
#define	JU_LEAF6POPTOWORDS(Pop1)	J1_LEAF6POPTOWORDS(Pop1)
#define	JU_LEAF7POPTOWORDS(Pop1)	J1_LEAF7POPTOWORDS(Pop1)
#endif
#define	JU_LEAFWPOPTOWORDS(Pop1)	J1_LEAFWPOPTOWORDS(Pop1)

#ifndef JU_64BIT
#define	JU_LEAF1GROWINPLACE(Pop1)	J1_LEAF1GROWINPLACE(Pop1)
#endif
#define	JU_LEAF2GROWINPLACE(Pop1)	J1_LEAF2GROWINPLACE(Pop1)
#define	JU_LEAF3GROWINPLACE(Pop1)	J1_LEAF3GROWINPLACE(Pop1)
#ifdef JU_64BIT
#define	JU_LEAF4GROWINPLACE(Pop1)	J1_LEAF4GROWINPLACE(Pop1)
#define	JU_LEAF5GROWINPLACE(Pop1)	J1_LEAF5GROWINPLACE(Pop1)
#define	JU_LEAF6GROWINPLACE(Pop1)	J1_LEAF6GROWINPLACE(Pop1)
#define	JU_LEAF7GROWINPLACE(Pop1)	J1_LEAF7GROWINPLACE(Pop1)
#endif
#define	JU_LEAFWGROWINPLACE(Pop1)	J1_LEAFWGROWINPLACE(Pop1)

#define	__JudyCreateBranchL	__Judy1CreateBranchL
#define	__JudyCreateBranchB	__Judy1CreateBranchB
#define	__JudyCreateBranchU	__Judy1CreateBranchU
#define	__JudyCascade1		__Judy1Cascade1
#define	__JudyCascade2		__Judy1Cascade2
#define	__JudyCascade3		__Judy1Cascade3
#ifdef JU_64BIT
#define	__JudyCascade4		__Judy1Cascade4
#define	__JudyCascade5		__Judy1Cascade5
#define	__JudyCascade6		__Judy1Cascade6
#define	__JudyCascade7		__Judy1Cascade7
#endif
#define	__JudyCascadeL		__Judy1CascadeL
#define	__JudyInsertBranch	__Judy1InsertBranch

#define	__JudyBranchBToBranchL	__Judy1BranchBToBranchL
#ifndef JU_64BIT
#define	__JudyLeafB1ToLeaf1	__Judy1LeafB1ToLeaf1
#endif
#define	__JudyLeaf1ToLeaf2	__Judy1Leaf1ToLeaf2
#define	__JudyLeaf2ToLeaf3	__Judy1Leaf2ToLeaf3
#ifndef JU_64BIT
#define	__JudyLeaf3ToLeafW	__Judy1Leaf3ToLeafW
#else
#define	__JudyLeaf3ToLeaf4	__Judy1Leaf3ToLeaf4
#define	__JudyLeaf4ToLeaf5	__Judy1Leaf4ToLeaf5
#define	__JudyLeaf5ToLeaf6	__Judy1Leaf5ToLeaf6
#define	__JudyLeaf6ToLeaf7	__Judy1Leaf6ToLeaf7
#define	__JudyLeaf7ToLeafW	__Judy1Leaf7ToLeafW
#endif

#define	jpm_t			j1pm_t
#define	Pjpm_t			Pj1pm_t

#define	jlb_t			j1lb_t
#define	Pjlb_t			Pj1lb_t

#define	JU_JLB_BITMAP		J1_JLB_BITMAP

#define	__JudyAllocJPM		__Judy1AllocJ1PM
#define	__JudyAllocJBL		__Judy1AllocJBL
#define	__JudyAllocJBB		__Judy1AllocJBB
#define	__JudyAllocJBBJP	__Judy1AllocJBBJP
#define	__JudyAllocJBU		__Judy1AllocJBU
#ifndef JU_64BIT
#define	__JudyAllocJLL1		__Judy1AllocJLL1
#endif
#define	__JudyAllocJLL2		__Judy1AllocJLL2
#define	__JudyAllocJLL3		__Judy1AllocJLL3
#ifdef JU_64BIT
#define	__JudyAllocJLL4		__Judy1AllocJLL4
#define	__JudyAllocJLL5		__Judy1AllocJLL5
#define	__JudyAllocJLL6		__Judy1AllocJLL6
#define	__JudyAllocJLL7		__Judy1AllocJLL7
#endif
#define	__JudyAllocJLW		__Judy1AllocJLW
#define	__JudyAllocJLB1		__Judy1AllocJLB1
#define	__JudyFreeJPM		__Judy1FreeJ1PM
#define	__JudyFreeJBL		__Judy1FreeJBL
#define	__JudyFreeJBB		__Judy1FreeJBB
#define	__JudyFreeJBBJP		__Judy1FreeJBBJP
#define	__JudyFreeJBU		__Judy1FreeJBU
#ifndef JU_64BIT
#define	__JudyFreeJLL1		__Judy1FreeJLL1
#endif
#define	__JudyFreeJLL2		__Judy1FreeJLL2
#define	__JudyFreeJLL3		__Judy1FreeJLL3
#ifdef JU_64BIT
#define	__JudyFreeJLL4		__Judy1FreeJLL4
#define	__JudyFreeJLL5		__Judy1FreeJLL5
#define	__JudyFreeJLL6		__Judy1FreeJLL6
#define	__JudyFreeJLL7		__Judy1FreeJLL7
#endif
#define	__JudyFreeJLW		__Judy1FreeJLW
#define	__JudyFreeJLB1		__Judy1FreeJLB1
#define	__JudyFreeSM		__Judy1FreeSM

#define	__juMaxWords		__ju1MaxWords

#ifdef DEBUG
#define	JudyCheckPop		Judy1CheckPop
#endif

#else // JUDYL ****************************************************************

#define	cJU_JAPLEAF_MAXPOP1	cJL_JAPLEAF_MAXPOP1
#define	cJU_LEAF1_MAXPOP1	cJL_LEAF1_MAXPOP1
#define	cJU_LEAF2_MAXPOP1	cJL_LEAF2_MAXPOP1
#define	cJU_LEAF3_MAXPOP1	cJL_LEAF3_MAXPOP1
#ifdef JU_64BIT
#define	cJU_LEAF4_MAXPOP1	cJL_LEAF4_MAXPOP1
#define	cJU_LEAF5_MAXPOP1	cJL_LEAF5_MAXPOP1
#define	cJU_LEAF6_MAXPOP1	cJL_LEAF6_MAXPOP1
#define	cJU_LEAF7_MAXPOP1	cJL_LEAF7_MAXPOP1
#endif
#define	cJU_IMMED1_MAXPOP1	cJL_IMMED1_MAXPOP1
#define	cJU_IMMED2_MAXPOP1	cJL_IMMED2_MAXPOP1
#define	cJU_IMMED3_MAXPOP1	cJL_IMMED3_MAXPOP1
#ifdef JU_64BIT
#define	cJU_IMMED4_MAXPOP1	cJL_IMMED4_MAXPOP1
#define	cJU_IMMED5_MAXPOP1	cJL_IMMED5_MAXPOP1
#define	cJU_IMMED6_MAXPOP1	cJL_IMMED6_MAXPOP1
#define	cJU_IMMED7_MAXPOP1	cJL_IMMED7_MAXPOP1
#endif

#define	JU_LEAF1POPTOWORDS(Pop1)	JL_LEAF1POPTOWORDS(Pop1)
#define	JU_LEAF2POPTOWORDS(Pop1)	JL_LEAF2POPTOWORDS(Pop1)
#define	JU_LEAF3POPTOWORDS(Pop1)	JL_LEAF3POPTOWORDS(Pop1)
#ifdef JU_64BIT
#define	JU_LEAF4POPTOWORDS(Pop1)	JL_LEAF4POPTOWORDS(Pop1)
#define	JU_LEAF5POPTOWORDS(Pop1)	JL_LEAF5POPTOWORDS(Pop1)
#define	JU_LEAF6POPTOWORDS(Pop1)	JL_LEAF6POPTOWORDS(Pop1)
#define	JU_LEAF7POPTOWORDS(Pop1)	JL_LEAF7POPTOWORDS(Pop1)
#endif
#define	JU_LEAFWPOPTOWORDS(Pop1)	JL_LEAFWPOPTOWORDS(Pop1)

#define	JU_LEAF1GROWINPLACE(Pop1)	JL_LEAF1GROWINPLACE(Pop1)
#define	JU_LEAF2GROWINPLACE(Pop1)	JL_LEAF2GROWINPLACE(Pop1)
#define	JU_LEAF3GROWINPLACE(Pop1)	JL_LEAF3GROWINPLACE(Pop1)
#ifdef JU_64BIT
#define	JU_LEAF4GROWINPLACE(Pop1)	JL_LEAF4GROWINPLACE(Pop1)
#define	JU_LEAF5GROWINPLACE(Pop1)	JL_LEAF5GROWINPLACE(Pop1)
#define	JU_LEAF6GROWINPLACE(Pop1)	JL_LEAF6GROWINPLACE(Pop1)
#define	JU_LEAF7GROWINPLACE(Pop1)	JL_LEAF7GROWINPLACE(Pop1)
#endif
#define	JU_LEAFWGROWINPLACE(Pop1)	JL_LEAFWGROWINPLACE(Pop1)

#define	__JudyCreateBranchL	__JudyLCreateBranchL
#define	__JudyCreateBranchB	__JudyLCreateBranchB
#define	__JudyCreateBranchU	__JudyLCreateBranchU
#define	__JudyCascade1		__JudyLCascade1
#define	__JudyCascade2		__JudyLCascade2
#define	__JudyCascade3		__JudyLCascade3
#ifdef JU_64BIT
#define	__JudyCascade4		__JudyLCascade4
#define	__JudyCascade5		__JudyLCascade5
#define	__JudyCascade6		__JudyLCascade6
#define	__JudyCascade7		__JudyLCascade7
#endif
#define	__JudyCascadeL		__JudyLCascadeL
#define	__JudyInsertBranch	__JudyLInsertBranch

#define	__JudyBranchBToBranchL	__JudyLBranchBToBranchL
#define	__JudyLeafB1ToLeaf1	__JudyLLeafB1ToLeaf1
#define	__JudyLeaf1ToLeaf2	__JudyLLeaf1ToLeaf2
#define	__JudyLeaf2ToLeaf3	__JudyLLeaf2ToLeaf3
#ifndef JU_64BIT
#define	__JudyLeaf3ToLeafW	__JudyLLeaf3ToLeafW
#else
#define	__JudyLeaf3ToLeaf4	__JudyLLeaf3ToLeaf4
#define	__JudyLeaf4ToLeaf5	__JudyLLeaf4ToLeaf5
#define	__JudyLeaf5ToLeaf6	__JudyLLeaf5ToLeaf6
#define	__JudyLeaf6ToLeaf7	__JudyLLeaf6ToLeaf7
#define	__JudyLeaf7ToLeafW	__JudyLLeaf7ToLeafW
#endif

#define	jpm_t			jLpm_t
#define	Pjpm_t			PjLpm_t

#define	jlb_t			jLlb_t
#define	Pjlb_t			PjLlb_t

#define	JU_JLB_BITMAP		JL_JLB_BITMAP

#define	__JudyAllocJPM		__JudyLAllocJLPM
#define	__JudyAllocJBL		__JudyLAllocJBL
#define	__JudyAllocJBB		__JudyLAllocJBB
#define	__JudyAllocJBBJP	__JudyLAllocJBBJP
#define	__JudyAllocJBU		__JudyLAllocJBU
#define	__JudyAllocJLL1		__JudyLAllocJLL1
#define	__JudyAllocJLL2		__JudyLAllocJLL2
#define	__JudyAllocJLL3		__JudyLAllocJLL3
#ifdef JU_64BIT
#define	__JudyAllocJLL4		__JudyLAllocJLL4
#define	__JudyAllocJLL5		__JudyLAllocJLL5
#define	__JudyAllocJLL6		__JudyLAllocJLL6
#define	__JudyAllocJLL7		__JudyLAllocJLL7
#endif
#define	__JudyAllocJLW		__JudyLAllocJLW
#define	__JudyAllocJLB1		__JudyLAllocJLB1
//				__JudyLAllocJV
#define	__JudyFreeJPM		__JudyLFreeJLPM
#define	__JudyFreeJBL		__JudyLFreeJBL
#define	__JudyFreeJBB		__JudyLFreeJBB
#define	__JudyFreeJBBJP		__JudyLFreeJBBJP
#define	__JudyFreeJBU		__JudyLFreeJBU
#define	__JudyFreeJLL1		__JudyLFreeJLL1
#define	__JudyFreeJLL2		__JudyLFreeJLL2
#define	__JudyFreeJLL3		__JudyLFreeJLL3
#ifdef JU_64BIT
#define	__JudyFreeJLL4		__JudyLFreeJLL4
#define	__JudyFreeJLL5		__JudyLFreeJLL5
#define	__JudyFreeJLL6		__JudyLFreeJLL6
#define	__JudyFreeJLL7		__JudyLFreeJLL7
#endif
#define	__JudyFreeJLW		__JudyLFreeJLW
#define	__JudyFreeJLB1		__JudyLFreeJLB1
#define	__JudyFreeSM		__JudyLFreeSM
//				__JudyLFreeJV

#define	__juMaxWords		__juLMaxWords

#ifdef DEBUG
#define	JudyCheckPop		JudyLCheckPop
#endif

#endif // JUDYL

#endif // _JUDYPRIVATE1L_INCLUDED
