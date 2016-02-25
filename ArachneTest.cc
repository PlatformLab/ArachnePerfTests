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

int main(){
    // Initialize the library
    Arachne::threadInit();
    uint64_t startTime = Cycles::rdtsc();

    // Measure the thread creation overhead in the creating thread.
    for (int i = 0; i < NUM_THREADS; i++)
        Arachne::createThread(-1, printEveryTwo,1,i);
//        Arachne::createThread([=]() {printEveryTwo(1,i);});
    
    uint64_t timePerYield = (Cycles::rdtsc() - startTime) / NUM_THREADS;
    printf("Thread creation average time %lu\n", Cycles::toNanoseconds(timePerYield));

    TimeTrace::getGlobalInstance()->print();

    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
