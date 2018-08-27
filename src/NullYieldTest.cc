#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#include "Common.h"

#define NUM_SAMPLES 10000000

using PerfUtils::Cycles;

uint64_t latencies[NUM_SAMPLES];

/**
 * This benchmark computes a median time for a NULL yield, where there are no
 * competing threads on a core.
 */
int
realMain(std::vector<int>* coresOrderedByHT, Options* options) {
    // Idle hypertwins if they aren't needed to be active.
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
    }
    PerfUtils::Util::serialize();

    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES * sizeof(uint64_t));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t beforeYield = Cycles::rdtsc();
        Arachne::yield();
        latencies[i] = Cycles::rdtsc() - beforeYield;
    }

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
    printStatistics("Null Yield Latency", latencies, NUM_SAMPLES, "data");
}
