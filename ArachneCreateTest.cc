#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"

#define NUM_THREADS 10000

volatile int flag;

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void ObjectTask(void *objectPointer) {
    PerfUtils::TimeTrace::record("Inside thread");
    objectPointer = (char*)objectPointer+1;
    PerfUtils::TimeTrace::record("Incremented pointer that was passed to this thread");
    flag = 1;
}


int realMain() {
    // Cross-core creation
    void *dummy = (void*) 0x0;
    for (int i = 0; i < NUM_THREADS; i++) {
//        PerfUtils::TimeTrace::record("DummyTrace!");
//        PerfUtils::TimeTrace::record("DummyTrace2!");
        PerfUtils::TimeTrace::record("A thread is about to be born!");
        flag = 0;
        Arachne::createThread(1, ObjectTask, dummy);
        while (!flag);
    }

    TimeTrace::setOutputFileName("TimeTrace.log");
    TimeTrace::print();
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
