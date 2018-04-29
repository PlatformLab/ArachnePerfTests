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

#define NUM_TRIALS 1000000

std::atomic<uint64_t> startCycles(0);
uint64_t arrayIndex = 0;
uint64_t latencies[NUM_TRIALS];

/**
 * This thread will get unblocked when a core is allocated, and will block
 * itself again when the number of cores is decreased.
 */
void
coreExec(CoreArbiterClient* client) {
    for (int i = 0; i < NUM_TRIALS; i++) {
        client->blockUntilCoreAvailable();
        uint64_t endCycles = Cycles::rdtsc();
        latencies[arrayIndex++] = endCycles - startCycles;
        while (!client->mustReleaseCore())
            ;
    }
}

void
coreRequest(CoreArbiterClient* client) {
    std::vector<uint32_t> oneCoreRequest = {1, 0, 0, 0, 0, 0, 0, 0};

    client->setRequestedCores(oneCoreRequest);
    client->blockUntilCoreAvailable();

    std::vector<uint32_t> twoCoresRequest = {2, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < NUM_TRIALS; i++) {
        // When the number of blocked threads becomes nonzero, we request a
        // core.
        while (client->getNumBlockedThreads() == 0)
            ;

        startCycles = Cycles::rdtsc();
        client->setRequestedCores(twoCoresRequest);
        // When the number of blocked threads becomes zero, we release a core.
        while (client->getNumBlockedThreads() == 1)
            ;
        client->setRequestedCores(oneCoreRequest);
    }
}

int
main(int argc, const char** argv) {
    Logger::setLogLevel(ERROR);
    CoreArbiterClient* client = CoreArbiterClient::getInstance();
    std::thread requestThread(coreRequest, std::ref(client));
    while (client->getNumOwnedCores() == 0)
        ;

    std::thread coreThread(coreExec, std::ref(client));

    coreThread.join();
    requestThread.join();

    for (int i = 0; i < NUM_TRIALS; i++) {
        latencies[i] = Cycles::toNanoseconds(latencies[i]);
    }
    printStatistics("core_request_noncontended_latencies", latencies,
                    NUM_TRIALS, argc > 1 ? "data" : NULL);
}
