# Process Scheduler Simulation
The Process Scheduler Simulation simulates how an OS would schedule processes using three different scheduling algorithms:
First Come First Serve, Shortest Remaining Time, and Round Robin.

## Compile and Run
g++ -std=c++11 *.cpp

./a.out input-file output-file

### Input File
The program expects a formatted input file with the following formatting rules:
- lines that start with # are ignored
- each process must be on its own line and take the form: proc-id|initial-arrival-time|cpu-burst-time|num-bursts|io-time , including the '|' delimiter
- process IDs are single character

An example input file may look something like this:

\# example simulator input file

\# <--- commented line

\# proc-id|initial-arrival-time|cpu-burst-time|num-bursts|io-time

A|0|168|5|287

B|0|385|1|0

D|250|1770|2|822

C|190|97|5|2499

E|200|50|1|50

### Output File
The initially blank output file will be populated by timestamped notable events that happen during the simulation such as:
- Process arrival
- Process finishes a CPU burst
- Process finishes I/O block
- Process termination
- etc.

### Statistics
Statistics for each algorithm are written to stdout. The simulation tracks the following statistics for each algorithm:
- Average CPU burst time
- Average wait time
- Average turnaround time
- Total number of context switches
- Total number of preemptions
