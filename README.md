# Hospital Patient Triage & Bed Allocator System

Production-quality Operating Systems project built in C using POSIX APIs on Linux.

This project simulates a real-world hospital environment where patients are registered, prioritized according to severity, assigned beds, treated concurrently, and discharged automatically.

The system demonstrates core Operating System concepts including:

- Process Management
- Inter-Process Communication (IPC)
- Thread Synchronization
- CPU Scheduling Algorithms
- Shared Memory
- Semaphores and Mutexes
- Dynamic Memory Allocation
- Signal Handling
- Concurrent Processing

---

# Features

## Patient Triage System
- Patient registration using shell scripts
- Severity-based prioritization
- ICU and isolation ward handling
- Automated patient discharge

## Multi-Process Architecture
- Separate patient simulation processes
- `fork()` and `exec()` based execution
- Concurrent patient handling

## IPC Mechanisms
- Named Pipes (FIFOs)
- Shared Memory
- POSIX Semaphores

## Thread Synchronization
- Mutex locks
- Condition variables
- Thread-safe queues
- Resource synchronization

## Scheduling Algorithms
- Priority Scheduling
- FCFS (First Come First Served)
- Shortest Job First (SJF)
- Round Robin Scheduling

## Custom Memory Manager
- Best-Fit Allocation
- First-Fit Allocation
- Worst-Fit Allocation
- Fragmentation tracking
- Memory coalescing

## Logging System
- System activity logs
- Scheduling logs
- Memory allocation logs

---

# System Architecture

```text
triage.sh --> Named Pipe --> admissions (Main Process)
                                 |
                                 |--> Receptionist Thread --> Patient Queue
                                 |
                                 |--> Scheduler Thread --> Bed Manager
                                 |
                                 |--> Nurse Thread --> Discharge FIFO
                                 |
                                 |--> fork()/exec() --> patient_simulator Processes
```

---

# Project Structure

```text
.
├── src/
├── include/
├── scripts/
├── logs/
├── Makefile
└── README.md
```

---

# Technologies Used

| Component | Technology |
|---|---|
| Language | C |
| Platform | Linux |
| Compiler | GCC |
| APIs | POSIX APIs |
| Concurrency | pthreads |
| IPC | FIFO, Shared Memory, Semaphores |
| Build System | Makefile |

---

# Build Instructions

## 1. Clone Repository

```bash
git clone https://github.com/YOUR_USERNAME/hospital-patient-triage-bed-allocator-os.git

cd hospital-patient-triage-bed-allocator-os
```

---

## 2. Compile Project

```bash
make clean && make
```

---

## 3. Start Hospital System

```bash
./scripts/start_hospital.sh
```

---

## 4. Register Patients

```bash
./scripts/triage.sh "John Emergency" 45 5

./scripts/triage.sh "Alice Critical" 62 4

./scripts/triage.sh "Bob High" 30 3
```

Severity Levels:

| Severity | Priority |
|---|---|
| 5 | Emergency |
| 4 | Critical |
| 3 | High |
| 2 | Medium |
| 1 | Low |

---

## 5. View Logs

```bash
cat logs/system_log.txt

cat logs/schedule_log.txt

cat logs/memory_log.txt
```

---

## 6. Stop System

```bash
./scripts/stop_hospital.sh
```

---

# Complete Project Demo

Run the complete simulation:

```bash
chmod +x scripts/sample.sh

./scripts/sample.sh
```

The demo includes:

- Clean build
- System startup
- Patient registration
- Concurrent patient simulation
- Scheduling
- Memory allocation
- Logging
- IPC communication
- Automatic discharge
- System shutdown

---

# Example Output

```text
PATIENT TRIAGE RESULT

Name: John Emergency
Age: 45
Severity Level: 5
Priority: EMERGENCY

Patient successfully registered in hospital system
Patient 0 arrived at hospital
Patient 0 started treatment
```

---

# Operating System Concepts Demonstrated

## Process Management
- `fork()`
- `exec()`
- process lifecycle handling

## Inter-Process Communication
- FIFOs
- Shared Memory
- Semaphores

## Synchronization
- Mutexes
- Condition Variables
- Producer-Consumer coordination

## CPU Scheduling
- Priority Scheduling
- FCFS
- SJF
- Round Robin

## Memory Management
- Dynamic allocation
- Best-Fit strategy
- First-Fit strategy
- Worst-Fit strategy
- Fragmentation analysis

---

# Modules

| Module | Responsibility |
|---|---|
| admissions.c | Main controller process |
| patient_simulator.c | Patient lifecycle simulation |
| ipc_manager.c | IPC handling |
| scheduler.c | Scheduling algorithms |
| memory_manager.c | Memory allocation |
| thread_pool.c | Thread management |
| logger.c | Logging system |

---

# Performance Metrics

| Metric | Value |
|---|---|
| Compilation Warnings | 0 |
| Maximum Concurrent Patients | 15 |
| Scheduling Algorithms | 4 |
| Memory Strategies | 3 |
| Memory Pool Size | 4096 bytes |

---

# Future Improvements

- GUI dashboard
- Real-time monitoring
- Database integration
- Network-based hospital communication
- Web-based administration panel
- Advanced scheduling optimization
- Patient history management

---

# Learning Outcomes

This project provides practical understanding of:

- Linux system programming
- Concurrent programming
- Thread synchronization
- Process coordination
- IPC mechanisms
- Scheduling algorithms
- Memory management
- Production-quality modular architecture

---

# Author

Muhammad Hamza  
Computer Science Student

---

# License

This project is licensed under the MIT License.
