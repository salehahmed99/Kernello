#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdlib.h>
#include <stdbool.h>
typedef enum
{
    HPF,
    SRTN,
    RR
} AlgoName;

typedef enum
{
    READY,
    RUNNING,
    FINISHED,
} State;

typedef enum {
    PROCESS_ARRIVAL = 1,
    TERMINATION ,
} MessageType;

typedef struct
{
    int id;
    int arrival;
    int runtime;
    int priority;
    int pid;         // Actual process PID when forked
    State state;     // 0: not arrived, 1: ready, 2: running, 3: finished
    int remaining;   // Remaining time
    int waiting;     // Waiting time
    int start;       // Start time
    int finish_time; // Time when the process finished
    int memsize;     // Memory size
    int memoryAddress;      // Pointer to allocated memory
    int memoryPower;
} Process;

// Message structure for IPC
typedef struct
{
    long mtype;
    Process data;
} MsgBuff;

typedef struct
{   
    long mtype;
    int pid;
} TerminationMsgBuff;

typedef struct Node
{
    Process* data;
    struct Node *next;
} Node;

typedef struct
{
    Node *front;
    Node *rear;
    int size;
} Queue;

typedef struct BuddyNode {
    int size;
    int is_free;
    int start;
    struct BuddyNode *left;
    struct BuddyNode *right;
} BuddyNode;

void initQueue(Queue *q);

void enqueue(Queue *q, Process* data);

Process* dequeue(Queue *q);

Process* peek(Queue *q);

bool isEmpty(Queue q);

void priorityEnqueue(Queue *q, Process* newProcess, AlgoName algo);

BuddyNode* create_node(int size, int start);

BuddyNode* allocate_buddy(BuddyNode *node, int size, int process_id, int memsize);

void free_buddy(BuddyNode *root, BuddyNode *node, int start, int size, int process_id, int memsize);

int merge_buddy_nodes(BuddyNode *node);

#endif