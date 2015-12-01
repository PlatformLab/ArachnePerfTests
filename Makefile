
DEBUG=-g
LIBS=-I../Arachne -L../Arachne -lArachne -lpthread -I../PerfUtils -L../PerfUtils -lPerfUtils

ArachneTest: ArachneTest.cc
	g++ $(DEBUG) -std=c++11 -o ArachneTest ArachneTest.cc $(LIBS)

clean:
	rm -f ArachneTest
