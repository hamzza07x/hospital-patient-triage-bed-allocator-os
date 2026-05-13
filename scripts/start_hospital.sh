#!/bin/bash

echo "====================================="
echo "Starting Hospital Management System"
echo "====================================="

# Create necessary directories
mkdir -p logs

# Clean up any existing IPC resources
echo "Cleaning up old IPC resources..."
rm -f /tmp/triage_pipe
rm -f /tmp/discharge_fifo
rm -f /tmp/patient_fifo_*

# Initialize semaphores and shared memory
ipcrm -a 2>/dev/null

# Compile the system if needed
if [ ! -f "./admissions" ] || [ ! -f "./patient_simulator" ]; then
    echo "Building system..."
    make clean
    make
fi

# Start the hospital system
echo "Starting hospital admission system..."
./admissions &
ADMISSIONS_PID=$!

# Wait for system to initialize
sleep 2

# Check if system is running
if kill -0 $ADMISSIONS_PID 2>/dev/null; then
    echo "Hospital system started successfully (PID: $ADMISSIONS_PID)"
    echo ""
    echo "=== HOW TO USE ==="
    echo "1. Register patients:  ./triage.sh <name> <age> <severity>"
    echo "2. Check logs:         tail -f logs/system_log.txt"
    echo "3. Stop system:        ./stop_hospital.sh"
    echo ""
    echo "Severity Levels:"
    echo "  1 = Low (General Ward)"
    echo "  2 = Medium (General Ward)"
    echo "  3 = High (Isolation)"
    echo "  4 = Critical (ICU)"
    echo "  5 = Emergency (ICU)"
    
    # Save PID for cleanup
    echo $ADMISSIONS_PID > .hospital_pid
else
    echo "Error: Failed to start hospital system"
    exit 1
fi