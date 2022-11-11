DIRSEP = /
DEFS = -DHAVE_CONFIG_H

PACKAGE = diffutils
VERSION = 2.7.1

PROGRAMS = diff.exe diff3.exe cmp.exe sdiff.exe

all: $(PROGRAMS) # info

version.c:
	echo #include <config.h> >$@
	echo char const version_string[] = "GNU $(PACKAGE) $(VERSION)" >>$@

COMPILE = $(CC) -c $(DEFS) -I. -Ipc $(CFLAGS)

.SUFFIXES: .c $o

.c$o:
	$(COMPILE) $<

common_o = error$o getopt$o getopt1$o version$o xmalloc$o
diff_o = diff$o analyze$o cmpbuf$o dir$o io$o util$o \
	context$o ed$o ifdef$o normal$o side$o \
	fnmatch$o $(REGEX) $(common_o) $(ALLOCA) $(LIBOBJS)
diff.exe: $(diff_o)
	$(CC) $(OUT)$@ $(LDFLAGS) $(diff_o) $(LIBS)
diff3_o = diff3$o $(common_o) $(LIBOBJS)
diff3.exe: $(diff3_o)
	$(CC) $(OUT)$@ $(LDFLAGS) $(diff3_o) $(LIBS)
sdiff_o = sdiff$o $(common_o) $(LIBOBJS)
sdiff.exe: $(sdiff_o)
	$(CC) $(OUT)$@ $(LDFLAGS) $(sdiff_o) $(LIBS)
cmp_o = cmp$o cmpbuf$o $(common_o) $(LIBOBJS)
cmp.exe: $(cmp_o)
	$(CC) $(OUT)$@ $(LDFLAGS) $(cmp_o) $(LIBS)

info: diff.info
diff.info: diff.texi version.texi
	makeinfo --no-split diff.texi --output=$@

dvi: diff.dvi
diff.dvi: diff.texi
	texi2dvi diff.texi

$(diff_o): diff.h system.h
cmp$o diff3$o sdiff$o: system.h
context$o diff$o regex$o: regex.h
cmp$o diff$o diff3$o sdiff$o getopt$o getopt1$o: getopt.h
diff$o fnmatch$o: fnmatch.h
analyze$o cmpbuf$o cmp$o: cmpbuf.h
$(diff_o) $(cmp_o) $(sdiff_o) $(diff3_o): pc$(DIRSEP)config.h

pc$o: pc/pc.c
	$(COMPILE) pc$(DIRSEP)pc.c
popen$o: pc/popen.c
	$(COMPILE) pc$(DIRSEP)popen.c
dirent$o: pc/dirent.c
	$(COMPILE) pc$(DIRSEP)dirent.c

gnuregex.dll: regex$o pc/gnuregex.def
	$(CC) $(LDFLAGS) $(DLDFLAGS) regex$o -o $@ pc/gnuregex.def
regex.lib : pc/gnuregex.def
	emximp -o $@ pc/gnuregex.def
regexnt.lib : gnuregex.dll
	makelib gnuregex.dll -o $@
regexnt.a gnuregex.exp: pc/gnuregex.def
	dlltool -d pc/gnuregex.def -D gnuregex.dll -e gnuregex.exp -l regexnt.a

emx-debug: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="gcc -g" CFLAGS="-O" \
		o=.o OUT="-o ./" LDFLAGS="" \
		REGEX="regex.o"

emxnt-debug: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="gcc -Zwin32 -g" CFLAGS="-O" \
		o=.o OUT="-o ./" LDFLAGS="" \
		REGEX="regex.o"

emxdos: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="gcc" CFLAGS="-O" \
		o=.o OUT="-o ./" LDFLAGS="-s" \
		REGEX="regex.o"

emxos2: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="gcc -Zomf" CFLAGS="-O -D__OS2__" \
		o=.obj OUT="-o ./" LDFLAGS="-s -Zcrtdll -Zstack 512" \
		DLDFLAGS="-Zdll" LIBOBJS="dirent.obj pc.obj" \
		REGEX="regex.obj"

emxnt: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="gcc -Zwin32 -Zcrtdll=crtrsxnt" CFLAGS="-O" \
		o=.o OUT="-o ./" LDFLAGS="-s -Zstack 512" \
		DLDFLAGS="-Zdll" LIBOBJS="popen.o pc.o dirent.o" \
		REGEX="regex.o" LIBS="-lmscompat"

mingw32: pc/config.h
	$(MAKE) -f pc/Makefile all \
		o=.o OUT="-o ./" LDFLAGS="-s" DLDFLAGS="-mdll" \
		LIBOBJS="popen.o pc.o dirent.o" REGEX="regex.o" 

w32dos: pc/config.h
	$(MAKE) -f pc/Makefile diff.exe DIRSEP="\\" \
		CC="wcl386 -zq -bt=dos4g" CFLAGS="-s -Zp1" \
		o=.obj OUT="-Fe=" LDFLAGS="-k0x80000 -x -l=pmodew" \
		LIBOBJS="popen.obj pc.obj" REGEX="regex.obj"

w32os2: pc/config.h
	$(MAKE) -f pc/Makefile all DIRSEP="\\" \
		CC="wcl386 -zq -bt=os2v2" CFLAGS="-s -Zp1" \
		o=.obj OUT="-Fe=" LDFLAGS="-k0x80000 -x -l=os2v2" \
		LIBOBJS="popen.obj pc.obj" REGEX="regex.obj"

w32nt: pc/config.h
	$(MAKE) -f pc/Makefile all DIRSEP="\\" \
		CC="wcl386 -zq -bt=nt" CFLAGS="-s -Zp1" \
		o=.obj OUT="-Fe=" LDFLAGS="-k0x80000 -x -l=nt" \
		LIBOBJS="popen.obj pc.obj" REGEX="regex.obj"

ibmnt ibmos2: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="icc -q" CFLAGS="-O -wpro- -wcnd-" \
		o=.obj OUT="-Fe " LDFLAGS="-B/ST:0x80000 -B/NOE setargv.obj" \
		LIBOBJS="popen.obj pc.obj dirent.obj" REGEX="regex.obj"

ibm-debug: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="icc -q -Ti" CFLAGS="-wpro- -wcnd-" \
		o=.obj OUT="-Fe " LDFLAGS="-B/ST:0x80000 -B/NOE setargv.obj" \
		LIBOBJS="popen.obj pc.obj dirent.obj" REGEX="regex.obj"

mscnt: pc/config.h
	$(MAKE) -f pc/Makefile all \
		CC="cl -nologo -MD" CFLAGS="-J -Zp -Ogti -Gs -W1" \
		o=.obj OUT="-Fe " LDFLAGS="setargv.obj -F 80000" \
		LIBOBJS="popen.obj pc.obj dirent.obj" REGEX="regex.obj"
