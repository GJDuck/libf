
CC = clang++-4.0 -std=c++1z -fno-exceptions -fno-rtti -nodefaultlibs 
COPTS = -fPIC -O2
CLIBS = -lc -lgc
CLIB = $(OBJS)

test: test.cpp
	$(CC) -O2 -o test test.cpp -I ../ -L ../ -lf++ $(CLIBS) -Wl,-rpath \
        $(PWD)/..

libf2html: libf2html.cpp
	$(CC) -O2 -o libf2html libf2html.cpp -I ../ -L ../ -lf++ $(CLIBS) \
        -Wl,-rpath $(PWD)/..

bench: bench.cpp
	$(CC) -O2 -o bench bench.cpp -I ../ -L ../ -lf++ $(CLIBS) \
        -lstdc++ -Wl,-rpath $(PWD)/..

clean:
	rm -f *.o *.s *.i test libf2html bench

