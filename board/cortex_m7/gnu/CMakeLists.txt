
target_sources(${PROJECT_NAME} PRIVATE
    # {{BEGIN_TARGET_SOURCES}}
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_restore.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_save.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_control.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_schedule.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_stack_build.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_system_return.S
	${CMAKE_CURRENT_LIST_DIR}/src/tx_timer_interrupt.S
    # {{END_TARGET_SOURCES}}

	${CMAKE_CURRENT_LIST_DIR}/start.S
	${CMAKE_CURRENT_LIST_DIR}/armv7m_crt.c
	${CMAKE_CURRENT_LIST_DIR}/armv7m_cache.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
	${CMAKE_CURRENT_LIST_DIR}/..
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../stm32h7 soc)
