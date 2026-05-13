#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define MAX_PATIENTS 50
#define MAX_NAME_LEN 50
#define ICU_CAPACITY 3
#define ISOLATION_CAPACITY 2
#define GENERAL_CAPACITY 5
#define TOTAL_BEDS 10

/* Bed types */
typedef enum {
    BED_GENERAL,
    BED_ICU,
    BED_ISOLATION,
    BED_EMPTY
} BedType;

/* Patient severity levels */
typedef enum {
    SEVERITY_LOW = 1,
    SEVERITY_MEDIUM = 2,
    SEVERITY_HIGH = 3,
    SEVERITY_CRITICAL = 4,
    SEVERITY_EMERGENCY = 5
} Severity;

/* Priority mapping (lower number = higher priority) */
typedef enum {
    PRIORITY_EMERGENCY = 0,
    PRIORITY_CRITICAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_MEDIUM = 3,
    PRIORITY_LOW = 4
} Priority;

/* Patient structure */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int age;
    Severity severity;
    Priority priority;
    time_t arrival_time;
    time_t treatment_start;
    time_t discharge_time;
    BedType required_bed;
    int bed_id;
    pid_t process_id;
} Patient;

/* Bed structure */
typedef struct {
    int id;
    BedType type;
    int occupied;
    int patient_id;
    size_t size; /* For memory management simulation */
    void* address; /* For memory management simulation */
} Bed;

/* Shared memory structure */
typedef struct {
    Bed beds[TOTAL_BEDS];
    int patient_count;
    int waiting_count;
    pthread_mutex_t mutex;
    pthread_cond_t bed_available;
    sem_t icu_sem;
    sem_t isolation_sem;
    int system_running;
} HospitalState;

/* Scheduling algorithms */
typedef enum {
    FCFS,
    SJF,
    PRIORITY_SCHED,
    ROUND_ROBIN
} SchedulingAlgorithm;

/* Memory allocation algorithms */
typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} AllocationAlgorithm;

#endif /* COMMON_H */
