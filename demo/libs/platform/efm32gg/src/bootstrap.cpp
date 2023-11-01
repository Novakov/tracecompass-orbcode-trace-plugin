#include "em_cmu.h"
#include "em_gpio.h"
#include "platform.hpp"
#include "trace.hpp"

extern void demo_main();

void SetupClock()
{
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLK);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_DBG, cmuSelect_HFCLK);

    /* EFM32 specific configuration to enable the TRACESWO IO pin */
    CMU_AUXHFRCOBandSet(cmuAUXHFRCOBand_14MHz);
    CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);

    SystemCoreClockUpdate();

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_DBG, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DBG, true);
}

void SetupGPIO()
{
    // LEDS
    {
        GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 1);
        GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 0);
    }

    // Trace
    {
        auto route = GPIO->ROUTE;
        route &= ~_GPIO_ROUTE_ETMLOCATION_MASK;
        route |= GPIO_ROUTE_ETMLOCATION_LOC0;

        route |= GPIO_ROUTE_TCLKPEN;
        route |= GPIO_ROUTE_TD0PEN;
        route |= GPIO_ROUTE_TD1PEN;
        route |= GPIO_ROUTE_TD2PEN;
        route |= GPIO_ROUTE_TD3PEN;

        GPIO->ROUTE = route;
        // ETM location 0:
        // CLK - PD7
        // D0  - PD6
        // D1  - PD3
        // D2  - PD4
        // D3  - PD5

        GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 4, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 5, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 0);
    }
}

void ToggleLed(Led led)
{
    switch(led)
    {
        case Led::Led1:
            GPIO_PinOutToggle(gpioPortE, 2);
            break;
        case Led::Led2:
            GPIO_PinOutToggle(gpioPortE, 3);
            break;
    }
}

void EnableTrace()
{
    TpiuOptions tpiu = {
        .Protocol = TpiuProtocolParallel,
        .FormattingEnabled = true,
        .TracePortWidth = 4,
    };

    ITMOptions itm = {
        .TraceBusID = 1,
        .GlobalTimestampFrequency = ITMGlobalTimestampFrequencyDisabled,
        .LocalTimestampPrescaler = ITMLocalTimestampPrescalerNoPrescaling,
        .EnableLocalTimestamp = true,
        .ForwardDWT = true,
        .EnableSyncPacket = true,
        .EnabledStimulusPorts = ITM_ENABLE_STIMULUS_PORTS_ALL,
    };

    DWTOptions dwt;
    dwt.CycleTap = DWTCycleTap10;
    dwt.CPICounterEvent = false;
    dwt.ExceptionOverheadCounterEvent = false;
    dwt.ExceptionTrace = true;
    dwt.FoldedInstructionCounterEvent = false;
    dwt.LSUCounterEvent = false;
    dwt.PCSampling = false;
    dwt.SamplingPrescaler = 1;
    dwt.SleepCounterEvent = false;
    dwt.SyncTap = DWTSyncTap28;

    TpiuSetup(&tpiu);
    ITMSetup(&itm);
    DWTSetup(&dwt);
}

int main()
{
    SetupClock();
    SetupGPIO();

    EnableTrace();

    for(int i = 0; i < 10'000; i++)
    {
        __asm__ volatile("nop");
    }

    demo_main();
}