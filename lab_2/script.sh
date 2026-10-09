#!/bin/bash

exit_handler() {
    local rc=$?
    trap - EXIT
    docker compose down
    exit $rc    
}

trap exit_handler HUP INT QUIT PIPE TERM


if [ $1 -eq "stop" ]; then
    docker compose down
fi

if [ -z $1 ]; then
    echo "Usage: ./script <replicas>"
    exit 1
elif [ $1 -ge 1 ]; then
    replicas=$1
else
    echo "Argument must be greather or equal 1"
    exit 1    
fi

if ! docker volume inspect shared_volume > /dev/null 2>&1; then
    docker volume create shared_volume 
fi

docker compose down

docker run --rm -v shared_volume:/shared alpine sh -c 'rm -f /shared/*'

docker compose up --build --scale competitors="$replicas"
