#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
#include <vector>

// This set of options contains the union of all options used by any of the
// latency microbenchmarks which are pat of ArachnePerfTests. Not all options
// are used by all benchmarks.
struct Options {
    bool hypertwinsActive;
    int numSleepers;
};

/**
 * Determine how many sleeping threads to run with and whether to run with
 * hyperthreads idle or working.
 */
struct Options parseOptions(int argc, char** argv) {
    Options options;
    memset(&options, 0, sizeof(options));
    int opt;
    while ((opt = getopt(argc, argv, "as:")) != -1) {
        switch (opt) {
            case 'a':
                options.hypertwinsActive = true;
                break;
            case 's':
                options.numSleepers = atoi(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-a] [-s numSleepingThreads]\n"
                        "-a  If specified, hypertwins are active. Otherwise they are idle.\n"
                        "-n  provide the number of threads that should sleep when the benchmark runs\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    return options;
}

/**
 * This function will abort if there aren't exact hypertwin pairs available in
 * the set of offered cores.
 * Index 0 and 1 are hypertwins are each other; Index 2 and 3 are hypertwins of
 * each other.
 */
std::vector<int> getCoresOrderedByHT() {
    Arachne::CorePolicy::CoreList coreList = Arachne::getCorePolicy()->getCores(0);

    // We can't have both HT's if there is an odd number of cores.
    assert(!(coreList.size() & 1));

    std::vector<int> coresOrderedByHT;
    coresOrderedByHT.resize(coreList.size(), -1);

    // The first HT will live at an even index, and the second will live at an
    // odd index.
    std::unordered_map<int, int> coresIdToIndex;
    int nextEvenIndex = 0;

    for (int i = 0; i < coreList.size(); i++) {
        int cId = coreList[i];
        int htId = PerfUtils::Util::getHyperTwin(cId);
        auto coreIndex = coresIdToIndex.find(htId);

        // Hypertwin is already in the array
        if (coreIndex != coresIdToIndex.end()) {
            coresOrderedByHT[coreIndex->second + 1] = cId;
            continue;
        }

        coresOrderedByHT[nextEvenIndex] = cId;
        coresIdToIndex[cId] = nextEvenIndex;
        nextEvenIndex+=2;
    }

    for (int i = 0; i < coreList.size(); i++) {
        if (coresOrderedByHT[i] == -1) {
            fprintf(stderr, "CoreList does not include complete HT pairs!\n");
            exit(1);
        }
    }
    return coresOrderedByHT;
}
#endif  // COMMON_H
