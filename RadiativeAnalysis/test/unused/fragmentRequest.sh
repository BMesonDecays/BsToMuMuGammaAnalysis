#!/bin/bash
seasons=("Summer" "Spring" "Fall")
years=("22" "23" "24")
base_url="https://cms-pdmv-prod.web.cern.ch/mcm/public/restapi/requests/get_fragment/"
base_id="TSG-Run3"

for year in "${years[@]}"; do
  for season in "${seasons[@]}"; do
    for i in {20..30}; do
      request_id="${base_id}${season}${year}EEGS-000$i"
      output_file="AskingForTemplate/${request_id}.py"
      
      curl -s --insecure "${base_url}${request_id}" --retry 2 --create-dirs -o "${output_file}"
      
      if grep -q "404 Not Found" "${output_file}"; then
        echo "File for ${request_id} not found, removing placeholder."
        rm -f "${output_file}"
      else
        echo "File for ${request_id} downloaded successfully."
      fi
    done
  done
done

