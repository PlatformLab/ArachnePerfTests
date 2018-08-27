#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <random>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"
#include "PerfUtils/TimeTrace.h"

#include "Common.h"

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

#define NUM_SAMPLES 10000000
#define MEAN_DELAY 0.000002

// Uncomment the following line to enable TimeTraces.
// #define TIME_TRACE 1

// Provides a cleaner way of invoking TimeTrace::record, with the code
// conditionally compiled in or out by the TIME_TRACE #ifdef. Arguments
// are made uint64_t (as opposed to uin32_t) so the caller doesn't have to
// frequently cast their 64-bit arguments into uint32_t explicitly: we will
// help perform the casting internally.
static inline void
timeTrace(const char* format,
        uint64_t arg0 = 0, uint64_t arg1 = 0, uint64_t arg2 = 0,
        uint64_t arg3 = 0)
{
#if TIME_TRACE
    TimeTrace::record(format, uint32_t(arg0), uint32_t(arg1),
            uint32_t(arg2), uint32_t(arg3));
#endif
}

namespace Arachne {
extern bool disableLoadEstimation;
}

volatile int consumerIsReady = 0;


uint64_t latencies[NUM_SAMPLES];
std::atomic<uint64_t> beforeSignal;

// This is used for signalling
Arachne::ThreadId consumerId;

void
producer(std::vector<int>* coresOrderedByHT, Options* options) {
    // Idle hypertwins if they aren't needed to be active.
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
        Arachne::idleCore((*coresOrderedByHT)[3]);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> uniformIG(0, MEAN_DELAY * 2);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        while (!consumerIsReady)
            ;
        consumerIsReady = 0;
        // Wait a random interval for consumer to actually sleep
        uint64_t signalTime =
            Cycles::rdtsc() + Cycles::fromSeconds(uniformIG(gen));
        while (Cycles::rdtsc() < signalTime)
            ;

        PerfUtils::Util::serialize();
        beforeSignal = Cycles::rdtsc();
        timeTrace("About to send signal");
        Arachne::signal(consumerId);
    }
    Arachne::join(consumerId);

    if (!options->hypertwinsActive) {
        Arachne::unidleCore((*coresOrderedByHT)[1]);
        Arachne::unidleCore((*coresOrderedByHT)[3]);
    }
    Arachne::shutDown();
}

void
consumer() {
    // Tell the producer we are ready.
    consumerIsReady = 1;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        Arachne::block();
        timeTrace("Just unblocked");
        uint64_t stopTime = Cycles::rdtsc();
        PerfUtils::Util::serialize();
        latencies[i] = stopTime - beforeSignal;
        while (consumerIsReady)
            ;
        consumerIsReady = 1;
    }
}

void
sleeper() {
    Arachne::block();
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 4;
    Arachne::maxNumCores = 4;
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
    Arachne::init(&argc, argv);

    Options options = parseOptions(argc, const_cast<char**>(argv));
    printf("Active Hypertwins: %d\nNumber of Sleeping Threads: %d\n", options.hypertwinsActive, options.numSleepers);

    std::vector<int> coresOrderedByHT = getCoresOrderedByHT();

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < options.numSleepers; i++)
        Arachne::createThreadOnCore(coresOrderedByHT[2], sleeper);

    // Add some work
    consumerId = Arachne::createThreadOnCore(coresOrderedByHT[2], consumer);
    Arachne::createThreadOnCore(coresOrderedByHT[0], producer, &coresOrderedByHT, &options);
    // Must be the last call
    Arachne::waitForTermination();

    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Block Signal Latency", latencies, NUM_SAMPLES, "data");
#if TIME_TRACE
    TimeTrace::setOutputFileName("ArachneBlockSignal_TimeTrace.log");
    TimeTrace::print();
#endif
    return 0;
}
