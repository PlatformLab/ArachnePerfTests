
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread
CXXFLAGS=-std=c++11 -O3

BINS = ArachneCreateTest  ArachneYieldTest ArachneCVTest   ArachneBlockSignalTest \
		ArachneBlockSignal_ContextSwitchTest ArachneTripleYieldTest TestSpinLock
EXTRAS = ThreadCreationTest GoThreadCreate ThreadYieldTest GoThreadYield
all: $(BINS) $(EXTRAS)

################################################################################
# Arachne Targets

$(BINS) : % : %.cc ../Arachne/libArachne.a
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  $< $(LIBS) -o $@


################################################################################
# Non-Arachne Targets

ThreadCreationTest: ThreadCreationTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadCreationTest.cc $(LIBS) -o ThreadCreationTest

ThreadYieldTest: ThreadYieldTest.cc
	g++ -Wall -Werror $(DEBUG) $(CXXFLAGS)  ThreadYieldTest.cc $(LIBS) -o ThreadYieldTest

%: %.go
	go build $<

clean:
	rm -f $(BINS) $(EXTRAS)
