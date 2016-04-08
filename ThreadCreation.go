package main

import "fmt"
import "time"
import "runtime"

const numRuns = 10000

var timeStamps []time.Time
func threadMain(a, b int) {
    timeStamps = append(timeStamps, time.Now())
}

func main() {
    timeStamps = make([]time.Time, 0, numRuns*2)
    for i := 0; i < numRuns; i++ {
        timeStamps = append(timeStamps, time.Now())
        go threadMain(0, 1)
        time.Sleep(100 * time.Microsecond)
        runtime.Gosched()
    }

    for i := 1; i < len(timeStamps) ; i++ {
        fmt.Println(timeStamps[i].Sub(timeStamps[i-1]).Nanoseconds())
    }
//    2919249.0 ns (+45122.6 ns): Inside thread
//    3044439.6 ns (+125190.7 ns): A thread is about to be born!
}
