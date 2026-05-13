#ifndef LOGGER_H
#define LOGGER_H

#include "common.h"

// Log files
extern FILE *schedule_log;
extern FILE *memory_log;
extern FILE *system_log;

// Logger functions
int init_logging();
void close_logging();
void log_schedule_event(const char *event);
void log_memory_event(const char *event);
void log_system_event(const char *event);
char* get_timestamp();

#endif
