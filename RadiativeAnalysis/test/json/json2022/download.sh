for f in $(curl -s https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions22/ \
    | grep -o 'Cert_Collisions2022_era[A-G]_[0-9_]*_Golden\.json'); do
    wget https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions22/$f
done
