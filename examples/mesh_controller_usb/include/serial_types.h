/* Copyright (c) 2010 - 2020, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SERIAL_TYPES_H__
#define SERIAL_TYPES_H__

#include <stdint.h>
#include "access.h"

/**
 * @defgroup SERIAL_DEFINES Defines
 * Serial interface defines
 * @{
 */

/** Serial status codes */
typedef enum
{
    SERIAL_STATUS_SUCCESS                 = 0x00,
    SERIAL_STATUS_ERROR_UNKNOWN           = 0x80,
    SERIAL_STATUS_ERROR_INTERNAL          = 0x81,
    SERIAL_STATUS_ERROR_CMD_UNKNOWN       = 0x82,
    SERIAL_STATUS_ERROR_INVALID_STATE     = 0x83,
    SERIAL_STATUS_ERROR_INVALID_LENGTH    = 0x84,
    SERIAL_STATUS_ERROR_INVALID_PARAMETER = 0x85,
    SERIAL_STATUS_ERROR_BUSY              = 0x86,
    SERIAL_STATUS_ERROR_INVALID_DATA      = 0x87,
    SERIAL_STATUS_ERROR_REJECTED          = 0x8e,
    SERIAL_STATUS_ERROR_TIMEOUT           = 0x93,
} serial_status_t;

/** Device operating mode. */
typedef enum
{
    SERIAL_DEVICE_OPERATING_MODE_TEST = 0,           /**< Testing operating mode. */
    SERIAL_DEVICE_OPERATING_MODE_BOOTLOADER = 1,     /**< Bootloader operating mode. */
    SERIAL_DEVICE_OPERATING_MODE_APPLICATION = 2,    /**< Regular operating mode. */

    /** @internal Largest number in the enum. */
    SERIAL_DEVICE_OPERATING_MODE__LAST = 2,
} serial_device_operating_mode_t;

/** TX power values available. */
typedef enum
{
    SERIAL_CMD_TX_POWER_VALUE_0dBm     = 0x00, /**< 0dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Pos4dBm  = 0x04, /**< +4dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg30dBm = 0xD8, /**< -30dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg20dBm = 0xEC, /**< -20dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg16dBm = 0xF0, /**< -16dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg12dBm = 0xF4, /**< -12dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg8dBm  = 0xF8, /**< -8dBm. */
    SERIAL_CMD_TX_POWER_VALUE_Neg4dBm  = 0xFC, /**< -4dBm. */
} serial_cmd_tx_power_value_t;



#endif /* SERIAL_TYPES_H__ */
