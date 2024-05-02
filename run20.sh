#!/bin/bash

# Function to generate a random number between 1 and 20
random_sleep() {
    echo $((RANDOM % 10 + 1))
}

# Function to run the tests
run_tests() {
    local num_tests=20

    for ((i = 1; i <= num_tests; i++)); do
        sleep_time=$(random_sleep)
        ms_time=$((sleep_time * 1000))

        echo "Running test $i: ./client execute -u $ms_time sleep $sleep_time"
        ./client execute $ms_time -u "sleep $sleep_time"
    done

    echo "All requests sent completed."
}

# Run the tests
run_tests

