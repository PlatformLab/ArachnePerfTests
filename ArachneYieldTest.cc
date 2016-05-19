#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryN(int start, int end, int increment) {
    uint64_t startTime = Cycles::rdtsc();
    for (int i = start; i < end; i+=increment) {
        Arachne::yield();
    }

    uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
    printf("%lu\n", Cycles::toNanoseconds(timePerYield));
    fflush(stdout);
}

int main(int argc, char** argv){
    // Initialize the library
    Arachne::threadInit();

    // Add some work
    if (argc < 2) {
        Arachne::createThread(0, printEveryN, 1, 9999, 2);
        Arachne::createThread(0, printEveryN, 2, 10000, 2);
    }
    else {
        int numThreads = atoi(argv[1]);
        for (int i = 0; i < numThreads; i++)
            Arachne::createThread(i % 3, printEveryN,i,i+99998,numThreads);
    }
    fflush(stdout);
    usleep(1000000);
    // Must be the last call
//    Arachne::mainThreadJoinPool();
}
