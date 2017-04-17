#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "Util.h"
#include "Stats.h"

#define NUM_SAMPLES 1000000

using PerfUtils::Cycles;

std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];
std::atomic<uint64_t> beforeYield;

/**
  * This benchmark computes a median time for a yield in one thread to return
  * in another thread in the same core.
  * Note that this benchmark has no direct analog in golang or std::thread,
  * since we cannot finely control core placement in these systems.
  *
  * Alternatively, we could measure the time it takes to return to the original
  * thread, but this seems less intuitive and still would not match with
  * std::thread or Golang.
  */
void yielder() {
    for (int i = 0; i < NUM_SAMPLES; i++) {
        latencies[arrayIndex++] = Cycles::rdtsc() - beforeYield;
        Arachne::yield();
    }
}

int realMain() {
    // Page in our data store
    memset(latencies, 0, NUM_SAMPLES*sizeof(uint64_t));

    Arachne::ThreadId id = Arachne::createThreadOnCore(0, yielder);
    for (int i = 0; i < NUM_SAMPLES; i++) {
        beforeYield = Cycles::rdtsc();
        Arachne::yield();
    }
    Arachne::join(id);
    Arachne::shutDown();
    return 0;
}

int main(int argc, const char** argv){
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::init(&argc, argv);
    Arachne::createThreadOnCore(0, realMain);
    // Must be the last call
    Arachne::waitForTermination();
    if (arrayIndex != NUM_SAMPLES) abort();
    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Thread Yield Latency", latencies, NUM_SAMPLES, "data");
}
