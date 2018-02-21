#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include "process.h"

// IO SORT COMPARATOR ============================================================================
bool IO_sort(Process a, Process b) {
    return (a.endIOTime() < b.endIOTime());
}

// QUEUE CONTENTS ================================================================================
/* Returns a string of the queue contents in the format:
    [Q <empty>]   or   [Q pid1 pid2 pid3 (etc.)]                                                */
std::string queue_contents(std::list<Process> &process_queue) {
    std::stringstream ss;
    ss << "[Q";
    if (process_queue.size() == 0) {
        ss << " <empty>";
    } else {
        for (auto proc : process_queue) {
            ss << " " << proc.getPID();
        }
    }
    ss << "]";

    return ss.str();
}

// PROCESS ARRIVAL ===============================================================================
/* Handles process arrival in the ready queue                                                   */
void process_arrival(std::list<Process> &ready_queue, Process &proc, int time) {
    proc.setAsREADY(time);
    ready_queue.push_back(proc);
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " arrived and added to ready queue " << queue_contents(ready_queue) << "\n";
}

// PROCESS START =================================================================================
/* Handles pulling a process from the ready queue and beginning execution                       */
void process_start(std::list<Process> &ready_queue, Process &proc, int time) {
    proc.setAsRUNNING(time);
    ready_queue.pop_front();
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " started using the CPU " << queue_contents(ready_queue)
              << "\n";
}

// PROCESS BLOCK =================================================================================
/* Handles process arrival in the IO blocked queue                                              */
void process_block(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, Process &proc, int time) {
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " completed a CPU burst; " << proc.getNumBursts()
              << " burst";

    if (proc.getNumBursts() > 1)
        std::cout << "s";

    std::cout << " to go " << queue_contents(ready_queue)
              << "\n";


    proc.setAsBLOCKED(time);
    IO_blocked.push_back(proc);

    // maintain the IO queue in sorted order by shortest remaining blocked time
    IO_blocked.sort(IO_sort);

    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " switching out of CPU; will block on I/O until time "
              << proc.endIOTime() << "ms " << queue_contents(ready_queue)
              << "\n";
}

// PROCESS FINISHED IO ===========================================================================
/* Handles process finishing IO block                                                           */
void process_finished_IO(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, int time) {
    IO_blocked.front().setAsREADY(time);
    ready_queue.push_back(IO_blocked.front());

    std::cout << "time " << time << "ms: Process " << IO_blocked.front().getPID()
          << " completed I/O; added to ready queue " << queue_contents(ready_queue)
          << "\n";

    IO_blocked.pop_front();
}

// PROCESS TERMINATION ===========================================================================
/* Handles process finishing its last CPU burst                                                 */
void process_termination(std::list<Process> &ready_queue, Process &proc, int time) {
    proc.setAsTERMINATED();
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " terminated " << queue_contents(ready_queue)
              << "\n";
}
