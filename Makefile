DEBUG=-g
LIBS=-I../Arachne  -L../Arachne -lArachne  -L../CoreArbiter -lCoreArbiter -I../PerfUtils ../PerfUtils/libPerfUtils.a   -pthread
CXXFLAGS=-std=c++11 -O3 -Wall -Werror -Wformat=2 -Wextra -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute -Wno-non-template-friend -Woverloaded-virtual -Wcast-qual -Wcast-align -Wconversion -fomit-frame-pointer

BINS = ArachneCreateTest  NullYieldTest ArachneYieldTest ArachneCVTest \
		ArachneBlockSignalTest ArachneBlockSignal_ContextSwitchTest
all: $(BINS)

$(BINS) : % : %.cc ../Arachne/libArachne.a
	g++  $(DEBUG) $(CXXFLAGS)  $< $(LIBS) -o $@

clean:
	rm -f $(BINS) $(EXTRAS) *.log
