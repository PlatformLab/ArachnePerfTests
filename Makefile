
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread

ArachneTest: ArachneTest.cc
	g++ -Wall -Werror -O2 $(DEBUG) -std=c++11  ArachneTest.cc $(LIBS) -o ArachneTest

clean:
	rm -f ArachneTest
