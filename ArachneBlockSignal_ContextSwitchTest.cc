#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <atomic>

#include "Arachne.h"
#include "Condition.h"
#include "Cycles.h"
#include "TimeTrace.h"
#include "Util.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;


#define NUM_ITERATIONS 10000
#define NUM_THREADS_IN_CYCLE 1


Arachne::ThreadId tids[NUM_THREADS_IN_CYCLE];

// Initialized to 1, since we create consumers first.
volatile int consumerIsReady = 1;

void producer() {
    while (!tids[NUM_THREADS_IN_CYCLE - 1]);
	for (int i = 0; i < NUM_ITERATIONS*NUM_THREADS_IN_CYCLE; i++) {
        int index = i % NUM_THREADS_IN_CYCLE;
		while (!consumerIsReady);
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

void consumer(int cid) {
    tids[cid] = Arachne::getThreadId();
	for (int i = 0; i < NUM_ITERATIONS; i++) {
        Arachne::block();
        TimeTrace::record("Consumer just woke up %x", cid);
		consumerIsReady = 1;
	}
    printf("Consumer %d finished\n", cid);
    fflush(stdout);
}

void sleeper() {
    Arachne::block();
}

int main(int argc, char** argv){
    int threadListLength = 0;
    if (argc > 1) threadListLength = atoi(argv[1]);
    // Initialize the library
    Arachne::threadInit();

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < threadListLength; i++) {
        Arachne::createThread(1, sleeper);
    }

    for (int i = 0; i < NUM_THREADS_IN_CYCLE; i++) {
        Arachne::createThread(1, consumer,  i);
    }
	Arachne::createThread(0, producer);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
