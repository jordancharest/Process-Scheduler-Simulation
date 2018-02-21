#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include <string>
#include <list>

// GLOBAL VARIABLES ==============================================================================
#define T_CS 8  // time needed for a context switch
extern int total_processes;

enum class Status {
    READY,      // ready to use the CPU
    RUNNING,    // actively using the CPU
    BLOCKED,    // blocked on I/O
    TERMINATED
};

typedef struct simulation_statistics {
    std::string algorithm;
    float avg_burst_time = 0;
    float avg_wait_time = 0;
    float avg_turnaround_time = 0;
    int num_context_switches = 1; // count the context switch needed to start the first process
    int num_preemptions = 0;
} stat_t;


// PROCESS CLASS =================================================================================
class Process {
public:
    // Constructor
    Process(char _pid, int _a_time, int _b_time, int _num_b, int _io) :
        pid(_pid), arrival_time(_a_time), burst_time(_b_time), num_bursts(_num_b), total_bursts(_num_b), io_time(_io) {}

    // Accessors
    const char getPID() const { return pid; }
    const long getArrivalTime() const { return arrival_time; }
    const int getBurstTime() const { return burst_time; }
    const int getNumBursts() const { return num_bursts; }
    const int getTotalBursts() const { return total_bursts; }
    const int getIOTime() const { return io_time; }
    const Status getStatus() const { return s; }
    const int getStartTime() const { return start_time; }
    const int endBurstTime() const { return (start_time + burst_time); }
    const int endIOTime() const { return (start_time + io_time); }

    // Modifiers
    void decrementNumBursts() { num_bursts--; }
    void setAsTERMINATED() { s = Status::TERMINATED; }
    void setAsREADY(int time) {
        ready_time = time;
        s = Status::READY;
    }
    void setAsRUNNING(int time) {
        start_time = time;
        s = Status::RUNNING;
    }
    void setAsBLOCKED(int time) {
        start_time = time + T_CS/2;
        s = Status::BLOCKED;
    }

private:
    char pid;
    int arrival_time;
    int burst_time;
    int num_bursts;
    int total_bursts;
    int io_time;
    int start_time = 0; // used for both CPU bursts and IO blocks
    int ready_time = 0; // the time the process was last added to the ready queue
    Status s = Status::READY;
};



// FUNCTION DECLARATIONS =========================================================================
stat_t First_Come_First_Serve(std::vector<Process> &processes);
stat_t Shortest_Remaining_Time(std::vector<Process> &processes);
stat_t Round_Robin(std::vector<Process> &processes);

std::string queue_contents(std::list<Process> &process_queue);
void process_arrival(std::list<Process> &ready_queue, Process &proc, int time);
void process_start(std::list<Process> &ready_queue, Process &proc, int time);
void process_block(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, Process &proc, int time);
void process_finished_IO(std::list<Process> &ready_queue, std::list<Process> &IO_blocked, int time);
void process_termination(std::list<Process> &ready_queue, Process &proc, int time);




#endif // PROCESS_H_INCLUDED
