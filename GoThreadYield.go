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


var timeStamps []timeRecord
func threadMain(start, end int64) {
    startTime := time.Now()
    for i := start; i < end; i+=2 {
        timeStamps = append(timeStamps, timeRecord{time.Now(), "Inside thread", start})
        runtime.Gosched()
    }
    if start == 2 {
        timePerYield := (time.Now().Sub(startTime)).Nanoseconds() / (end - start);
        fmt.Printf("%v\n", timePerYield);
    }
}

func main() {
    go threadMain(1, 9999)
    go threadMain(2,10000)
    timeStamps = make([]timeRecord, 0, 10000)
    time.Sleep(5 * time.Second)
    // Regularize 
    regularizedTime := make([]time.Duration, numRuns*2)
    for i := 0; i < len(timeStamps) ; i++ {
        regularizedTime[i] = timeStamps[i].Ts.Sub(timeStamps[0].Ts)
    }

    // Fake timetraced
    fmt.Printf("%6d ns (+%6d ns): %s %v\n", 0, 0, timeStamps[0].Msg, timeStamps[0].Index)
    for i := 1; i < len(timeStamps) ; i++ {
        fmt.Printf("%8d ns (+%6d ns): %s %v\n", regularizedTime[i], (regularizedTime[i] - regularizedTime[i-1]).Nanoseconds(), timeStamps[i].Msg, timeStamps[i].Index)
    }
//    2919249.0 ns (+45122.6 ns): Inside thread
//    3044439.6 ns (+125190.7 ns): A thread is about to be born!
}
