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

// @(#) $Revision: 4.45 $ $Source: /judy/src/JudyCommon/JudyMallocIF.c $
//
// Judy malloc/free interface functions for Judy1 and JudyL.
//
// Compile with one of -DJUDY1 or -DJUDYL.
//
// Compile with -DTRACEMI (Malloc Interface) to turn on tracing of malloc/free
// calls at the interface level.  (See also TRACEMF in lower-level code.)
// Use -DTRACEMI2 for a terser format suitable for trace analysis.
//
// There can be malloc namespace bits in the LSBs of "raw" addresses from most,
// but not all, of the __Judy*Alloc*() functions; see also JudyPrivate.h.  To
// test the Judy code, compile this file with -DMALLOCBITS and use debug flavor
// only (for assertions).  This test ensures that (a) all callers properly mask
// the namespace bits out before dereferencing a pointer (or else a core dump
// occurs), and (b) all callers send "raw" (unmasked) addresses to
// __Judy*Free*() calls.
//
// Note:  Currently -DDEBUG turns on MALLOCBITS automatically.

#if (! (JUDY1 || JUDYL))
    Error:  One of -DJUDY1 or -DJUDYL must be specified.
#endif

#ifdef JUDY1
#include "Judy1.h"
#else
#include "JudyL.h"
#endif

#include "JudyPrivate1L.h"
#include "JudyMalloc.h"

// Set "hidden" global __juMaxWords to the maximum number of words to allocate
// to any one array (large enough to have a JPM, otherwise __juMaxWords is
// ignored), to trigger a fake malloc error when the number is exceeded.  Note,
// this code is always executed, not #ifdef'd, because it's virtually free.
//
// Note:  To keep the MALLOC macro faster and simpler, set __juMaxWords to
// MAXINT, not zero, by default.

Word_t __juMaxWords = ~0UL;

// This macro hides the faking of a malloc failure:
//
// Note:  To keep this fast, just compare WordsPrev to __juMaxWords without the
// complexity of first adding WordsNow, meaning the trigger point is not
// exactly where you might assume, but it shouldn't matter.

#define	MALLOC(MallocFunc,WordsPrev,WordsNow) \
	(((WordsPrev) > __juMaxWords) ?	0UL : MallocFunc(WordsNow))

// Clear words starting at address:
//
// Note:  Only use this for objects that care; in other cases, it doesn't
// matter if the object's memory is pre-zeroed.

#define	ZEROWORDS(Addr,Words)			\
	{					\
	    Word_t  __Words = (Words);		\
	    PWord_t __Addr  = (PWord_t) (Addr);	\
	    while (__Words--) *__Addr++ = 0UL;	\
	}

#ifdef TRACEMI

// TRACING SUPPORT:
//
// Note:  For TRACEMI, use a format for address printing compatible with other
// tracing facilities; in particular, %x not %lx, to truncate the "noisy" high
// part on 64-bit systems.
//
// TBD: The trace macros need fixing for alternate address types.
//
// Note:  TRACEMI2 supports trace analysis no matter the underlying malloc/free
// engine used.

#include <stdio.h>

static Word_t __JudyMemSequence = 0L;	// event sequence number.

#define TRACE_ALLOC5(a,b,c,d,e)   (void) printf(a, (b), c, d)
#define TRACE_FREE5( a,b,c,d,e)   (void) printf(a, (b), c, d)
#define TRACE_ALLOC6(a,b,c,d,e,f) (void) printf(a, (b), c, d, e)
#define TRACE_FREE6( a,b,c,d,e,f) (void) printf(a, (b), c, d, e)

#else

#ifdef TRACEMI2

#include <stdio.h>

#define	_bpw cJU_BYTESPERWORD

#define TRACE_ALLOC5(a,b,c,d,e)	  \
	    (void) printf("a %lx %lx %lx\n", (b), (d) * _bpw, e)
#define TRACE_FREE5( a,b,c,d,e)	  \
	    (void) printf("f %lx %lx %lx\n", (b), (d) * _bpw, e)
#define TRACE_ALLOC6(a,b,c,d,e,f)	  \
	    (void) printf("a %lx %lx %lx\n", (b), (e) * _bpw, f)
