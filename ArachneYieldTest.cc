#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

Arachne::ThreadId mainThreadId;

void printEveryN(int start, int end, int increment) {
    Arachne::signal(mainThreadId);
    uint64_t startTime = Cycles::rdtsc();
    for (int i = start; i < end; i+=increment) {
        Arachne::yield();
    }

    uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
    printf("%lu\n", Cycles::toNanoseconds(timePerYield));
    fflush(stdout);
    printf("%d Start Finished!\n", start);
}

int realMain(int argc, char** argv) {
    mainThreadId = Arachne::getThreadId();
    // Add some work
    if (argc < 2) {
        Arachne::createThread(0, printEveryN, 1, 99999, 2);
        Arachne::block();
        Arachne::createThread(0, printEveryN, 2, 100000, 2);
    }
    else {
        int numThreads = atoi(argv[1]);
        for (int i = 0; i < numThreads; i++)
            Arachne::createThread(i % 3, printEveryN,i,i+99998,numThreads);
    }
    return 0;
}

int main(int argc, char** argv){
    // Initialize the library
    Arachne::threadInit();
    Arachne::createThread(3, realMain, argc, argv);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}
