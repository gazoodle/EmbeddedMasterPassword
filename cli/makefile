all: cli

cli: cli.o mpw.o io.o command.o persistence.o
	gcc -Wall cli.o mpw.o io.o command.o persistence.o -o cli -lstdc++ 

cli.o: cli.cpp ../src/lib/*.h
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 cli.cpp

mpw.o: ../src/lib/mpw.cpp ../src/lib/str_ptr.h
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 ../src/lib/mpw.cpp

io.o: ../src/lib/io.cpp
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 ../src/lib/io.cpp

command.o: ../src/app/command.cpp ../src/app/command.h ../src/app/userinfo.h ../src/app/siteinfo.h ../src/app/persistence.h mpw.o
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 -DCONSOLE ../src/app/command.cpp

persistence.o: ../src/app/persistence.cpp ../src/app/persistence.h
	gcc -c -Wall -I ../src/lib -lstdc++ -O3 -DCONSOLE ../src/app/persistence.cpp

clean:
	rm -rf *.o cli
