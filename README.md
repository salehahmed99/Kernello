# Kernello: CPU Scheduler with Buddy Memory Allocation

## Overview
This project, developed for the Operating Systems course (CMPN303) at Cairo University, Faculty of Engineering, Computer Engineering Department, implements a CPU scheduler with three scheduling algorithms and a buddy memory allocation system. Written in C for a Linux platform, it focuses on process scheduling, inter-process communication (IPC), and efficient memory management for a single-core CPU with a 1024-byte memory space.

## Project Structure
### Key Components
- **process_generator.c**: Reads input files, prompts the user for the scheduling algorithm and parameters, initiates scheduler and clock processes, manages process data structures, and handles IPC resources.
- **clk.c**: A pre-built module emulating an integer time clock for process timing.
- **scheduler.c**: The core component managing process scheduling based on the selected algorithm, maintaining process control blocks (PCBs), and handling memory allocation/deallocation.
- **process.c**: Simulates CPU-bound processes that notify the scheduler upon termination.

### Scheduling Algorithms
1. **Non-preemptive Highest Priority First (HPF)**:
   - Selects the process with the highest priority (0 = highest, 10 = lowest) from the ready queue.
   - Runs the process to completion without interruption.
   - Ties are resolved by selecting the process with the earliest arrival time.

2. **Shortest Remaining Time Next (SRTN)**:
   - Preemptively schedules the process with the shortest remaining execution time.
   - Ties are resolved by selecting the process with the highest priority.

3. **Round Robin (RR)**:
   - Assigns a fixed time quantum to each process in the ready queue.
   - Processes are cycled in a circular queue, preempted after the quantum expires.
   - The quantum size is user-specified at runtime.

### Memory Allocation
- Uses the buddy memory allocation system to allocate memory for processes upon arrival and deallocate upon termination.
- Total memory: 1024 bytes.
- Process memory size: Up to 256 bytes, specified in the input file.
- Memory allocation remains constant for a process throughout its execution.

## Input/Output
### Input File Format
The input file (e.g., `processes.txt`) contains process information with tab-separated columns:
```
#id    arrival    runtime    priority    memsize
1      1          6          5           200
2      3          3          3           170
```
- Comments start with `#` and are ignored.
- Processes are sorted by arrival time; multiple processes may arrive simultaneously.
- `memsize` specifies the memory required by each process.

### Output Files
1. **scheduler.log**: Logs process scheduling events (e.g., process start, stop, finish times, and states).
2. **scheduler.perf**: Reports performance metrics:
   - CPU utilization.
   - Average weighted turnaround time.
   - Average waiting time.
   - Standard deviation of weighted turnaround time.
3. **memory.log**: Logs memory allocation and deallocation events for each process.

## Prerequisites
- **Operating System**: Linux
- **Compiler**: GCC
- **Libraries**: Standard C libraries (e.g., `stdio.h`, `stdlib.h`, `unistd.h`, `sys/ipc.h`, `sys/types.h`)

## Installation and Usage
1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. **Compile the Code**:
   Use a `Makefile` or compile manually:
   ```bash
   gcc -o process_generator process_generator.c
   gcc -o scheduler scheduler.c
   gcc -o process process.c
   gcc -o clk clk.c
   ```

3. **Run the Program**:
   Execute the process generator, which prompts for the scheduling algorithm and parameters:
   ```bash
   ./process_generator
   ```
   - Select the scheduling algorithm (HPF, SRTN, or RR).
   - For RR, specify the time quantum.
   - Provide the input file path (e.g., `processes.txt`).

4. **Output**:
   - Check `scheduler.log`, `scheduler.perf`, and `memory.log` for results.

## License
This project is for educational purposes only and is not licensed for commercial use.
