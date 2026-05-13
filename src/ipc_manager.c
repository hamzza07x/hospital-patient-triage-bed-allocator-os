#include "../include/ipc_manager.h"
#include "../include/logger.h"
#include <sys/shm.h>
#include <sys/sem.h>

int init_shared_memory(HospitalState **state) {
    int shm_id;
    key_t key = ftok("/tmp", SHM_KEY);
    
    // Create shared memory segment
    shm_id = shmget(key, sizeof(HospitalState), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        return -1;
    }
    
    // Attach shared memory
    *state = (HospitalState *)shmat(shm_id, NULL, 0);
    if (*state == (void *)-1) {
        perror("shmat failed");
        return -1;
    }
    
    return shm_id;
}

void cleanup_shared_memory(HospitalState *state) {
    if (state) {
        shmdt(state);
    }
    
    key_t key = ftok("/tmp", SHM_KEY);
    int shm_id = shmget(key, 0, 0);
    if (shm_id >= 0) {
        shmctl(shm_id, IPC_RMID, NULL);
    }
}

int init_semaphores() {
    key_t key = ftok("/tmp", SEM_KEY);
    int sem_id = semget(key, 3, IPC_CREAT | 0666);
    
    if (sem_id < 0) {
        perror("semget failed");
        return -1;
    }
    
    // Initialize semaphores
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } argument;
    
    unsigned short values[3] = {ICU_CAPACITY, ISOLATION_CAPACITY, GENERAL_CAPACITY};
    argument.array = values;
    
    if (semctl(sem_id, 0, SETALL, argument) < 0) {
        perror("semctl failed");
        return -1;
    }
    
    return sem_id;
}

void cleanup_semaphores(int sem_id) {
    if (sem_id >= 0) {
        semctl(sem_id, 0, IPC_RMID);
    }
}

int create_pipes() {
    // Create named pipe for triage
    if (mkfifo(TRIAGE_PIPE, 0666) < 0) {
        if (errno != EEXIST) {
            perror("Failed to create triage pipe");
            return -1;
        }
    }
    return 0;
}

int create_fifos() {
    // Create discharge FIFO
    if (mkfifo(DISCHARGE_FIFO, 0666) < 0) {
        if (errno != EEXIST) {
            perror("Failed to create discharge FIFO");
            return -1;
        }
    }
    return 0;
}

void cleanup_fifos() {
    unlink(TRIAGE_PIPE);
    unlink(DISCHARGE_FIFO);
}

int read_from_triage_pipe(char *buffer, size_t size) {
    int fd = open(TRIAGE_PIPE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open triage pipe for reading");
        return -1;
    }
    
    ssize_t bytes_read = read(fd, buffer, size - 1);
    close(fd);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return bytes_read;
    }
    
    return -1;
}

int write_to_discharge_fifo(int patient_id, int bed_id) {
    int fd = open(DISCHARGE_FIFO, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open discharge FIFO");
        return -1;
    }
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "DISCHARGE:%d:%d", patient_id, bed_id);
    write(fd, buffer, strlen(buffer) + 1);
    close(fd);
    return 0;
}