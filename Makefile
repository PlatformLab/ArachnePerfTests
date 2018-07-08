CXX ?= g++
CXXFLAGS=-std=c++11 -O3 -Wall -Werror -Wformat=2 -Wextra -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute -Wno-non-template-friend -Woverloaded-virtual -Wcast-qual -Wcast-align -Wconversion -fomit-frame-pointer $(EXTRA_CXXFLAGS)

# Output directories
SRC_DIR = src
BIN_DIR = bin

# Dependencies
PERFUTILS=../PerfUtils
ARACHNE=../Arachne
COREARBITER=../CoreArbiter
LIBS=-I$(ARACHNE)/include -I$(COREARBITER)/include  -I$(PERFUTILS)/include \
	-L$(ARACHNE)/lib -lArachne -L$(COREARBITER)/lib -lCoreArbiter \
	$(PERFUTILS)/lib/libPerfUtils.a -lpcrecpp -pthread

BINS = ArachneCreateTest  NullYieldTest ArachneYieldTest ArachneCVTest \
		ArachneBlockSignalTest ArachneBlockSignal_ContextSwitchTest \
		ThreadTurnaround CoreRequest_Contended_Latency \
		CoreRequest_Contended_Timeout_Latency CoreRequest_Noncontended_Latency

FULL_BINS = $(patsubst %,$(BIN_DIR)/%,$(BINS))

all: $(FULL_BINS)

$(BIN_DIR)/% : $(SRC_DIR)/%.cc $(ARACHNE)/lib/libArachne.a | $(BIN_DIR)
	$(CXX)  $(CXXFLAGS)  $< $(LIBS) -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
