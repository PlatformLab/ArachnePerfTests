#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;

void printEveryTwo(int start, int end) {
    for (int i = start; i < end; i+=2) {
       if (start == 2) PerfUtils::TimeTrace::getGlobalInstance()->record("user thread is yielding...");
       Arachne::yield();
       if (start == 2) PerfUtils::TimeTrace::getGlobalInstance()->record("returned from yield...");
    }
    if (start == 2) {
        PerfUtils::TimeTrace::getGlobalInstance()->print();
    }
}

int main(){
    // Initialize the library
    Arachne::threadInit();

    // Add some work
    Arachne::createTask([](){ printEveryTwo(1,999); });
    Arachne::createTask([](){ printEveryTwo(2,1000); });
    
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}

