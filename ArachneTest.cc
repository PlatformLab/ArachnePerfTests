#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"

#define NUM_THREADS 1000

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
}

int realMain() {
    uint64_t startTime = Cycles::rdtsc();

    // Measure the thread creation overhead in the creating thread.
    for (int i = 0; i < NUM_THREADS - 1; i++) {
        int id = Arachne::createThread(-1, printEveryTwo,1,i);
        if (id) {
            fprintf(stderr, "Thread Creation Error on i = %d\n", i);
            exit(-1);
        }
    }

    uint64_t timePerYield = (Cycles::rdtsc() - startTime) /(NUM_THREADS - 1);
    printf("Thread creation average time %lu\n", Cycles::toNanoseconds(timePerYield));
    TimeTrace::getGlobalInstance()->print();
    fflush(stdout);
    return 0;
}

int main(){
    // Initialize the library
    Arachne::threadInit();
    Arachne::createThread(-1, realMain);
    // Must be the last call
    Arachne::mainThreadJoinPool();
    return 0;
}
