#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include "clk.h"
#include "data_structures.h"
#include "scheduler.h"
#include "IO.h"
#include "process.h"

Queue readyQueue;
Process **allProcesses = NULL;
int currentProcessCount = 0;
int finishedCount = 0;
int processMsgQueueId;
int terminationMsgQueueId;
AlgoName algorithm;

float total_wta = 0;
float total_waiting = 0;
float total_ta = 0;
float totalRuntime = 0;
float variance = 0;
float finalFinishTime = 1;
int processCount;
int startTime = 0;

bool terminationReceived = false;

void run_scheduler(AlgoName algo, int quantum, int totalProcessCount)
{
    processCount = totalProcessCount;
    algorithm = algo;
    // Initialize data structures
    initQueue(&readyQueue);

    // Get message queue
    processMsgQueueId = msgget(1234, 0644);
    if (processMsgQueueId == -1)
    {
        perror("Error accessing message queue");
        exit(1);
    }

    terminationMsgQueueId = msgget(5678, 0644);
    if (terminationMsgQueueId == -1)
    {
        perror("Error accessing termination message queue");
        exit(1);
    }

    // Synchronize with the clock
    sync_clk();

    // Allocate memory for processes (assuming max 100 processes)
    allProcesses = (Process **)malloc(totalProcessCount * sizeof(Process *));

    Process *currentProcess = NULL;
    startTime = 0;
    int elapsedTime = 0;
    int loopCount = 0;
    bool processFinished = false;

    // Main scheduler loop
    while (true)
    {
        if (processFinished)
        {
            processFinished = false;
            usleep(1000);
        }
        // Check for new processes
        checkForNewProcesses();

        // If no current process is running, get the next one
        if (currentProcess == NULL)
        {
            currentProcess = getNextProcess(algorithm);
            if (currentProcess != NULL)
            {
                startTime = get_clk();
                elapsedTime = 0;

                // Update waiting time if first time running
                if (currentProcess->runtime == currentProcess->remaining)
                {
                    currentProcess->waiting = startTime - currentProcess->arrival;
                }

                // Start or resume the process
                if (currentProcess->start == -1)
                {
                    currentProcess->start = startTime;
                    currentProcess->state = RUNNING;
                    logProcessStart(startTime, currentProcess);
                }
                else
                {
                    currentProcess->state = RUNNING;
                    logProcessResumption(startTime, currentProcess);
                }

                // Continue the process execution
                kill(currentProcess->pid, SIGCONT);
                printf("Starting process %d at time %d\n", currentProcess->id, startTime);
            }
        }

        // Process is running, check if we need to preempt
        if (currentProcess != NULL)
        {
            // Update elapsed time
            elapsedTime = get_clk() - startTime;

            // Check if process has completed
            if (elapsedTime >= currentProcess->remaining)
            {
                // Process completed
                int endTime = get_clk();
                currentProcess->remaining = 0;
                currentProcess->finish_time = endTime;
                finishedCount++;

                checkForProcessTermination();

                if (currentProcess->state == FINISHED)
                {
                    printf("Process %d finished at time %d\n", currentProcess->id, endTime);
                }
                else
                {
                    printf("Time Error at time %d\n", endTime);
                }
                logProcessFinish(endTime, currentProcess);

                // Calculate TA, WTA, Waiting Time
                int turnaround_time = currentProcess->finish_time - currentProcess->arrival;
                float weighted_turnaround = currentProcess->runtime > 0 ? (float)turnaround_time / currentProcess->runtime : 0;
                int waiting_time = turnaround_time - currentProcess->runtime;

                // Update totals
                total_ta += turnaround_time;
                total_wta += weighted_turnaround;
                total_waiting += waiting_time;
                totalRuntime += currentProcess->runtime;
                variance += weighted_turnaround * weighted_turnaround;
                finalFinishTime = currentProcess->finish_time;
                free(currentProcess);
                currentProcess = NULL;
                processFinished = true;
            }
            // Check if we should preempt based on the algorithm
            else if (shouldPreempt(currentProcess, algorithm, quantum, elapsedTime))
            {
                // Preempt the process
                int endTime = get_clk();
                currentProcess->remaining -= elapsedTime;
                currentProcess->state = READY;

                logProcessStop(endTime, currentProcess);

                // Stop the process
                kill(currentProcess->pid, SIGSTOP);
                printf("Preempting process %d at time %d\n", currentProcess->id, endTime);

                // Put the process back in the ready queue

                if (algorithm == RR)
                {
                    enqueue(&readyQueue, currentProcess);
                }
                else
                {
                    priorityEnqueue(&readyQueue, currentProcess, algorithm);
                }

                // Reset current process
                currentProcess = NULL;
            }
        }

        // Check if scheduler should terminate
        if (terminationReceived && finishedCount == currentProcessCount)
        {
            break;
        }
        loopCount++;
        if (loopCount >= 2) {
            loopCount = 0;
            pause();
        }
    }

    printf("Scheduler terminating...\n");

    calculatePerformance();
    free(allProcesses);
    destroy_clk(0);
}

