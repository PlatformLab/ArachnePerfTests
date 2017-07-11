#include <stdio.h>
#include <string.h>
#include <vector>
#include <random>

#include "Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Util.h"
#include "Stats.h"

#define NUM_SAMPLES 1000000
#define MEAN_DELAY 0.000002

namespace Arachne {
    extern bool disableLoadEstimation;
}

using PerfUtils::Cycles;

uint64_t latencies[NUM_SAMPLES];
static uint64_t arrayIndex = 0;

void task(uint64_t creationTime) {
    uint64_t startTime = Cycles::rdtsc();
    PerfUtils::Util::serialize();
    uint64_t latency = startTime - creationTime;
    latencies[arrayIndex++] = latency;
}


int realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES*sizeof(uint64_t));

    // Set up random smoothing.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> uniformIG(0, MEAN_DELAY * 2);

    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Wait a random interval before next creation
        uint64_t signalTime = Cycles::rdtsc() + Cycles::fromSeconds(uniformIG(gen));
        while (Cycles::rdtsc() < signalTime);
        PerfUtils::Util::serialize();
        uint64_t creationTime = Cycles::rdtsc();
        Arachne::ThreadId id = Arachne::createThreadOnCore(1, task, creationTime);
        Arachne::join(id);
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
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Creation Latency (NLB)", latencies, NUM_SAMPLES, "data");
    return 0;
}
