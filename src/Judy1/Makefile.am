INCLUDES =  -I. -I.. -I../JudyCommon/ 
AM_CFLAGS = -DJUDY1 @WARN_CFLAGS@ 

noinst_LTLIBRARIES = libJudy1.la libnext.la libprev.la libcount.la libinline.la

libJudy1_la_SOURCES = Judy1Test.c Judy1Tables.c Judy1Set.c Judy1SetArray.c Judy1Unset.c Judy1Cascade.c Judy1Count.c Judy1CreateBranch.c Judy1Decascade.c Judy1First.c Judy1FreeArray.c Judy1InsertBranch.c Judy1MallocIF.c Judy1MemActive.c Judy1MemUsed.c 

libnext_la_SOURCES = Judy1Next.c Judy1NextEmpty.c 
libnext_la_CFLAGS = $(AM_CFLAGS) -DJUDYNEXT

libprev_la_SOURCES = Judy1Prev.c Judy1PrevEmpty.c 
libprev_la_CFLAGS = $(AM_CFLAGS) -DJUDYPREV

libcount_la_SOURCES = Judy1ByCount.c
libcount_la_CFLAGS = $(AM_CFLAGS) -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB

libinline_la_SOURCES = j__udy1Test.c
libinline_la_CFLAGS = $(AM_CFLAGS) -DJUDYGETINLINE

Judy1Tables.c: Judy1TablesGen.c
	$(CC) $(INCLUDES) $(AM_CFLAGS) @CFLAGS@ -o Judy1TablesGen Judy1TablesGen.c; ./Judy1TablesGen


Judy1Test.c: symlinks

symlinks:
	ln -sf ../JudyCommon/JudyByCount.c      	Judy1ByCount.c   
	ln -sf ../JudyCommon/JudyCascade.c      	Judy1Cascade.c
	ln -sf ../JudyCommon/JudyCount.c        	Judy1Count.c
	ln -sf ../JudyCommon/JudyCreateBranch.c 	Judy1CreateBranch.c
	ln -sf ../JudyCommon/JudyDecascade.c    	Judy1Decascade.c
	ln -sf ../JudyCommon/JudyDel.c          	Judy1Unset.c
	ln -sf ../JudyCommon/JudyFirst.c        	Judy1First.c
	ln -sf ../JudyCommon/JudyFreeArray.c    	Judy1FreeArray.c
	ln -sf ../JudyCommon/JudyGet.c          	Judy1Test.c
	ln -sf ../JudyCommon/JudyGet.c          	j__udy1Test.c
	ln -sf ../JudyCommon/JudyInsArray.c     	Judy1SetArray.c
	ln -sf ../JudyCommon/JudyIns.c          	Judy1Set.c
	ln -sf ../JudyCommon/JudyInsertBranch.c 	Judy1InsertBranch.c
	ln -sf ../JudyCommon/JudyMallocIF.c     	Judy1MallocIF.c
	ln -sf ../JudyCommon/JudyMemActive.c    	Judy1MemActive.c
	ln -sf ../JudyCommon/JudyMemUsed.c      	Judy1MemUsed.c
	ln -sf ../JudyCommon/JudyPrevNext.c     	Judy1Next.c
	ln -sf ../JudyCommon/JudyPrevNext.c     	Judy1Prev.c
	ln -sf ../JudyCommon/JudyPrevNextEmpty.c	Judy1NextEmpty.c
	ln -sf ../JudyCommon/JudyPrevNextEmpty.c	Judy1PrevEmpty.c
	ln -sf ../JudyCommon/JudyTables.c	        Judy1TablesGen.c

