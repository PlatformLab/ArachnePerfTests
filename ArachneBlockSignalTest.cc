#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <atomic>
#include <random>

#include "Arachne.h"
#include "Cycles.h"
#include "Util.h"
#include "Stats.h"


using PerfUtils::Cycles;

#define NUM_SAMPLES 10000000
#define MEAN_DELAY 0.000002

namespace Arachne {
    extern bool disableLoadEstimation;
}

volatile int consumerIsReady = 0;

uint64_t latencies[NUM_SAMPLES];
std::atomic<uint64_t> beforeSignal;

// This is used for signalling
Arachne::ThreadId consumerId;

void producer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> uniformIG(0, MEAN_DELAY * 2);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        while (!consumerIsReady);
        consumerIsReady = 0;
        // Wait a random interval for consumer to actually sleep
        uint64_t signalTime = Cycles::rdtsc() + Cycles::fromSeconds(uniformIG(gen));
        while (Cycles::rdtsc() < signalTime);

        PerfUtils::Util::serialize();
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
        uint64_t stopTime = Cycles::rdtsc();
        PerfUtils::Util::serialize();
        latencies[i] = stopTime - beforeSignal;
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
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
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

    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Block Signal Latency", latencies, NUM_SAMPLES, "data");
    return 0;
}
