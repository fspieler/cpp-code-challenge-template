#!/bin/bash

mkdir -p test/inputs
mkdir -p test/expected_outputs
if [ ! -e test/test_number ] ; then
    echo 0 > test/test_number
fi
TEST_NUM=$(expr $(cat test/test_number) + 1)
touch test/inputs/$TEST_NUM test/expected_outputs/$TEST_NUM
echo $TEST_NUM > test/test_number
