set(NAME busy_blinker)

add_executable(${NAME})

target_sources(${NAME} PRIVATE
    src/main.cpp
)

target_link_libraries(${NAME} PRIVATE
    platform
    free_rtos
)

target_flash(${NAME})
