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

Arachne::condition_variable productIsReady;
Arachne::condition_variable productIsConsumed;
Arachne::SpinLock mutex;

// Used for filling up the run queue
char separator[Arachne::CACHE_LINE_SIZE];

void producer() {
    printf("producerId = %p\n", Arachne::getThreadId());
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (flag);
		flag = 1;
        mutex.lock();
        TimeTrace::record("Producer about to signal");
	    productIsReady.notify_one();
        TimeTrace::record("Producer finished signaling");
        mutex.unlock();
        TimeTrace::record("Producer manually unlocked");
        Arachne::sleep(500);
	}
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("CV.log");
    TimeTrace::print();
}

void consumer() {
    printf("ConsumerId = %p\n", Arachne::getThreadId());
	mutex.lock();
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (!flag) {
			productIsReady.wait(mutex);
		}
        TimeTrace::record("Consumer just woke up");
		flag = 0;
	}
	mutex.unlock();
    printf("Consumer finished\n");
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

    // Add some work
	Arachne::createThread(0, producer);
	Arachne::createThread(1, consumer);
    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
