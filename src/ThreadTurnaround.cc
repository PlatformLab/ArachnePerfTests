#include <stdio.h>
#include <string.h>
#include <vector>
#include <random>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Util.h"
#include "PerfUtils/Stats.h"

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

void exitingTask() {
    // Wait until the other task is ready 
    exitBlocker.wait();
    exitTime = Cycles::rdtsc();
}

void startingTask() {
    uint64_t startTime = Cycles::rdtsc();
    uint64_t latency = startTime - exitTime;
    latencies[arrayIndex++] = latency;
}


int realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES*sizeof(uint64_t));

    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Start both
        Arachne::createThreadOnCore(1, exitingTask);
        Arachne::ThreadId id = Arachne::createThreadOnCore(1, startingTask);
        // Awaken the first thread.
        exitBlocker.notify();
        // Wait for the exit of the second thread
        Arachne::join(id);
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull,"%lu\n", k);
    fclose(devNull);

    Arachne::shutDown();
    return 0;
}

/**
 * This benchmark measures the time for one thread to exit and another thread
 * (already runnable) to begin.
 */
int main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
    Arachne::init(&argc, argv);

    Arachne::createThreadOnCore(0, realMain);
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Exit To Next Run", latencies, NUM_SAMPLES, "data");
    return 0;
}