void checkForNewProcesses()
{
    MsgBuff message;
    int recv_val;

    int retries = 100;
    while ((recv_val = msgrcv(processMsgQueueId, &message, sizeof(MsgBuff) - sizeof(long), 0, IPC_NOWAIT)) != -1 && retries-- > 0)
    {
        if (message.mtype == TERMINATION)
        {
            printf("Scheduler received termination message. No new processes will be sent\n");
            terminationReceived = true;
        }
        else if (message.mtype == PROCESS_ARRIVAL)
        {
            Process *newProcess = (Process *)malloc(sizeof(Process));
            *newProcess = message.data; // Copy the process data from the message
            newProcess->start = -1;
            newProcess->state = READY;

            allProcesses[currentProcessCount] = newProcess;
            // Insert the new process into the linked list
            if (algorithm == RR)
            {
                enqueue(&readyQueue, newProcess);
            }
            else
            {
                priorityEnqueue(&readyQueue, newProcess, algorithm);
            }

            currentProcessCount++;

            printf("Scheduler received process %d with PID %d at time %d\n",
                   newProcess->id, newProcess->pid, get_clk());
        }
        usleep(100); // sleep 1000 microseconds
    }
}

void checkForProcessTermination()
{
    TerminationMsgBuff message;

    // Block and wait for the termination message
    int recv_val = msgrcv(terminationMsgQueueId, &message, sizeof(TerminationMsgBuff) - sizeof(long), 1, 0);

    // Handle msgrcv failure
    if (recv_val == -1)
    {
        perror("Error receiving termination message");
        return;
    }

    for (int i = 0; i < currentProcessCount; i++)
    {
        if (allProcesses[i]->pid == message.pid)
        {
            allProcesses[i]->state = FINISHED;
            break;
        }
    }

    printf("Process %d terminated and state updated\n", message.pid);
}

Process *getNextProcess()
{
    if (isEmpty(readyQueue))
    {
        return NULL;
    }

    Process *nextProcess = NULL;
    nextProcess = dequeue(&readyQueue);
    return nextProcess;
}

bool shouldPreempt(Process *currentProcess, AlgoName algorithm, int quantum, int elapsedTime)
{
    switch (algorithm)
    {
    case SRTN:
        if (!isEmpty(readyQueue))
        {
            Process *shortestProcess = peek(&readyQueue);
            return shortestProcess->remaining < (currentProcess->remaining - elapsedTime);
        }
        return false;

    case HPF:
        return false;

    case RR:
        if (!isEmpty(readyQueue))
        {
            return elapsedTime >= quantum;
        }
        if (elapsedTime == quantum)
        {
            startTime = get_clk();
            currentProcess->remaining -= elapsedTime;    
        }
        return false;
    default:
        return false;
    }
}

void calculatePerformance()
{

    // Calculate CPU utilization
    float utilization = (totalRuntime / finalFinishTime) * 100;

    // Calculate standard deviation of WTA
    float avrg_wta = total_wta / processCount;
    float avrg_total_waiting = total_waiting / processCount;

    float mean_square = variance / processCount;
    float square_mean = avrg_wta * avrg_wta;
    float final_variance = mean_square - square_mean;
    float std_wta = sqrt(final_variance);

    logFinalStatus(utilization, avrg_wta, avrg_total_waiting, std_wta);
}