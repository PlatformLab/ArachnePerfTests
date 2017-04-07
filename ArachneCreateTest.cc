#include <stdio.h>
#include <string.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "Util.h"
#include "Stats.h"

#define NUM_SAMPLES 1000000

volatile int flag;

using PerfUtils::Cycles;

std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];

void task(uint64_t creationTime) {
    uint64_t latency = Cycles::rdtsc() - creationTime;
    latencies[arrayIndex++] = latency;
    flag = 1;
}


int realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES*sizeof(uint64_t));
    // Do some extra work before starting the next thread.
    uint64_t k = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        flag = 0;
        Arachne::createThreadOnCore(1, task, Cycles::rdtsc());
        while (!flag) Arachne::yield();
        for (uint64_t j = 0; j < 10000U; j++) k += j;
    }
    FILE* devNull = fopen("/dev/null", "w");
    fprintf(devNull,"%lu\n", k);
    fclose(devNull);
    while (!flag) Arachne::yield();
    Arachne::shutDown();
    return 0;
}

int main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::init(&argc, argv);
    Arachne::createThreadOnCore(0, realMain);
    Arachne::waitForTermination();
    if (arrayIndex != NUM_SAMPLES) abort();
    printStatistics("Thread Creation Latency", latencies, NUM_SAMPLES, "data");
    return 0;
}
