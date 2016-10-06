
DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -I../PerfUtils -L../PerfUtils -lPerfUtils  -pthread
CXXFLAGS=-std=c++11 -O3 -Wall -Werror -Wformat=2 -Wextra -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute -Wno-non-template-friend -Woverloaded-virtual -Wcast-qual -Wcast-align -Wconversion -Weffc++ -fomit-frame-pointer

BINS = ArachneCreateTest  ArachneYieldTest ArachneCVTest   ArachneBlockSignalTest \
		ArachneBlockSignal_ContextSwitchTest ArachneTripleYieldTest TestSpinLock
EXTRAS = ThreadCreationTest GoThreadCreate ThreadYieldTest GoThreadYield
all: $(BINS) $(EXTRAS)

################################################################################
# Arachne Targets

$(BINS) : % : %.cc ../Arachne/libArachne.a
	g++  $(DEBUG) $(CXXFLAGS)  $< $(LIBS) -o $@


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
