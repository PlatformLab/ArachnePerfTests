
DEBUG=-g
LIBS=-I../Arachne -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils -lpthread

ArachneTest: ArachneTest.cc
	g++ -Wall -Werror -O2 $(DEBUG) -std=c++11 -o ArachneTest ArachneTest.cc $(LIBS)

clean:
	rm -f ArachneTest
