cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(refctr_test)

# generate runner for the test
test_runner_generate(src/refctr_test.c)

# add test file
target_sources(app PRIVATE src/refctr_test.c)
