set(NAME task_notify_isr)

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
