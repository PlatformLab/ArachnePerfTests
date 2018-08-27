#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <vector>

#include "Arachne/Arachne.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Util.h"

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

#define NUM_ITERATIONS 10000
#define NUM_THREADS_IN_CYCLE 2

Arachne::ThreadId tids[NUM_THREADS_IN_CYCLE];

volatile int consumerIsReady = 0;

void
producer() {
    for (int i = 0; i < NUM_ITERATIONS * NUM_THREADS_IN_CYCLE; i++) {
        int index = i % NUM_THREADS_IN_CYCLE;
        while (!consumerIsReady)
            ;
        consumerIsReady = 0;
        TimeTrace::record("Producer about to signal %x", index);
        Arachne::signal(tids[index]);
        TimeTrace::record("Producer finished signaling %x", index);
    }
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("BlockSignal_ContextSwitch.log");
    TimeTrace::print();
}

void
consumer(int cid) {
    if (cid == 0) {
        consumerIsReady = 1;
    }
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        Arachne::block();
        TimeTrace::record("Consumer just woke up %x", cid);
        consumerIsReady = 1;
    }
    printf("Consumer %d finished\n", cid);
    fflush(stdout);
}

int
main(int argc, const char** argv) {
    // Initialize the library
    Arachne::init(&argc, argv);

    int core0 = Arachne::getCorePolicy()->getCores(0)[0];
    int core1 = Arachne::getCorePolicy()->getCores(0)[1];
    for (int i = 0; i < NUM_THREADS_IN_CYCLE; i++) {
        tids[i] = Arachne::createThreadOnCore(core1, consumer, i);
    }
    Arachne::createThreadOnCore(core0, producer);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::waitForTermination();
}
