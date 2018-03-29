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
		while (next < total_processes && processes[next].getArrivalTime() == time + i) {
			process_arrival_RR(ready_queue, processes[next], time + i, rr_add);
			next++;
		}
	}

	// start the first process
	Process running = ready_queue.front();
	process_start(ready_queue, running, time + T_CS / 2);   // allow time for the half context switch on the first process

	int context_counter = 0;
	int CPU_available = 0;
	bool preemption = false;
	//Process preempting_process;
	std::list<Process> IO_blocked;

	while (next < total_processes || ready_queue.size() > 0 || IO_blocked.size() > 0 || running.getStatus() == Status::RUNNING) {

		// preempt the currently running process
		if (preemption) {
			if (context_counter == 0) {
				running.setAsREADY(time -1 + T_CS/2);
			}else if (context_counter == T_CS / 2) {
				
				running.preempt(time -1 - T_CS / 2);    // preemption actually occurred last ms
				running.setAsREADY(time - 1);
				ready_queue.push_back(running);
				stats.num_context_switches++;
				stats.num_preemptions++;
				running = ready_queue.front();
				ready_queue.pop_front();
			}

			context_counter++;

			// context switch is completed
			if (context_counter >= T_CS) {
				//running = ready_queue.front();
				stats.avg_wait_time += (time - running.getReadyTime() - T_CS / 2);
				running.setAsRUNNING(time);
				std::cout << "time " << time << "ms: Process " << running.getPID() << " started using the CPU ";
				if (running.wasPreempted())
					std::cout << "with " << (running.endRemainingTime() - time) << "ms remaining ";

				std::cout << queue_contents(ready_queue) << "\n";
				context_counter = 0;
				preemption = false;
			}

			// check if a new process should be started (don't check for this if a preemption is occurring)
		}else if (running.getStatus() != Status::RUNNING  &&  ready_queue.size() > 0) {
			context_counter++;

			if (time >= CPU_available  && context_counter >= T_CS / 2) {
				running = ready_queue.front();
				context_counter = 0;
				stats.avg_wait_time += (time - running.getReadyTime() - T_CS / 2);
				process_start(ready_queue, running, time);
			}
		}

		// check if the current running process is done using the CPU, or if time slice is used up
		if (running.getStatus() == Status::RUNNING) {

			if (running.wasPreempted()) {
				if (running.endRemainingTime() == time) {
					calculate_turnaround(&stats, running, time);
					process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
				}
				else if ((time - running.getStartTime()) >= T_SLICE) {
					//preempting_process = running;
					if (ready_queue.size() == 0) {
						preemption = false;
						std::cout << "time " << time << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]\n";
						running.setRemaining_time(time);
						running.setAsRUNNING(time);
					}
					else {
						preemption = true;
						std::cout << "time " << time << "ms: Time slice expired; process " << running.getPID()
							<< " preempted with ";
						if (running.wasPreempted()) std::cout << running.endRemainingTime() - time;
						else std::cout << running.endBurstTime() - time;

						std::cout << "ms to go " << queue_contents(ready_queue)
							<< "\n";

						// time when the CPU will next be available (after context switch)
						CPU_available = time + T_CS;
					}
					//process_preempted(ready_queue, running, &CPU_available, &stats, time, rr_add);
				}
			}
			else {
				if (running.endBurstTime() == time) {
					calculate_turnaround(&stats, running, time);
					process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
				}
				else if ((time - running.getStartTime()) >= T_SLICE) {
					
					//preempting_process = running;
					if (ready_queue.size() == 0) {
						preemption = false;
						std::cout << "time " << time << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]\n";
						running.setRemaining_time(time);
						running.setAsRUNNING(time);
					}
					else {
						preemption = true;
						std::cout << "time " << time << "ms: Time slice expired; process " << running.getPID()
							<< " preempted with ";
						if (running.wasPreempted()) std::cout << running.endRemainingTime() - time;
						else std::cout << running.endBurstTime() - time;

						std::cout << "ms to go " << queue_contents(ready_queue)
							<< "\n";

						// time when the CPU will next be available (after context switch)
						CPU_available = time + T_CS;
					}
				}
			}
		}


		// check if any process is done IO
		if (IO_blocked.size() != 0) {
			if (IO_blocked.front().endIOTime() == time) {
				process_finished_IO_RR(ready_queue, IO_blocked, time, rr_add);
			}
		}

		// check if any processes are arriving
		if (next < total_processes  &&  processes[next].getArrivalTime() == time) {
			process_arrival_RR(ready_queue, processes[next], time, rr_add);
			next++;
		}

		time++;
	}

	// Calculate statistics
	int total_bursts = 0;
	for (auto &proc : processes) {
		stats.avg_burst_time += (proc.getTotalBursts() * proc.getBurstLength());
		total_bursts += proc.getTotalBursts();
	}
	stats.avg_burst_time /= total_bursts;
	stats.avg_turnaround_time = stats.avg_turnaround_time/total_bursts + T_CS/2;
	//stats.avg_turnaround_time /= total_bursts;
	//stats.avg_turnaround_time += T_CS / 2;
	stats.avg_wait_time /= total_bursts;

	time += (T_CS / 2 - 1);   // allow time for context switch
	std::cout << "time " << time << "ms: Simulator ended for RR" << std::endl;
	return stats;
}
