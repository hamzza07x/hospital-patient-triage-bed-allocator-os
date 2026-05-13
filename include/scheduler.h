#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

// Queue node for scheduling
typedef struct QueueNode {
    Patient patient;
    struct QueueNode *next;
} QueueNode;

// Scheduler functions
QueueNode* create_priority_queue();
void enqueue_patient(QueueNode **head, Patient patient);
Patient dequeue_patient(QueueNode **head);
int is_queue_empty(QueueNode *head);
void display_queue(QueueNode *head, FILE *log_file);
void cleanup_queue(QueueNode *head);

// Scheduling algorithms
void schedule_fcfs(QueueNode *queue, FILE *log_file);
void schedule_sjf(QueueNode *queue, FILE *log_file);
void schedule_priority(QueueNode *queue, FILE *log_file);
void schedule_round_robin(QueueNode *queue, int quantum, FILE *log_file);

// Statistics
typedef struct {
    double avg_waiting_time;
    double avg_turnaround_time;
    double avg_response_time;
} SchedulingStats;

SchedulingStats calculate_statistics(Patient patients[], int count);

#endif