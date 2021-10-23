#include "enetkit.h"
#include "thread.h"
#include "stm32f4x/adc.h"


void Stm32Adc::AdcComplete(bool) {
}

void Stm32Adc::HandleInterrupt() {
    const bool ovr = reg(Register::SR) & BIT(OVR);
    reg(Register::SR) &= ~BIT(OVR);
    AdcComplete(ovr);
}

static void Interrupt(void* token) {
    ((Stm32Adc*)token)->HandleInterrupt();
}
