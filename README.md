# Arachne Latency Microbenchmarks

These tests benchmark best-case scenarios for the cost of threading primitives
offered by the Arachne threading runtime and core arbiter.

In the current implementation, one must first start up the core arbiter server
before executing any benchmarks.

    make
    sudo bin/server --coresUsed 1,2
    scripts/RunAllTests.sh

If you want to run the Core Arbiter benchmarks, then you must add an extra flag, since these take longer to run.

    scripts/RunAllTests.sh --runArbiterBenchmarks
