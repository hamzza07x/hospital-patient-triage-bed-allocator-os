#!/bin/bash

PROJECT_DIR="/home/hamzza07x/1/OS project"
cd "$PROJECT_DIR"

echo "╔══════════════════════════════════════════╗"
echo "║  HOSPITAL SYSTEM - COMPLETE DEMO        ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# SECTION 1: Build
echo "=== SECTION 1: CLEAN BUILD ==="
make clean > /dev/null 2>&1
make
echo ""

# SECTION 2: Start
echo "=== SECTION 2: START SYSTEM ==="
./scripts/start_hospital.sh
sleep 3

# SECTION 3: Register patients
echo ""
echo "=== SECTION 3: REGISTER PATIENTS ==="
./scripts/triage.sh "John Emergency" 45 5
sleep 1
./scripts/triage.sh "Alice Critical" 62 4
sleep 1
./scripts/triage.sh "Bob High" 30 3
sleep 1
./scripts/triage.sh "Carol Medium" 25 2
sleep 1
./scripts/triage.sh "David Low" 35 1
sleep 8

# SECTION 4: Show logs
echo ""
echo "=== SECTION 4: SYSTEM LOGS ==="
echo "--- System Log ---"
cat logs/system_log.txt 2>/dev/null | tail -20
echo ""
echo "--- Schedule Log ---"
cat logs/schedule_log.txt 2>/dev/null
echo ""
echo "--- Memory Log ---"
cat logs/memory_log.txt 2>/dev/null
echo ""

# SECTION 5: Concurrent patients
echo "=== SECTION 5: CONCURRENT PATIENTS ==="
./scripts/triage.sh "Emma Urgent" 55 5 &
./scripts/triage.sh "Frank Severe" 40 4 &
./scripts/triage.sh "Grace Moderate" 28 3 &
./scripts/triage.sh "Henry Mild" 33 2 &
./scripts/triage.sh "Iris Stable" 22 1 &
wait
sleep 5

# SECTION 6: Final logs
echo ""
echo "=== SECTION 6: UPDATED LOGS ==="
echo "--- System Log (last 10 lines) ---"
tail -10 logs/system_log.txt
echo ""

# SECTION 7: Stop
echo "=== SECTION 7: STOP SYSTEM ==="
./scripts/stop_hospital.sh

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║         DEMO COMPLETE!                   ║"
echo "╚══════════════════════════════════════════╝"
echo ""
echo "Log files:"
echo "  logs/system_log.txt"
echo "  logs/schedule_log.txt"
echo "  logs/memory_log.txt"
echo ""
echo "Report: PROJECT_REPORT.md"
echo "Project: /home/hamzza07x/1/OS project/"
