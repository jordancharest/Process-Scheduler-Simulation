#include <iostream>
#include <vector>
#include <list>
#include "process.h"


// SHORTEST REMAINING TIME =======================================================================
/* Control function for shortest remaining time simulator                                       */
stat_t Shortest_Remaining_Time(std::vector<Process> &processes) {

    stat_t stats;
    stats.algorithm = "SRT";
    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for SRT " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
    int time = processes[0].getArrivalTime();

    std::cout << "time " << time << "ms: Simulator ended for SRT\n" << std::endl;
    return stats;


}
