#!/bin/bash

# Hospital Patient Triage Script
# Usage: ./triage.sh <name> <age> <severity>

# Validate input
if [ $# -ne 3 ]; then
    echo "Error: Invalid arguments"
    echo "Usage: $0 <name> <age> <severity>"
    echo "Severity levels: 1-Low, 2-Medium, 3-High, 4-Critical, 5-Emergency"
    exit 1
fi

NAME=$1
AGE=$2
SEVERITY=$3

# Validate name (only letters and spaces)
if ! [[ "$NAME" =~ ^[a-zA-Z\ ]+$ ]]; then
    echo "Error: Name must contain only letters and spaces"
    exit 1
fi

# Validate age (positive integer, 0-150)
if ! [[ "$AGE" =~ ^[0-9]+$ ]] || [ "$AGE" -lt 0 ] || [ "$AGE" -gt 150 ]; then
    echo "Error: Age must be a number between 0 and 150"
    exit 1
fi

# Validate severity (1-5)
if ! [[ "$SEVERITY" =~ ^[1-5]$ ]]; then
    echo "Error: Severity must be between 1 and 5"
    echo "1-Low, 2-Medium, 3-High, 4-Critical, 5-Emergency"
    exit 1
fi

# Map severity to priority
case $SEVERITY in
    5) PRIORITY="EMERGENCY" ;;
    4) PRIORITY="CRITICAL" ;;
    3) PRIORITY="HIGH" ;;
    2) PRIORITY="MEDIUM" ;;
    1) PRIORITY="LOW" ;;
esac

# Display triage result
echo "====================================="
echo "PATIENT TRIAGE RESULT"
echo "====================================="
echo "Name: $NAME"
echo "Age: $AGE"
echo "Severity Level: $SEVERITY"
echo "Priority: $PRIORITY"
echo "====================================="

# Send to hospital system via named pipe
PIPE_PATH="/tmp/triage_pipe"

if [ ! -p "$PIPE_PATH" ]; then
    echo "Error: Hospital system not running (pipe not found)"
    echo "Please start the hospital system first: ./start_hospital.sh"
    exit 1
fi

# Send data as CSV format
echo "$NAME,$AGE,$SEVERITY" > "$PIPE_PATH"

if [ $? -eq 0 ]; then
    echo "Patient successfully registered in hospital system"
else
    echo "Error: Failed to send patient data to hospital system"
    exit 1
fi