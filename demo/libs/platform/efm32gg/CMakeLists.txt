set(NAME platform_efm32gg)

add_library(${NAME} STATIC)

target_include_directories(${NAME} PUBLIC include)

target_link_options(${NAME} PUBLIC
    -specs=nano.specs
    -specs=nosys.specs
)

target_link_libraries(${NAME} PUBLIC
    emlib
    platform_interface
    Orbcode::Trace
)

target_sources(${NAME} PRIVATE
    src/bootstrap.cpp
)

set_property(TARGET ${NAME} PROPERTY OPENOCD_FLASH_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/openocd/flash.cfg)