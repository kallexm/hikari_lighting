cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(slab_glower_test)

# create mock
cmock_handle(${HIKARI_DIR}/include/slab.h)
cmock_handle(${HIKARI_DIR}/include/slab_event.h)
cmock_handle(${HIKARI_DIR}/lib/slab/events/slab_event_hsv.h)
cmock_handle(${HIKARI_DIR}/lib/slab/events/slab_event_tick.h)
cmock_handle(${HIKARI_DIR}/include/glow_func.h)
#cmock_handle(${ZEPHYR_BASE}/include/zephyr/random/rand32.h)

# generate runner for the test
test_runner_generate(src/slab_glower_test.c)

# add test file
target_sources(app PRIVATE src/slab_glower_test.c)

# add unit under test
#target_sources(app PRIVATE ${HIKARI_DIR}/lib/slab/slab_glower.c)

# include paths
#target_include_directories(app PRIVATE ${HIKARI_DIR}/include)
