#include <stdio.h>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include "Cycles.h"


using PerfUtils::Cycles;

void printEveryTwo(int start, int end) {
    usleep(100);
    uint64_t startTime = Cycles::rdtsc();
    for (int i = start; i < end; i+=2) {
        printf("Yielding after %d\n", i);
        std::this_thread::yield();
    }

    if (start == 2) {
        uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
        printf("%lu\n", Cycles::toNanoseconds(timePerYield));
    }
}

int main(){
    struct sched_param param;
    param.sched_priority = 99;
    sched_setscheduler(0, SCHED_RR, &param);

    std::thread(printEveryTwo,1,9999).detach();
    std::thread(printEveryTwo,2,10000).detach();

    fflush(stdout);
    while (true);
}
