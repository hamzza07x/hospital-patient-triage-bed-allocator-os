#include "../include/memory_manager.h"
#include "../include/logger.h"

MemoryBlock* init_memory_pool(size_t total_size) {
    MemoryBlock *head = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    head->size = total_size;
    head->free = 1;
    head->bed_id = -1;
    head->next = NULL;
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Memory pool initialized with size: %zu bytes", total_size);
    log_memory_event(log_msg);
    
    return head;
}

int allocate_bed_first_fit(MemoryBlock *head, size_t size) {
    MemoryBlock *current = head;
    int bed_id = 0;
    
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Found first fit
            if (current->size > size + sizeof(MemoryBlock)) {
                // Split the block
                MemoryBlock *new_block = (MemoryBlock *)((char *)current + sizeof(MemoryBlock) + size);
                new_block->size = current->size - size - sizeof(MemoryBlock);
                new_block->free = 1;
                new_block->bed_id = -1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            current->bed_id = bed_id++;
            
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "First-Fit: Allocated bed %d of size %zu", 
                    current->bed_id, size);
            log_memory_event(log_msg);
            
            return current->bed_id;
        }
        current = current->next;
    }
    
    return -1; // No suitable block found
}

int allocate_bed_best_fit(MemoryBlock *head, size_t size) {
    MemoryBlock *current = head;
    MemoryBlock *best_fit = NULL;
    size_t min_diff = SIZE_MAX;
    int bed_id = 0;
    
    // Find best fit block
    while (current != NULL) {
        if (current->free && current->size >= size) {
            size_t diff = current->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best_fit = current;
            }
        }
        current = current->next;
    }
    
    if (best_fit == NULL) {
        return -1; // No suitable block found
    }
    
    // Allocate best fit block
    if (best_fit->size > size + sizeof(MemoryBlock)) {
        MemoryBlock *new_block = (MemoryBlock *)((char *)best_fit + sizeof(MemoryBlock) + size);
        new_block->size = best_fit->size - size - sizeof(MemoryBlock);
        new_block->free = 1;
        new_block->bed_id = -1;
        new_block->next = best_fit->next;
        
        best_fit->size = size;
        best_fit->next = new_block;
    }
    
    best_fit->free = 0;
    best_fit->bed_id = bed_id++;
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Best-Fit: Allocated bed %d of size %zu (wasted: %zu)", 
            best_fit->bed_id, size, min_diff);
    log_memory_event(log_msg);
    
    return best_fit->bed_id;
}

int allocate_bed_worst_fit(MemoryBlock *head, size_t size) {
    MemoryBlock *current = head;
    MemoryBlock *worst_fit = NULL;
    size_t max_diff = 0;
    int bed_id = 0;
    
    // Find worst fit block
    while (current != NULL) {
        if (current->free && current->size >= size) {
            size_t diff = current->size - size;
            if (diff > max_diff) {
                max_diff = diff;
                worst_fit = current;
            }
        }
        current = current->next;
    }
    
    if (worst_fit == NULL) {
        return -1; // No suitable block found
    }
    
    // Allocate worst fit block
    if (worst_fit->size > size + sizeof(MemoryBlock)) {
        MemoryBlock *new_block = (MemoryBlock *)((char *)worst_fit + sizeof(MemoryBlock) + size);
        new_block->size = worst_fit->size - size - sizeof(MemoryBlock);
        new_block->free = 1;
        new_block->bed_id = -1;
        new_block->next = worst_fit->next;
        
        worst_fit->size = size;
        worst_fit->next = new_block;
    }
    
    worst_fit->free = 0;
    worst_fit->bed_id = bed_id++;
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Worst-Fit: Allocated bed %d of size %zu (largest block: %zu)", 
            worst_fit->bed_id, size, max_diff + size);
    log_memory_event(log_msg);
    
    return worst_fit->bed_id;
}

void free_bed(MemoryBlock *head, int bed_id) {
    MemoryBlock *current = head;
    
    while (current != NULL) {
        if (current->bed_id == bed_id && !current->free) {
            current->free = 1;
            current->bed_id = -1;
            
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Freed bed %d of size %zu", bed_id, current->size);
            log_memory_event(log_msg);
            
            coalesce_free_blocks(head);
            return;
        }
        current = current->next;
    }
}

void coalesce_free_blocks(MemoryBlock *head) {
    MemoryBlock *current = head;
    int coalesced = 0;
    
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            // Coalesce adjacent free blocks
            current->size += current->next->size + sizeof(MemoryBlock);
            MemoryBlock *temp = current->next;
            current->next = temp->next;
            free(temp);
            coalesced++;
        } else {
            current = current->next;
        }
    }
    
    if (coalesced > 0) {
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Coalesced %d free blocks", coalesced);
        log_memory_event(log_msg);
    }
}

float calculate_external_fragmentation(MemoryBlock *head) {
    size_t total_free = 0;
    size_t largest_free = 0;
    MemoryBlock *current = head;
    
    while (current != NULL) {
        if (current->free) {
            total_free += current->size;
            if (current->size > largest_free) {
                largest_free = current->size;
            }
        }
        current = current->next;
    }
    
    if (total_free == 0) return 0.0;
    
    // External fragmentation = (total_free - largest_free) / total_free * 100
    float fragmentation = ((float)(total_free - largest_free) / total_free) * 100.0;
    return fragmentation;
}

float calculate_internal_fragmentation(size_t allocated, size_t required) {
    if (allocated <= required) return 0.0;
    return ((float)(allocated - required) / allocated) * 100.0;
}

void print_memory_state(MemoryBlock *head, FILE *log_file) {
    MemoryBlock *current = head;
    fprintf(log_file, "\n=== Memory State ===\n");
    fprintf(log_file, "%-10s %-10s %-10s %s\n", "Address", "Size", "Status", "Bed ID");
    fprintf(log_file, "-------------------------------------------\n");
    
    while (current != NULL) {
        fprintf(log_file, "0x%-8lx %-10zu %-10s %d\n", 
                (unsigned long)current, current->size, 
                current->free ? "FREE" : "ALLOC", 
                current->bed_id);
        current = current->next;
    }
    
    float ext_frag = calculate_external_fragmentation(head);
    fprintf(log_file, "\nExternal Fragmentation: %.2f%%\n", ext_frag);
}

void cleanup_memory_pool(MemoryBlock *head) {
    MemoryBlock *current = head;
    while (current != NULL) {
        MemoryBlock *next = current->next;
        free(current);
        current = next;
    }
}