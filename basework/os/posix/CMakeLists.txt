zephyr_include_directories(.)

target_sources(basework
    PRIVATE
    os_platform.cc
    os_vflash_impl.cc
    os_timer_impl.c
)

if (CONFIG_XIPFS)
    target_sources(basework
        PRIVATE
        os_xipfs_impl.cc
    )
endif()

if (CONFIG_PTFS)
    target_sources(basework
        PRIVATE
        os_ptfs_impl.cc
    )
endif()
