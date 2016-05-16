#!/bin/bash
# run script
# removes old run file
# removes old output files
# recompiles code

rm run
rm output.txt
gcc pcb.c queue.c errors.c discontinuities.c -o run -pthread

