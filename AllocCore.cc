#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include "Util.h"


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<int> parseCores(const char* coreDesc) {
    std::vector<int> cores;
    std::vector<std::string> ranges = split(coreDesc, ',');
    for (size_t i = 0; i < ranges.size(); i++) {
        if (ranges[i].find("-") == std::string::npos)
            cores.push_back(atoi(ranges[i].c_str()));
        else {
            auto bounds = split(ranges[i], '-');
            for (int k = atoi(bounds[0].c_str()); k <= atoi(bounds[1].c_str());
                    k++)
                cores.push_back(k);
        }
    }
    return cores;
}

std::vector<int> getAllCores(){
    FILE *fp;
    char path[1024];

    fp = popen("cat /sys/fs/cgroup/cpuset$(cat /proc/self/cpuset)/cpuset.cpus", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    if (fgets(path, sizeof(path)-1, fp) == NULL) {
        fprintf(stderr, "No cores found!\n");
        return std::vector<int>();
    }
    return parseCores(path);
}

void pinAvailableCore() {
    static std::vector<int> cores = getAllCores();
    static std::mutex coreAllocMutex;
    coreAllocMutex.lock();
    int coreId = cores.back();
    cores.pop_back();
    coreAllocMutex.unlock();
    PerfUtils::Util::pinThreadToCore(coreId);
}
