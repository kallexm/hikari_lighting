set(HIKARI_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE FILEPATH "Hikari project path")

message(STATUS "HIKARI_DIR: ${HIKARI_DIR}")

zephyr_include_directories(${HIKARI_DIR}/include)

add_subdirectory(lib)
