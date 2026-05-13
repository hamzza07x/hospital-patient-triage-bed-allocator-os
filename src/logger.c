#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include "../include/logger.h"

FILE *schedule_log = NULL;
FILE *memory_log = NULL;
FILE *system_log = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int init_logging() {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        if (mkdir("logs", 0755) < 0) {
            return -1;
        }
    }

    schedule_log = fopen("logs/schedule_log.txt", "a");
    memory_log = fopen("logs/memory_log.txt", "a");
    system_log = fopen("logs/system_log.txt", "a");
    if (!schedule_log || !memory_log || !system_log) {
        return -1;
    }
    return 0;
}

void close_logging() {
    pthread_mutex_lock(&log_mutex);
    if (schedule_log) fclose(schedule_log);
    if (memory_log) fclose(memory_log);
    if (system_log) fclose(system_log);
    schedule_log = memory_log = system_log = NULL;
    pthread_mutex_unlock(&log_mutex);
}

static char* timestamp_now(void) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char *buf = malloc(32);
    if (!buf) return NULL;
    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

void log_schedule_event(const char *event) {
    if (!schedule_log) return;
    char *ts = timestamp_now();
    pthread_mutex_lock(&log_mutex);
    fprintf(schedule_log, "%s - %s\n", ts ? ts : "", event);
    fflush(schedule_log);
    pthread_mutex_unlock(&log_mutex);
    free(ts);
}

void log_memory_event(const char *event) {
    if (!memory_log) return;
    char *ts = timestamp_now();
    pthread_mutex_lock(&log_mutex);
    fprintf(memory_log, "%s - %s\n", ts ? ts : "", event);
    fflush(memory_log);
    pthread_mutex_unlock(&log_mutex);
    free(ts);
}

void log_system_event(const char *event) {
    if (!system_log) return;
    char *ts = timestamp_now();
    pthread_mutex_lock(&log_mutex);
    fprintf(system_log, "%s - %s\n", ts ? ts : "", event);
    fflush(system_log);
    pthread_mutex_unlock(&log_mutex);
    free(ts);
}

char* get_timestamp() {
    return timestamp_now();
}
