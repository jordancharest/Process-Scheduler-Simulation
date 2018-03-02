#include <iostream>
#include <vector>
#include <list>
#include "process.hpp"


// ROUND ROBIN ===================================================================================
/* Control function for round robin simulator                                                   */
stat_t Round_Robin(std::vector<Process> &processes, char* rr_add) {

    stat_t stats;
    stats.algorithm = "RR";
    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for RR " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
    int time = processes[0].getArrivalTime();
	int next = 0;

	// check for all processes that will arrive before the first process can start running
	for (int i = 0; i < (T_CS / 2); i++) {
		while (processes[next].getArrivalTime() == time + i  &&  next < total_processes) {
			process_arrival_RR(ready_queue, processes[next], time + i, rr_add);
			next++;
		}
	}

	// start the first process
	Process running = ready_queue.front();
	process_start(ready_queue, running, time + T_CS / 2);   // allow time for the half context switch on the first process

	int context_counter = 0;
	int CPU_available = 0;
	int num_bursts = 1;
	int total_burst_time = 0;
	std::list<Process> IO_blocked;

	while (next < total_processes || ready_queue.size() > 0 || IO_blocked.size() > 0 || running.getStatus() == Status::RUNNING) {

		// check if any processes are arriving
        if (next < total_processes  &&  processes[next].getArrivalTime() == time) {
            process_arrival_RR(ready_queue, processes[next], time, rr_add);
            next++;
        }

		// check if a new process should be started
		if (running.getStatus() != Status::RUNNING  &&  ready_queue.size() > 0) {
			context_counter++;

			if (time >= CPU_available  && context_counter >= T_CS / 2) {
				running = ready_queue.front();
				process_start(ready_queue, running, time);
				num_bursts++;
				context_counter = 0;
			}
		}

		// check if the current running process is done using the CPU, or if time slice is used up
		if (running.getStatus() == Status::RUNNING) {

			if (running.wasPreempted()) {
				if (running.endRemainingTime() == time) {
					total_burst_time += (time - running.getStartTime());
					process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
				}
				else if ((time - running.getStartTime()) >= T_SLICE && ready_queue.size() !=0 ) {
					total_burst_time += (time - running.getStartTime());
					process_preempted(ready_queue, running, &CPU_available, &stats, time, rr_add);
				}
			}
			else {
				if (running.endBurstTime() == time) {
					total_burst_time += (time - running.getStartTime());
					process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
				}
				else if ((time - running.getStartTime()) >= T_SLICE && ready_queue.size() != 0) {
					total_burst_time += (time - running.getStartTime());
					process_preempted(ready_queue, running, &CPU_available, &stats, time, rr_add);
				}
			}

		}


		// check if any process is done IO
		if (IO_blocked.size() != 0) {
			if (IO_blocked.front().endIOTime() == time) {
				process_finished_IO_RR(ready_queue, IO_blocked, time, rr_add);
			}
		}

		time++;
	}

	// Calculate statistics
	stats.avg_burst_time = (float)total_burst_time / num_bursts;
	stats.avg_turnaround_time /= total_processes;
	stats.avg_wait_time /= total_processes;

	time += (T_CS / 2 - 1);   // allow time for context switch
	std::cout << "time " << time << "ms: Simulator ended for RR\n" << std::endl;
	return stats;
}
