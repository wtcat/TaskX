cmake_minimum_required(VERSION 3.8)

# Set up the project
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(app)
enable_language(C CXX ASM)

include(cmake/extensions.cmake)
if (CMAKE_HOST_WIN32)
    include(cmake/win32.cmake)
endif()

# Compile options
include_directories(
  ${PROJECT_SOURCE_DIR}
)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_HOST_UNIX)
    add_compile_options(
        -Wall
        -O0
        -ggdb
        # -Werror
    )
endif()

# ThreadX configuration
set(THREADX_CUSTOM_PORT ${PROJECT_SOURCE_DIR}/board)
# set(NETXDUO_CUSTOM_PORT ${THREADX_CUSTOM_PORT})

if (THREAD_LINUX)
  set(TX_EXTRA_LIB   pthread)
else ()
  set(TX_USER_FILE   ${PROJECT_SOURCE_DIR}/board/${THREADX_ARCH}/tx_user.h)
  set(TX_EXTRA_LIB   )
endif()
set(NX_USER_FILE  ${PROJECT_SOURCE_DIR}/board/${THREADX_ARCH}/nx_user.h)

# Core component
add_subdirectory(threadx)

# Network component
add_subdirectory_ifdef(CONFIG_NET netxduo)

# Filesystem component
add_subdirectory_ifdef(CONFIG_FS  filex)

# USB component
add_subdirectory_ifdef(CONFIG_USB usbx)

# Application
add_subdirectory(apps)

# Generate target
set(APP ${PROJECT_NAME}.elf)

if (CONFIG_NET)
  add_executable(${APP} tests/demo_netx_duo_tcp.c)
else ()
  add_executable(${APP} main.c)
endif()

collect_link_libraries(deplibs ${APP})
target_link_libraries(${APP}
    ${deplibs}
    ${TX_EXTRA_LIB}
)

# Post build
if (NOT CMAKE_HOST_WIN32)
    add_custom_command(
      TARGET  ${APP}
      POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} -Obinary ${APP} ${PROJECT_NAME}.bin
      COMMAND ${CMAKE_OBJDUMP} -d ${APP} > ${PROJECT_NAME}.lst
    )
endif()