
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread
CXXFLAGS=-std=c++11 -O2

all: ArachneTest ThreadCreationTest
ArachneTest: ArachneTest.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ArachneTest.cc $(LIBS) -o ArachneTest

ThreadCreationTest: ThreadCreationTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadCreationTest.cc $(LIBS) -o ThreadCreationTest

clean:
	rm -f ArachneTest ThreadCreationTest

