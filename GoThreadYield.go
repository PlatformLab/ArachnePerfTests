package main

import "fmt"
import "time"
import "runtime"

const numRuns = 10000

type  timeRecord struct{
    Ts time.Time
    Msg string
}


var timeStamps []timeRecord
func threadMain(start, end int64) {
    startTime := time.Now()
//    timeStamps = append(timeStamps, timeRecord{time.Now(), "Inside thread"})
    for i := start; i < end; i+=2 {
        runtime.Gosched()
    }
    if start == 2 {
        timePerYield := (time.Now().Sub(startTime)).Nanoseconds() / (end - start);
        fmt.Printf("%v\n", timePerYield);
//        TimeTrace::getGlobalInstance()->print();
    }
}

func main() {
    go threadMain(1, 9999)
    go threadMain(2,10000)
    for ;; {}
//    timeStamps = make([]timeRecord, 0, numRuns*2)
//    for i := 0; i < numRuns; i++ {
//        timeStamps = append(timeStamps, timeRecord{time.Now(), "Before creation"})
//        go threadMain(0, 1)
//        time.Sleep(100 * time.Microsecond)
////        runtime.Gosched()
//    }
//    // Regularize 
//    regularizedTime := make([]time.Duration, numRuns*2)
//    for i := 0; i < len(timeStamps) ; i++ {
//        regularizedTime[i] = timeStamps[i].Ts.Sub(timeStamps[0].Ts)
//    }
//
//    // Fake timetraced
//    fmt.Printf("%6d ns (+%6d ns): %s\n", 0, 0, timeStamps[0].Msg)
//    for i := 1; i < len(timeStamps) ; i++ {
//        fmt.Printf("%8d ns (+%6d ns): %s\n", regularizedTime[i], (regularizedTime[i] - regularizedTime[i-1]).Nanoseconds(), timeStamps[i].Msg)
//    }
//    2919249.0 ns (+45122.6 ns): Inside thread
//    3044439.6 ns (+125190.7 ns): A thread is about to be born!
}
