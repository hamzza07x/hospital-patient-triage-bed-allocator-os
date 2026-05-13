#!/bin/bash

echo "Stopping Hospital Management System..."

# Kill admissions process
if pgrep -x "admissions" > /dev/null; then
    echo "Stopping admissions process..."
    pkill -TERM admissions
    sleep 2
    pkill -9 admissions 2>/dev/null
fi

# Kill all patient processes
if pgrep -f "patient_simulator" > /dev/null; then
    echo "Stopping patient processes..."
    pkill -TERM -f patient_simulator
    sleep 1
    pkill -9 -f patient_simulator 2>/dev/null
fi

# Clean up IPC resources
echo "Cleaning up IPC resources..."
rm -f /tmp/triage_pipe
rm -f /tmp/discharge_fifo
rm -f /tmp/patient_fifo_*

# Clean up shared memory and semaphores
ipcrm -a 2>/dev/null

# Remove PID file
rm -f .hospital_pid

echo "Cleanup complete"
