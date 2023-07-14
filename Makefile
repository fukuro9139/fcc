CFLAGS=-std=c++20 -g -static

fcc: fcc.cpp

test: fcc
		./test.sh

clean:
		rm -f fcc *.o *~ tmp*

.PHONY: test clean
