set(NAME idle_timer_service)

add_executable(${NAME})

target_include_directories(${NAME} PRIVATE include)

target_sources(${NAME} PRIVATE
    src/main.cpp
)

target_link_libraries(${NAME} PRIVATE
    platform
    free_rtos
    rtos_trace
)

target_flash(${NAME})
