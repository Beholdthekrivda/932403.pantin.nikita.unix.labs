#!/bin/sh

id=$(hostname)
n=0

exec 9>/shared/.lock

while :; do
    flock 9
    i=1 
    while :; do
        file=$(printf '%03d' "$i")
	if [ ! -e "/shared/$file" ]; then
	    break
	fi
	i=$((i+1))
    done	
    
    echo "$id $((n+1))" > /shared/"$file"
    flock -u 9
    n=$((n+1))
    sleep 1
    rm -f /shared/"$file"
    sleep 1
done
