#!/bin/sh
cd "$(dirname "$0")"

set -e

make 01
./01 & 
TID=$!
sleep 0.1 #give time to start

../build/memleax -e 1 $TID &
sleep 1 #give time to attach
(echo sending sighup ...; kill -SIGUSR1 $TID)

wait $TIP
wait 
