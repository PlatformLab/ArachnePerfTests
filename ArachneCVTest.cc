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

void producer() {
    Arachne::sleep(15);
	mutex.lock();
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (flag) {
			productIsConsumed.wait(mutex);
		}
        TimeTrace::record("Producer just woke up");
		flag = 1;
        TimeTrace::record("Producer about to signal");
	    productIsReady.notify_one();
	}
	mutex.unlock();
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("CVTrace.log");
    TimeTrace::print();
}

void consumer() {
	mutex.lock();
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (!flag) {
			productIsReady.wait(mutex);
		}
        TimeTrace::record("Consumer just woke up");
		flag = 0;
        TimeTrace::record("Consumer about to signal");
		productIsConsumed.notify_one();
	}
	mutex.unlock();
    printf("Consumer finished\n");
    fflush(stdout);
}

int main(int argc, char** argv){
    // Initialize the library
    Arachne::threadInit();

    // Add some work
	Arachne::createThread(0, producer);
	Arachne::createThread(1, consumer);
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
