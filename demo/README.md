# RTOS Trace Demo App

This project is aimed at generating different RTOS traces which can be used during devlopment of RTOS tracing tools as well as demo for users.

## Avilable scenarios
* `blink_demo` - Blinks two LEDs using `vTaskDalay`
* `busy_blinker` - Blinks two LEDs by monitoring uptime and relaying on preemption for blinking both LEDs

## Hardware
Right now demo project is capable of being run on EFM32GG-STK3700 (https://www.silabs.com/development-tools/mcu/32-bit/efm32gg-starter-kit) kit but structure is prepared for adding more boards.

## Building
**Prerequisites:** CMake, Ninja (or GNU Make), ARM GCC toolchain, OpenOCD.

1. Create CMake preset in `<repo>/demo/CMakeUserPresets.json`:
```
{
    "version": 6,
    "configurePresets": [
        {
            "name": "Default",
            "binaryDir": "${sourceDir}/build",
            "generator": "Ninja",
            "cacheVariables": {
                "TOOLCHAIN_ROOT": "<path to arm-none-eabi-gcc toolchain>",
                "OPENOCD_DIR": "<path to openocd>",
                "OPENOCD_INTERFACE_SCRIPT": "<semicolon separate list of OpenOCD interface scripts>"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "Default",
            "configurePreset": "Default"
        }
    ]
}
```
2. Generate build: `<repo>/demo> cmake --preset Default`
3. Build: `<repo>/demo> cmake --build --preset Default`
4. Flash: `<repo>/demo> cmake --build --preset Default --target <scenario>.flash`

**Variables:**
|          Variable          |                                                Description                                                 |
| -------------------------- | ---------------------------------------------------------------------------------------------------------- |
| `TOOLCHAIN_ROOT`           | Path to ARM GCC toolchain (folder containing `bin/arm-none-eabi-gcc`)                                      |
| `OPENOCD_DIR`              | Path to OpenOCD (folder containing `bin/openocd`)                                                          |
| `OPENOCD_INTERFACE_SCRIPT` | Semicolon separated list of OpenOCD interface scripts (e.g. `interface/cmsis-dap.cfg` in case of Orbtrace) |
