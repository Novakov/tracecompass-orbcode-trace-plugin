#pragma once

#define Interrupt1_IRQHandler LCD_IRQHandler
#define Interrupt2_IRQHandler MSC_IRQHandler
#define Interrupt3_IRQHandler AES_IRQHandler

#define Interrupt1_IRQn ((IRQn_Type)34)
#define Interrupt2_IRQn ((IRQn_Type)35)
#define Interrupt3_IRQn ((IRQn_Type)36)
