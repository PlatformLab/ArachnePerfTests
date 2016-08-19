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

// Used for filling up the run queue
volatile int creationFlag;

Arachne::ThreadId tids[NUM_THREADS_IN_CYCLE];
// Initialized to 0, let consumers go first.
volatile int flags[NUM_THREADS_IN_CYCLE] = {0};

void producer() {
	for (int i = 0; i < NUM_ITERATIONS*NUM_THREADS_IN_CYCLE; i++) {
        int index = i % NUM_THREADS_IN_CYCLE;
        int nextIndex = index + 1;
        if (nextIndex == NUM_THREADS_IN_CYCLE) nextIndex = 0;
		while (!flags[index]);
		flags[index] = 0;
        TimeTrace::record("Producer about to signal %x", nextIndex);
        Arachne::signal(tids[nextIndex]); 
        TimeTrace::record("Producer finished signaling %x", nextIndex);
        Arachne::sleep(200);
	}
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("BlockSignalWithSwap.log");
    TimeTrace::print();
}

void consumer(int cid) {
    tids[cid] = Arachne::getThreadId();
     __asm__ __volatile__("sfence" ::: "memory");
    creationFlag = 0;
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (flags[cid]);
		flags[cid] = 1;
        Arachne::block();
        TimeTrace::record("Consumer just woke up %x", cid);
	}
    printf("Consumer %d finished\n", cid);
    fflush(stdout);
}

void sleeper() {
    creationFlag = 0;
    Arachne::block();
}

int main(int argc, char** argv){
    int threadListLength = 0;
    if (argc > 1) threadListLength = atoi(argv[1]);
    // Initialize the library
    Arachne::threadInit();

    // Add a bunch of threads to the run list that will never get to run again.
    for (int i = 0; i < threadListLength; i++) {
        creationFlag = 1;
        Arachne::createThread(1, sleeper);
        while (creationFlag);
    }

    for (int i = 0; i < NUM_THREADS_IN_CYCLE; i++) {
        creationFlag = 1;     
        Arachne::createThread(1, consumer,  i);
        while (creationFlag);
    }
	Arachne::createThread(0, producer);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
