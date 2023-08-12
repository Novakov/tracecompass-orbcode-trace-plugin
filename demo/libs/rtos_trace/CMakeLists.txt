set(NAME rtos_trace)

add_library(${NAME} STATIC)

target_include_directories(${NAME} PUBLIC include)

target_sources(${NAME} PRIVATE
    src/trace_write.cpp
)

target_link_libraries(${NAME} PUBLIC 
    free_rtos
    platform
)

add_library(${NAME}_hooks INTERFACE)
target_include_directories(${NAME}_hooks INTERFACE include)
