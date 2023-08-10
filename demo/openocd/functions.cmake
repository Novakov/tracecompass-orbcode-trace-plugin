function(target_flash TARGET)
    list(TRANSFORM OPENOCD_INTERFACE_SCRIPT PREPEND "--file=" OUTPUT_VARIABLE IFACE_SCRIPTS)
    add_custom_target(${TARGET}.flash
        COMMAND OpenOCD::OpenOCD ${IFACE_SCRIPTS} -c "set IMAGE $<TARGET_FILE:${TARGET}>" -f $<TARGET_PROPERTY:platform,OPENOCD_FLASH_SCRIPT>
        USES_TERMINAL
        DEPENDS ${TARGET}
    )
endfunction()