cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(slab_base_test)

# create mock
#cmock_handle(${ZEPHYR_HAL_NORDIC_MODULE_DIR}/nrfx/drivers/include/nrfx_uarte.h )

# generate runner for the test
test_runner_generate(src/slab_base.c)

target_include_directories(app PRIVATE src)

# add test file
target_sources(app PRIVATE src/slab_base.c)

# add unit under test
target_sources(app PRIVATE ${HIKARI_DIR}/lib/slab_base/slab_base.c)

# include paths
target_include_directories(app PRIVATE ${HIKARI_DIR}/include)
