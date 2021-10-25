#include "enetkit.h"
#include "thread.h"
#include "stm32f4x/adc.h"


void Stm32Adc::AdcComplete(uint16_t, bool) {
}

void Stm32Adc::HandleInterrupt() {
    // Check for actual EOC.  There's only one interrupt handler for
    // all 3 ADCs, so this allows a single handler to call all three
    // to process any pending conversions.
    volatile uint32_t& sr = reg(Register::SR);
    while (sr & BIT(EOC)) {
        const bool ovr = sr & BIT(OVR);
        sr &= ~BIT(OVR);
        AdcComplete(reg(Register::DR), ovr);
    }
}

void Stm32Adc::Interrupt(void* token) {
    ((Stm32Adc*)token)->HandleInterrupt();
}
