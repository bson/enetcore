// Copyright 2026 Jan Brittenson
// See LICENSE for details. 

#pragma once

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

public:
    enum Result : int {
        SUCCESS = 0,
        CMD_TIMEOUT = -1,
        CMD_CRC_FAIL = -2,
        DATA_TIMEOUT = -3,
        DATA_CRC_FAIL = -4,
    };

    virtual void set_clock(uint32_t hz) = 0;
    virtual void set_bus_width_4bit() = 0;
    virtual void set_bus_width_1bit() = 0;
    virtual Result send_cmd(uint8_t cmd,
                            uint32_t arg,
                            uint32_t resp_type,
                            uint32_t *response) = 0;
    virtual Result data_read(void *buf, uint32_t bytes) = 0;
    virtual Result data_write(const void *buf, uint32_t bytes) = 0;

    virtual bool get_data_crc_error() = 0;
    virtual bool get_data_timeout() = 0;
    virtual void reset_datapath() = 0;
};
