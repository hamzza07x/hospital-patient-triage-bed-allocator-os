CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
INCLUDES = -I./include

# Source files
SRCS = src/admissions.c src/patient_simulator.c src/ipc_manager.c \
       src/scheduler.c src/memory_manager.c src/thread_pool.c src/logger.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executables
TARGETS = admissions patient_simulator

.PHONY: all clean run stress_test

all: $(TARGETS)

admissions: src/admissions.o src/ipc_manager.o src/scheduler.o src/memory_manager.o \
            src/thread_pool.o src/logger.o
	$(CC) $(CFLAGS) -o $@ $^

patient_simulator: src/patient_simulator.o src/ipc_manager.o src/logger.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: all
	./start_hospital.sh

stress_test: all
	./scripts/stress_test.sh

clean:
	rm -f $(TARGETS) src/*.o
	rm -f logs/*.txt
	rm -f /tmp/triage_pipe /tmp/discharge_fifo

install:
	mkdir -p logs
	chmod +x scripts/*.sh