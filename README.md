Jinda:

main.cpp
- Simulator is controlled by main.cpp. I don't think any more changes need to be made to this file
  - parses input file, then runs three separate routines (one for each algorithm)
  - sends each routine a vector of all the processes, ordered by arrival time
  
process.h
- enum class Status
  - contains enums for each process state (READY, RUNNING, BLOCKED, TERMINATED)
  
- struct simulation_statistics (stat_t)
  - struct containing all the statistics that need to be output at the end of simulation
  - needs to be returned to main
  
- class Process

- also defines T_CS (time for a context switch) and declares all shared functions

process.cpp
- implementation file for all shared functions

<algorithm>.cpp
- control functions for all three simulation algorithms
