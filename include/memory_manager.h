#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "common.h"

// Memory block for simulation
typedef struct MemoryBlock {
    size_t size;
    int free;
    int bed_id;
    struct MemoryBlock *next;
} MemoryBlock;

// Memory manager functions
MemoryBlock* init_memory_pool(size_t total_size);
int allocate_bed_first_fit(MemoryBlock *head, size_t size);
int allocate_bed_best_fit(MemoryBlock *head, size_t size);
int allocate_bed_worst_fit(MemoryBlock *head, size_t size);
void free_bed(MemoryBlock *head, int bed_id);
void coalesce_free_blocks(MemoryBlock *head);
float calculate_external_fragmentation(MemoryBlock *head);
float calculate_internal_fragmentation(size_t allocated, size_t required);
void print_memory_state(MemoryBlock *head, FILE *log_file);
void cleanup_memory_pool(MemoryBlock *head);

#endif