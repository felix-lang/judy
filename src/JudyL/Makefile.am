INCLUDES =  -I. -I.. -I../JudyCommon/ 
AM_CFLAGS = -DJUDYL @WARN_CFLAGS@ 

noinst_LTLIBRARIES = libJudyL.la libnext.la libprev.la libcount.la libinline.la

libJudyL_la_SOURCES = JudyLCascade.c JudyLTables.c JudyLCount.c JudyLCreateBranch.c JudyLDecascade.c JudyLDel.c JudyLFirst.c JudyLFreeArray.c JudyLGet.c JudyLInsArray.c JudyLIns.c JudyLInsertBranch.c JudyLMallocIF.c JudyLMemActive.c JudyLMemUsed.c 

libnext_la_SOURCES = JudyLNext.c JudyLNextEmpty.c
libnext_la_CFLAGS = $(AM_CFLAGS) -DJUDYNEXT

libprev_la_SOURCES = JudyLPrev.c JudyLPrevEmpty.c
libprev_la_CFLAGS = $(AM_CFLAGS) -DJUDYPREV

libcount_la_SOURCES = JudyLByCount.c
libcount_la_CFLAGS = $(AM_CFLAGS) -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB

libinline_la_SOURCES = j__udyLGet.c
libinline_la_CFLAGS = $(AM_CFLAGS) -DJUDYGETINLINE

JudyLTables.c: JudyLTablesGen.c
	$(CC) $(INCLUDES) $(AM_CFLAGS) @CFLAGS@ -o JudyLTablesGen JudyLTablesGen.c; ./JudyLTablesGen 


JudyLCascade.c: symlinks

symlinks: 
	ln -sf ../JudyCommon/JudyByCount.c      	JudyLByCount.c   
	ln -sf ../JudyCommon/JudyCascade.c              JudyLCascade.c
	ln -sf ../JudyCommon/JudyCount.c        	JudyLCount.c
	ln -sf ../JudyCommon/JudyCreateBranch.c 	JudyLCreateBranch.c
	ln -sf ../JudyCommon/JudyDecascade.c    	JudyLDecascade.c
	ln -sf ../JudyCommon/JudyDel.c          	JudyLDel.c
	ln -sf ../JudyCommon/JudyFirst.c        	JudyLFirst.c
	ln -sf ../JudyCommon/JudyFreeArray.c    	JudyLFreeArray.c
	ln -sf ../JudyCommon/JudyGet.c          	JudyLGet.c
	ln -sf ../JudyCommon/JudyGet.c          	j__udyLGet.c
	ln -sf ../JudyCommon/JudyInsArray.c     	JudyLInsArray.c
	ln -sf ../JudyCommon/JudyIns.c          	JudyLIns.c
	ln -sf ../JudyCommon/JudyInsertBranch.c 	JudyLInsertBranch.c
	ln -sf ../JudyCommon/JudyMallocIF.c     	JudyLMallocIF.c
	ln -sf ../JudyCommon/JudyMemActive.c    	JudyLMemActive.c
	ln -sf ../JudyCommon/JudyMemUsed.c      	JudyLMemUsed.c
	ln -sf ../JudyCommon/JudyPrevNext.c     	JudyLNext.c
	ln -sf ../JudyCommon/JudyPrevNext.c     	JudyLPrev.c
	ln -sf ../JudyCommon/JudyPrevNextEmpty.c	JudyLNextEmpty.c
	ln -sf ../JudyCommon/JudyPrevNextEmpty.c	JudyLPrevEmpty.c
	ln -sf ../JudyCommon/JudyTables.c	        JudyLTablesGen.c

