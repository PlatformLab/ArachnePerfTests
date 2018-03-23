#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/Util.h"

#define NUM_SAMPLES 1000000

using PerfUtils::Cycles;
std::atomic<uint64_t> arrayIndex;
uint64_t latencies[NUM_SAMPLES];

std::atomic<uint64_t> beforeNotify;

Arachne::ConditionVariable productIsReady;
Arachne::SpinLock mutex;
Arachne::ThreadId producerId;

std::atomic<bool> producerHasStarted;

void
producer() {
    producerHasStarted = true;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        Arachne::block();
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
        Arachne::signal(producerId);
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
    Arachne::init(&argc, argv);

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < threadListLength; i++) {
        Arachne::createThreadOnCore(1, sleeper);
    }

    // Add some work
    producerId = Arachne::createThreadOnCore(0, producer);
    asm volatile("" : : : "memory");
    // Wait for producer to start running before starting consumer, to mitigate
    // a race where the consumer signals before initialization of the kernel
    // thread that runs the producer.
    while (!producerHasStarted)
        ;
    if (argc > 1)
        Arachne::createThreadOnCore(0, consumer);
    else
        Arachne::createThreadOnCore(1, consumer);
    // Must be the last call
    Arachne::waitForTermination();

    for (int i = 0; i < NUM_SAMPLES; i++)
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    printStatistics("Condition Variable Wakeup", latencies, NUM_SAMPLES,
                    "data");
    return 0;
}
