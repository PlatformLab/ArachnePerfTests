#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne.h"
#include "Condition.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;


#define NUM_ITERATIONS 10000

volatile int flag = 0;

// Used for filling up the run queue
volatile int creationFlag;

// This is used for signalling
volatile Arachne::thread_id consumerId;

// TODO(hq6): Fix this test to test the block and signal stuff
// Need to make sure fully blocked before signalling for this test.
void producer() {
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (!flag);
		flag = 0;
        TimeTrace::record("Producer about to signal");
        Arachne::signal(consumerId); 
        TimeTrace::record("Producer finished signaling");
        Arachne::sleep(500);
	}
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("BlockSignalTrace.log");
    TimeTrace::print();
}

void consumer() {
    consumerId = Arachne::getThreadId();
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (flag);
        Arachne::set_blocking_state();
		flag = 1;
        Arachne::block();
        TimeTrace::record("Consumer just woke up");
	}
    printf("Consumer finished\n");
    fflush(stdout);
}

void sleeper() {
    creationFlag = 0;
    Arachne::set_blocking_state();
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
        creationFlag = 1;     
        while (creationFlag);
    }

    // Add some work
	Arachne::createThread(0, producer);
	Arachne::createThread(1, consumer);
    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
