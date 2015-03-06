
FILES=\
    fbase.c \
    fcompare.c \
    flist.c \
    fseq.c \
    fshow.c \
    fstring.c \
    ftree.c \
    fvector.c

OBJS=\
    fbase.o \
    fcompare.o \
    flist.o \
    ftree.o \
    fseq.o \
    fshow.o \
    fstring.o \
    fvector.o

    # IMPORTANT!
    #
    # -std=c++11: libf requires C++11
    # -fno-exceptions: libf does not use exceptions.
    # -fno-rtti: libf does not use runtime type information.
    # -nodefaultlibs: libf must not depend on the C++ stdlib.  It does depend
    #                 on libc, so we use must use -lc linker option.
CC = clang++ -std=c++11 -fno-exceptions -fno-rtti -nodefaultlibs -fPIC -O3
COPTS = -fPIC 
CLIBS = -lc -lgc
CLIB = $(OBJS)

libf.so: $(OBJS)
	$(CC) -shared -o libf.so $(OBJS) $(CLIBS)

all:
	$(CC) $(FILES) -o main \
	    -Wall -O2 -nodefaultlibs -lc --save-temps -Wl,-rpath,$(PWD) \
        -L. -lgc

libf.a: $(OBJS)
	ar -cvq libf.a $(OBJS)

debug:
	clang++ -std=c++11 -fno-exceptions -fno-rtti $(FILES) -o main \
	    -Wall -lmygc -O0 -g -nodefaultlibs -lc --save-temps -L.

gcc:
	g++ -std=c++11 -fno-exceptions -fno-rtti $(FILES) -o main \
	    -Wall -l:libgc.so.1.0.3 -O2 -g -nodefaultlibs -lc --save-temps

clean:
	rm -f *.o *.s *.i main

