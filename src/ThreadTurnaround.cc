#include <stdio.h>
#include <string.h>
#include <random>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"
#include "Common.h"

#define NUM_SAMPLES 10000000
#define MEAN_DELAY 0.000002

namespace Arachne {
extern bool disableLoadEstimation;
}

// Uncomment the following line to enable TimeTraces.
// #define TIME_TRACE 1

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

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

thread_local uint64_t exitTime;
std::atomic<bool> canExit;

std::atomic<bool> exitStarted(false);

void
exitingTask() {
    // Wait until the other task is ready
    exitStarted.store(true);
    while (!canExit)
        ;
    exitTime = Cycles::rdtsc();
    timeTrace("First thread is exiting");
}

void
startingTask() {
    timeTrace("Second thread has begun");
    uint64_t startTime = Cycles::rdtsc();
    uint64_t latency = startTime - exitTime;
    latencies[arrayIndex++] = latency;
}

void
sleeper() {
    Arachne::block();
}

int
realMain(std::vector<int>* coresOrderedByHT, Options* options) {
    // Idle hypertwins if they aren't needed to be active.
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
        Arachne::idleCore((*coresOrderedByHT)[3]);
    }
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    int core1 = (*coresOrderedByHT)[2];

    // Start one thread before creating sleeper threads to reserve slot 0
    Arachne::ThreadId id = Arachne::createThreadOnCore(core1, exitingTask);

    // Add a bunch of threads to the run list that will never run again, to
    // check for interference with creation and turn around.
    for (int i = 0; i < options->numSleepers; i++)
        Arachne::createThreadOnCore(core1, sleeper);

    // Awaken the exitingTask thread and wait it to exit
    canExit.store(true);
    Arachne::join(id);

    exitStarted.store(false);
    canExit.store(false);

    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Start exiting task
        Arachne::createThreadOnCore(core1, exitingTask);
        // Make sure that exiting task has actually started running before
        // creating startingTask. This ensures that startTime cannot run before
        // exitingTask.
        while (!exitStarted)
            ;
        id = Arachne::createThreadOnCore(core1, startingTask);
        // Allow the first thread to continue
        canExit.store(true);
        // Wait for the exit of the second thread
        Arachne::join(id);
        exitStarted.store(false);
        canExit.store(false);
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

/**
 * This benchmark measures the time for one thread to exit and another thread
 * (already runnable) to begin.
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

    Arachne::createThreadOnCore(coresOrderedByHT[0], realMain, &coresOrderedByHT, &options);
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Exit To Next Run", latencies, NUM_SAMPLES, "data");


#if TIME_TRACE
        TimeTrace::setOutputFileName("ThreadTurnaround_TimeTrace.log");
        TimeTrace::keepOldEvents = true;
        TimeTrace::print();
#endif

    return 0;
}
