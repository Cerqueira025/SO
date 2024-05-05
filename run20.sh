#!/bin/bash

random_sleep() {
    echo $((RANDOM % 10 + 1))
}

run_tests() {
    local num_tests=20

    for ((i = 1; i <= num_tests; i++)); do
        sleep_time=$(random_sleep)
        ms_time=$((sleep_time * 1000))

        bin/client execute $ms_time -u "sleep $sleep_time"
    done

    echo "All requests sent completed."
}

run_tests

