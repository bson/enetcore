// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#if defined(STMF) && (STM >= 400) && (STM < 500)
#include "stm32f4x.h"
#else
#error "Unsupported STM32 variant"
#endif
