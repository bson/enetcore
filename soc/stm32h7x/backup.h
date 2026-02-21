// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_BACKUP_H__
#define __STM32_BACKUP_H__

class Stm32Backup {
public:
    static void* base() { return (void*)BASE_BKPSRAM; }
};
    
#endif // __STM32_BACKUP_H__
