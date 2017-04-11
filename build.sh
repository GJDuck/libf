#!/bin/sh

set -e

if [ ! -e /usr/include/gc.h ]
then
    echo "\033[31mWARNING\033[0m: missing Boehm garbage collector development \
files."
    echo "         Ubuntu users can try \"sudo apt-get install libgc-dev\""
fi

if [ ! -x /usr/bin/clang++-4.0 ]
then
    echo "\033[31mWARNING\033[0m: the current build assumes clang++-4.0 for \
c++17."
    echo "         Ubuntu users can install it using the instructions here:"
    echo "         http://apt.llvm.org/"
fi

make clean
make -j4

cd examples
make clean

make test
make libf2html
make bench

cd ..

for BASENAME in compare list map maybe show string tuple value vector
do
    examples/libf2html f${BASENAME}.h > doc/${BASENAME}.html
done

