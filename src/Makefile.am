# I'm know this is file is a complete mess and completely the wrong way to
# do it. However, Automake sucks and I haven't figured out a better way yet. 
# If you know the 'right' way to do it please send me a patch! 

# The problem is that almost every single .c file is compiled to two different
# object files based on defines. Then are later assembled into a single
#library.
#
# This prevents all of the default rules from working as far as I can tell.

INCLUDES =  -I. 
AM_CFLAGS = @CFLAGS@ @WARN_CFLAGS@

CLEANFILES = libJudy.la Judy1TablesGen JudyLTablesGen

lib_LTLIBRARIES = libJudy.la

JUDY1_FLAGS = -DJU_LINUX -DJU_LINUX_IA32 -DJU_FLAVOR_PRODUCT -DJUDY1 -UJUDYL  
JUDYL_FLAGS = -DJU_LINUX -DJU_LINUX_IA32 -DJU_FLAVOR_PRODUCT -DJUDYL -UJUDY1

libJudy_la_SOURCES = 

libJudy_la_LIBADD = Judy1Test.o Judy1SetArray.o Judy1Unset.o Judy1First.o Judy1Count.o Judy1FreeArray.o Judy1MemUsed.o Judy1MemActive.o Judy1Tables.o Judy1Cascade.o Judy1Decascade.o Judy1CreateBranch.o Judy1InsertBranch.o Judy1MallocIF.o __Judy1Test.o Judy1Set.o Judy1Next.o Judy1Prev.o Judy1NextEmpty.o Judy1PrevEmpty.o Judy1ByCount.o Judy1TablesGen JudyLGet.o JudyLInsArray.o JudyLDel.o JudyLFirst.o JudyLCount.o JudyLFreeArray.o JudyLMemUsed.o JudyLMemActive.o JudyLTables.o JudyLCascade.o JudyLDecascade.o JudyLCreateBranch.o __JudyLGet.o JudyLIns.o JudyLInsertBranch.o JudyLMallocIF.o JudyLTablesGen JudyLNext.o JudyLPrev.o JudyLNextEmpty.o JudyLPrevEmpty.o JudyMalloc.o JudySearchLeaf.o JudySL.o

##############################################################################################
## Judy Generic Objects
##############################################################################################
JudyMalloc.o: JudyMalloc.c
	$(CC) $(AM_CFLAGS) -c -o $@ JudyMalloc.c

JudySearchLeaf.o: JudySearchLeaf.c
	$(CC) $(AM_CFLAGS) -c -o $@ JudySearchLeaf.c

##############################################################################################
## Judy 1 Objects
##############################################################################################
Judy1Test.o: JudyGet.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyGet.c

Judy1SetArray.o: JudyInsArray.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyInsArray.c

Judy1Unset.o: JudyDel.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyDel.c

Judy1First.o: JudyFirst.c	
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyFirst.c	

Judy1Count.o: JudyCount.c	
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyCount.c	

Judy1FreeArray.o: JudyFreeArray.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyFreeArray.c

Judy1MemUsed.o: JudyMemUsed.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyMemUsed.c

Judy1MemActive.o: JudyMemActive.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyMemActive.c

Judy1Tables.o: JudyTables.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyTables.c

Judy1Cascade.o: JudyCascade.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyCascade.c

Judy1Decascade.o: JudyDecascade.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyDecascade.c

Judy1CreateBranch.o: JudyCreateBranch.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyCreateBranch.c

Judy1InsertBranch.o: JudyInsertBranch.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyInsertBranch.c

Judy1MallocIF.o: JudyMallocIF.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o $@ JudyMallocIF.c


__Judy1Test.o: JudyGet.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJUDYGETINLINE -c -o $@ JudyGet.c

Judy1Set.o: JudyIns.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJU_COV_INLINES -c -o $@ JudyIns.c

Judy1Next.o: JudyPrevNext.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJUDYNEXT -c -o $@ JudyPrevNext.c

Judy1Prev.o: JudyPrevNext.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJUDYPREV -c -o $@ JudyPrevNext.c

Judy1NextEmpty.o: JudyPrevNextEmpty.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJUDYNEXT -c -o $@ JudyPrevNextEmpty.c	

Judy1PrevEmpty.o: JudyPrevNextEmpty.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DJUDYPREV -c -o $@ JudyPrevNextEmpty.c	

Judy1ByCount.o: JudyByCount.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB -c -o $@ JudyByCount.c

Judy1TablesGen: JudyTables.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -o Judy1TablesGen JudyTables.c
	rm -f Judy1Tables.c
	Judy1TablesGen > Judy1Tables.c
	$(CC) $(AM_CFLAGS) $(JUDY1_FLAGS) -c -o Judy1Tables.o Judy1Tables.c

##############################################################################################
## Judy L Objects
##############################################################################################
JudyLGet.o: JudyGet.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyGet.c

JudyLInsArray.o: JudyInsArray.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyInsArray.c

JudyLDel.o: JudyDel.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyDel.c

JudyLFirst.o: JudyFirst.c	
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyFirst.c	

JudyLCount.o: JudyCount.c	
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyCount.c	

JudyLFreeArray.o: JudyFreeArray.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyFreeArray.c

JudyLMemUsed.o: JudyMemUsed.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyMemUsed.c

JudyLMemActive.o: JudyMemActive.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyMemActive.c

JudyLTables.o: JudyTables.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyTables.c

JudyLCascade.o: JudyCascade.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyCascade.c

JudyLDecascade.o: JudyDecascade.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyDecascade.c

JudyLCreateBranch.o: JudyCreateBranch.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyCreateBranch.c

JudyLInsertBranch.o: JudyInsertBranch.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyInsertBranch.c

JudyLMallocIF.o: JudyMallocIF.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o $@ JudyMallocIF.c


__JudyLGet.o: JudyGet.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJUDYGETINLINE -c -o $@ JudyGet.c

JudyLIns.o: JudyIns.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJU_COV_INLINES -c -o $@ JudyIns.c

JudyLNext.o: JudyPrevNext.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJUDYNEXT -c -o $@ JudyPrevNext.c

JudyLPrev.o: JudyPrevNext.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJUDYPREV -c -o $@ JudyPrevNext.c

JudyLNextEmpty.o: JudyPrevNextEmpty.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJUDYNEXT -c -o $@ JudyPrevNextEmpty.c	

JudyLPrevEmpty.o: JudyPrevNextEmpty.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DJUDYPREV -c -o $@ JudyPrevNextEmpty.c	

JudyLByCount.o: JudyByCount.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB -c -o $@ JudyByCount.c

JudyLTablesGen: JudyTables.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -o JudyLTablesGen JudyTables.c
	rm -f JudyLTables.c
	JudyLTablesGen > JudyLTables.c
	$(CC) $(AM_CFLAGS) $(JUDYL_FLAGS) -c -o JudyLTables.o JudyLTables.c

##############################################################################################
## Judy SL Objects
##############################################################################################
JudySL.o: JudySL.c
	$(CC) $(AM_CFLAGS) -c -o $@ JudySL.c







