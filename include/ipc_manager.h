#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include "common.h"

#define SHM_KEY 1234
#define SEM_KEY 5678
#define TRIAGE_PIPE "/tmp/triage_pipe"
#define DISCHARGE_FIFO "/tmp/discharge_fifo"
#define PATIENT_FIFO_PREFIX "/tmp/patient_fifo_"

// IPC Manager functions
int init_shared_memory(HospitalState **state);
void cleanup_shared_memory(HospitalState *state);
int init_semaphores();
void cleanup_semaphores(int sem_id);
int create_pipes();
int create_fifos();
void cleanup_fifos();
int read_from_triage_pipe(char *buffer, size_t size);
int write_to_discharge_fifo(int patient_id, int bed_id);

#endif