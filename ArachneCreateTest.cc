#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"

#define NUM_THREADS 10000

volatile int flag;

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    PerfUtils::TimeTrace::getGlobalInstance()->record("Inside thread");
    flag = 1;
}


int realMain() {
    // Cross-core creation
    for (int i = 0; i < NUM_THREADS; i++) {
        PerfUtils::TimeTrace::getGlobalInstance()->record("A thread is about to be born!");
        flag = 0;
        Arachne::createThread(1, printEveryTwo, 1, 1000);
        while (!flag);
        // Delay a full microsecond before creating the next thread
        Arachne::sleep(1000);
    }

    TimeTrace::getGlobalInstance()->print();
    fflush(stdout);
    return 0;
}

int main() {
    // Initialize the library
    Arachne::threadInit();
    Arachne::createThread(-1, realMain);
    // Must be the last call
    Arachne::mainThreadJoinPool();
    return 0;
}
