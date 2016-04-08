#include <stdio.h>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include "Cycles.h"
#include "TimeTrace.h"


using PerfUtils::Cycles;
using PerfUtils::TimeTrace;

void printEveryTwo(int start, int end) {
    TimeTrace::getGlobalInstance()->record("Inside thread");
//    printf("Our scheduler: %d, SCHED_RR: %d\n", sched_getscheduler(0), SCHED_RR);
//    fflush(stdout);
}

int main(){
    struct sched_param param;
    param.sched_priority = 99;
    sched_setscheduler(0, SCHED_RR, &param);

    // Measure the thread creation overhead in the creating thread.
    for (int i = 0; i < 10000; i++) {
        TimeTrace::getGlobalInstance()->record("Before creation");
        std::thread(printEveryTwo,1,i).detach();
//        pthread_setschedparam(t.native_handle(), SCHED_RR, &param);
//        t.detach();
        usleep(100);
//        TimeTrace::getGlobalInstance()->record("Finished usleep 20!");
    }

    TimeTrace::getGlobalInstance()->print();
    fflush(stdout);
}
