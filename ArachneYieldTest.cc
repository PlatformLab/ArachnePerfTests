#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

// Used for filling up the run queue


void printEveryN(int start, int end, int increment) {
    printf("start = %d\n", start);
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
    // Add some work
    Arachne::createThread(0, printEveryN, 0, 99999, 2);
    Arachne::createThread(0, printEveryN, 1, 100000, 2);
    return 0;
}

int main(int argc, char** argv){
    // Initialize the library
    int nArgs = 2;
    const char* args[] = {"--numCores", "2"};
    Arachne::init(&nArgs, args);
    Arachne::createThread(3, realMain, argc, argv);
    // Must be the last call
    Arachne::waitForTermination();
}
