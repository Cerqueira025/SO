#!/bin/bash

run_tests() {
    local num_tests=$1

    for ((i = num_tests; i >= 1; i--)); do
        sleep_time=$i
        ms_time=$((sleep_time * 1000))

        bin/client execute $ms_time -u "sleep $sleep_time"
    done

    echo "All requests sent completed."
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <num_tests>"
    exit 1
fi

run_tests $1
