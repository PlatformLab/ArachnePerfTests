#include <stdio.h>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    uint64_t startTime = Cycles::rdtsc();
    for (int i = start; i < end; i+=2) {
//        TimeTrace::getGlobalInstance()->record("Thread %d is yielding", start);
        std::this_thread::yield();
    }

    if (start == 2) {
        uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
        printf("%lu\n", Cycles::toNanoseconds(timePerYield));
//        TimeTrace::getGlobalInstance()->print();
    }
}

int main(){
    struct sched_param param;
    param.sched_priority = 99;
    int err = sched_setscheduler(0, SCHED_RR, &param);
    if (err) {
        printf("Error on sched_setscheduler: %d, %s\n", err, strerror(err));
        exit(-1);
    }

    std::thread(printEveryTwo,1,9999).detach();
    std::thread(printEveryTwo,2,10000).detach();

    fflush(stdout);
    while (true);
}
