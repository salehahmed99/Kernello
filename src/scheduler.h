#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "clk.h"
#include"data_structures.h"

void run_scheduler(AlgoName algo, int RRquantum, int totalProcessCount);
void calculatePerformance();
Process* getNextProcess();
bool shouldPreempt(Process* currentProcess, AlgoName algorithm, int quantum, int elapsedTime);
void checkForNewProcesses();
void checkForProcessTermination();

#endif