#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <atomic>
#include <thread>

#include "CoreArbiter/CoreArbiterClient.h"
#include "CoreArbiter/Logger.h"
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Stats.h"
#include "PerfUtils/TimeTrace.h"
#include "PerfUtils/Util.h"

using CoreArbiter::CoreArbiterClient;
using PerfUtils::Cycles;
using PerfUtils::TimeTrace;
using namespace CoreArbiter;

/**
 * This benchmark measures the minimum amount of time it takes for the
 * CoreArbiter to reclaim a core from one application and give it to another.
 * To create the expected contention, the Core Arbiter MUST be started with
 * only two cores.
 */

// Uncomment the following line to dump the TimeTraces from the Core Arbiter
// client library. Note that this is only useful if you compile the CoreArbiter
// with TimeTraces enabled. Further note that the duplicate calls to
// TimeTrace::print() below are intentional; they occur in two different
// processes.
// #define TIME_TRACE 1

#define NUM_TRIALS 1000000

std::atomic<uint64_t> startCycles(0);
uint64_t arrayIndex = 0;
uint64_t latencies[NUM_TRIALS];

void
highPriorityRequest(CoreArbiterClient* client,
                    volatile bool* lowPriorityRunning) {
    client->blockUntilCoreAvailable();

    // Wait until the other high priority thread is running
    while (client->getNumOwnedCores() < 2)
        ;

    for (int i = 0; i < NUM_TRIALS; i++) {
        client->setRequestedCores({1, 0, 0, 0, 0, 0, 0, 0});
        while (client->getNumBlockedThreadsFromServer() == 0)
            ;
        while (!(*lowPriorityRunning))
            ;

        startCycles = Cycles::rdtsc();
        client->setRequestedCores({2, 0, 0, 0, 0, 0, 0, 0});
        while (client->getNumBlockedThreads() == 1)
            ;
    }

    client->unregisterThread();
}

void
highPriorityBlock(CoreArbiterClient* client) {
    client->blockUntilCoreAvailable();

    // Wait until the other high priority thread is running
    while (client->getNumOwnedCores() < 2)
        ;

    for (int i = 0; i < NUM_TRIALS; i++) {
        while (!client->mustReleaseCore())
            ;
        client->blockUntilCoreAvailable();
        uint64_t endCycles = Cycles::rdtsc();
        latencies[arrayIndex++] = endCycles - startCycles;
    }

    client->unregisterThread();
}

void
lowPriorityExec(CoreArbiterClient* client, volatile bool* lowPriorityRunning) {
    std::vector<uint32_t> lowPriorityRequest = {0, 0, 0, 0, 0, 0, 0, 1};
    client->setRequestedCores(lowPriorityRequest);
    client->blockUntilCoreAvailable();

    // Wait for other process to join
    while (client->getNumProcessesOnServer() == 1)
        ;

    for (int i = 0; i < NUM_TRIALS; i++) {
        while (!client->mustReleaseCore())
            ;
        *lowPriorityRunning = false;
        client->blockUntilCoreAvailable();
        *lowPriorityRunning = true;
    }

    client->unregisterThread();
}

int
main(int argc, const char** argv) {
    Logger::setLogLevel(ERROR);

    int sharedMemFd =
        open("/tmp/benchmark_sharedmem", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
    if (sharedMemFd < 0) {
        fprintf(stderr, "Error opening shared memory page: %s\n",
                strerror(errno));
        return -1;
    }

    if (ftruncate(sharedMemFd, sizeof(bool)) == -1) {
        fprintf(stderr, "Error truncating sharedMemFd: %s\n", strerror(errno));
        return -1;
    }

    volatile bool* lowPriorityRunning =
        (bool*)mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED,
                    sharedMemFd, 0);
    if (lowPriorityRunning == MAP_FAILED) {
        fprintf(stderr, "Error on global stats mmap: %s\n", strerror(errno));
        return -1;
    }
    *lowPriorityRunning = false;

    pid_t pid = fork();
    if (pid == 0) {
        CoreArbiterClient* client = CoreArbiterClient::getInstance();

        // Wait for the low priority thread to be put on a core
        while (client->getNumUnoccupiedCores() == 2)
            ;
        client->setRequestedCores({2, 0, 0, 0, 0, 0, 0, 0});

        // The order matters here beceause the thread that will be preempted
        // must contact the arbiter later later.
        std::thread highPriorityThread2(highPriorityRequest, std::ref(client),
                                        lowPriorityRunning);
        while (client->getNumOwnedCores() != 1)
            ;
        std::thread highPriorityThread1(highPriorityBlock, std::ref(client));
        highPriorityThread1.join();
        highPriorityThread2.join();

        for (int i = 0; i < NUM_TRIALS; i++) {
            latencies[i] = Cycles::toNanoseconds(latencies[i]);
        }
        printStatistics("core_request_cooperative_latencies", latencies,
                        NUM_TRIALS, argc > 1 ? "data" : NULL);
#if TIME_TRACE
        TimeTrace::setOutputFileName("CoreRequest_Contended_HighPriority.log");
        TimeTrace::print();
#endif
    } else {
        CoreArbiterClient* client = CoreArbiterClient::getInstance();
        lowPriorityExec(client, lowPriorityRunning);

        wait(NULL);
#if TIME_TRACE
        TimeTrace::setOutputFileName("CoreRequest_Contended_LowPriority.log");
        TimeTrace::print();
#endif
    }
}
