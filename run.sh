#!/bin/bash

run_tests() {
    local num_tests=$1
    local sleep_limit=$2

    for ((i = num_tests; i >= 1; i--)); do
        sleep_time=$((i % sleep_limit + 1))
        ms_time=$((sleep_time * 1000))

        echo "Running task $i: bin/client execute -u $ms_time 'sleep $sleep_time'"
        bin/client execute $ms_time -u "sleep $sleep_time"
    done

    echo "All requests sent completed."
}

if [ $# -ne 2 ]; then
    echo "Usage: $0 num_tasks sleep_limit"
    exit 1
fi

run_tests $1 $2
