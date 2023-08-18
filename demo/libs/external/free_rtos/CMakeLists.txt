include(FetchContent)

FetchContent_Declare(
    free_rtos
    URL https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/download/V10.6.0/FreeRTOS-KernelV10.6.0.zip
    URL_HASH SHA256=94EC4E4CD147CB3116CE32B1ACAF6013A794FC7B441077351A44B4774F246266
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_Populate(free_rtos)

set(NAME free_rtos)

add_library(${NAME} INTERFACE)

target_include_directories(${NAME} INTERFACE
    include
    ${free_rtos_SOURCE_DIR}/include
    ${free_rtos_SOURCE_DIR}/portable/GCC/ARM_CM3
)

target_sources(${NAME} INTERFACE
    ${free_rtos_SOURCE_DIR}/tasks.c
    ${free_rtos_SOURCE_DIR}/timers.c
    ${free_rtos_SOURCE_DIR}/list.c
    ${free_rtos_SOURCE_DIR}/queue.c
    ${free_rtos_SOURCE_DIR}/event_groups.c

    ${free_rtos_SOURCE_DIR}/portable/GCC/ARM_CM3/port.c

    src/static.cpp
)

target_link_libraries(${NAME} INTERFACE
    rtos_trace_hooks
)