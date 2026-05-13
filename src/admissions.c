#include "../include/admissions.h"
#include "../include/ipc_manager.h"
#include "../include/scheduler.h"
#include "../include/memory_manager.h"
#include "../include/thread_pool.h"
#include "../include/logger.h"

HospitalState *hospital_state = NULL;
int shm_id;
pthread_t receptionist, scheduler, nurse;
QueueNode *patient_queue = NULL;
MemoryBlock *memory_pool = NULL;
ThreadPool *thread_pool = NULL;

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        running = 0;
        if (hospital_state) {
            hospital_state->system_running = 0;
        }
    }
}

void initialize_system() {
    // Initialize logging
    init_logging();
    
    // Initialize shared memory
    shm_id = init_shared_memory(&hospital_state);
    if (shm_id < 0) {
        fprintf(stderr, "Failed to initialize shared memory\n");
        exit(1);
    }
    
    // Initialize hospital state
    memset(hospital_state, 0, sizeof(HospitalState));
    hospital_state->system_running = 1;
    
    // Initialize beds
    for (int i = 0; i < TOTAL_BEDS; i++) {
        hospital_state->beds[i].id = i;
        hospital_state->beds[i].occupied = 0;
        hospital_state->beds[i].patient_id = -1;
        
        if (i < GENERAL_CAPACITY) {
            hospital_state->beds[i].type = BED_GENERAL;
            hospital_state->beds[i].size = 100; // 100 bytes for general bed
        } else if (i < GENERAL_CAPACITY + ICU_CAPACITY) {
            hospital_state->beds[i].type = BED_ICU;
            hospital_state->beds[i].size = 500; // 500 bytes for ICU bed
        } else {
            hospital_state->beds[i].type = BED_ISOLATION;
            hospital_state->beds[i].size = 300; // 300 bytes for isolation bed
        }
    }
    
    // Initialize synchronization primitives
    pthread_mutex_init(&hospital_state->mutex, NULL);
    pthread_cond_init(&hospital_state->bed_available, NULL);
    sem_init(&hospital_state->icu_sem, 1, ICU_CAPACITY);
    sem_init(&hospital_state->isolation_sem, 1, ISOLATION_CAPACITY);
    
    // Initialize IPC
    create_pipes();
    create_fifos();
    
    // Initialize data structures
    patient_queue = create_priority_queue();
    memory_pool = init_memory_pool(4096); // 4KB total memory
    thread_pool = create_thread_pool();
    
    log_system_event("Hospital system initialized");
}

void run_system() {
    // Create main threads
    pthread_create(&receptionist, NULL, receptionist_thread, hospital_state);
    pthread_create(&scheduler, NULL, scheduler_thread, hospital_state);
    pthread_create(&nurse, NULL, nurse_thread, hospital_state);
    
    log_system_event("All system threads started");
    
    // Main loop
    while (running) {
        // Process incoming patients from pipe
        char buffer[1024];
        if (read_from_triage_pipe(buffer, sizeof(buffer)) > 0) {
            // Parse and add to queue
            char *name = strtok(buffer, ",");
            char *age_str = strtok(NULL, ",");
            char *severity_str = strtok(NULL, ",");
            
            if (name && age_str && severity_str) {
                Patient patient;
                patient.id = hospital_state->patient_count++;
                strncpy(patient.name, name, MAX_NAME_LEN - 1);
                patient.age = atoi(age_str);
                patient.severity = atoi(severity_str);
                patient.arrival_time = time(NULL);
                
                // Map severity to priority
                switch (patient.severity) {
                    case SEVERITY_EMERGENCY: patient.priority = PRIORITY_EMERGENCY; break;
                    case SEVERITY_CRITICAL: patient.priority = PRIORITY_CRITICAL; break;
                    case SEVERITY_HIGH: patient.priority = PRIORITY_HIGH; break;
                    case SEVERITY_MEDIUM: patient.priority = PRIORITY_MEDIUM; break;
                    default: patient.priority = PRIORITY_LOW;
                }
                
                // Assign bed type
                if (patient.severity >= SEVERITY_CRITICAL) {
                    patient.required_bed = BED_ICU;
                } else if (patient.severity == SEVERITY_HIGH) {
                    patient.required_bed = BED_ISOLATION;
                } else {
                    patient.required_bed = BED_GENERAL;
                }
                
                enqueue_patient(&patient_queue, patient);
                
                char log_msg[256];
                snprintf(log_msg, sizeof(log_msg), "Patient %s admitted and queued", patient.name);
                log_system_event(log_msg);
                
                // Fork patient process
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    char patient_id_str[10], bed_type_str[10], severity_str[10];
                    snprintf(patient_id_str, sizeof(patient_id_str), "%d", patient.id);
                    snprintf(bed_type_str, sizeof(bed_type_str), "%d", patient.required_bed);
                    snprintf(severity_str, sizeof(severity_str), "%d", patient.severity);
                    
                    execl("./patient_simulator", "patient_simulator", 
                          patient_id_str, bed_type_str, severity_str, NULL);
                    exit(1);
                } else {
                    patient.process_id = pid;
                }
            }
        }
        
        // Run scheduling algorithms periodically
        if (!is_queue_empty(patient_queue) && hospital_state->patient_count % 5 == 0) {
            display_queue(patient_queue, schedule_log);
            
            schedule_fcfs(patient_queue, schedule_log);
            schedule_sjf(patient_queue, schedule_log);
            schedule_priority(patient_queue, schedule_log);
            schedule_round_robin(patient_queue, 5, schedule_log);
            
            // Test memory allocation
            size_t bed_size = 100;
            int bed_id = allocate_bed_best_fit(memory_pool, bed_size);
            if (bed_id >= 0) {
                char log_msg[256];
                snprintf(log_msg, sizeof(log_msg), "Best-Fit allocation successful for bed %d", bed_id);
                log_memory_event(log_msg);
            }
            
            print_memory_state(memory_pool, memory_log);
        }
        
        usleep(100000); // 100ms
    }
}

void cleanup_system() {
    log_system_event("Shutting down hospital system");
    
    // Stop threads
    hospital_state->system_running = 0;
    pthread_join(receptionist, NULL);
    pthread_join(scheduler, NULL);
    pthread_join(nurse, NULL);
    
    // Cleanup resources
    destroy_thread_pool(thread_pool);
    cleanup_queue(patient_queue);
    cleanup_memory_pool(memory_pool);
    cleanup_fifos();
    
    pthread_mutex_destroy(&hospital_state->mutex);
    pthread_cond_destroy(&hospital_state->bed_available);
    sem_destroy(&hospital_state->icu_sem);
    sem_destroy(&hospital_state->isolation_sem);
    
    cleanup_shared_memory(hospital_state);
    close_logging();
    
    log_system_event("Hospital system shutdown complete");
}

int main() {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    
    initialize_system();
    run_system();
    cleanup_system();
    
    return 0;
}