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

volatile int flag;

// Used for filling up the run queue
volatile int creationFlag;

// This is used for signalling
volatile Arachne::ThreadId consumerId;

void producer() {

    printf("producerId = %p\n", Arachne::getThreadId());
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (!flag);
		flag = 0;
        TimeTrace::record("Producer about to signal");
        Arachne::signal(consumerId); 
        TimeTrace::record("Producer finished signaling");
        Arachne::yield();
	}
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("BlockSignalTrace.log");
    TimeTrace::print();
}

void consumer() {
    consumerId = Arachne::getThreadId();
    printf("ConsumerId = %p\n", consumerId);
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (flag);
		flag = 1;
        Arachne::block();
        TimeTrace::record("Consumer just woke up");
	}
    printf("Consumer finished\n");
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
        Arachne::createThread(1, sleeper);
        creationFlag = 1;     
        while (creationFlag);
    }

    // Add some work
	Arachne::createThread(1, consumer);
    sleep(1);
	Arachne::createThread(0, producer);
    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
