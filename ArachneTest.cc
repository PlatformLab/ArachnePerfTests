#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    uint64_t startTime = Cycles::rdtsc();
    for (int i = start; i < end; i+=2) {
//        printf("Yielding after %d\n", i);
        Arachne::yield();
    }
    if (start == 1) {
        uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
        printf("%lu\n", Cycles::toNanoseconds(timePerYield));
    }
}

int main(){
    // Initialize the library
    Arachne::threadInit();

    // Add some work
    Arachne::createThread(-1, printEveryTwo,1,9999);
    Arachne::createThread(-1, printEveryTwo,2,10000);
    
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
