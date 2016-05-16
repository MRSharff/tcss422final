#!/bin/bash
# debug script
# removes old run file
# removes old output files
# recompiles code with -g flag

rm run
rm output.txt
gcc -g pcb.c queue.c errors.c discontinuities.c -o run -pthread

