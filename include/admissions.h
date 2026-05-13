#ifndef ADMISSIONS_H
#define ADMISSIONS_H

#include "common.h"

/* Global state declarations */
extern HospitalState *hospital_state;
extern int shm_id;

/* System functions */
void initialize_system(void);
void run_system(void);
void cleanup_system(void);
void signal_handler(int sig);

#endif /* ADMISSIONS_H */
