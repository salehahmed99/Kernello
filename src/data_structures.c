#include"data_structures.h"
#include "IO.h"
#include "clk.h"
#include <stdio.h>

#define MIN_BLOCK_SIZE 1  // You can set this to smallest allocatable unit (e.g., 4, 8, etc.)

// Queue operations
void initQueue(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

void enqueue(Queue* q, Process* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    
    q->size++;
}

Process* dequeue(Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }
    
    Node* temp = q->front;
    Process* data = temp->data;
    
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->size--;
    
    return data;
}

Process* peek(Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }
    
    return q->front->data;
}

bool isEmpty(Queue q) {
    return q.size == 0;
}

void priorityEnqueue(Queue *q, Process* newProcess, AlgoName algo)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->data = newProcess;
    newNode->next = NULL;

    if (q->front == NULL)
    {
        q->front = q->rear = newNode;
    }
    else
    {
        int newValue, frontValue;
        if (algo == SRTN)
        {
            newValue = newProcess->remaining;
            frontValue = q->front->data->remaining;
        }
        else
        {
            newValue = newProcess->priority;
            frontValue = q->front->data->priority;
        }

        if (newValue < frontValue)
        {
            newNode->next = q->front;
            q->front = newNode;
        }
        else
        {
            Node *current = q->front;
            while (current->next != NULL)
            {
                int nextValue = (algo == SRTN) ? current->next->data->remaining : current->next->data->priority;
                if (newValue < nextValue)
                    break;
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;

            if (newNode->next == NULL)
                q->rear = newNode;
        }
    }

    q->size++;
}

BuddyNode* create_node(int size, int start) {
    BuddyNode* node = (BuddyNode*)malloc(sizeof(BuddyNode));
    node->size = size;
    node->is_free = 1;
    node->start = start;
    node->left = node->right = NULL;
    return node;
}

BuddyNode* allocate_buddy(BuddyNode *node, int size, int process_id, int memsize) {
    if (!node || !node->is_free || node->size < size)
    {
        return NULL;
    }

    if (node->size == size && !node->left && !node->right) {
        printf("Allocating %d bytes at %d\n", size, node->start);
        logMemoryAllocation(get_clk(), memsize, process_id, node->start, node->start + size - 1);
        node->is_free = 0;
        return node;
    }

    // Split node if not yet split
    if (!node->left && node->size > MIN_BLOCK_SIZE) {
        node->left = create_node(node->size / 2, node->start);
        node->right = create_node(node->size / 2, node->start + node->size / 2);
    }

    BuddyNode* left_alloc = allocate_buddy(node->left, size, process_id, memsize);
    if (left_alloc)
        return left_alloc;

    return allocate_buddy(node->right, size, process_id, memsize);
}

void free_buddy(BuddyNode* root, BuddyNode *node, int start, int size, int process_id, int memsize) {
    if (!node) return;
    
    if (node->start == start && node->size == size) {
        printf("Freeing %d bytes at %d\n", size, start);
        logMemoryDeallocation(get_clk(), memsize, process_id, node->start, node->start + size - 1);
        node->is_free = 1;
        
        // Check if this node has a parent that should be merged
        merge_buddy_nodes(root);
        return;
    }

    // If we're not at the target node, recursively check children
    if (start < node->start + node->size/2) {
        free_buddy(root, node->left, start, size, process_id, memsize);
    } else {
        free_buddy(root, node->right, start, size, process_id, memsize);
    }
}

int merge_buddy_nodes(BuddyNode *node) {
    if (!node) return 1; // NULL nodes are considered free
    
    // If leaf node, just return its status
    if (!node->left && !node->right) {
        return node->is_free;
    }
    
    // Check if children can be merged
    int left_free = merge_buddy_nodes(node->left);
    int right_free = merge_buddy_nodes(node->right);
    
    if (left_free && right_free) {
        printf("Merging nodes at %d and %d\n", node->left->start, node->right->start);
        // Both children are free, merge them
        BuddyNode *left_temp = node->left;
        BuddyNode *right_temp = node->right;
        
        // Set pointers to NULL before freeing
        node->left = NULL;
        node->right = NULL;
        
        // Free child nodes
        free(left_temp);
        free(right_temp);
        
        // Mark this node as free
        return 1;
    }
    
    return 0;
}