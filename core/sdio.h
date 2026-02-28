//
// Copyright 2026 Jan Brittenson
// See LICENSE for details. 
//

#ifndef __SDIO_H__
#define __SDIO_H__

#include <stdint.h>

// Abstract interface for SDIO hardware implementation

class Sdio {
public:
    // Response types
    enum {
        SD_RESP_NONE = 0,
        SD_RESP_R1   = 1,
        SD_RESP_R2   = 2,
        SD_RESP_R3   = 3,
        SD_RESP_R6   = 6,
        SD_RESP_R7   = 7
    };

    virtual void set_clock(uint32_t hz) = 0;
    virtual void set_bus_width_4bit() = 0;
    virtual void set_bus_width_1bit() = 0;
    virtual int send_cmd(uint8_t cmd,
                         uint32_t arg,
                         uint32_t resp_type,
                         uint32_t *response) = 0;
    virtual int data_read(void *buf, uint32_t bytes) = 0;
    virtual int data_write(const void *buf, uint32_t bytes) = 0;
    virtual int wait_data_done() = 0;

    virtual int get_data_crc_error() = 0;
    virtual int get_data_timeout() = 0;
    virtual void reset_datapath() = 0;
};

#endif // __SDIO_H__
