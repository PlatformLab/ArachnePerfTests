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
    for (int i = start; i < end; i+=increment) {
        std::this_thread::yield();
    }

    uint64_t timePerYield = (Cycles::rdtsc() - startTime) / (end - start);
    printf("%lu\n", Cycles::toNanoseconds(timePerYield));
}

/**
  * This program measures the cost of a context switch on the same core for two kernel threads.
  */
int main(int argc, char** argv) {
    struct sched_param param;
    param.sched_priority = 99;
    int err = sched_setscheduler(0, SCHED_RR, &param);
    if (err) {
        printf("Error on sched_setscheduler: %d, %s\n", err, strerror(err));
        exit(-1);
    }
    core = sched_getcpu();

    if (argc < 2) {
        std::thread a(printEveryN,1,9999,2);
        std::thread b(printEveryN,2,10000,2);
        a.join();
        b.join();
    }
    else {
        int numThreads = atoi(argv[1]);
        std::thread threadRefs[numThreads];
        for (int i = 0; i < numThreads; i++)
            threadRefs[i] = std::thread(printEveryN,i,i+99998,numThreads);

        // Join the threads
        for (int i = 0; i < numThreads; i++)
            threadRefs[i].join();
    }

    fflush(stdout);
}
