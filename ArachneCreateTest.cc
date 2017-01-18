#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"

#define NUM_THREADS 10000

volatile int flag;

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void pinAvailableCore();

void ObjectTask(void *objectPointer) {
    if (objectPointer == NULL) {
        pinAvailableCore();
        return;
    }
    PerfUtils::TimeTrace::record("Inside thread");
    objectPointer = (char*)objectPointer+1;
    PerfUtils::TimeTrace::record("Incremented pointer that was passed to this thread");
    flag = 1;
}


int realMain() {
    // Cross-core creation
    pinAvailableCore();
    void *dummy = (void*) 0x1;

    Arachne::createThread(1, ObjectTask, (void*) NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        flag = 0;
        PerfUtils::TimeTrace::record("A thread is about to be born!");
        Arachne::createThread(1, ObjectTask, dummy);
        while (!flag) Arachne::yield();
    }

    TimeTrace::setOutputFileName("Create.log");
    TimeTrace::print();
    printf("Creation Test Complete\n");
    fflush(stdout);
    return 0;
}

int main(int argc, const char** argv) {
    // Initialize the library
    Arachne::init(&argc, argv);
    Arachne::createThread(0, realMain);
    // Must be the last call
    Arachne::waitForTermination();
    return 0;
}
