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
    TimeTrace::record("Inside thread");
//    printf("Our scheduler: %d, SCHED_RR: %d\n", sched_getscheduler(0), SCHED_RR);
//    fflush(stdout);
}

int main(){
    struct sched_param param;
    param.sched_priority = 99;
    int err = sched_setscheduler(0, SCHED_RR, &param);
    if (err) {
        printf("Error on sched_setscheduler: %d, %s\n", err, strerror(err));
        exit(-1);
    }

    // Measure the thread creation overhead in the creating thread.
    for (int i = 0; i < 10000; i++) {
        TimeTrace::record("Before creation");
        std::thread(printEveryTwo,1,i).detach();
//        pthread_setschedparam(t.native_handle(), SCHED_RR, &param);
//        t.detach();
        usleep(100);
//        TimeTrace::record("Finished usleep 20!");
    }

    TimeTrace::print();
    fflush(stdout);
}
