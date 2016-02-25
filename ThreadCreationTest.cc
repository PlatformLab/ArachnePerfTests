#include <stdio.h>
#include <vector>
#include <thread>

#include "Cycles.h"


using PerfUtils::Cycles;

void printEveryTwo(int start, int end) {
}

int main(){
    uint64_t startTime = Cycles::rdtsc();

    // Measure the thread creation overhead in the creating thread.
    for (int i = 0; i < 1000; i++)
        std::thread(printEveryTwo,1,i).detach();
    
    uint64_t timePerYield = (Cycles::rdtsc() - startTime) / 1000;
    printf("Thread creation average time %lu\n", Cycles::toNanoseconds(timePerYield));

    fflush(stdout);
}
