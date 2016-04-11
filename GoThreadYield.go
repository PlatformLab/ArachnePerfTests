package main

import "fmt"
import "time"
import "runtime"

const numRuns = 10000

type  timeRecord struct{
    Ts time.Time
    Msg string
    Index int64
}

func threadMain(start, end int64) {
    startTime := time.Now()
    for i := start; i < end; i+=2 {
        runtime.Gosched()
    }
    timePerYield := (time.Now().Sub(startTime)).Nanoseconds() / (end - start);
    fmt.Printf("%v\n", timePerYield);
}

func main() {
    go threadMain(1, 9999)
    go threadMain(2,10000)
    time.Sleep(5 * time.Second)
}
