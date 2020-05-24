all: test

test: test.o mpw.o io.o
	gcc -Wall test.o mpw.o io.o -o test -lstdc++ 

test.o: test.cpp ../src/lib/*.h
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 test.cpp

mpw.o: ../src/lib/mpw.cpp
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 ../src/lib/mpw.cpp

io.o: ../src/lib/io.cpp
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 ../src/lib/io.cpp

clean:
	rm -rf *.o test
