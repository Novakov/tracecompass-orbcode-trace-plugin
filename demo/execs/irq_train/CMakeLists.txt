set(NAME irq_train)

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
