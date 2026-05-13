#!/bin/bash

echo "====================================="
echo "HOSPITAL SYSTEM STRESS TEST"
echo "====================================="
echo ""

# Start the hospital system if not already running
if ! pgrep -x "admissions" > /dev/null; then
    echo "Starting hospital system..."
    ./scripts/start_hospital.sh
    sleep 3
else
    echo "Hospital system already running"
fi

echo ""
echo "Generating 10+ concurrent patients..."
echo ""

# Patient data arrays
NAMES=("John Smith" "Alice Brown" "Bob Johnson" "Carol White" "David Lee" 
       "Emma Davis" "Frank Miller" "Grace Wilson" "Henry Moore" "Iris Taylor"
       "Jack Anderson" "Kate Thomas" "Larry Jackson" "Mary Martin" "Nick Garcia")

SEVERITIES=(5 4 3 2 1 5 4 3 2 1 5 4 3 2 1)

echo "Starting concurrent patient admissions..."

# Simulate 15 patients arriving over 30 seconds
for i in {0..14}; do
    NAME="${NAMES[$i]}"
    AGE=$((20 + RANDOM % 60))
    SEVERITY="${SEVERITIES[$i]}"
    
    echo "[$(date +%H:%M:%S)] Admitting: $NAME (Age: $AGE, Severity: $SEVERITY)"
    ./scripts/triage.sh "$NAME" "$AGE" "$SEVERITY" &
    
    # Random delay between admissions (0.5-2 seconds)
    sleep 0.$((RANDOM % 5 + 1))
done

# Wait for all admissions to complete
wait

echo ""
echo "All patients admitted. Monitoring system..."
echo ""

# Monitor for 15 seconds
for i in {1..15}; do
    echo -ne "\r[$(date +%H:%M:%S)] Waiting for patients to complete treatment... ($i/15)"
    sleep 1
done

echo ""
echo ""
echo "====================================="
echo "STRESS TEST COMPLETED"
echo "====================================="
echo ""
echo "Patient Statistics:"
echo "  Total Patients: 15"
echo "  Emergency (5): 3"
echo "  Critical (4): 3"
echo "  High (3): 3"
echo "  Medium (2): 3"
echo "  Low (1): 3"
echo ""
echo "Check logs for detailed results:"
echo "  cat logs/system_log.txt"
echo "  cat logs/schedule_log.txt"
echo "  cat logs/memory_log.txt"
echo ""

# Don't stop the system automatically
echo "System is still running. Use ./scripts/stop_hospital.sh to stop it."
