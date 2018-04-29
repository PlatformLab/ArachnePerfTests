#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#define NUM_SAMPLES 1000000

using PerfUtils::Cycles;

uint64_t latencies[NUM_SAMPLES];

/**
 * This benchmark computes a median time for a NULL yield, where there are no
 * competing threads on a core.
 */
int
realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t beforeYield = Cycles::rdtsc();
        Arachne::yield();
        latencies[i] = Cycles::rdtsc() - beforeYield;
    }
    Arachne::shutDown();
    return 0;
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::init(&argc, argv);
    Arachne::createThread(realMain);
    // Must be the last call
    Arachne::waitForTermination();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Null Yield Latency", latencies, NUM_SAMPLES, "data");
}
