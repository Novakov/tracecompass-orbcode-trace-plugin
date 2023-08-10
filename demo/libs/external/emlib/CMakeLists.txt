set(NAME emlib)

add_library(${NAME} STATIC)

target_include_directories(${NAME} PUBLIC include)

target_sources(${NAME} PRIVATE
    src/em_system.c
    src/em_gpio.c
    src/em_core.c
    src/em_cmu.c
    src/startup_efm32gg.S
    src/system_efm32gg.c
)

target_compile_definitions(${NAME} PUBLIC
    EFM32GG990F1024
)

target_link_options(${NAME} PUBLIC
    -T${CMAKE_CURRENT_SOURCE_DIR}/efm32gg.ld
)

set_property(TARGET ${NAME} APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/efm32gg.ld)
