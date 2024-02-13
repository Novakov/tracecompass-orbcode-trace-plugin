set(NAME irq_nested)

add_executable(${NAME})

target_sources(${NAME} PRIVATE
    src/main.cpp
)

target_link_libraries(${NAME} PRIVATE
    platform
    free_rtos
    rtos_trace
)

target_flash(${NAME})
target_capture_trace(${NAME})
