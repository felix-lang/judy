#ifndef _JUDY_INCLUDED
#define	_JUDY_INCLUDED
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

// @(#) $Revision: 4.52 $ $Source: /judy/src/Judy.h $
//
// HEADER FILE FOR EXPORTED FEATURES IN JUDY LIBRARY, libJudy.*
//
// See the manual entries for details.
//
// Note:  This header file uses old-style comments on #-directive lines and
// avoids "()" on macro names in comments for compatibility with older cc -Aa
// and some tools on some platforms.


// PLATFORM-SPECIFIC OVERHEAD:

#ifdef notdef
The following lines allow Judy.h to be included on both Windows and
non-Windows systems, and for Judy.h to be used both as-is (for internal
compiles) and after unifdef-ing (when delivered).  See also the definitions of
uint*_t in JudyPrivate.h.
#endif
#ifndef JU_WIN
#ifdef JU_FLAVOR_COV
#pragma C-Cover off	/* exclude inline functions in public header files */
#endif
#include <inttypes.h>	/* pre-include explicitly, for Linux */
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif
#endif

#ifdef JU_FLAVOR_COV
#pragma C-Cover off	/* exclude inline functions in public header files */
#endif
#include <stdlib.h>	/* auto-includes inttypes.h on some platforms */
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#ifdef __cplusplus		/* support use by C++ code */
extern "C" {
#endif


// ****************************************************************************
// DECLARE SOME BASE TYPES IN CASE THEY ARE MISSING:
//
// These base types include "const" where appropriate, but only where of
// interest to the caller.  For example, a caller cares that a variable passed
// by reference will not be modified, such as, "const void * Pindex", but not
// that the called function internally does not modify the pointer itself, such
// as, "void * const Pindex".
//
// Note that it's OK to pass a Pvoid_t to a Pcvoid_t; the latter is the same,
// only constant.  Callers need to do this so they can also pass & Pvoid_t to
// PPvoid_t (non-constant).

#ifndef	_PCVOID_T
#define	_PCVOID_T
typedef const void * Pcvoid_t;
#endif

#ifndef	_PVOID_T
#define	_PVOID_T
typedef	void *	 Pvoid_t;
typedef	void **	PPvoid_t;
#endif

#ifndef _WORD_T
#define _WORD_T
#ifdef notdef
The following lines allow Judy.h to be included on both Windows and
non-Windows systems, and for Judy.h to be used both as-is (for internal
compiles) and after unifdef-ing (when delivered).
#endif
#ifndef JU_WIN_IPF
typedef unsigned long	 Word_t, * PWord_t;  // expect 32-bit or 64-bit words.
#else
typedef unsigned __int64 Word_t, * PWord_t;  // expect 64-bit words.
#endif
#endif

// TBD:  For compatibility with existing regression tests that need ulong_t,
// which is not always available on all platforms, although Judy itself, and
// newly written Judy-using code, should not need ulong_t:

#ifndef _ULONG_T
#define _ULONG_T
typedef Word_t ulong_t;
#endif

#ifndef	NULL
#define	NULL 0
#endif


// ****************************************************************************
// SUPPORT FOR ERROR HANDLING:
//
// Judy error numbers:
//
// Note:  These are an enum so there's a related typedef, but the numbers are
// spelled out so you can map a number back to its name.

typedef	enum		// uint8_t -- but C does not support this type of enum.
{

// Note:  JU_ERRNO_NONE and JU_ERRNO_FULL are not real errors.  They specify
// conditions which are otherwise impossible return values from 32-bit
// Judy1Count, which has 2^32 + 1 valid returns (0..2^32) plus one error
// return.  These pseudo-errors support the return values that cannot otherwise
// be unambiguously represented in a 32-bit word, and will never occur on a
// 64-bit system.

	JU_ERRNO_NONE		= 0,
	JU_ERRNO_FULL		= 1,
	JU_ERRNO_NFMAX		= JU_ERRNO_FULL,

// JU_ERRNO_NOMEM comes from malloc(3C) when Judy cannot obtain needed memory.
// The system errno value is also set to ENOMEM.  This error can be recoverable
// if the calling application frees other memory.
//
// TBD:  Currently there is no guarantee the Judy array has no memory leaks
// upon JU_ERRNO_NOMEM.

	JU_ERRNO_NOMEM		= 2,

// Problems with parameters from the calling program:
//
// JU_ERRNO_NULLPPARRAY means PPArray was null; perhaps PArray was passed where
// &PArray was intended.  Similarly, JU_ERRNO_NULLPINDEX means PIndex was null;
// perhaps &Index was intended.  Also, JU_ERRNO_NONNULLPARRAY,
// JU_ERRNO_NULLPVALUE, and JU_ERRNO_UNSORTED, all added later (hence with
// higher numbers), mean:  A non-null array was passed in where a null pointer
// was required; PValue was null; and unsorted indexes were detected.

	JU_ERRNO_NULLPPARRAY	= 3,	// see above.
	JU_ERRNO_NONNULLPARRAY	= 10,	// see above.
	JU_ERRNO_NULLPINDEX	= 4,	// see above.
	JU_ERRNO_NULLPVALUE	= 11,	// see above.
	JU_ERRNO_NOTJUDY1	= 5,	// PArray is not to a Judy1 array.
	JU_ERRNO_NOTJUDYL	= 6,	// PArray is not to a JudyL array.
	JU_ERRNO_NOTJUDYSL	= 7,	// PArray is not to a JudySL array.
	JU_ERRNO_UNSORTED	= 12,	// see above.

// Errors below this point are not recoverable; further tries to access the
// Judy array might result in EFAULT and a core dump:
//
// JU_ERRNO_OVERRUN occurs when Judy detects, upon reallocation, that a block
// of memory in its own freelist was modified since being freed.

	JU_ERRNO_OVERRUN	= 8,

// JU_ERRNO_CORRUPT occurs when Judy detects an impossible value in a Judy data
// structure:
//
// Note:  The Judy data structure contains some redundant elements that support
// this type of checking.

	JU_ERRNO_CORRUPT	= 9

// Warning:  At least some C or C++ compilers do not tolerate a trailing comma
// above here.  At least we know of one case, in aCC; see JAGad58928.

} JU_Errno_t;


// Judy errno structure:
//
// WARNING:  For compatibility with possible future changes, the fields of this
// struct should not be referenced directly.  Instead use the macros supplied
// below.

// This structure should be declared on the stack in a threaded process.

typedef	struct _JUDY_ERROR_STRUCT
{
	JU_Errno_t je_Errno;		// one of the enums above.
	int	   je_ErrID;		// often an internal source line number.
	Word_t	   je_reserved[4];	// for future backward compatibility.

} JError_t, * PJError_t;


// Related macros:
//
// Fields from error struct:

#define	JU_ERRNO(PJError)  ((PJError)->je_Errno)
#define	JU_ERRID(PJError)  ((PJError)->je_ErrID)

// For checking return values from various Judy functions:
//
// Note:  Define JERR as -1, not as the seemingly more portable (Word_t)
// (~0UL), to avoid a compiler "overflow in implicit constant conversion"
// warning.

#define	  JERR (-1)			/* functions returning int or Word_t */
#define	 PJERR ((Pvoid_t)  (~0UL))	/* mainly for use here, see below    */
#define	PPJERR ((PPvoid_t) (~0UL))	/* functions that return PPvoid_t    */

// Convenience macro for when detailed error information (PJError_t) is not
// desired by the caller; a purposely short name:

#define	PJE0  ((PJError_t) NULL)


// ****************************************************************************
// JUDY FUNCTIONS:
//
// PJE is a shorthand for use below:

#define	_PJE  PJError_t PJError

extern int	__Judy1Test(	 Pvoid_t   Pjpm,   Word_t   Index);
extern int	Judy1Test(	 Pcvoid_t  PArray, Word_t   Index,   _PJE);
extern int	Judy1Set(	 PPvoid_t PPArray, Word_t   Index,   _PJE);
extern int	Judy1SetArray(	 PPvoid_t PPArray, Word_t   Count,
					     const Word_t * const PIndex,
								     _PJE);
extern int	Judy1Unset(	 PPvoid_t PPArray, Word_t   Index,   _PJE);
extern Word_t	Judy1Count(	 Pcvoid_t  PArray, Word_t   Index1,
						   Word_t   Index2,  _PJE);
extern int	Judy1ByCount(	 Pcvoid_t  PArray, Word_t   Count,
						   Word_t * PIndex,  _PJE);
extern Word_t	Judy1FreeArray(	 PPvoid_t PPArray,		     _PJE);
extern Word_t	Judy1MemUsed(	 Pcvoid_t  PArray);
extern Word_t	Judy1MemActive(	 Pcvoid_t  PArray);
extern int	Judy1First(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1Next(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	__Judy1Next(	 Pvoid_t   Pjpm,   Word_t * PIndex);
extern int	Judy1Last(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1Prev(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1FirstEmpty( Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1NextEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1LastEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	Judy1PrevEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);

extern PPvoid_t	__JudyLGet(	 Pvoid_t   Pjpm,   Word_t    Index);
extern PPvoid_t	JudyLGet(	 Pcvoid_t  PArray, Word_t    Index,  _PJE);
extern PPvoid_t	JudyLIns(	 PPvoid_t PPArray, Word_t    Index,  _PJE);
extern int	JudyLInsArray(	 PPvoid_t PPArray, Word_t    Count,
					     const Word_t * const PIndex,
					     const Word_t * const PValue,
								     _PJE);
extern int	JudyLDel(	 PPvoid_t PPArray, Word_t    Index,  _PJE);
extern Word_t	JudyLCount(	 Pcvoid_t  PArray, Word_t    Index1,
						   Word_t    Index2, _PJE);
extern PPvoid_t	JudyLByCount(	 Pcvoid_t  PArray, Word_t    Count,
						   Word_t *  PIndex, _PJE);
extern Word_t	JudyLFreeArray(	 PPvoid_t PPArray,		     _PJE);
extern Word_t	JudyLMemUsed(	 Pcvoid_t  PArray);
extern Word_t	JudyLMemActive(	 Pcvoid_t  PArray);
extern PPvoid_t	JudyLFirst(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern PPvoid_t	JudyLNext(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern PPvoid_t	__JudyLNext(	 Pvoid_t   Pjpm,   Word_t * PIndex);
extern PPvoid_t	JudyLLast(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern PPvoid_t	JudyLPrev(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	JudyLFirstEmpty( Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	JudyLNextEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	JudyLLastEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);
extern int	JudyLPrevEmpty(	 Pcvoid_t  PArray, Word_t * PIndex,  _PJE);

extern PPvoid_t	JudySLGet(	 Pcvoid_t  PArray, const char * Index, _PJE);
extern PPvoid_t	JudySLIns(	 PPvoid_t PPArray, const char * Index, _PJE);
extern int	JudySLDel(	 PPvoid_t PPArray, const char * Index, _PJE);
extern Word_t	JudySLFreeArray( PPvoid_t PPArray,		       _PJE);
extern PPvoid_t	JudySLFirst(	 Pcvoid_t  PArray,	 char * Index, _PJE);
extern PPvoid_t	JudySLNext(	 Pcvoid_t  PArray,	 char * Index, _PJE);
extern PPvoid_t	JudySLLast(	 Pcvoid_t  PArray,	 char * Index, _PJE);
extern PPvoid_t	JudySLPrev(	 Pcvoid_t  PArray,	 char * Index, _PJE);

// The following are exported but not (yet) documented, and they have no macro
// equivalents:

extern Word_t	JudyGetTotalMem(void);

// JUDYL ARRAY POINTER LOW BITS PROTECTED FOR USE BY APPLICATIONS:
//
// The JLAP_INVALID pattern never appears in a JudyL array pointer.  Hence
// applications that build trees of JudyL arrays can use this pattern to
// distinguish a JudyL array value that is a pointer to another JudyL array
// from one that is a pointer to some other, application-defined object.
//
// Note:  J1LAP_NEXTTYPE is used to build a mask, but must agree with the
// J1*/JL* values below.
//
// Note:  Use old-style comments here because some tools insist on it for
// macros referenced in other macros.

#define	J1LAP_NEXTTYPE	0x8		/* first enum value beyond J*AP types */

#define	JLAP_MASK    (J1LAP_NEXTTYPE-1) /* mask pointer for JLAP_INVALID     */
#define JLAP_INVALID	0x4		/* not a JLAP			     */


// JUDY ARRAY POINTER LOW BITS = TYPES:
//
// Note:  Judy libraries can be constructed with JLAPLEAF_POPU* unused, but
// external callers cannot tell if this is the case, so in this header file
// they are still defined and supported.

enum {
	JLAPNULL	= 0x0,		/* must be == 0			    */
	J1APNULL	= 0x0,		/* must be == 0			    */
	JLAPLEAF	= 0x1,		/* Word_t leafW with Pop0 in word 1 */
//	JLAPLEAF_POPU2	= 0x2,		/* Word_t leafW with Pop1 == 2      */
	JLAPBRANCH	= 0x3,		/* pointer to jLpm_t => branch	    */
	JLAPINVALID	= JLAP_INVALID,	/* not a JLAP, assume == 4	    */
//	JLAPLEAF_POPU1	= 0x5,		/* Word_t leafW with Pop1 == 1      */
	J1APLEAF	= 0x6,		/* Word_t leafW with Pop0 in word 1 */
	J1APBRANCH	= 0x7		/* pointer to j1pm_t => branch	    */
};


// ****************************************************************************
// MACRO EQUIVALENTS FOR JUDY FUNCTIONS:
//
// The following macros, such as J1T, are shorthands for calling Judy functions
// with parameter address-of and detailed error checking included.  Since they
// are macros, the error checking code is replicated each time the macro is
// used, but it runs fast in the normal case of no error.
//
// If the caller does not like the way the default JUDYERROR macro handles
// errors (such as an exit(1) call when out of memory), they may define their
// own before the "#include <Judy.h>".  A routine such as HandleJudyError
// could do checking on specific error numbers and print a different message
// dependent on the error.  The following is one example:
//
// Note: the back-slashes are removed because some compilers will not accept
// them in comments.
//
// void HandleJudyError(char *, int, char *, int, int);
// #define JUDYERROR(CallerFile, CallerLine, JudyFunc, JudyErrno, JudyErrID)
//  {
//    HandleJudyError(CallerFile, CallerLine, JudyFunc, JudyErrno, JudyErrID);
//  }
//
// The routine HandleJudyError could do checking on specific error numbers and
// print a different message dependent on the error.
//
// The macro receives five parameters that are:
//
// 1.  CallerFile:  Source filename where a Judy call returned a serious error.
// 2.  CallerLine:  Line number in that source file.
// 3.  JudyFunc:    Name of Judy function reporting the error.
// 4.  JudyErrno:   One of the JU_ERRNO* values enumerated above.
// 5.  JudyErrID:   The je_ErrID field described above.

#ifndef JUDYERROR_NOTEST
#ifndef JUDYERROR	/* supply a default error macro */
#ifdef JU_FLAVOR_COV
#pragma C-Cover off	/* exclude inline functions in public header files */
#endif
#include <stdio.h>
#ifdef JU_FLAVOR_COV
#pragma C-Cover on
#endif

#define JUDYERROR(CallerFile, CallerLine, JudyFunc, JudyErrno, JudyErrID) \
	{							\
	    (void) fprintf(stderr, "File '%s', line %d: %s(), "	\
			   "JU_ERRNO_* == %d, ID == %d\n",	\
			   CallerFile, CallerLine,		\
			   JudyFunc, JudyErrno, JudyErrID);	\
	    exit(1);						\
	}

#endif /* JUDYERROR */
#endif /* JUDYERROR_NOTEST */

// If the JUDYERROR macro is not desired at all, then the following eliminates
// it.  However, the return code from each Judy function (that is, the first
// parameter of each macro) must be checked by the caller to assure that an
// error did not occur.
//
// Example:
//
//   #define JUDYERROR_NOTEST 1
//   #include <Judy.h>
//
// or use this cc option at compile time:
//
//   cc -DJUDYERROR_NOTEST ...
//
// Example code:
//
//   J1S(Rc, PArray, Index);
//   if (Rc == JERR) goto ...error
//
// or:
//
//   JLI(Pvalue, PArray, Index);
//   if (Pvalue == PJERR) goto ...error


// Internal shorthand macros for writing the J1S, etc. macros:

#ifdef JUDYERROR_NOTEST /* ============================================ */

// "Judy Set Error":

#define	_JSE(FuncName,Errno)  ((void) 0)

// Note:  In each _J*() case below, the digit is the number of key parameters
// to the Judy*() call.  Just assign the Func result to the caller's Rc value
// without a cast because none is required, and this keeps the API simpler.
// However, a family of different _J*() macros is needed to support the
// different numbers of key parameters (0,1,2) and the Func return type.
//
// In the names below, "I" = integer result; "P" = pointer result.  Note, the
// Funcs for _J*P() return PPvoid_t, but cast this to a Pvoid_t for flexible,
// error-free assignment, and then compare to PJERR.

#define	_J0I(Rc,PArray,Func,FuncName) \
	{ (Rc) = Func(PArray, PJE0); }

#define	_J1I(Rc,PArray,Index,Func,FuncName) \
	{ (Rc) = Func(PArray, Index, PJE0); }

#define	_J1P(PV,PArray,Index,Func,FuncName) \
	{ (PV) = (Pvoid_t) Func(PArray, Index, PJE0); }

#define	_J2I(Rc,PArray,Index,Arg2,Func,FuncName) \
	{ (Rc) = Func(PArray, Index, Arg2, PJE0); }

#define	_J2C(Rc,PArray,Index1,Index2,Func,FuncName) \
	{ (Rc) = Func(PArray, Index1, Index2, PJE0); }

#define	_J2P(PV,PArray,Index,Arg2,Func,FuncName) \
	{ (PV) = (Pvoid_t) Func(PArray, Index, Arg2, PJE0); }

// Variations for Judy*Set/InsArray functions:

#define	_J2AI(Rc,PArray,Count,PIndex,Func,FuncName) \
	{ (Rc) = Func(PArray, Count, PIndex, PJE0); }
#define	_J3AI(Rc,PArray,Count,PIndex,PValue,Func,FuncName) \
	{ (Rc) = Func(PArray, Count, PIndex, PValue, PJE0); }

#else /* ================ ! JUDYERROR_NOTEST ============================= */

#define	_JE(FuncName,PJE) \
	JUDYERROR(__FILE__, __LINE__, FuncName, JU_ERRNO(PJE), JU_ERRID(PJE))

#define	_JSE(FuncName,Errno)						\
	{								\
	    JError_t _JError;						\
	    JU_ERRNO(&_JError) = (Errno);				\
	    JU_ERRID(&_JError) = __LINE__;				\
	    _JE(FuncName, &_JError);					\
	}

// Note:  In each _J*() case below, the digit is the number of key parameters
// to the Judy*() call.  Just assign the Func result to the caller's Rc value
// without a cast because none is required, and this keeps the API simpler.
// However, a family of different _J*() macros is needed to support the
// different numbers of key parameters (0,1,2) and the Func return type.
//
// In the names below, "I" = integer result; "P" = pointer result.  Note, the
// Funcs for _J*P() return PPvoid_t, but cast this to a Pvoid_t for flexible,
// error-free assignment, and then compare to PJERR.

#define	_J0I(Rc,PArray,Func,FuncName)					\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = Func(PArray, &_JError)) == JERR)		\
		_JE(FuncName, &_JError);				\
	}

#define	_J1I(Rc,PArray,Index,Func,FuncName)				\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = Func(PArray, Index, &_JError)) == JERR)		\
		_JE(FuncName, &_JError);				\
	}

#define	_J1P(Rc,PArray,Index,Func,FuncName)				\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = (Pvoid_t) Func(PArray, Index, &_JError)) == PJERR) \
		_JE(FuncName, &_JError);				\
	}

#define	_J2I(Rc,PArray,Index,Arg2,Func,FuncName)			\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = Func(PArray, Index, Arg2, &_JError)) == JERR)	\
		_JE(FuncName, &_JError);				\
	}

// Variation for Judy*Count functions, which return 0, not JERR, for error (and
// also for other non-error cases):
//
// Note:  JU_ERRNO_NFMAX should only apply to 32-bit Judy1, but this header
// file lacks the necessary ifdef's to make it go away otherwise, so always
// check against it.

#define	_J2C(Rc,PArray,Index1,Index2,Func,FuncName)			\
	{								\
	    JError_t _JError;						\
	    if ((((Rc) = Func(PArray, Index1, Index2, &_JError)) == 0)	\
	     && (JU_ERRNO(&_JError) > JU_ERRNO_NFMAX))			\
	    {								\
		_JE(FuncName, &_JError);				\
	    }								\
	}

#define	_J2P(PV,PArray,Index,Arg2,Func,FuncName)			\
	{								\
	    JError_t _JError;						\
	    if (((PV) = (Pvoid_t) Func(PArray, Index, Arg2, &_JError))	\
		== PJERR) _JE(FuncName, &_JError);			\
	}

// Variations for Judy*Set/InsArray functions:

#define	_J2AI(Rc,PArray,Count,PIndex,Func,FuncName)			\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = Func(PArray, Count, PIndex, &_JError)) == JERR)	\
		_JE(FuncName, &_JError);				\
	}

#define	_J3AI(Rc,PArray,Count,PIndex,PValue,Func,FuncName)		\
	{								\
	    JError_t _JError;						\
	    if (((Rc) = Func(PArray, Count, PIndex, PValue, &_JError))	\
		== JERR) _JE(FuncName, &_JError);			\
	}

#endif /* ================ ! JUDYERROR_NOTEST ============================= */

// Some of the macros are special cases that use inlined shortcuts for speed
// with root-level leaves:

#define	J1T(Rc,PArray,Index)					\
{								\
    Word_t  _jpt = ((Word_t) (PArray)) & JLAP_MASK;		\
    PWord_t _PL  = (PWord_t) (((Word_t) (PArray)) ^ _jpt);	\
    (Rc)	 = 0;						\
								\
    if (_jpt == J1APLEAF)					\
    {								\
	Word_t  _pop1 = _PL[0] + 1;				\
	if ((Index) <= _PL[_pop1])				\
	{							\
	    for(;;)						\
	    {							\
	        if (*(++_PL) >= (Index))			\
	        {						\
	            if (*_PL == (Index)) (Rc) = 1;		\
		    break;                                      \
	        }						\
	    }                                                   \
	}                                                       \
    }                                                           \
    else if (_jpt == J1APBRANCH)				\
    {								\
	/* note: no error possible here: */			\
	(Rc) = __Judy1Test((Pvoid_t) _PL, (Index));		\
    }								\
    else if (PArray != (Pvoid_t) NULL)				\
    {								\
	(Rc) = JERR;						\
	_JSE("Judy1Test", JU_ERRNO_NOTJUDY1);			\
    }								\
}

#define	J1S( Rc,    PArray,   Index) \
	_J1I(Rc, (&(PArray)), Index,  Judy1Set,   "Judy1Set")
#define	J1SA(Rc,    PArray,   Count, PIndex) \
	_J2AI(Rc,(&(PArray)), Count, PIndex, Judy1SetArray, "Judy1SetArray")
#define	J1U( Rc,    PArray,   Index) \
	_J1I(Rc, (&(PArray)), Index,  Judy1Unset, "Judy1Unset")
#define	J1F( Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1First, "Judy1First")
#define	J1N( Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1Next,  "Judy1Next")
#define	J1L( Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1Last,  "Judy1Last")
#define	J1P( Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1Prev,  "Judy1Prev")
#define	J1FE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1FirstEmpty, "Judy1FirstEmpty")
#define	J1NE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1NextEmpty,  "Judy1NextEmpty")
#define	J1LE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1LastEmpty,  "Judy1LastEmpty")
#define	J1PE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), Judy1PrevEmpty,  "Judy1PrevEmpty")
#define	J1C( Rc,    PArray,   Index1,  Index2) \
	_J2C(Rc,    PArray,   Index1,  Index2, Judy1Count,   "Judy1Count")
#define	J1BC(Rc,    PArray,   Count,   Index) \
	_J2I(Rc,    PArray,   Count, &(Index), Judy1ByCount, "Judy1ByCount")
#define	J1FA(Rc,    PArray) \
	_J0I(Rc, (&(PArray)), Judy1FreeArray, "Judy1FreeArray")
#define	J1MU(Rc,    PArray) \
	(Rc) = Judy1MemUsed(PArray)

#define	JLG(PV,PArray,Index)						\
{									\
    Word_t  _jpt = ((Word_t) (PArray)) & JLAP_MASK;			\
    PWord_t _PL  = (PWord_t) (((Word_t) (PArray)) ^ _jpt);		\
    extern const unsigned char __jL_LeafWOffset[];			\
									\
    (PV) = (Pvoid_t) NULL;						\
									\
    if (_jpt == JLAPLEAF)						\
    {									\
	Word_t  _pop1 = _PL[0] + 1;					\
	if ((Index) <= _PL[_pop1])					\
	{								\
	    while (1)							\
	    {								\
	        if (*(++_PL) >= (Index))				\
	        {							\
	            if (*_PL == (Index))				\
		    {							\
	                (PV) = (Pvoid_t)(_PL+__jL_LeafWOffset[_pop1]-1);\
		    }							\
		    break;						\
	        }							\
	    }								\
	}								\
    }									\
    else if (_jpt == JLAPBRANCH)					\
    {									\
	(PV) = (Pvoid_t) __JudyLGet((Pvoid_t) _PL, (Index));		\
    }									\
    else if (PArray != (Pvoid_t) NULL)					\
    {									\
	(PV) = PJERR;							\
	_JSE("JudyLGet", JU_ERRNO_NOTJUDYL);				\
    }									\
}

#define	JLI( PV,    PArray,   Index) \
	_J1P(PV, (&(PArray)), Index,  JudyLIns,   "JudyLIns")
#define	JLIA(Rc,    PArray,   Count, PIndex, PValue) \
	_J3AI(Rc,(&(PArray)), Count, PIndex, PValue, JudyLInsArray, \
						    "JudyLInsArray")
#define	JLD( Rc,    PArray,   Index) \
	_J1I(Rc, (&(PArray)), Index,  JudyLDel,   "JudyLDel")
#define	JLF( PV,    PArray,   Index) \
	_J1P(PV,    PArray, &(Index), JudyLFirst, "JudyLFirst")

#define	JLN(PV,PArray,Index)						\
{									\
    Word_t  _jpt = ((Word_t) (PArray)) & JLAP_MASK;			\
    PWord_t _PL  = (PWord_t) (((Word_t) (PArray)) ^ _jpt);		\
    extern const unsigned char __jL_LeafWOffset[];			\
									\
    (PV) = (Pvoid_t) NULL;						\
									\
    if (_jpt == JLAPLEAF)						\
    {									\
	Word_t _pop1 = _PL[0] + 1;					\
	if ((Index) < _PL[_pop1])					\
	{								\
	    while (1)							\
	    {								\
	        if ((Index) < *(++_PL))					\
	        {							\
	            (Index) = *_PL;					\
	            (PV) = (Pvoid_t) (_PL + __jL_LeafWOffset[_pop1] -1);\
		    break;						\
	        }							\
	    }								\
	}								\
    }									\
    else if (_jpt == JLAPBRANCH)					\
    {									\
	(PV) = (Pvoid_t) JudyLNext((Pvoid_t) PArray, &(Index), PJE0);	\
    }									\
    else if (PArray != (Pvoid_t) NULL)					\
    {									\
	(PV) = PJERR;							\
	_JSE("JudyLNext", JU_ERRNO_NOTJUDYL);				\
    }									\
}

#define	JLL( PV,    PArray,   Index) \
	_J1P(PV,    PArray, &(Index), JudyLLast,  "JudyLLast")
#define	JLP( PV,    PArray,   Index) \
	_J1P(PV,    PArray, &(Index), JudyLPrev,  "JudyLPrev")
#define	JLFE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), JudyLFirstEmpty, "JudyLFirstEmpty")
#define	JLNE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), JudyLNextEmpty,  "JudyLNextEmpty")
#define	JLLE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), JudyLLastEmpty,  "JudyLLastEmpty")
#define	JLPE(Rc,    PArray,   Index) \
	_J1I(Rc,    PArray, &(Index), JudyLPrevEmpty,  "JudyLPrevEmpty")
#define	JLC( Rc,    PArray,   Index1,  Index2) \
	_J2C(Rc,    PArray,   Index1,  Index2, JudyLCount,   "JudyLCount")
#define	JLBC(PV,    PArray,   Count,   Index) \
	_J2P(PV,    PArray,   Count, &(Index), JudyLByCount, "JudyLByCount")
#define	JLFA(Rc,    PArray) \
	_J0I(Rc, (&(PArray)), JudyLFreeArray, "JudyLFreeArray")
#define	JLMU(Rc,    PArray) \
	(Rc) = JudyLMemUsed(PArray)

#define	JSLG( PV,    PArray,   Index) \
	_J1P( PV,    PArray,   Index, JudySLGet,   "JudySLGet")
#define	JSLI( PV,    PArray,   Index) \
	_J1P( PV, (&(PArray)), Index, JudySLIns,   "JudySLIns")
#define	JSLD( Rc,    PArray,   Index) \
	_J1I( Rc, (&(PArray)), Index, JudySLDel,   "JudySLDel")
#define	JSLF( PV,    PArray,   Index) \
	_J1P( PV,    PArray,   Index, JudySLFirst, "JudySLFirst")
#define	JSLN( PV,    PArray,   Index) \
	_J1P( PV,    PArray,   Index, JudySLNext,  "JudySLNext")
#define	JSLL( PV,    PArray,   Index) \
	_J1P( PV,    PArray,   Index, JudySLLast,  "JudySLLast")
#define	JSLP( PV,    PArray,   Index) \
	_J1P( PV,    PArray,   Index, JudySLPrev,  "JudySLPrev")
#define	JSLFA(Rc,    PArray) \
	_J0I( Rc, (&(PArray)), JudySLFreeArray, "JudySLFreeArray")

#ifdef __cplusplus
}
#endif
#endif /* ! _JUDY_INCLUDED */
