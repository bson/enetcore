// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#if defined(STMF) && STM == 405
#include "stm32f405.h"
#else
#error "Unsupported STM32 variant"
#endif
