#include "../include/scheduler.h"
#include "../include/logger.h"

QueueNode* create_priority_queue() {
    return NULL;
}

void enqueue_patient(QueueNode **head, Patient patient) {
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    new_node->patient = patient;
    
    if (*head == NULL || patient.priority < (*head)->patient.priority) {
        new_node->next = *head;
        *head = new_node;
    } else {
        QueueNode *current = *head;
        while (current->next != NULL && 
               current->next->patient.priority <= patient.priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Patient %s (Priority: %d) added to queue", 
            patient.name, patient.priority);
    log_schedule_event(log_msg);
}

Patient dequeue_patient(QueueNode **head) {
    if (*head == NULL) {
        Patient empty_patient = {0};
        return empty_patient;
    }
    
    QueueNode *temp = *head;
    Patient patient = temp->patient;
    *head = (*head)->next;
    free(temp);
    
    return patient;
}

int is_queue_empty(QueueNode *head) {
    return head == NULL;
}

void display_queue(QueueNode *head, FILE *log_file) {
    QueueNode *current = head;
    int count = 0;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    if (count == 0) {
        fprintf(log_file, "\n=== Current Queue: EMPTY ===\n");
        return;
    }
    
    current = head;
    fprintf(log_file, "\n=== Current Queue (%d patients) ===\n", count);
    fprintf(log_file, "%-5s %-15s %-5s %-10s\n", "ID", "Name", "Age", "Priority");
    fprintf(log_file, "----------------------------------------\n");
    
    while (current != NULL) {
        fprintf(log_file, "%-5d %-15s %-5d %-10d\n",
                current->patient.id,
                current->patient.name,
                current->patient.age,
                current->patient.priority);
        current = current->next;
    }
    fprintf(log_file, "\n");
}

void cleanup_queue(QueueNode *head) {
    while (head != NULL) {
        QueueNode *temp = head;
        head = head->next;
        free(temp);
    }
}

static int count_patients(QueueNode *head) {
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->next;
    }
    return count;
}

void schedule_fcfs(QueueNode *queue, FILE *log_file) {
    if (!queue) return;
    
    fprintf(log_file, "\n=== FCFS Scheduling ===\n");
    fprintf(log_file, "%-20s %-10s\n", "Patient", "Start Time");
    fprintf(log_file, "--------------------------------\n");
    
    QueueNode *current = queue;
    time_t current_time = time(NULL);
    int count = 0;
    
    while (current != NULL && count < 10) {
        fprintf(log_file, "%-20s %s", current->patient.name, ctime(&current_time));
        current_time += rand() % 10 + 5;
        current = current->next;
        count++;
    }
}

void schedule_sjf(QueueNode *queue, FILE *log_file) {
    if (!queue) return;
    
    fprintf(log_file, "\n=== SJF Scheduling (by severity) ===\n");
    fprintf(log_file, "%-20s %-10s %-10s\n", "Patient", "Severity", "Start Time");
    fprintf(log_file, "----------------------------------------\n");
    
    Patient patients[MAX_PATIENTS];
    int count = 0;
    QueueNode *current = queue;
    
    while (current != NULL && count < MAX_PATIENTS) {
        patients[count++] = current->patient;
        current = current->next;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (patients[j].severity > patients[j + 1].severity) {
                Patient temp = patients[j];
                patients[j] = patients[j + 1];
                patients[j + 1] = temp;
            }
        }
    }
    
    time_t current_time = time(NULL);
    for (int i = 0; i < count && i < 10; i++) {
        fprintf(log_file, "%-20s %-10d %s", 
                patients[i].name, patients[i].severity, ctime(&current_time));
        current_time += rand() % 10 + 5;
    }
}

void schedule_priority(QueueNode *queue, FILE *log_file) {
    if (!queue) return;
    
    fprintf(log_file, "\n=== Priority Scheduling ===\n");
    fprintf(log_file, "%-20s %-10s %-10s\n", "Patient", "Priority", "Start Time");
    fprintf(log_file, "----------------------------------------\n");
    
    QueueNode *current = queue;
    time_t current_time = time(NULL);
    int count = 0;
    
    while (current != NULL && count < 10) {
        fprintf(log_file, "%-20s %-10d %s", 
                current->patient.name, current->patient.priority, ctime(&current_time));
        current_time += rand() % 10 + 5;
        current = current->next;
        count++;
    }
}

void schedule_round_robin(QueueNode *queue, int quantum, FILE *log_file) {
    if (!queue) return;
    
    int patient_count = count_patients(queue);
    if (patient_count == 0) return;
    
    fprintf(log_file, "\n=== Round Robin Scheduling (Quantum: %d, Patients: %d) ===\n", 
            quantum, patient_count);
    fprintf(log_file, "%-20s %-10s %-10s\n", "Patient", "Time Slot", "Status");
    fprintf(log_file, "--------------------------------------------\n");
    
    Patient patients[MAX_PATIENTS];
    QueueNode *current = queue;
    int count = 0;
    
    while (current != NULL && count < MAX_PATIENTS) {
        patients[count++] = current->patient;
        current = current->next;
    }
    
    int time_slot = 0;
    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < count; i++) {
            fprintf(log_file, "%-20s %-10d %-10s\n", 
                    patients[i].name, time_slot, "Processing");
            time_slot += quantum;
        }
    }
    
    fprintf(log_file, "--- Round Robin Complete (%d rounds) ---\n\n", 2);
}

SchedulingStats calculate_statistics(Patient patients[], int count) {
    SchedulingStats stats = {0};
    if (count == 0) return stats;
    
    for (int i = 0; i < count; i++) {
        if (patients[i].treatment_start > 0 && patients[i].discharge_time > 0) {
            double waiting = difftime(patients[i].treatment_start, patients[i].arrival_time);
            double turnaround = difftime(patients[i].discharge_time, patients[i].arrival_time);
            stats.avg_waiting_time += waiting;
            stats.avg_turnaround_time += turnaround;
        }
    }
    
    stats.avg_waiting_time /= count;
    stats.avg_turnaround_time /= count;
    return stats;
}
