
FILES=\
    fcompare.cpp \
    flist.cpp \
    fseq.cpp \
    fshow.cpp \
    fstring.cpp \
    ftree.cpp \
    fvector.cpp

OBJS=\
    fcompare.o \
    flist.o \
    ftree.o \
    fseq.o \
    fshow.o \
    fstring.o \
    fvector.o

    # IMPORTANT!
    #
    # -std=c++1z: libf requires C++17
    # -fno-exceptions: libf does not use exceptions.
    # -fno-rtti: libf does not use runtime type information.
    # -nodefaultlibs: libf must not depend on the C++ stdlib.  It does depend
    #                 on libc, so we use must use -lc linker option.
CXX = clang++-4.0 -std=c++1z -fno-exceptions -fno-rtti -nodefaultlibs -fPIC \
    -O2
COPTS = -fPIC 
CLIBS = -lc -lgc
CLIB = $(OBJS)

libf.so: $(OBJS)
	$(CXX) -shared -o libf.so $(OBJS) $(CLIBS)

clean:
	rm -f *.o *.s *.i main

