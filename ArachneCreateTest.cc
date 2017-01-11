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
        flag = 0;
        PerfUtils::TimeTrace::record("A thread is about to be born!");
        Arachne::createThread(ObjectTask, dummy);
        while (!flag) Arachne::yield();
    }

    TimeTrace::setOutputFileName("Create.log");
    TimeTrace::print();
    printf("Creation Test Complete\n");
    fflush(stdout);
    return 0;
}

int main() {
    // Initialize the library
    int nArgs = 2;
    const char* args[] = {"--numCores", "2"};
    Arachne::init(&nArgs, args);
    Arachne::createThread(-1, realMain);
    // Must be the last call
    Arachne::waitForTermination();
    return 0;
}
