
CC = clang++ -std=c++1y -fno-exceptions -fno-rtti -nodefaultlibs 
COPTS = -fPIC -O2
CLIBS = -lc -lgc
CLIB = $(OBJS)

test: test.c
	$(CC) -O2 -o test test.c -I ../ -L ../ -lf $(CLIBS) -Wl,-rpath ..

read: read.c
	$(CC) -O2 -o read read.c -I ../ -L ../ -lf $(CLIBS) -Wl,-rpath ..

clean:
	rm -f *.o *.s *.i test

