#include <stdio.h>
#include <string.h>
#include <random>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#define NUM_SAMPLES 1000000
#define MEAN_DELAY 0.000002

namespace Arachne {
extern bool disableLoadEstimation;
}

using PerfUtils::Cycles;

uint64_t latencies[NUM_SAMPLES];
static uint64_t arrayIndex = 0;

std::atomic<uint64_t> exitTime;
Arachne::Semaphore exitBlocker;

static int threadListLength = 0;

void
exitingTask() {
    // Wait until the other task is ready
    exitBlocker.wait();
    exitTime = Cycles::rdtsc();
}

void
startingTask() {
    uint64_t startTime = Cycles::rdtsc();
    uint64_t latency = startTime - exitTime;
    latencies[arrayIndex++] = latency;
}

void
sleeper() {
    Arachne::block();
}

int
realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    // Start one thread before creating sleeper threads to reserve slot 0
    Arachne::ThreadId id = Arachne::createThreadOnCore(1, exitingTask);

    // Add a bunch of threads to the run list that will never run again, to
    // check for interference with creation and turn around.
    for (int i = 0; i < threadListLength; i++)
        Arachne::createThreadOnCore(1, sleeper);

    // Awaken the exitingTask thread and wait it to exit
    exitBlocker.notify();
    Arachne::join(id);

    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Start both
        Arachne::createThreadOnCore(1, exitingTask);
        id = Arachne::createThreadOnCore(1, startingTask);
        // Awaken the first thread.
        exitBlocker.notify();
        // Wait for the exit of the second thread
        Arachne::join(id);
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull, "%lu\n", k);
    fclose(devNull);

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
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
    Arachne::init(&argc, argv);

    if (argc > 1)
        threadListLength = atoi(argv[1]);

    Arachne::createThreadOnCore(0, realMain);
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Exit To Next Run", latencies, NUM_SAMPLES, "data");
    return 0;
}
