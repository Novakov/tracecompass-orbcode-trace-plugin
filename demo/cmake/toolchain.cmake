list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_SYSTEM_NAME Embedded)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(COMPILER_PREFIX arm-none-eabi-)

find_program(CMAKE_C_COMPILER   NAMES ${COMPILER_PREFIX}gcc         HINTS ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}g++         HINTS ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin REQUIRED)
find_program(CMAKE_OBJCOPY      NAMES ${COMPILER_PREFIX}objcopy     HINTS ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin REQUIRED)
find_program(CMAKE_OBJDUMP      NAMES ${COMPILER_PREFIX}objdump     HINTS ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin REQUIRED)
find_program(OPENOCD            NAMES openocd                       HINTS ${OPENOCD_DIR} ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin REQUIRED)

find_program(ORBUCULUM NAMES orbuculum HINTS ${ORBUCULUM_DIR} ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin)
find_program(ORBDUMP   NAMES orbdump   HINTS ${ORBUCULUM_DIR} ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin)
find_program(ORBTRACE  NAMES orbtrace  HINTS ${ORBUCULUM_DIR} ${TOOLCHAIN_ROOT} PATH_SUFFIXES bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
    -mcpu=cortex-m3
    -ffunction-sections
    -fdata-sections
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>

    -Wall
    -g
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-Og>
)

add_link_options(
    LINKER:--build-id=none
    LINKER:--gc-sections
    -mcpu=cortex-m3
    -g
)
