PERFUTILS=../PerfUtils
ARACHNE=../Arachne
COREARBITER=../CoreArbiter

DEBUG=-g
LIBS=-I$(ARACHNE)/include  -L$(ARACHNE)/lib -lArachne  -L$(COREARBITER)/lib -lCoreArbiter -I$(PERFUTILS)/include $(PERFUTILS)/lib/libPerfUtils.a -pthread
CXXFLAGS=-std=c++11 -O3 -Wall -Werror -Wformat=2 -Wextra -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute -Wno-non-template-friend -Woverloaded-virtual -Wcast-qual -Wcast-align -Wconversion -fomit-frame-pointer

BINS = ArachneCreateTest  NullYieldTest ArachneYieldTest ArachneCVTest \
		ArachneBlockSignalTest ArachneBlockSignal_ContextSwitchTest \
		ThreadTurnaround
all: $(BINS)

$(BINS) : % : %.cc $(ARACHNE)/lib/libArachne.a
	g++  $(DEBUG) $(CXXFLAGS)  $< $(LIBS) -o $@

clean:
	rm -f $(BINS) $(EXTRAS) *.log
