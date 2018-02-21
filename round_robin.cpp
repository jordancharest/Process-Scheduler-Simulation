#include <iostream>
#include <vector>
#include <list>
#include "process.h"


// ROUND ROBIN ===================================================================================
/* Control function for round robin simulator                                                   */
stat_t Round_Robin(std::vector<Process> &processes) {

    stat_t stats;
    stats.algorithm = "RR";
    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for RR " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
    int time = processes[0].getArrivalTime();

    std::cout << "time " << time << "ms: Simulator ended for RR\n" << std::endl;
    return stats;
}
