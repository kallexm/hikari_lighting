cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(slab_test)

# create mock
cmock_handle(${HIKARI_DIR}/include/slab_event.h)
cmock_handle(${HIKARI_DIR}/include/slabs/slab_led.h)
cmock_handle(${HIKARI_DIR}/include/slabs/slab_delay.h)
cmock_handle(${HIKARI_DIR}/include/slabs/slab_glower.h)
cmock_handle(${HIKARI_DIR}/include/slabs/slab_hsv2rgb.h)
cmock_handle(${HIKARI_DIR}/include/slabs/slab_rgb2hsv.h)

cmock_handle(${ZEPHYR_BASE}/include/zephyr/sys/dlist.h)

# generate runner for the test
test_runner_generate(src/slab_test.c)

# add test file
target_sources(app PRIVATE src/slab_test.c)

# add unit under test
#target_sources(app PRIVATE ${HIKARI_DIR}/lib/slab/slab.c)

# include paths
#target_include_directories(app PRIVATE ${HIKARI_DIR}/include)
