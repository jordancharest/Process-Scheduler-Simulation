#include <iostream>
#include <vector>
#include <list>
#include "process.hpp"

// SHORTEST REMAINING TIME SORT ==================================================================
bool SRT_sort(Process &a, Process &b) {
    return (a.getBurstLength() < b.getBurstLength());
}


// SHORTEST REMAINING TIME =======================================================================
/* Control function for shortest remaining time simulator                                       */
stat_t Shortest_Remaining_Time(std::vector<Process> &processes) {

    stat_t stats;
    stats.algorithm = "SRT";
    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for SRT " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
	std::list<Process> buffered_arrival;
	int buffered_ct = 0;
	int time = processes[0].getArrivalTime();
	int next = 0;
	// check for all processes that will arrive before the first process can start running
	for (int i = 0; i < (T_CS / 2); i++) {
		while (next < total_processes && processes[next].getArrivalTime() == time + i) {
			buffered_arrival.push_back(processes[next]);
			next++;
			buffered_ct++;
		}
		buffered_arrival.sort(AB_sort);
		for (int j = 0; j < buffered_ct; j++) {
			process_arrival(ready_queue, buffered_arrival.front(), time + i);
			buffered_arrival.pop_front();
		}
		buffered_ct = 0;
	}

    // start the first process
    Process running = ready_queue.front();
    process_start(ready_queue, running, time+T_CS/2);   // allow time for the half context switch on the first process


    bool preemption = false;
    Process preempting_process;
    int CPU_available = 0;
    int context_counter = 0;
    int num_bursts = 1;
    int total_burst_time = 0;
    std::list<Process> IO_blocked;

    while (next < total_processes  ||  ready_queue.size() > 0  ||  IO_blocked.size() > 0  ||  running.getStatus() == Status::RUNNING) {

        // preempt the currently running process
        if (preemption) {
            if (context_counter == 0) {
                running.preempt(time-1);    // preemption actually occurred last ms
                running.setAsREADY(time-1+ T_CS/2);
                ready_queue.push_back(running);
                ready_queue.sort(SRT_sort);
                stats.num_context_switches++;
                stats.num_preemptions++;

                running = preempting_process;
            }
            context_counter++;

            // context switch is completed
            if (context_counter >= T_CS) {
                process_start(ready_queue, running, time);
                context_counter = 0;
                preemption = false;
            }

        // check if a new process should be started (don't check for this if a preemption is occurring)
        } else if (running.getStatus() != Status::RUNNING  &&  ready_queue.size() > 0) {
            context_counter++;

            if (time >= CPU_available  &&  context_counter >= T_CS/2) {
                running = ready_queue.front();
                stats.avg_wait_time += (time - running.getReadyTime() - T_CS/2);
                num_bursts++;
                context_counter = 0;

                process_start(ready_queue, running, time);
            }

        // check if the current running process is done using the CPU (don't check for this if a preemption is occurring)
        // or
        // check if a previously preempted process is done using the CPU
        } else if ((running.getStatus() == Status::RUNNING  &&  (running.endBurstTime() == time))
                    || (running.wasPreempted()  &&  running.getStatus() == Status::RUNNING  &&  running.endRemainingTime() == time)){

            total_burst_time += (time - running.getStartTime());
            calculate_turnaround(&stats, running, time);
            process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
        }

        // check if any process is done IO
		if (IO_blocked.size() != 0) {
			if (IO_blocked.front().endIOTime() == time) {

				if (((!running.wasPreempted() && (IO_blocked.front().getBurstLength() < (running.endBurstTime() - time)))          // running was not preempted (check full burst time)
					|| (running.wasPreempted() && (IO_blocked.front().getBurstLength() < (running.endRemainingTime() - time))))    // running was preempted (check remaining time)
					&& (running.getStatus() == Status::RUNNING)) {

					preemption = true;
					preempting_process = IO_blocked.front();
					preempt_after_IO(ready_queue, IO_blocked, preempting_process, running, time);

				}
				else {
					process_finished_IO(ready_queue, IO_blocked, time, &stats);
					ready_queue.sort(SRT_sort);
				}
			}
		}

        // check if any processes are arriving
        if (next < total_processes  &&  processes[next].getArrivalTime() == time) {

            // if the arriving process has a shorter remaining time than the running process as preemption may occur
            if (((processes[next].getBurstLength() < (running.endBurstTime() - time))
                ||  (processes[next].getBurstLength() < (running.endRemainingTime() - time)))
                &&  (running.getStatus() == Status::RUNNING)) {

                preemption = true;
                preempting_process = processes[next];
                preempt_on_arrival(ready_queue, preempting_process, running, time);

            // else just add it to the ready queue
            } else {
                //process_arrival(ready_queue, processes[next], time);
				processes[next].setAsREADY(time);
				processes[next].setWholeBurstReadyTime(time);
				ready_queue.push_back(processes[next]);
				ready_queue.sort(SRT_sort);
				std::cout << "time " << time << "ms: Process " << processes[next].getPID()
					<< " arrived and added to ready queue " << queue_contents(ready_queue) << "\n";
            }

            next++;
        }

        time++;
    }

    // Calculate stats
    int total_bursts = 0;
    for (auto &proc : processes) {
        stats.avg_burst_time += (proc.getTotalBursts() * proc.getBurstLength());
        total_bursts += proc.getTotalBursts();
    }
    stats.avg_burst_time /= total_bursts;
    stats.avg_turnaround_time = stats.avg_turnaround_time/total_bursts + T_CS/2;
    stats.avg_wait_time /= total_bursts;

    time += (T_CS/2 - 1);   // allow time for context switch
    std::cout << "time " << time << "ms: Simulator ended for SRT\n" << std::endl;
    return stats;
}
