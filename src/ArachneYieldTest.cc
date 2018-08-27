#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#include "Common.h"

#define NUM_SAMPLES 10000000

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;


/**
 * This benchmark computes a median time for a yield in one thread to return
 * in another thread in the same core.
 * Note that this benchmark has no direct analog in golang or std::thread,
 * since we cannot finely control core placement in these systems.
 *
 * Alternatively, we could measure the time it takes to return to the original
 * thread, but this seems less intuitive and still would not match with
 * std::thread or Golang.
 */

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

uint64_t latencies[NUM_SAMPLES];

void
yielder() {
    for (int i = 0; i < NUM_SAMPLES; i++) {
        timeTrace("About to yield in thread 2");
        Arachne::yield();
        timeTrace("Returned from yield in thread 2");
    }
}

int
realMain(std::vector<int>* coresOrderedByHT, Options* options) {
    // Idle hypertwins if they aren't needed to be active.
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
    }
    PerfUtils::Util::serialize();
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    Arachne::ThreadId id = Arachne::createThreadOnCore((*coresOrderedByHT)[0], yielder);
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t beforeYield = Cycles::rdtsc();
        timeTrace("About to yield in thread 1");
        Arachne::yield();
        timeTrace("Returned from yield in thread 1");
        latencies[i] = (Cycles::rdtsc() - beforeYield) / 2;
    }
    Arachne::join(id);

    PerfUtils::Util::serialize();
    if (!options->hypertwinsActive) {
        Arachne::unidleCore((*coresOrderedByHT)[1]);
    }
    Arachne::shutDown();
    return 0;
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::init(&argc, argv);

    std::vector<int> coresOrderedByHT = getCoresOrderedByHT();

    Options options = parseOptions(argc, const_cast<char**>(argv));
    printf("Active Hypertwin: %d\n", options.hypertwinsActive);

    Arachne::createThreadOnCore(coresOrderedByHT[0], realMain, &coresOrderedByHT, &options);
    // Must be the last call
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Yield Latency", latencies, NUM_SAMPLES, "data");
#if TIME_TRACE
        TimeTrace::setOutputFileName("ArachneYieldTest_TimeTrace.log");
        TimeTrace::print();
#endif
}
