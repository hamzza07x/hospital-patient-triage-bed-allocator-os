#ifndef PATIENT_H
#define PATIENT_H

#include "common.h"

/* Patient process functions */
void patient_lifecycle(int patient_id, BedType bed_type, int severity);

#endif /* PATIENT_H */