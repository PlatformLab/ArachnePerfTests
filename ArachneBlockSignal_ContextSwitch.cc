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


void producer(Arachne::ThreadId tid1, Arachne::ThreadId tid2) {
    printf("producerId = %p\n", Arachne::getThreadId());
    Arachne::signal(tid1); 
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (!flag);
		flag = 0;
        TimeTrace::record("Producer about to signal");
        Arachne::signal(tid1); 
        TimeTrace::record("Producer finished signaling");
        Arachne::yield();
	}
    printf("Producer finished\n");
    fflush(stdout);
    TimeTrace::setOutputFileName("BlockSignalWithSwap.log");
    TimeTrace::print();
}

void consumer(Arachne::ThreadId& tid) {
    tid = Arachne::getThreadId();
     __asm__ __volatile__("sfence" ::: "memory");
    creationFlag = 0;
    printf("&tid = %p\n", &tid);
    Arachne::block();
    printf("ConsumerId = %p\n", tid);
    fflush(stdout);
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
        creationFlag = 1;     
        Arachne::createThread(1, sleeper);
        while (creationFlag);
    }

    // Add some work
    // This is used for signalling
    // TODO: Why is the that is declared second always nil?
    // There seems to be something wrong with the assignment of references.
    // It seems that passing references does not work in the expected way with
    // my parameter passing.
    // Reference parameters are passed almost entirely incorrectly, and it
    // seems we may be overwriting arbitrary memory instead of of the things we
    // meant to pass.
    // DONE: It appears that my threading library has the same quirk with reference parameters that std::thread has, and also the same solution.
    // http://stackoverflow.com/questions/21048906/stdthread-pass-by-reference-calls-copy-constructor
    Arachne::ThreadId tid1;
    Arachne::ThreadId tid2;
    printf("&tid1 = %p, &tid2 = %p\n", &tid1, &tid2);

    creationFlag = 1;     
	Arachne::createThread(1, consumer, std::ref(tid1));
    while (creationFlag);
    creationFlag = 1;     
	Arachne::createThread(1, consumer, std::ref(tid2));
    while (creationFlag);
    printf("%p %p\n", tid1, tid2);
    sleep(1);
	Arachne::createThread(0, producer, tid1, tid2);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