#define TRACE_FREE6( a,b,c,d,e,f)	  \
	    (void) printf("f %lx %lx %lx\n", (b), (e) * _bpw, f)

static Word_t __JudyMemSequence = 0L;	// event sequence number.

#else

#define TRACE_ALLOC5(a,b,c,d,e)   // null.
#define TRACE_FREE5( a,b,c,d,e)   // null.
#define TRACE_ALLOC6(a,b,c,d,e,f) // null.
#define TRACE_FREE6( a,b,c,d,e,f) // null.

#endif // ! TRACEMI2
#endif // ! TRACEMI


// MALLOC NAMESPACE SUPPORT:

#if (DEBUG && (! defined(MALLOCBITS)))	// at least now, DEBUG => MALLOCBITS:
#define MALLOCBITS 1
#endif

#ifdef MALLOCBITS
#define	MALLOCBITS_VALUE 0x3	// bit pattern to use.
#define	MALLOCBITS_MASK	 0x7	// note: matches __mask in JudyPrivate.h.

#define	MALLOCBITS_SET( Type,Addr) \
	((Addr) = (Type) ((Word_t) (Addr) |  MALLOCBITS_VALUE))
#define	MALLOCBITS_TEST(Type,Addr) \
	assert((((Word_t) (Addr)) & MALLOCBITS_MASK) == MALLOCBITS_VALUE); \
	((Addr) = (Type) ((Word_t) (Addr) & ~MALLOCBITS_VALUE))
#else
#define	MALLOCBITS_SET( Type,Addr)  // null.
#define	MALLOCBITS_TEST(Type,Addr)  // null.
#endif


// SAVE ERROR INFORMATION IN A Pjpm:
//
// "Small" (invalid) Addr values are used to distinguish overrun and no-mem
// errors.  (TBD, non-zero invalid values are no longer returned from
// lower-level functions, that is, JU_ERRNO_OVERRUN is no longer detected.)

#define __JUDYSETALLOCERROR(Addr)					\
	{								\
	    JU_ERRID(Pjpm) = __LINE__;					\
	    if ((Word_t) (Addr) > 0) JU_ERRNO(Pjpm) = JU_ERRNO_OVERRUN;	\
	    else		     JU_ERRNO(Pjpm) = JU_ERRNO_NOMEM;	\
	    return(0);							\
	}


// ****************************************************************************
// ALLOCATION FUNCTIONS:
//
// To help the compiler catch coding errors, each function returns a specific
// object type.
//
// Note:  Only __JudyAllocJPM() and __JudyAllocJLW() return multiple values <=
// sizeof(Word_t) to indicate the type of memory allocation failure.  Other
// allocation functions convert this failure to a JU_ERRNO.


// Note:  Unlike other __JudyAlloc*() functions, Pjpm's are returned non-raw,
// that is, without malloc namespace or root pointer type bits:

FUNCTION Pjpm_t __JudyAllocJPM(void)
{
	Word_t Words = sizeof(jpm_t) / cJU_BYTESPERWORD;
	Pjpm_t Pjpm  = (Pjpm_t) MALLOC(JudyMalloc, Words, Words);

	assert((Words * cJU_BYTESPERWORD) == sizeof(jpm_t));

	if ((Word_t) Pjpm > sizeof(Word_t))
	{
	    ZEROWORDS(Pjpm, Words);
	    Pjpm->jpm_TotalMemWords = Words;
	}

	TRACE_ALLOC5("0x%x %8lu = __JudyAllocJPM(), Words = %lu\n",
		     Pjpm, __JudyMemSequence++, Words, cJU_JAPLEAF_MAXPOP1 + 1);
	// MALLOCBITS_SET(Pjpm_t, Pjpm);  // see above.
	return(Pjpm);

} // __JudyAllocJPM()


