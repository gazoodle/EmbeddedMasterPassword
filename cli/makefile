all: cli

cli: cli.o mpw.o
	gcc -Wall cli.o mpw.o -o cli -lstdc++ 

cli.o: cli.cpp ../src/lib/*.h
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 cli.cpp

mpw.o: ../src/lib/mpw.cpp
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 ../src/lib/mpw.cpp

clean:
	rm -rf *.o cli
