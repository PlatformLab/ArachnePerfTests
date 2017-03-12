#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"
#include "Util.h"

#define NUM_THREADS 1000000

volatile int flag;

using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void ObjectTask(void *objectPointer) {
    if (objectPointer == NULL) {
        return;
    }
    PerfUtils::TimeTrace::record("Inside thread");
    objectPointer = (char*)objectPointer+1;
    PerfUtils::TimeTrace::record("Incremented pointer that was passed to this thread");
    flag = 1;
}


int realMain() {
    // Cross-core creation
    void *dummy = (void*) 0x1;

    // Do some extra work before starting the next thread.
    uint64_t k = 0;

    Arachne::createThreadOnCore(1, ObjectTask, (void*) NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        flag = 0;
        PerfUtils::TimeTrace::record("A thread is about to be born!");
        Arachne::createThreadOnCore(1, ObjectTask, dummy);
        while (!flag) Arachne::yield();
        for (uint64_t j = 0; j < 10000U; j++) k += j;
    }

    TimeTrace::setOutputFileName("Create.log");
    TimeTrace::print();
    printf("Creation Test Complete\n");
    printf("%lu\n", k);
    fflush(stdout);
    return 0;
}

int main(int argc, const char** argv) {
    // Initialize the library
    Arachne::init(&argc, argv);
    Arachne::createThreadOnCore(0, realMain);
    // Must be the last call
    Arachne::waitForTermination();
    return 0;
}
