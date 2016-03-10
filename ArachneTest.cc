#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"

#define NUM_THREADS 10000

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    PerfUtils::TimeTrace::getGlobalInstance()->record("Inside thread");
}

void sleeper(int id, uint64_t ns) {
    for (int i = 0; i < 1000; i++) {
        if (id == 1) PerfUtils::TimeTrace::getGlobalInstance()->record("Going to sleep 1");
        else PerfUtils::TimeTrace::getGlobalInstance()->record("Going to sleep 2");
        Arachne::sleep(ns);
    }
}

int realMain() {
    Arachne::createThread(1, sleeper, 1, 1000);
    // Measure the thread creation overhead in the creating thread.
    Arachne::sleep(10000000);
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
