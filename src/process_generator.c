#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include "clk.h"
#include "scheduler.h"
#include "process.h"
#include "IO.h"

#define MAX_MEM_SIZE 1024

void cleanup();
void sigchld_handler();
void insertIntoPendingProcesses(Process *process);
int processMessageQueueId;
int terminationMessageQueueId;
Process **processes = NULL;
int process_count = 0;
int scheduler_pid = -1;
BuddyNode *root = NULL;

Node *pendingProcesses = NULL;
 

int main(int argc, char *argv[])
{
    signal(SIGINT, cleanup);
    signal(SIGCHLD,sigchld_handler);
    signal(SIGUSR1, dummy_handler);

    root = create_node(MAX_MEM_SIZE, 0);

    int quantum;
    AlgoName algo;
    char *fileName = NULL;
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s -s <scheduling algorithm> -f <processes.txt>\n", argv[0]);
        return 1;
    }
    else if (argc == 5)
    {
        if (strcmp(argv[1], "-s") || (strcmp(argv[2], "srtn") && strcmp(argv[2], "hpf")) || strcmp(argv[3], "-f"))
        {
            fprintf(stderr, "Usage: %s -s <scheduling algorithm> -f <processes.txt>\n", argv[0]);
            return 1;
        }
        fileName = argv[4];
        quantum = 0;
        if (strcmp(argv[2], "srtn") == 0)
            algo = SRTN;
        else if (strcmp(argv[2], "hpf") == 0)
            algo = HPF;
        else
        {
            fprintf(stderr, "Unknown scheduling algorithm: %s\n", argv[2]);
            return 1;
        }
    }
    else if (argc == 7)
    {
        if (strcmp(argv[1], "-s") || strcmp(argv[2], "rr") || strcmp(argv[3], "-q") || strcmp(argv[5], "-f"))
        {
            fprintf(stderr, "for round robin Usage: %s -s rr -q <quantum> -f <processes.txt>\n", argv[0]);
            return 1;
        }
        quantum = atoi(argv[4]);
        fileName = argv[6];
        algo = RR;
    }

    int initFilesStat = initFiles(fileName);

    if (initFilesStat == -1)        // Check if the files were opened successfully
        return 1;

    process_count = getNumProcesses();  // Get the number of processes from the input file

    // Allocate memory for process data
    processes = (Process **)malloc(process_count * sizeof(Process*));

    // Read process data
    readProcesses(processes);

    // Create IPC message queue
    processMessageQueueId = msgget(1234, IPC_CREAT | 0644);
    if (processMessageQueueId == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    terminationMessageQueueId = msgget(5678, IPC_CREAT | 0644);  
    if (terminationMessageQueueId == -1)
    {
        perror("Error creating termination message queue");
        exit(1);
    }

    // Fork/exec the clock process
    int clock_pid = fork();
    if (clock_pid == 0)
    {
        // Child process - execute clock
        init_clk();
        sync_clk();
        run_clk();
        exit(0);
    }

    // Fork/exec the scheduler process
    scheduler_pid = fork();
    if (scheduler_pid == 0)
    {
        // Child process - execute scheduler
        run_scheduler(algo, quantum , process_count);
        exit(0);
    }

    // Sync with the clock
    sync_clk();

    // Monitor clock and fork processes as they arrive
    int current_time;
    int processes_sent = 0;

    while (processes_sent < process_count || pendingProcesses != NULL)
    {
        current_time = get_clk();
        for (int i = 0; i < process_count; i++)
        {
            if (processes[i]->pid == -1 && processes[i]->arrival <= current_time)
            {
                insertIntoPendingProcesses(processes[i]);
                printf("GENERATOR: Process %d arrived at time %d\n", processes[i]->id, current_time);
                processes[i]->pid = -2; // mark as inserted (but not yet forked)
            }
        }
 
        Node *prev = NULL;
        Node *current = pendingProcesses;
        while (current != NULL)
        {
            Process *process = current->data;

            // Round memsize to nearest power of 2 
            int power = 1;
            while (power < process->memsize)
                power *= 2;

            BuddyNode *allocated = allocate_buddy(root, power, process->id, process->memsize);
            if (allocated != NULL)
            {
                process->memoryAddress = allocated->start;  // Storing start as a fake pointer (simulate address)
                process->memoryPower = power;

                if (prev == NULL) {
                    pendingProcesses = current->next;
                } else {
                    prev->next = current->next;
                }
                Node *temp = current;
                current = current->next;
                free(temp);

                printf("GENERATOR: Creating process %d at time %d\n", process->id, current_time);

                int process_pid = fork();
                if (process_pid == 0) {
                    run_process(process->runtime);
                    exit(0);
                }

                process->pid = process_pid;
                kill(process_pid, SIGSTOP);

                MsgBuff message;
                message.mtype = PROCESS_ARRIVAL;
                message.data = *process;
                if (msgsnd(processMessageQueueId, &message, sizeof(MsgBuff) - sizeof(long), 0) == -1) {
                    perror("Error sending message to scheduler");
                    return 1;
                }
                printf("GENERATOR: Sent process %d to scheduler\n", process->id);
                processes_sent++;
            }
            else if (current)
            {
                prev = current;
                current = current->next;
            }
        }
        kill(scheduler_pid, SIGUSR1);
        pause(); 
    }
    // Wait for all processes to finish
    // Send termination message to scheduler
    MsgBuff term_msg;
    term_msg.mtype = TERMINATION;    // Special message type for termination

    msgsnd(processMessageQueueId, &term_msg, sizeof(MsgBuff) - sizeof(long), 0);

    // Wait for scheduler to finish
    int status;
    // Wait for scheduler to finish using non-blocking wait
    while (waitpid(scheduler_pid, &status, WNOHANG) == 0) {
        pause(); 
        kill(scheduler_pid, SIGUSR1);
    }

    // Clean up
    cleanup();
    return 0;
}