FUNCTION Pjbl_t __JudyAllocJBL(Pjpm_t Pjpm)
{
	Word_t Words   = sizeof(jbl_t) / cJU_BYTESPERWORD;
	Pjbl_t PjblRaw = (Pjbl_t) MALLOC(JudyMallocVirtual,
					 Pjpm->jpm_TotalMemWords, Words);

	assert((Words * cJU_BYTESPERWORD) == sizeof(jbl_t));

	if ((Word_t) PjblRaw > sizeof(Word_t))
	{
	    ZEROWORDS(P_JBL(PjblRaw), Words);
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjblRaw); }

	TRACE_ALLOC5("0x%x %8lu = __JudyAllocJBL(), Words = %lu\n", PjblRaw,
		     __JudyMemSequence++, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjbl_t, PjblRaw);
	return(PjblRaw);

} // __JudyAllocJBL()


FUNCTION Pjbb_t __JudyAllocJBB(Pjpm_t Pjpm)
{
	Word_t Words   = sizeof(jbb_t) / cJU_BYTESPERWORD;
	Pjbb_t PjbbRaw = (Pjbb_t) MALLOC(JudyMallocVirtual,
					 Pjpm->jpm_TotalMemWords, Words);

	assert((Words * cJU_BYTESPERWORD) == sizeof(jbb_t));

	if ((Word_t) PjbbRaw > sizeof(Word_t))
	{
	    ZEROWORDS(P_JBB(PjbbRaw), Words);
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjbbRaw); }

	TRACE_ALLOC5("0x%x %8lu = __JudyAllocJBB(), Words = %lu\n", PjbbRaw,
		     __JudyMemSequence++, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjbb_t, PjbbRaw);
	return(PjbbRaw);

} // __JudyAllocJBB()


FUNCTION Pjp_t __JudyAllocJBBJP(Word_t NumJPs, Pjpm_t Pjpm)
{
	Word_t Words = JU_BRANCHJP_NUMJPSTOWORDS(NumJPs);
	Pjp_t  PjpRaw;

	PjpRaw = (Pjp_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjpRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjpRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJBBJP(%lu), Words = %lu\n", PjpRaw,
		     __JudyMemSequence++, NumJPs, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjp_t, PjpRaw);
	return(PjpRaw);

} // __JudyAllocJBBJP()


FUNCTION Pjbu_t __JudyAllocJBU(Pjpm_t Pjpm)
{
	Word_t Words   = sizeof(jbu_t) / cJU_BYTESPERWORD;
	Pjbu_t PjbuRaw = (Pjbu_t) MALLOC(JudyMallocVirtual,
					 Pjpm->jpm_TotalMemWords, Words);

	assert((Words * cJU_BYTESPERWORD) == sizeof(jbu_t));

	if ((Word_t) PjbuRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjbuRaw); }

	TRACE_ALLOC5("0x%x %8lu = __JudyAllocJBU(), Words = %lu\n", PjbuRaw,
		     __JudyMemSequence++, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjbu_t, PjbuRaw);
	return(PjbuRaw);

} // __JudyAllocJBU()


#if (JUDYL || (! JU_64BIT))

