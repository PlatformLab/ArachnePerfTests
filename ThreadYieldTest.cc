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
#include "Util.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;
using PerfUtils::Util::pinThreadToCore;

int core = 0;
void printEveryN(int start, int end, int increment) {
    pinThreadToCore(core);
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

/**
  * This program measures the cost of a context switch on the same core for two kernel threads.
  */
int main(){
    struct sched_param param;
    param.sched_priority = 99;
    int err = sched_setscheduler(0, SCHED_RR, &param);
    if (err) {
        printf("Error on sched_setscheduler: %d, %s\n", err, strerror(err));
        exit(-1);
    }
    core = sched_getcpu();

    std::thread(printEveryN,1,9999,2).detach();
    std::thread(printEveryN,2,10000,2).detach();

    fflush(stdout);
    while (true);
}
