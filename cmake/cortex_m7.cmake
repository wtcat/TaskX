# Name of the target
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m7)

set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

set(MCPU_FLAGS "-mthumb -mcpu=cortex-m7")
set(VFP_FLAGS "-mfpu=fpv5-d16 -mfloat-abi=hard")
set(SPEC_FLAGS "--specs=nosys.specs")
set(LD_FLAGS "-Wl,-T${CMAKE_CURRENT_LIST_DIR}/cortexm.ld -Wl,-Map=app.map")

include(${CMAKE_CURRENT_LIST_DIR}/arm-none-eabi.cmake)