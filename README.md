# ⚡ Kernello: CPU Scheduler and Memory Manager
Kernello is a comprehensive operating system simulation that implements multiple CPU scheduling algorithms with buddy memory allocation. The system provides complete process lifecycle management, performance analysis, and inter-process communication using C on Linux platforms.

---

## ✨ Features

- 🔄 **Multiple Scheduling Algorithms**: Non-preemptive HPF, SRTN, and Round Robin with configurable parameters.
- 💾 **Buddy Memory Allocation**: Dynamic memory management with allocation and deallocation tracking.
- 📊 **Performance Metrics**: CPU utilization, turnaround time, waiting time, and standard deviation analysis.
- 🔗 **Inter-Process Communication**: Seamless IPC between scheduler, process generator, and clock modules.
- 📝 **Process Control Blocks**: Complete PCB management tracking process states and execution details.
- 🔄 **Context Switching**: Efficient process switching with state preservation.
- 📋 **Comprehensive Logging**: Detailed scheduler events and memory allocation logs.
- ⚙️ **Process Simulation**: CPU-bound process simulation with arrival time management.

---

## 🏗️ System Architecture

The system consists of four interconnected components:

| 🔧 Component | 📁 File | 🛠️ Responsibility |
|--------------|---------|-------------------|
| **Process Generator** | `process_generator.c` | Input handling, algorithm selection, process creation |
| **Scheduler** | `scheduler.c` | Core scheduling logic, memory allocation, PCB management |
| **Clock** | `clk.c` | Time simulation and synchronization |
| **Process** | `process.c` | Individual process simulation and execution |

---


## 📋 Scheduling Algorithms

### 1. 🎯 Non-preemptive Highest Priority First (HPF)
- Priority-based scheduling (0 = highest, 10 = lowest)
- Non-preemptive execution until completion
- Tie-breaking by arrival time

### 2. ⚡ Shortest Remaining Time Next (SRTN)
- Preemptive algorithm selecting shortest remaining time
- Dynamic priority adjustment
- Optimal for minimizing average waiting time

### 3. 🔄 Round Robin (RR)
- Time-sharing with configurable quantum
- Fair CPU time distribution
- Circular queue implementation

---

## 💾 Memory Management

**Buddy Allocation System**:
- **Total Memory**: 1024 bytes
- **Max Process Size**: 256 bytes
- **Block Sizes**: Powers of 2 (256, 512, 1024 bytes)
- **Dynamic Allocation**: Split and coalesce blocks as needed
- **Fragmentation Control**: Efficient memory utilization

---

## 📥 Input Format

### Process Input File (`processes.txt`)
```
#id arrival runtime priority memsize
1   1       6       5        200
2   3       3       3        170
3   5       2       8        100
```

**Fields**:
- `id`: Process identifier
- `arrival`: Arrival time (seconds)
- `runtime`: Total execution time
- `priority`: Priority level (0-10)
- `memsize`: Required memory (bytes)

---

## 📤 Output Files

### 📊 Scheduler Log (`scheduler.log`)
```
At time 1 process 1 started arr 1 total 6 remain 6 wait 0
At time 3 process 1 stopped arr 1 total 6 remain 4 wait 0
At time 6 process 2 finished arr 3 total 3 remain 0 wait 0 TA 3 WTA 1.00
```

### 📈 Performance Report (`scheduler.perf`)
```
CPU utilization = 95%
Avg WTA = 1.34
Avg Waiting = 1.5
Std WTA = 0.34
```

### 💾 Memory Log (`memory.log`)
```
At time 1 allocated 200 bytes for process 1 from 0 to 255
At time 3 allocated 170 bytes for process 2 from 256 to 511
At time 6 freed 170 bytes from process 2 from 256 to 511
```

---

## 🚀 Getting Started

### Prerequisites
- Linux environment (Ubuntu/CentOS/Debian)
- GCC compiler
- Make utility (optional)

### 🔨 Building the Project
```bash
# Compile all components
gcc -o process_generator process_generator.c
gcc -o scheduler scheduler.c
gcc -o process process.c
gcc -o clk clk.c

# Or use Makefile if available
make all
```

### ▶️ Running the Scheduler
```bash
# Start the process generator
./process_generator

# Follow prompts to select scheduling algorithm
# Enter parameters (e.g., time quantum for Round Robin)

```
