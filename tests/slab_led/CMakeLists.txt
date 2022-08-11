cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(slab_rgb_test)

# create mock
cmock_handle(${HIKARI_DIR}/include/slab.h)
cmock_handle(${HIKARI_DIR}/include/slab_event.h)
cmock_handle(${HIKARI_DIR}/lib/slab/events/slab_event_rgb.h)
cmock_handle(${HIKARI_DIR}/include/rgb_hsv.h)

# generate runner for the test
test_runner_generate(src/slab_led_test.c)

# add test file
target_sources(app PRIVATE src/slab_led_test.c)

# add unit under test
#target_sources(app PRIVATE ${HIKARI_DIR}/lib/slab/slab_led.c)

# include paths
#target_include_directories(app PRIVATE ${HIKARI_DIR}/include)
