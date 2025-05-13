// IO.h

#ifndef IO_H
#define IO_H

#include <stdio.h>
#include "data_structures.h" // Ensure this file defines the Process type

int initFiles(char *inFilePath);
int getNumProcesses();
int readProcesses(Process **processList);
int readProcesses2(Node**processList);
void logProcessStart(int time, Process *p);
void logProcessStop(int time, Process *p);
void logProcessResumption(int time, Process *p);
void logProcessFinish(int time, Process *p);
void logFinalStatus(float cpuUtilization, float AWTA, float AW, float STDWTA);
void logMemoryAllocation(int time, int memsize, int processId, int start, int end);
void logMemoryDeallocation(int time, int memsize, int processId, int start, int end);
#endif