FUNCTION Pjll_t __JudyAllocJLL1(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF1POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL1(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL1()

#endif // (JUDYL || (! JU_64BIT))


FUNCTION Pjll_t __JudyAllocJLL2(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF2POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL2(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL2()


FUNCTION Pjll_t __JudyAllocJLL3(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF3POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL3(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL3()


#ifdef JU_64BIT

FUNCTION Pjll_t __JudyAllocJLL4(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF4POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL4(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL4()


FUNCTION Pjll_t __JudyAllocJLL5(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF5POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL5(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL5()


FUNCTION Pjll_t __JudyAllocJLL6(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF6POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL6(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL6()


FUNCTION Pjll_t __JudyAllocJLL7(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF7POPTOWORDS(Pop1);
	Pjll_t PjllRaw;

	PjllRaw = (Pjll_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjllRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjllRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLL7(%lu), Words = %lu\n", PjllRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjll_t, PjllRaw);
	return(PjllRaw);

} // __JudyAllocJLL7()

#endif // JU_64BIT


// Note:  Root-level leaf addresses are always whole words (Pjlw_t), and unlike
// other __JudyAlloc*() functions, they are returned non-raw, that is, without
// malloc namespace or root pointer type bits (the latter are added later by
// the caller):

FUNCTION Pjlw_t __JudyAllocJLW(Word_t Pop1)
{
	Word_t Words = JU_LEAFWPOPTOWORDS(Pop1);
	Pjlw_t Pjlw  = (Pjlw_t) MALLOC(JudyMalloc, Words, Words);

	if ((Word_t) Pjlw > sizeof(Word_t))

	TRACE_ALLOC6("0x%x %8lu = __JudyAllocJLW(%lu), Words = %lu\n", Pjlw,
		     __JudyMemSequence++, Pop1, Words, Pop1);
	// MALLOCBITS_SET(Pjlw_t, Pjlw);  // see above.
	return(Pjlw);

} // __JudyAllocJLW()


FUNCTION Pjlb_t __JudyAllocJLB1(Pjpm_t Pjpm)
{
	Word_t Words = sizeof(jlb_t) / cJU_BYTESPERWORD;
	Pjlb_t PjlbRaw;

	PjlbRaw = (Pjlb_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	assert((Words * cJU_BYTESPERWORD) == sizeof(jlb_t));

	if ((Word_t) PjlbRaw > sizeof(Word_t))
	{
	    ZEROWORDS(P_JLB(PjlbRaw), Words);
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjlbRaw); }

	TRACE_ALLOC5("0x%x %8lu = __JudyAllocJLB1(), Words = %lu\n", PjlbRaw,
		     __JudyMemSequence++, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjlb_t, PjlbRaw);
	return(PjlbRaw);

} // __JudyAllocJLB1()


#ifdef JUDYL

FUNCTION Pjv_t __JudyLAllocJV(Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JL_LEAFVPOPTOWORDS(Pop1);
	Pjv_t  PjvRaw;

	PjvRaw = (Pjv_t) MALLOC(JudyMalloc, Pjpm->jpm_TotalMemWords, Words);

	if ((Word_t) PjvRaw > sizeof(Word_t))
	{
	    Pjpm->jpm_TotalMemWords += Words;
	}
	else { __JUDYSETALLOCERROR(PjvRaw); }

	TRACE_ALLOC6("0x%x %8lu = __JudyLAllocJV(%lu), Words = %lu\n", PjvRaw,
		     __JudyMemSequence++, Pop1, Words, (Pjpm->jpm_Pop0) + 2);
	MALLOCBITS_SET(Pjv_t, PjvRaw);
	return(PjvRaw);

} // __JudyLAllocJV()

#endif // JUDYL


// ****************************************************************************
// FREE FUNCTIONS:
//
// To help the compiler catch coding errors, each function takes a specific
// object type to free.


// Note:  __JudyFreeJPM() receives a root pointer with NO root pointer type
// bits present, that is, they must be stripped by the caller using P_JPM():

FUNCTION void __JudyFreeJPM(Pjpm_t PjpmFree, Pjpm_t PjpmStats)
{
	Word_t Words = sizeof(jpm_t) / cJU_BYTESPERWORD;

	// MALLOCBITS_TEST(Pjpm_t, PjpmFree);   // see above.
	JudyFree((Pvoid_t) PjpmFree, Words);

	if (PjpmStats != (Pjpm_t) NULL) PjpmStats->jpm_TotalMemWords -= Words;

// Note:  Log PjpmFree->jpm_Pop0, similar to other __JudyFree*() functions, not
// an assumed value of cJU_JAPLEAF_MAXPOP1, for when the caller is
// Judy*FreeArray(), jpm_Pop0 is set to 0, and the population after the free
// really will be 0, not cJU_JAPLEAF_MAXPOP1.

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJPM(%lu), Words = %lu\n", PjpmFree,
		    __JudyMemSequence++, Words, Words, PjpmFree->jpm_Pop0);


} // __JudyFreeJPM()


FUNCTION void __JudyFreeJBL(Pjbl_t Pjbl, Pjpm_t Pjpm)
{
	Word_t Words = sizeof(jbl_t) / cJU_BYTESPERWORD;

	MALLOCBITS_TEST(Pjbl_t, Pjbl);
	JudyFreeVirtual((Pvoid_t) Pjbl, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE5("0x%x %8lu =  __JudyFreeJBL(), Words = %lu\n", Pjbl,
		    __JudyMemSequence++, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJBL()


FUNCTION void __JudyFreeJBB(Pjbb_t Pjbb, Pjpm_t Pjpm)
{
	Word_t Words = sizeof(jbb_t) / cJU_BYTESPERWORD;

	MALLOCBITS_TEST(Pjbb_t, Pjbb);
	JudyFreeVirtual((Pvoid_t) Pjbb, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE5("0x%x %8lu =  __JudyFreeJBB(), Words = %lu\n", Pjbb,
		    __JudyMemSequence++, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJBB()


FUNCTION void __JudyFreeJBBJP(Pjp_t Pjp, Word_t NumJPs, Pjpm_t Pjpm)
{
	Word_t Words = JU_BRANCHJP_NUMJPSTOWORDS(NumJPs);

	MALLOCBITS_TEST(Pjp_t, Pjp);
	JudyFree((Pvoid_t) Pjp, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJBBJP(%lu), Words = %lu\n", Pjp,
		    __JudyMemSequence++, NumJPs, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJBBJP()


FUNCTION void __JudyFreeJBU(Pjbu_t Pjbu, Pjpm_t Pjpm)
{
	Word_t Words = sizeof(jbu_t) / cJU_BYTESPERWORD;

	MALLOCBITS_TEST(Pjbu_t, Pjbu);
	JudyFreeVirtual((Pvoid_t) Pjbu, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE5("0x%x %8lu =  __JudyFreeJBU(), Words = %lu\n", Pjbu,
		    __JudyMemSequence++, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJBU()


#if (JUDYL || (! JU_64BIT))

FUNCTION void __JudyFreeJLL1(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF1POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL1(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL1()

#endif // (JUDYL || (! JU_64BIT))


FUNCTION void __JudyFreeJLL2(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF2POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL2(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL2()


FUNCTION void __JudyFreeJLL3(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF3POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL3(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL3()


#ifdef JU_64BIT

FUNCTION void __JudyFreeJLL4(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF4POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL4(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL4()


FUNCTION void __JudyFreeJLL5(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF5POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL5(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL5()


FUNCTION void __JudyFreeJLL6(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF6POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL6(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL6()


FUNCTION void __JudyFreeJLL7(Pjll_t Pjll, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAF7POPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjll_t, Pjll);
	JudyFree((Pvoid_t) Pjll, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLL7(%lu), Words = %lu\n", Pjll,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLL7()

#endif // JU_64BIT


// Note:  __JudyFreeJLW() receives a root pointer with NO root pointer type
// bits present, that is, they are stripped by P_JLW():

FUNCTION void __JudyFreeJLW(Pjlw_t Pjlw, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JU_LEAFWPOPTOWORDS(Pop1);

	// MALLOCBITS_TEST(Pjlw_t, Pjlw);	// see above.
	JudyFree((Pvoid_t) Pjlw, Words);

	if (Pjpm) Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu =  __JudyFreeJLW(%lu), Words = %lu\n", Pjlw,
		    __JudyMemSequence++, Pop1, Words, Pop1 - 1);


} // __JudyFreeJLW()


FUNCTION void __JudyFreeJLB1(Pjlb_t Pjlb, Pjpm_t Pjpm)
{
	Word_t Words = sizeof(jlb_t) / cJU_BYTESPERWORD;

	MALLOCBITS_TEST(Pjlb_t, Pjlb);
	JudyFree((Pvoid_t) Pjlb, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE5("0x%x %8lu =  __JudyFreeJLB1(), Words = %lu\n", Pjlb,
		    __JudyMemSequence++, Words, Pjpm->jpm_Pop0);


} // __JudyFreeJLB1()


#ifdef JUDYL

FUNCTION void __JudyLFreeJV(Pjv_t Pjv, Word_t Pop1, Pjpm_t Pjpm)
{
	Word_t Words = JL_LEAFVPOPTOWORDS(Pop1);

	MALLOCBITS_TEST(Pjv_t, Pjv);
	JudyFree((Pvoid_t) Pjv, Words);

	Pjpm->jpm_TotalMemWords -= Words;

	TRACE_FREE6("0x%x %8lu = __JudyLFreeJV(%lu), Words = %lu\n", Pjv,
		    __JudyMemSequence++, Pop1, Words, Pjpm->jpm_Pop0);


} // __JudyLFreeJV()

#endif // JUDYL
