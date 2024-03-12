cmake_minimum_required(VERSION 3.8)

# Set up the project
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(app)
enable_language(C CXX ASM)

include(cmake/extensions.cmake)

# Compile options
include_directories(
  ${PROJECT_SOURCE_DIR}
)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(
    -Wall
    -O2
    -Werror
)

# ThreadX configuration
set(THREADX_CUSTOM_PORT ${PROJECT_SOURCE_DIR}/board)
set(TX_USER_FILE        ${PROJECT_SOURCE_DIR}/board/${THREADX_ARCH}/tx_user.h)


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

add_executable(${APP} 
  main.c
)

collect_link_libraries(deplibs ${APP})
target_link_libraries(${APP}
    ${deplibs}
)

# Post build
add_custom_command(
  TARGET  ${APP}
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -Obinary ${APP} ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJDUMP} -d ${APP} > ${PROJECT_NAME}.lst
)