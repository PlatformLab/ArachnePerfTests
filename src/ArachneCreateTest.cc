#include <stdio.h>
#include <string.h>
#include <random>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#define NUM_SAMPLES 10000000
#define MEAN_DELAY 0.000002

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

namespace Arachne {
extern bool disableLoadEstimation;
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
realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    // Get the physical core ID of the core we are not on.
    int core1 = Arachne::getCorePolicy()->getCores(0)[1];

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
            Arachne::createThreadOnCore(core1, task, creationTime);
        Arachne::join(id);
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull, "%lu\n", k);
    fclose(devNull);

    Arachne::shutDown();
    return 0;
}

void
sleeper() {
    Arachne::block();
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
    Arachne::init(&argc, argv);

    int threadListLength = 0;
    if (argc > 1)
        threadListLength = atoi(argv[1]);

    int core0 = Arachne::getCorePolicy()->getCores(0)[0];
    int core1 = Arachne::getCorePolicy()->getCores(0)[1];
    // Add a bunch of threads to the run list that will never run again, to
    // check for interference with creation.
    for (int i = 0; i < threadListLength; i++)
        Arachne::createThreadOnCore(core1, sleeper);

    Arachne::createThreadOnCore(core0, realMain);
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Creation Latency (NLB)", latencies, NUM_SAMPLES,
                    "data");
#if TIME_TRACE
        TimeTrace::setOutputFileName("ArachneCreateTest_TimeTrace.log");
        TimeTrace::print();
#endif
    return 0;
}
