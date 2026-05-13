#include "../include/patient.h"
#include "../include/ipc_manager.h"
#include "../include/logger.h"
#include <signal.h>

volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        keep_running = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <patient_id> <bed_type> <severity>\n", argv[0]);
        return 1;
    }
    
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    
    int patient_id = atoi(argv[1]);
    int bed_type = atoi(argv[2]);
    int severity = atoi(argv[3]);
    (void)severity; /* severity not directly used here; silence unused warning */
    
    pid_t pid = getpid();
    
    // Initialize logging
    init_logging();
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Patient %d (PID: %d) arrived at hospital", patient_id, pid);
    log_system_event(log_msg);
    printf("Patient %d arrived at hospital\n", patient_id);
    
    // Simulate treatment time based on severity
    int treatment_time;
    switch (bed_type) {
        case BED_ICU:
            treatment_time = 10 + rand() % 10; // 10-20 seconds
            break;
        case BED_ISOLATION:
            treatment_time = 5 + rand() % 5; // 5-10 seconds
            break;
        default:
            treatment_time = 2 + rand() % 3; // 2-5 seconds
    }
    
    snprintf(log_msg, sizeof(log_msg), "Patient %d started treatment (duration: %d seconds)", 
            patient_id, treatment_time);
    log_system_event(log_msg);
    printf("Patient %d started treatment\n", patient_id);
    
    sleep(treatment_time);
    
    // Send discharge signal
    write_to_discharge_fifo(patient_id, 0); // Simplified - would use actual bed ID
    
    snprintf(log_msg, sizeof(log_msg), "Patient %d discharged after %d seconds", 
            patient_id, treatment_time);
    log_system_event(log_msg);
    printf("Patient %d discharged\n", patient_id);
    
    close_logging();
    return 0;
}