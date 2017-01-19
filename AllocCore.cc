#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include "Util.h"

#define gettid() syscall(SYS_gettid)
//#define VERBOSE

void pinAvailableCore() {
    static std::vector<int> cores = PerfUtils::Util::getAllUseableCores();
    static std::mutex coreAllocMutex;
    coreAllocMutex.lock();
    int coreId = cores.back();
    cores.pop_back();
    coreAllocMutex.unlock();
    #ifdef VERBOSE
    printf("Pinning %lu to Core %d.\n", gettid(), coreId);
    #endif
    PerfUtils::Util::pinThreadToCore(coreId);
}
