CC=g++

all: svg2lvl 

svg2lvl : main.o tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o
	$(CC) $(CFLAGS) main.o tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o -o svg2lvl 

main.o: main.cpp tinyxml.h	
	$(CC) $(CFLAGS) -c main.cpp

tinyxml.o: tinyxml.h tinystr.h
	$(CC) $(CFLAGS) -c tinyxml.cpp

tinystr.o: tinystr.h tinystr.cpp
	$(CC) $(CFLAGS) -c tinystr.cpp

tinyxmlparser.o: tinyxml.h tinystr.h
	$(CC) $(CFLAGS) -c tinyxmlparser.cpp

tinyxmlerror.o: tinyxml.h tinystr.h
	$(CC) $(CFLAGS) -c tinyxmlerror.cpp

clean:
	\rm *.o
