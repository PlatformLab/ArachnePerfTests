#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

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
producer() {
    producerHasStarted = true;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        while (!productIsConsumed);
        productIsConsumed = false;
        mutex.lock();
        beforeNotify = Cycles::rdtsc();
        productIsReady.notifyOne();
        mutex.unlock();
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
    int threadListLength = 0;
    if (argc > 1)
        threadListLength = atoi(argv[1]);
    // Initialize the library
    Arachne::minNumCores = 2;
    Arachne::maxNumCores = 2;
    Arachne::disableLoadEstimation = true;
    Arachne::init(&argc, argv);

    int core0 = Arachne::getCorePolicy()->getCores(0)[0];
    int core1 = Arachne::getCorePolicy()->getCores(0)[1];
    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < threadListLength; i++) {
        Arachne::createThreadOnCore(core1, sleeper);
    }

    // Add some work
    producerId = Arachne::createThreadOnCore(core0, producer);
    asm volatile("" : : : "memory");
    // Wait for producer to start running before starting consumer, to mitigate
    // a race where the consumer signals before initialization of the kernel
    // thread that runs the producer.
    while (!producerHasStarted)
        ;
    productIsConsumed = false;
    if (argc > 1)
        Arachne::createThreadOnCore(core0, consumer);
    else
        Arachne::createThreadOnCore(core1, consumer);
    // Must be the last call
    Arachne::waitForTermination();

    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Condition Variable Wakeup", latencies, NUM_SAMPLES,
                    "data");
    return 0;
}
