#include <stdio.h>
#include <string.h>
#include <random>
#include <vector>

#include "Arachne/Arachne.h"
#include "Arachne/CorePolicy.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#include "Common.h"

#define NUM_SAMPLES 10000000
#define MEAN_DELAY 0.000002

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

namespace Arachne {
extern bool disableLoadEstimation;
void idleCore(int coreId);
void unidleCore(int coreId);
}

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
static uint64_t arrayIndex = 0;

void
task(uint64_t creationTime) {
    timeTrace("Creation completed");
    uint64_t startTime = Cycles::rdtsc();
    PerfUtils::Util::serialize();
    uint64_t latency = startTime - creationTime;
    latencies[arrayIndex++] = latency;
}

int
realMain(std::vector<int>* coresOrderedByHT, Options* options) {
    // Idle hypertwins if they aren't needed to be active.
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
        Arachne::idleCore((*coresOrderedByHT)[3]);
    }
    int targetCore = (*coresOrderedByHT)[2];
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    // Set up random smoothing.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> uniformIG(0, MEAN_DELAY * 2);

    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Wait a random interval before next creation
        uint64_t signalTime =
            Cycles::rdtsc() + Cycles::fromSeconds(uniformIG(gen));
        while (Cycles::rdtsc() < signalTime)
            ;
        PerfUtils::Util::serialize();
        uint64_t creationTime = Cycles::rdtsc();
        timeTrace("About to create");
        Arachne::ThreadId id =
            Arachne::createThreadOnCore(targetCore, task, creationTime);
        Arachne::join(id);
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull, "%lu\n", k);
    fclose(devNull);

    if (!options->hypertwinsActive) {
        Arachne::unidleCore((*coresOrderedByHT)[1]);
        Arachne::unidleCore((*coresOrderedByHT)[3]);
    }
    Arachne::shutDown();
    return 0;
}

void
sleeper() {
    Arachne::block();
}


/**
 * The benchmark requires that the CoreArbiter is started with exactly 4
 * hyperthreads across 2 cores.
 */
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

    // Add a bunch of threads to the run list that will never run again, to
    // check for interference with creation.
    for (int i = 0; i < options.numSleepers; i++)
        Arachne::createThreadOnCore(coresOrderedByHT[0], sleeper);

    Arachne::createThreadOnCore(coresOrderedByHT[0], realMain, &coresOrderedByHT, &options);
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Creation Latency (NLB)", latencies, NUM_SAMPLES,
                    "data");
#if TIME_TRACE
        TimeTrace::setOutputFileName("ArachneCreateTest_TimeTrace.log");
        TimeTrace::print();
#endif

//    return 0;
}