void sigchld_handler() {
    pid_t pid;
    int status;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process %d terminated from sigchld_handler\n", pid);
        
        // Create a termination message for the scheduler
        TerminationMsgBuff term_msg;
        term_msg.mtype = 1;  // Use the correct message type
        term_msg.pid = pid;

        // Find the process in our array to update its state
        for (int i = 0; i < process_count; i++) {
            if (processes[i]->pid == pid) {
                processes[i]->state = FINISHED;
                free_buddy(root, root, processes[i]->memoryAddress, processes[i]->memoryPower, processes[i]->id, processes[i]->memsize);
                processes[i]->memoryAddress = -1; // Mark as freed
                break;
            }
        }
        kill(scheduler_pid, SIGUSR1);
        // Send termination message to the scheduler
        if (msgsnd(terminationMessageQueueId, &term_msg, sizeof(TerminationMsgBuff) - sizeof(long), 0) == -1) {
            perror("Error sending termination message to scheduler");
            exit(1);
        }
        
        printf("sigchld_handler notified scheduler about process %d termination\n", pid);
    }
}

void insertIntoPendingProcesses(Process *process)
 {
     Node *newNode = (Node *)malloc(sizeof(Node));
     newNode->data = process;
     newNode->next = NULL;
 
     if (pendingProcesses == NULL)
     {
         pendingProcesses = newNode;
     }
     else
     {
         Node *current = pendingProcesses;
         while (current->next != NULL)
         {
             current = current->next;
         }
         current->next = newNode;
     }
 }

// Cleanup resources
void cleanup()
{
    msgctl(processMessageQueueId, IPC_RMID, NULL);
    msgctl(terminationMessageQueueId, IPC_RMID, NULL);

    destroy_clk(1);
    for (int i = 0; i < process_count; i++) {
        if (processes[i] != NULL)
            free(processes[i]);
    }
    free(processes);
    exit(0);
}

