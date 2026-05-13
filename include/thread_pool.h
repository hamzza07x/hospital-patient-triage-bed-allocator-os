#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"

#define THREAD_POOL_SIZE 4
#define QUEUE_MAX_SIZE 100

// Thread pool task
typedef struct {
    Patient patient;
    void (*function)(void *);
    void *arg;
} Task;

// Thread pool
typedef struct {
    pthread_t threads[THREAD_POOL_SIZE];
    Task task_queue[QUEUE_MAX_SIZE];
    int queue_front;
    int queue_rear;
    int task_count;
    int shutdown;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
} ThreadPool;

// Thread pool functions
ThreadPool* create_thread_pool();
void destroy_thread_pool(ThreadPool *pool);
int add_task(ThreadPool *pool, Task task);
void* worker_thread(void *arg);
void* receptionist_thread(void *arg);
void* scheduler_thread(void *arg);
void* nurse_thread(void *arg);

#endif