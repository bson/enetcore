#ifndef __LPC_H__
#define __LPC_H__

#if (LPC < 2300) && (LPC >= 2200)
#include "lpc22xx.h"
#elif (LPC < 2500) && (LPC >= 2400)
#include "lpc24xx.h"
#else
#error "Unsupported LPC model"
#endif

#endif // __LPC_H__
