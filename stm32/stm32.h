// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_H__
#define __STM32_H__

#if defined(STMF) && STM == 405
#include "stm32f405.h"
#else
#error "Unsupported STM32 variant"
#endif

#endif // __STM32_H__
