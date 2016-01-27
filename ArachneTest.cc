#include <stdio.h>
#include <vector>

#include "Arachne.h"
#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    TimeTrace *tt = TimeTrace::getGlobalInstance();
    for (int i = start; i < end; i+=2) {
       if (start == 2) tt->record("T1 Yield...");
       else  tt->record("T2 Yield...");
       Arachne::yield();
       if (start == 2) tt->record("T1 Return from Yield...");
       else tt->record("T2 Return from Yield...");
    }
    if (start == 2) {
        tt->print();
    }
}

int main(){
    // Initialize the library
    Arachne::threadInit();

    // Add some work
    Arachne::createTask([](){ printEveryTwo(1,9999); });
    Arachne::createTask([](){ printEveryTwo(2,10000); });
    
    fflush(stdout);
    // Must be the last call
    Arachne::mainThreadJoinPool();
}

