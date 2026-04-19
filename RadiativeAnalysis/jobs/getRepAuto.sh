#!/usr/bin/bash

grep -B 6 "Status on the scheduler:" status.txt | grep -B 6 "FAILED" | grep "CRAB project directory" | cut -f 3 | cut -d '/' -f 10-11 \
| xargs -I {} crab report --dir {} --outputdir reports/{}