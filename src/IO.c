#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IO.h"

// Global file pointers and buffer
char line[256];
FILE *IN;
FILE *schedulerLog;
FILE *schedulePerf;
FILE *memoryLog;

// Open and prepare input/output files
int initFiles(char *inFilePath) {
    IN = fopen(inFilePath, "r");
    if (IN == NULL) {
        perror("Error opening input file");
        return -1;
    }

    schedulerLog = fopen("scheduler.log", "w");
    if (schedulerLog == NULL) {
        perror("Error creating scheduler.log");
        return -1;
    }

    schedulePerf = fopen("scheduler.perf", "w");
    if (schedulePerf == NULL) {
        perror("Error creating scheduler.perf");
        return -1;
    }

    memoryLog = fopen("memory.log", "w");
    if (memoryLog == NULL)
    {
        perror("Error creating memory.log");
        return -1;
    }

    return 0;
}

// Count the number of process entries
int getNumProcesses() {
    int count = 0;
    while (fgets(line, sizeof(line), IN)) {
        if (line[0] != '#') {
            count++;
        }
    }
    rewind(IN);
    return count;
}

// Read all process entries into processList array
int readProcesses(Process **processList) {
    int i = 0;
    while (fgets(line, sizeof(line), IN))
    {
        processList[i] = (Process *)malloc(sizeof(Process));
        if (line[0] == '#')
            continue; // skip comment lines
        sscanf(line, "%d %d %d %d %d", &processList[i]->id, &processList[i]->arrival, &processList[i]->runtime, &processList[i]->priority, &processList[i]->memsize);
        
        processList[i]->remaining = processList[i]->runtime;


        // why initialize these values here?
        processList[i]->finish_time = 0;
        processList[i]->pid = -1; // Initialize PID to -1
        
        ++i;
    }
    return i;
}

// Read all process entries into processList array
int readProcesses2(Node **processList) {

    int i = 0;
    while (fgets(line, sizeof(line), IN))
    {
        processList[i] = (Node *)malloc(sizeof(Node));
        if (line[0] == '#')
            continue; // skip comment lines
        sscanf(line, "%d %d %d %d %d", &processList[i]->data->id, &processList[i]->data->arrival, &processList[i]->data->runtime, &processList[i]->data->priority, &processList[i]->data->memsize);
        
        processList[i]->data->remaining = processList[i]->data->runtime;
        processList[i]->next = NULL;

        // why initialize these values here?
        processList[i]->data->finish_time = 0;
        processList[i]->data->pid = -1; // Initialize PID to -1
        
        ++i;
    }
    return i;
}

// Logging helper functions
void logProcessStart(int time, Process* p) {
    fprintf(schedulerLog, "At time %d process %d started arr %d total %d remain %d wait %d\n",
            time, p->id, p->arrival, p->runtime, p->remaining, time - p->arrival);
}

void logProcessStop(int time, Process* p) {
    fprintf(schedulerLog, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
            time, p->id, p->arrival, p->runtime, p->remaining, time - p->arrival - (p->runtime - p->remaining));
}

void logProcessResumption(int time, Process* p) {
    fprintf(schedulerLog, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
            time, p->id, p->arrival, p->runtime, p->remaining, time - p->arrival - (p->runtime - p->remaining));
}

void logProcessFinish(int time, Process* p) {
    fprintf(schedulerLog, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n",
            time, p->id, p->arrival, p->runtime, time - p->arrival - p->runtime, time - p->arrival, p->runtime!=0? (float)(time - p->arrival) / p->runtime:0);
}


// Final performance summary
void logFinalStatus(float cpuUtilization, float AWTA, float AW, float STDWTA) {
    fprintf(schedulePerf, "CPU utilization = %.2f%%\n", cpuUtilization);
    fprintf(schedulePerf, "Avg WTA = %.2f\n", AWTA);
    fprintf(schedulePerf, "Avg Waiting = %.2f\n", AW);
    fprintf(schedulePerf, "Std WTA = %.2f\n", STDWTA);
}

void logMemoryAllocation(int time, int memsize, int processId, int start, int end) {
    fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", time, memsize, processId, start, end);
    fflush(memoryLog);
}

void logMemoryDeallocation(int time, int memsize, int processId, int start, int end)
{
    fprintf(memoryLog, "At time %d freed %d bytes from process %d from %d to %d\n", time, memsize, processId,start, end);
    fflush(memoryLog);
}