#!/usr/bin/bash

grep -B 8 "Jobs status:" status.txt | grep -B 8 "failed" | grep "CRAB project directory" | cut -f 3 | cut -d '/' -f 10-11 \
| xargs -I {} crab report --dir {} --outputdir reports/{}