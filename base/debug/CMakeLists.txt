

zephyr_library_sources_ifdef(CONFIG_CORTEXM_DWT cortexm/dwt.c)

zephyr_library_sources(system_trace.c)