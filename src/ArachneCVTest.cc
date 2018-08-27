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
std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];

std::atomic<uint64_t> beforeNotify;

Arachne::ConditionVariable productIsReady;
Arachne::SpinLock mutex;
Arachne::ThreadId producerId;

std::atomic<bool> producerHasStarted;
std::atomic<bool> productIsConsumed;

void
producer(std::vector<int>* coresOrderedByHT, Options* options) {
    if (!options->hypertwinsActive) {
        Arachne::idleCore((*coresOrderedByHT)[1]);
        Arachne::idleCore((*coresOrderedByHT)[3]);
    }

    producerHasStarted = true;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        while (!productIsConsumed);
        productIsConsumed = false;
        mutex.lock();
        beforeNotify = Cycles::rdtsc();
        productIsReady.notifyOne();
        mutex.unlock();
    }

    if (!options->hypertwinsActive) {
        Arachne::unidleCore((*coresOrderedByHT)[1]);
        Arachne::unidleCore((*coresOrderedByHT)[3]);
    }
}

void
consumer() {
    mutex.lock();
    for (int i = 0; i < NUM_SAMPLES; i++) {
        productIsConsumed = true;
        productIsReady.wait(mutex);
        uint64_t wakeupTime = Cycles::rdtsc();
        PerfUtils::Util::serialize();
        latencies[i] = wakeupTime - beforeNotify;
    }
    mutex.unlock();
    Arachne::join(producerId);
    Arachne::shutDown();
}

void
sleeper() {
    Arachne::block();
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::minNumCores = 4;
    Arachne::maxNumCores = 4;
    Arachne::disableLoadEstimation = true;
    Arachne::init(&argc, argv);

    std::vector<int> coresOrderedByHT = getCoresOrderedByHT();
    Options options = parseOptions(argc, const_cast<char**>(argv));
    printf("Active Hypertwin: %d\nNumber of Sleeping Threads: %d\n",
            options.hypertwinsActive, options.numSleepers);

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < options.numSleepers; i++) {
        Arachne::createThreadOnCore(coresOrderedByHT[1], sleeper);
    }

    // Add some work
    producerId = Arachne::createThreadOnCore(coresOrderedByHT[0], producer, &coresOrderedByHT, &options);
    asm volatile("" : : : "memory");
    // Wait for producer to start running before starting consumer, to mitigate
    // a race where the consumer signals before initialization of the kernel
    // thread that runs the producer.
    while (!producerHasStarted)
        ;
    productIsConsumed = false;
    Arachne::createThreadOnCore(coresOrderedByHT[2], consumer);
    // Must be the last call
    Arachne::waitForTermination();

    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Condition Variable Wakeup", latencies, NUM_SAMPLES,
                    "data");
    return 0;
}
