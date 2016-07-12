
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread
CXXFLAGS=-std=c++11 -O3

all: ArachneCreateTest ThreadCreationTest GoThreadCreate ArachneYieldTest ThreadYieldTest GoThreadYield
ArachneCreateTest: ArachneCreateTest.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ArachneCreateTest.cc $(LIBS) -o ArachneCreateTest

ArachneYieldTest: ArachneYieldTest.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ArachneYieldTest.cc $(LIBS) -o ArachneYieldTest

ThreadCreationTest: ThreadCreationTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadCreationTest.cc $(LIBS) -o ThreadCreationTest

ThreadYieldTest: ThreadYieldTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadYieldTest.cc $(LIBS) -o ThreadYieldTest

%: %.go
	go build $<

clean:
	rm -f ArachneCreateTest ThreadCreationTest


