#include <sys/wait.h>
#include <stdio.h>
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
 * CoreArbiter to reclaim a core from an uncooperative application and give it
 * to another. To create the expected contention, the Core Arbiter MUST be
 * started with only two cores.
 */

#define NUM_TRIALS 10000

std::atomic<uint64_t> startCycles(0);
uint64_t arrayIndex = 0;
uint64_t latencies[NUM_TRIALS];

void
highPriorityRequest(CoreArbiterClient* client) {
    client->blockUntilCoreAvailable();

    // Wait until the other high priority thread is running
    while (client->getNumOwnedCores() < 2)
        ;

    for (int i = 0; i < NUM_TRIALS; i++) {
        client->setRequestedCores({1, 0, 0, 0, 0, 0, 0, 0});
        while (client->getNumBlockedThreadsFromServer() == 0)
            ;
        while (client->getNumUnoccupiedCores() > 0)
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
lowPriorityExec(CoreArbiterClient* client) {
    std::vector<uint32_t> lowPriorityRequest = {0, 0, 0, 0, 0, 0, 0, 1};
    client->setRequestedCores(lowPriorityRequest);
    client->blockUntilCoreAvailable();

    while (client->getNumProcessesOnServer() == 1)
        ;
    while (client->getNumProcessesOnServer() == 2)
        ;

    client->unregisterThread();
}

int
main(int argc, const char** argv) {
    Logger::setLogLevel(ERROR);

    pid_t pid = fork();
    if (pid == 0) {
        CoreArbiterClient* client = CoreArbiterClient::getInstance();

        // Wait for the low priority thread to be put on a core
        while (client->getNumUnoccupiedCores() == 2)
            ;
        client->setRequestedCores({2, 0, 0, 0, 0, 0, 0, 0});

        std::thread highPriorityThread1(highPriorityBlock, std::ref(client));
        std::thread highPriorityThread2(highPriorityRequest, std::ref(client));
        highPriorityThread1.join();
        highPriorityThread2.join();

        for (int i = 0; i < NUM_TRIALS; i++) {
            latencies[i] = Cycles::toNanoseconds(latencies[i]);
        }
        printStatistics("core_request_timeout_latencies", latencies, NUM_TRIALS,
                        argc > 1 ? "data" : NULL);

    } else {
        CoreArbiterClient* client = CoreArbiterClient::getInstance();
        lowPriorityExec(client);

        wait(NULL);

        fflush(stdout);
    }
}
