#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include "process.hpp"

int total_processes;


// PARSE INPUT ===================================================================================
/* Read all the processes from a formatted input file
    # Commented lines start with '#'
    # Processes are formatted as below (including delimiters):
    <proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>                    */

void parse_input(std::ifstream &InputStream, std::vector<Process> &processes) {

    char pid;
    int arrival_time;
    int burst_time;
    int num_bursts;
    int io_time;
    char delimiter;
    int i = 0;

    while (InputStream >> pid) {
        // line is commented, ignore until newline is found
        if (pid == '#') {
                InputStream.ignore(1000, '\n');
        } else {
            InputStream >> delimiter
                        >> arrival_time
                        >> delimiter
                        >> burst_time
                        >> delimiter
                        >> num_bursts
                        >> delimiter
                        >> io_time;

            processes.push_back( Process(pid, arrival_time, burst_time, num_bursts, io_time) );
            i++;
        }
    }
    total_processes = i;
}

// PROCESS ORDER =================================================================================
void process_order(std::vector<Process> &processes) {
    char delimiter = '|';
    for (auto &proc : processes) {
        std::cout   << proc.getPID()
                    << delimiter
                    << proc.getArrivalTime()
                    << delimiter
                    << proc.getBurstTime()
                    << delimiter
                    << proc.getNumBursts()
                    << delimiter
                    << proc.getIOTime()
                    << std::endl;
    }
}

// ARRIVAL SORT ==================================================================================
bool arrival_sort(Process &a, Process &b) {
    return (a.getArrivalTime() < b.getArrivalTime());
}

// OUTPUT STATS ==================================================================================
void print_stats (stat_t &stats, std::ofstream &OutputStream) {
    OutputStream << "Algorithm " << stats.algorithm << "\n" << std::fixed << std::setprecision(2)
                 << "-- average CPU burst time: " << stats.avg_burst_time << " ms\n"
                 << "-- average wait time: " << stats.avg_wait_time << " ms\n"
                 << "-- average turnaround time: " << stats.avg_turnaround_time << " ms\n"
                 << "-- total number of context switches: " << stats.num_context_switches << "\n"
                 << "-- total number of preemptions: " << stats.num_preemptions << std::endl;
}

// MAIN ==========================================================================================
int main(int argc, char* argv[]){

    if (argc < 3 || argc > 4) {
		std::cerr << "Invalid arguments\n";
        std::cerr << "USAGE: " << argv[0] << " <input-file> <stats-output-file> [<rr-add>]\n";
        return EXIT_FAILURE;
    }

    std::ifstream InputStream(argv[1]);
    if (!InputStream.good()){
        std::cerr << "Can't open " << argv[1] << " to read.\n";
        return EXIT_FAILURE;
    }

    std::ofstream OutputStream(argv[2]);
    if (!OutputStream.good()){
        std::cerr << "Can't open " << argv[2] << " to write.\n";
        return EXIT_FAILURE;
    }

	char rr_add[10] = "END\0";

	if (argc == 4) {		
		if ( !(strcmp(argv[3], "END")==0 || strcmp(argv[3], "BEGINNING")==0) ) {
			std::cerr << "Invalid arguments\n";
			std::cerr << "USAGE: " << argv[0] << " <input-file> <stats-output-file> [<rr-add>]\n";
			return EXIT_FAILURE;
		}
		else
			sscanf(argv[3], "%s", rr_add);
	}

    std::vector<Process> processes;
    parse_input(InputStream, processes);
    std::sort(processes.begin(), processes.end(), arrival_sort);
    InputStream.close();

#ifdef DEBUG_MODE
    process_order(processes);
#endif

    // Run three different scheduling simulations
    //stat_t FCFS_stats = First_Come_First_Serve(processes);
    //stat_t SRT_stats = Shortest_Remaining_Time(processes);
    stat_t RR_stats = Round_Robin(processes, rr_add);

    //print_stats(FCFS_stats, OutputStream);
    //print_stats(SRT_stats, OutputStream);
    print_stats(RR_stats, OutputStream);

    return EXIT_SUCCESS;
}
