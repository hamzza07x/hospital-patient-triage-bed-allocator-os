#include "../include/thread_pool.h"
#include "../include/logger.h"
#include "../include/ipc_manager.h"

ThreadPool* create_thread_pool(void) {
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (!pool) return NULL;
    
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->task_count = 0;
    pool->shutdown = 0;
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_not_empty, NULL);
    pthread_cond_init(&pool->queue_not_full, NULL);
    
    /* Create worker threads */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    }
    
    return pool;
}

void destroy_thread_pool(ThreadPool *pool) {
    if (!pool) return;
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_not_empty);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    /* Wait for threads to finish */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_not_empty);
    pthread_cond_destroy(&pool->queue_not_full);
    
    free(pool);
}

int add_task(ThreadPool *pool, Task task) {
    if (!pool) return -1;
    pthread_mutex_lock(&pool->queue_mutex);
    
    while (pool->task_count == QUEUE_MAX_SIZE && !pool->shutdown) {
        pthread_cond_wait(&pool->queue_not_full, &pool->queue_mutex);
    }
    
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_mutex);
        return -1;
    }
    
    pool->task_queue[pool->queue_rear] = task;
    pool->queue_rear = (pool->queue_rear + 1) % QUEUE_MAX_SIZE;
    pool->task_count++;
    
    pthread_cond_signal(&pool->queue_not_empty);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return 0;
}

void* worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_not_empty, &pool->queue_mutex);
        }
        
        if (pool->shutdown && pool->task_count == 0) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        /* Get task from queue */
        Task task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % QUEUE_MAX_SIZE;
        pool->task_count--;
        
        pthread_cond_signal(&pool->queue_not_full);
        pthread_mutex_unlock(&pool->queue_mutex);
        
        /* Execute task */
        if (task.function) task.function(task.arg);
    }
    
    return NULL;
}

void* receptionist_thread(void *arg) {
    HospitalState *state = (HospitalState *)arg;
    char log_msg[256];
    
    snprintf(log_msg, sizeof(log_msg), "Receptionist thread started");
    log_system_event(log_msg);
    
    char buffer[1024];
    while (state->system_running) {
        if (read_from_triage_pipe(buffer, sizeof(buffer)) > 0) {
            /* Parse patient data: "name,age,severity" */
            char *name = strtok(buffer, ",");
            char *age_str = strtok(NULL, ",");
            char *severity_str = strtok(NULL, ",");
            
            if (name && age_str && severity_str) {
                Patient patient;
                patient.id = state->patient_count++;
                strncpy(patient.name, name, MAX_NAME_LEN - 1);
                patient.name[MAX_NAME_LEN-1] = '\0';
                patient.age = atoi(age_str);
                patient.severity = atoi(severity_str);
                patient.arrival_time = time(NULL);
                
                /* Map severity to priority */
                switch (patient.severity) {
                    case SEVERITY_EMERGENCY:
                        patient.priority = PRIORITY_EMERGENCY;
                        break;
                    case SEVERITY_CRITICAL:
                        patient.priority = PRIORITY_CRITICAL;
                        break;
                    case SEVERITY_HIGH:
                        patient.priority = PRIORITY_HIGH;
                        break;
                    case SEVERITY_MEDIUM:
                        patient.priority = PRIORITY_MEDIUM;
                        break;
                    default:
                        patient.priority = PRIORITY_LOW;
                }
                
                /* Assign bed type based on severity */
                if (patient.severity >= SEVERITY_CRITICAL) {
                    patient.required_bed = BED_ICU;
                } else if (patient.severity == SEVERITY_HIGH) {
                    patient.required_bed = BED_ISOLATION;
                } else {
                    patient.required_bed = BED_GENERAL;
                }
                
                snprintf(log_msg, sizeof(log_msg), "Receptionist admitted patient: %s (Priority: %d)", 
                        patient.name, patient.priority);
                log_system_event(log_msg);
            }
        }
        usleep(100000); /* 100ms sleep */
    }
    return NULL;
}

void* scheduler_thread(void *arg) {
    HospitalState *state = (HospitalState *)arg;
    char log_msg[256];
    
    snprintf(log_msg, sizeof(log_msg), "Scheduler thread started");
    log_system_event(log_msg);
    
    while (state->system_running) {
        pthread_mutex_lock(&state->mutex);
        
        /* Wait for available beds if needed */
        while (state->waiting_count >= TOTAL_BEDS && state->system_running) {
            pthread_cond_wait(&state->bed_available, &state->mutex);
        }
        
        if (!state->system_running) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }
        
        /* Find and assign bed (simplified - would use memory manager) */
        for (int i = 0; i < TOTAL_BEDS; i++) {
            if (!state->beds[i].occupied) {
                state->beds[i].occupied = 1;
                state->waiting_count--;
                
                snprintf(log_msg, sizeof(log_msg), "Scheduler assigned bed %d", i);
                log_system_event(log_msg);
                
                /* Signal nurse thread */
                break;
            }
        }
        
        pthread_mutex_unlock(&state->mutex);
        usleep(500000); /* 500ms sleep */
    }
    return NULL;
}

void* nurse_thread(void *arg) {
    HospitalState *state = (HospitalState *)arg;
    char log_msg[256];
    
    snprintf(log_msg, sizeof(log_msg), "Nurse thread started");
    log_system_event(log_msg);
    
    int discharge_fd = open(DISCHARGE_FIFO, O_RDONLY | O_NONBLOCK);
    if (discharge_fd < 0) discharge_fd = -1;
    
    while (state->system_running) {
        /* Check for discharge messages */
        char buffer[256];
        if (discharge_fd >= 0) {
            ssize_t r = read(discharge_fd, buffer, sizeof(buffer)-1);
            if (r > 0) {
                buffer[r] = '\0';
                if (strncmp(buffer, "DISCHARGE:", 10) == 0) {
                    int patient_id = -1, bed_id = -1;
                    sscanf(buffer, "DISCHARGE:%d:%d", &patient_id, &bed_id);
                    
                    pthread_mutex_lock(&state->mutex);
                    
                    if (bed_id >= 0 && bed_id < TOTAL_BEDS) {
                        state->beds[bed_id].occupied = 0;
                        state->beds[bed_id].patient_id = -1;
                        
                        snprintf(log_msg, sizeof(log_msg), "Nurse discharged patient %d from bed %d", 
                                patient_id, bed_id);
                        log_system_event(log_msg);
                        
                        /* Signal scheduler that bed is available */
                        pthread_cond_signal(&state->bed_available);
                    }
                    
                    pthread_mutex_unlock(&state->mutex);
                }
            }
        }
        
        usleep(1000000); /* 1 second sleep */
    }
    
    if (discharge_fd >= 0) close(discharge_fd);
    return NULL;
}