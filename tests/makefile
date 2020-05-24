all: test

test: test.o mpw.o
	gcc -Wall test.o mpw.o -o test -lstdc++ 

test.o: test.cpp ../lib/*.h
	gcc -c -Wall -I ../lib -lstdc++ -O3 test.cpp

mpw.o: ../lib/mpw.cpp
	gcc -c -Wall -I ../lib -lstdc++ -O3 ../lib/mpw.cpp

clean:
	rm -rf *.o test
