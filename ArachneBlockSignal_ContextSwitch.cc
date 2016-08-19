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
#define NUM_THREADS_IN_CYCLE 5

// Used for filling up the run queue
volatile int creationFlag;


void producer(Arachne::ThreadId *tids, int numTids, volatile int *flags) {
    printf("producerId = %p\n", Arachne::getThreadId());
    for (int i = 0; i < numTids; i++)
        Arachne::signal(tids[i]); 

    printf("flags in producer = %p\n", flags);

	for (int i = 0; i < NUM_ITERATIONS*numTids; i++) {
        int index = i % numTids;
		while (!flags[index]);
		flags[index] = 0;
        int nextIndex = index + 1;
        if (nextIndex == numTids) nextIndex = 0;
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

    Arachne::ThreadId tids[NUM_THREADS_IN_CYCLE];

    // Initialized to 0, let consumers go first.
    volatile int flags[NUM_THREADS_IN_CYCLE] = {0}; 
    printf("flags in the original stack = %p\n", flags);

    for (int i = 0; i < NUM_THREADS_IN_CYCLE; i++) {
        creationFlag = 1;     
        // It appears that my threading library has the same quirk with reference
        // parameters that std::thread has, and also the same solution: std::ref.
        // http://stackoverflow.com/questions/21048906/stdthread-pass-by-reference-calls-copy-constructor
        Arachne::createThread(1, consumer, std::ref(tids[i]), i, flags + i);
        while (creationFlag);
    }
    sleep(1);
	Arachne::createThread(0, producer, tids, NUM_THREADS_IN_CYCLE, flags);

    printf("Created Producer and consumer threads\n");
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
