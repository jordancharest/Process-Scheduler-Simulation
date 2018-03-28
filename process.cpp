#include <iostream>
#include <cstring>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include "process.hpp"

// IO SORT COMPARATOR ============================================================================
bool IO_sort(Process &a, Process &b) {
    return (a.endIOTime() < b.endIOTime());
}

// READY QUEUE SORT ==============================================================================
bool RQ_sort(Process &a, Process &b) {
    return (a.getBurstLength() < b.getBurstLength());
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

// CALCULATE TURNAROUND ==========================================================================
/* Calculate stats when a process is leaves CPU (either preemption or finishing)                */
void calculate_turnaround(stat_t *stats, Process proc, int time) {
    stats->num_context_switches++;
    if (stats->algorithm == "FCFS")
        stats->avg_turnaround_time += (time - proc.getReadyTime());
    else
        stats->avg_turnaround_time += (time - proc.getWholeBurstReadyTime());
}

// PROCESS ARRIVAL ===============================================================================
/* Handles process arrival in the ready queue                                                   */
void process_arrival(std::list<Process> &ready_queue, Process &proc, int time) {
    proc.setAsREADY(time);
    proc.setWholeBurstReadyTime(time);
    ready_queue.push_back(proc);
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " arrived and added to ready queue " << queue_contents(ready_queue) << "\n";
}

// PREEMPT ON ARRIVAL ============================================================================
/* Process arriving and immediately preempting the running process                              */
void preempt_on_arrival(std::list<Process> &ready_queue, Process &arriving, Process &running, int time) {
    std::cout << "time " << time << "ms: Process " << arriving.getPID()
              << " arrived and will preempt " << running.getPID() << " " << queue_contents(ready_queue) << "\n";
    ready_queue.push_front(arriving);
}

// PROCESS ARRIVAL ===============================================================================
/* Handles process arrival in the ready queue for Round Robin                                   */
void process_arrival_RR(std::list<Process> &ready_queue, Process &proc,  int time, char* rr_add) {
	proc.setAsREADY(time);
	proc.setWholeBurstReadyTime(time);
	if (strcmp(rr_add, "BEGINNING") == 0)
		ready_queue.push_front(proc);
	else
		ready_queue.push_back(proc);

	std::cout << "time " << time << "ms: Process " << proc.getPID()
		<< " arrived and added to ready queue " << queue_contents(ready_queue) << "\n";
}

// PROCESS START =================================================================================
/* Handles pulling a process from the ready queue and beginning execution                       */
void process_start(std::list<Process> &ready_queue, Process &proc, int time) {
    proc.setAsRUNNING(time);
    ready_queue.pop_front();
    std::cout << "time " << time << "ms: Process " << proc.getPID() << " started using the CPU ";
    if (proc.wasPreempted())
        std::cout << "with " << (proc.endRemainingTime() - time) << "ms remaining ";

    std::cout << queue_contents(ready_queue) << "\n";
}

// PROCESS FINISHED BURST ========================================================================
/* Handles when a process finishes a CPU burst, sending it to IO or terminating it              */
void process_finished_burst(std::list<Process> &ready_queue, std::list<Process> &IO_blocked,
                            Process &proc, int* CPU_available, stat_t* stats, int time) {

    proc.decrementNumBursts();

    // calculate time when the CPU will next be available (after context switch)
    if (proc.wasPreempted())
        *CPU_available = proc.endRemainingTime() + T_CS;
    else
        *CPU_available = proc.endBurstTime() + T_CS;

    // Process termination
    if (proc.getNumBursts() == 0)
        process_termination(ready_queue, proc, time);

    // or IO burst
    else
        process_block(ready_queue, IO_blocked, proc, time);
}

// PROCESS PREEMPTED ========================================================================
/* Handles when a process is preempted, sending it to ready queue                          */
void process_preempted(std::list<Process> &ready_queue, Process &proc, int* CPU_available, stat_t* stats, int time, char* rr_add) {
	if (ready_queue.size() == 0) {
		std::cout << "time " << time << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]\n";
		proc.setRemaining_time(time);
		proc.setAsRUNNING(time);
	}
	else {
		std::cout << "time " << time << "ms: Time slice expired; process " << proc.getPID()
			<< " preempted with ";
		if (proc.wasPreempted()) std::cout << proc.endRemainingTime() - time;
		else std::cout << proc.endBurstTime() - time;

		std::cout << "ms to go " << queue_contents(ready_queue)
			<< "\n";

		stats->num_context_switches++;
		stats->num_preemptions++;

		proc.preempt(time);

		// time when the CPU will next be available (after context switch)
		*CPU_available = time + T_CS;
		proc.setAsREADY(time + T_CS/2);

		/*if (strcmp(rr_add, "BEGINNING") == 0)
			ready_queue.push_front(proc);
		else*/
		ready_queue.push_back(proc);
	}
}

// PROCESS BLOCK =================================================================================
/* Handles process arrival in the IO blocked queue                                              */
void process_block(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, Process &proc, int time) {
    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " completed a CPU burst; " << proc.getNumBursts()
              << " burst";

    if (proc.getNumBursts() > 1)
        std::cout << "s";

    std::cout << " to go " << queue_contents(ready_queue) << "\n";

    if (proc.wasPreempted())
        proc.noLongerPreempted();

    proc.setAsBLOCKED(time);
    IO_blocked.push_back(proc);
    IO_blocked.sort(IO_sort);   // maintain the IO queue in sorted order by shortest remaining blocked time

    std::cout << "time " << time << "ms: Process " << proc.getPID()
              << " switching out of CPU; will block on I/O until time "
              << proc.endIOTime() << "ms " << queue_contents(ready_queue)
              << "\n";
}

// PROCESS FINISHED IO ===========================================================================
/* Handles process finishing IO block                                                           */
void process_finished_IO(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, int time, stat_t* stats) {
    IO_blocked.front().setAsREADY(time);
    IO_blocked.front().setWholeBurstReadyTime(time);
    ready_queue.push_back(IO_blocked.front());

    if (stats->algorithm == "SRT")
        ready_queue.sort(RQ_sort);

    std::cout << "time " << time << "ms: Process " << IO_blocked.front().getPID()
          << " completed I/O; added to ready queue " << queue_contents(ready_queue)
          << "\n";

    IO_blocked.pop_front();
}

// PROCESS FINISHED IO ===========================================================================
/* Handles process finishing IO block for Round Robin                                           */
void process_finished_IO_RR(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, int time, char* rr_add) {
	IO_blocked.front().setAsREADY(time);
	IO_blocked.front().setWholeBurstReadyTime(time);
	if (strcmp(rr_add, "BEGINNING") == 0)
		ready_queue.push_front(IO_blocked.front());
	else
		ready_queue.push_back(IO_blocked.front());

	std::cout << "time " << time << "ms: Process " << IO_blocked.front().getPID()
		<< " completed I/O; added to ready queue " << queue_contents(ready_queue)
		<< "\n";
	IO_blocked.pop_front();
}

// PREEMPT AFTER IO ==============================================================================
/* Output for when a process finishes IO and preempts the running process                       */
void preempt_after_IO(std::list<Process> &ready_queue, std::list<Process> &IO_blocked,
                      Process &preempting, Process &running, int time) {

    std::cout << "time " << time << "ms: Process " << preempting.getPID()
              << " completed I/O and will preempt " << running.getPID() << " " << queue_contents(ready_queue) << "\n";

    running.setAsREADY(time);
    preempting.setWholeBurstReadyTime(time);
    ready_queue.push_front(preempting);
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
