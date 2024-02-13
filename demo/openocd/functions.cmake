function(target_flash TARGET)
    list(TRANSFORM OPENOCD_INTERFACE_SCRIPT PREPEND "--file=" OUTPUT_VARIABLE IFACE_SCRIPTS)
    add_custom_target(${TARGET}.flash
        COMMAND OpenOCD::OpenOCD ${IFACE_SCRIPTS} -c "set IMAGE $<TARGET_FILE:${TARGET}>" -f $<TARGET_PROPERTY:platform,OPENOCD_FLASH_SCRIPT>
        USES_TERMINAL
        DEPENDS ${TARGET}
    )
endfunction()

function(target_capture_trace TARGET)
    add_custom_target(${TARGET}.capture
        COMMAND
            Python3::Interpreter ${CMAKE_SOURCE_DIR}/run_demo.py
                --orbuculum ${ORBUCULUM}
                --orbdump ${ORBDUMP}
                --orbtrace ${ORBTRACE}
                --openocd ${OPENOCD}
                --openocd-config ${OPENOCD_INTERFACE_SCRIPT} $<TARGET_PROPERTY:platform,OPENOCD_INIT_SCRIPT>
                --trace-file $<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_BASE_NAME:${TARGET}>.trace
            USES_TERMINAL
        DEPENDS ${TARGET}.flash
    )
endfunction()