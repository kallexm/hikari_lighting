add_subdirectory_ifdef(CONFIG_ADRLEDRGB adrledrgb)
add_subdirectory(funcs)
add_subdirectory(rgb_hsv)
add_subdirectory(slab)

add_subdirectory_ifdef(CONFIG_HSM hsm)
add_subdirectory_ifdef(CONFIG_MSGPORT msgport)
add_subdirectory_ifdef(CONFIG_RBUF rbuf)

add_subdirectory(refctr)
