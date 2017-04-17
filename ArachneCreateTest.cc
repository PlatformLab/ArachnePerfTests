#include <stdio.h>
#include <string.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "Util.h"
#include "Stats.h"

#define NUM_SAMPLES 1000000

namespace Arachne {
    extern bool disableLoadEstimation;
}

using PerfUtils::Cycles;

std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];

void task(uint64_t creationTime) {
    uint64_t latency = Cycles::rdtsc() - creationTime;
    latencies[arrayIndex++] = latency;
}


int realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES*sizeof(uint64_t));
    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        Arachne::ThreadId id = Arachne::createThreadOnCore(1, task, Cycles::rdtsc());
        Arachne::join(id);
        PerfUtils::Util::serialize();
        for (uint64_t j = 0; j < 10000U; j++) k += j;
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull,"%lu\n", k);
    fclose(devNull);

    Arachne::shutDown();
    return 0;
}

void sleeper() {
    Arachne::block();
}

int main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::Logger::setLogLevel(Arachne::WARNING);
    Arachne::init(&argc, argv);

    int threadListLength = 0;
    if (argc > 1) threadListLength = atoi(argv[1]);

    // Add a bunch of threads to the run list that will never run again, to
    // check for interference with creation.
    for (int i = 0; i < threadListLength; i++)
        Arachne::createThreadOnCore(1, sleeper);

    Arachne::createThreadOnCore(0, realMain);
    Arachne::waitForTermination();
    if (arrayIndex != NUM_SAMPLES) abort();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Creation Latency", latencies, NUM_SAMPLES, "data");
    return 0;
}
