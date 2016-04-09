
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread
CXXFLAGS=-std=c++11 -O2

all: ArachneTest ThreadCreationTest GoThreadCreate ArachneYieldTest
ArachneTest: ArachneTest.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ArachneTest.cc $(LIBS) -o ArachneTest

ArachneYieldTest: ArachneYieldTest.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ArachneYieldTest.cc $(LIBS) -o ArachneYieldTest

ThreadCreationTest: ThreadCreationTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadCreationTest.cc $(LIBS) -o ThreadCreationTest

GoThreadCreate: GoThreadCreate.go
	go build GoThreadCreate.go

clean:
	rm -f ArachneTest ThreadCreationTest


