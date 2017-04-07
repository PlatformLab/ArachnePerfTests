#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <atomic>

#include "Arachne.h"
#include "Cycles.h"
#include "Util.h"
#include "Stats.h"


using PerfUtils::Cycles;


#define NUM_SAMPLES 1000000

volatile int consumerIsReady = 0;

std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];
std::atomic<uint64_t> beforeSignal;

// This is used for signalling
Arachne::ThreadId consumerId;

void producer() {
	for (int i = 0; i < NUM_SAMPLES; i++) {
		while (!consumerIsReady);
		consumerIsReady = 0;
        beforeSignal = Cycles::rdtsc();
        Arachne::signal(consumerId);
	}
    Arachne::join(consumerId);
    Arachne::shutDown();
}

void consumer() {
    // Tell the producer we are ready.
    consumerIsReady = 1;
	for (int i = 0; i < NUM_SAMPLES; i++) {
        Arachne::block();
        latencies[arrayIndex++] = Cycles::rdtsc() - beforeSignal;
        while (consumerIsReady);
		consumerIsReady = 1;
	}
}

void sleeper() {
    Arachne::block();
}

int main(int argc, const char** argv){
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::init(&argc, argv);

    int threadListLength = 0;
    if (argc > 1) threadListLength = atoi(argv[1]);

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < threadListLength; i++)
        Arachne::createThreadOnCore(1, sleeper);

    // Add some work
	consumerId = Arachne::createThreadOnCore(1, consumer);
	Arachne::createThreadOnCore(0, producer);
    // Must be the last call
    Arachne::waitForTermination();

    if (arrayIndex != NUM_SAMPLES) abort();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Block Signal Latency", latencies, NUM_SAMPLES, "data");
    return 0;
}
