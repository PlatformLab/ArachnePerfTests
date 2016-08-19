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

// Used for filling up the run queue
volatile int creationFlag;


void producer(Arachne::ThreadId *tids, int numTids, volatile int *flags) {
    printf("producerId = %p\n", Arachne::getThreadId());
    Arachne::signal(tids[0]); 
    Arachne::signal(tids[1]); 

    printf("flags in producer = %p\n", flags);

	for (int i = 0; i < NUM_ITERATIONS*numTids; i++) {
        int index = i % numTids;
		while (!flags[index]);
		flags[index] = 0;
        int nextIndex = (i+1) % numTids;
//        printf("Producer about to signal %d\n", nextIndex);
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

void consumer(Arachne::ThreadId& tid, int cid, volatile int* flag) {
    tid = Arachne::getThreadId();
     __asm__ __volatile__("sfence" ::: "memory");
    creationFlag = 0;
    printf("&tid = %p\n", &tid);
    printf("flag in consumer = %p\n", flag);
    Arachne::block();
    Arachne::yield();
    printf("cid = %d, ConsumerId = %p\n", cid, tid);
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		while (*flag);
		*flag = 1;
        Arachne::block();
        TimeTrace::record("Consumer just woke up %x", cid);
//        printf("ConsumerId = %d, i = %d\n", cid, i);
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
    Arachne::ThreadId tids[2];
    volatile int flags[2] = {0}; // Initialized to 0, let consumers go first.
    printf("flags in the original stack = %p\n", flags);

    creationFlag = 1;     
	Arachne::createThread(1, consumer, std::ref(tids[0]), 0, flags);
    while (creationFlag);
    creationFlag = 1;     
	Arachne::createThread(1, consumer, std::ref(tids[1]), 1, flags + 1);
    while (creationFlag);
    sleep(1);
	Arachne::createThread(0, producer, tids, 2, flags);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
