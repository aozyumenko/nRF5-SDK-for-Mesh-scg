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

#include "serial.h"
#include "serial_bearer.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <nrf_error.h>

#include "nrf.h"
#include "nrf_mesh.h"
#include "nrf_mesh_assert.h"
#include "nrf_mesh_opt.h"

#include "bearer_event.h"
#include "log.h"
#include "rand.h"

#include "serial_types.h"
#include "serial_packet.h"
#include "serial_cmd_handler.h"


/* The serial_device_operating_mode_t must fit inside a single byte, to make sure it can go into the packet. */
NRF_MESH_STATIC_ASSERT(SERIAL_DEVICE_OPERATING_MODE__LAST <= 0xFF);

/** Bitmap of reset reasons considered HW errors */
#define RESET_REASONS_HW_ERROR      (POWER_RESETREAS_LOCKUP_Msk)

/** Serial command handler function type */
typedef void (*serial_cmd_handler_t)(const serial_packet_t* p_cmd);

/**
 * Serial command handler entry. Lists a single command handler with its range
 * of opcodes.
 */
typedef struct
{
    uint8_t range_start;            /**< First opcode handled by this handler. */
    uint8_t range_end;              /**< Last opcode handled by this handler. */
    serial_cmd_handler_t handler;   /**< Handler function pointer. */
} serial_cmd_handler_entry_t;


static serial_state_t   m_state;
static bool             m_cmd_handler_scheduled;

static void serial_process_cmd(void * p_context __attribute((unused)))
{
    serial_packet_t packet_in;  /* Incoming packet */
    m_cmd_handler_scheduled = false;

    while (serial_bearer_rx_get(&packet_in))
    {
        serial_handler_rx(&packet_in);
    }
}

uint32_t serial_init(void)
{
    if (m_state != SERIAL_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    else
    {
        serial_bearer_init();
        m_state = SERIAL_STATE_INITIALIZED;
        return NRF_SUCCESS;
    }
}

uint32_t serial_start(void)
{
    if (m_state != SERIAL_STATE_INITIALIZED)
    {
        __LOG(LOG_SRC_SERIAL, LOG_LEVEL_WARN, "Not initialized.");
        return NRF_ERROR_INVALID_STATE;
    }
    m_state = SERIAL_STATE_RUNNING;
    /* Send device started event. */
    serial_packet_t * p_start_packet;
    /* Should not fail: */
    uint32_t err_code = serial_packet_buffer_get(sizeof(serial_evt_device_started_t) + SERIAL_PACKET_OVERHEAD, &p_start_packet);
    if (NRF_SUCCESS != err_code)
    {
        __LOG(LOG_SRC_SERIAL, LOG_LEVEL_ERROR, "Unable to get a serial packet buffer, error_code: %u\n", err_code);
        m_state = SERIAL_STATE_INITIALIZED;
    }
    else
    {
        p_start_packet->opcode = SERIAL_OPCODE_EVT_DEVICE_STARTED;
        p_start_packet->payload.evt.started.operating_mode = SERIAL_DEVICE_OPERATING_MODE_APPLICATION;
        p_start_packet->payload.evt.started.hw_error = NRF_POWER->RESETREAS & RESET_REASONS_HW_ERROR;
        p_start_packet->payload.evt.started.data_credit_available = SERIAL_PACKET_PAYLOAD_MAXLEN;
        serial_tx(p_start_packet);
    }
    return err_code;
}

uint32_t serial_packet_buffer_get(uint16_t packet_len, serial_packet_t ** pp_packet)
{
    if (m_state != SERIAL_STATE_RUNNING)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t status = serial_bearer_packet_buffer_get(packet_len, pp_packet);
    switch (status)
    {
        case NRF_ERROR_INVALID_LENGTH:
        case NRF_ERROR_NO_MEM:
            serial_handler_alloc_fail_report();
            return status;

        case NRF_SUCCESS:
            return status;

        default:
            NRF_MESH_ASSERT(false);
            return status;
    }
}

void serial_tx(const serial_packet_t * p_packet)
{
    NRF_MESH_ASSERT(m_state == SERIAL_STATE_RUNNING);
    serial_bearer_tx(p_packet);
}

void serial_process(void)
{
    if (!m_cmd_handler_scheduled && m_state == SERIAL_STATE_RUNNING)
    {
        /* Set the flag, act, then (potentially) unset the flag, in case the
         * callback fires before we can check the result of the post-call.
         */
        m_cmd_handler_scheduled = true;
        if (bearer_event_generic_post(serial_process_cmd, NULL) != NRF_SUCCESS)
        {
            m_cmd_handler_scheduled = false;
        }
    }
}

uint8_t serial_translate_error(uint32_t error)
{
    switch (error)
    {
        case NRF_SUCCESS:
            return SERIAL_STATUS_SUCCESS;
        case NRF_ERROR_SVC_HANDLER_MISSING:
            return SERIAL_STATUS_ERROR_INVALID_STATE;
        case NRF_ERROR_SOFTDEVICE_NOT_ENABLED:
            return SERIAL_STATUS_ERROR_INVALID_STATE;
        case NRF_ERROR_INTERNAL:
            return SERIAL_STATUS_ERROR_INTERNAL;
        case NRF_ERROR_NO_MEM:
            return SERIAL_STATUS_ERROR_REJECTED;
        case NRF_ERROR_NOT_FOUND:
            return SERIAL_STATUS_ERROR_REJECTED;
        case NRF_ERROR_NOT_SUPPORTED:
            return SERIAL_STATUS_ERROR_REJECTED;
        case NRF_ERROR_INVALID_PARAM:
            return SERIAL_STATUS_ERROR_INVALID_PARAMETER;
        case NRF_ERROR_INVALID_STATE:
            return SERIAL_STATUS_ERROR_INVALID_STATE;
        case NRF_ERROR_INVALID_LENGTH:
            return SERIAL_STATUS_ERROR_INVALID_LENGTH;
        case NRF_ERROR_INVALID_FLAGS:
            return SERIAL_STATUS_ERROR_INVALID_STATE;
        case NRF_ERROR_INVALID_DATA:
            return SERIAL_STATUS_ERROR_INVALID_DATA;
        case NRF_ERROR_DATA_SIZE:
            return SERIAL_STATUS_ERROR_INVALID_LENGTH;
        case NRF_ERROR_TIMEOUT:
            return SERIAL_STATUS_ERROR_TIMEOUT;
        case NRF_ERROR_NULL:
            return SERIAL_STATUS_ERROR_INTERNAL;
        case NRF_ERROR_FORBIDDEN:
            return SERIAL_STATUS_ERROR_REJECTED;
        case NRF_ERROR_INVALID_ADDR:
            return SERIAL_STATUS_ERROR_INVALID_DATA;
        case NRF_ERROR_BUSY:
            return SERIAL_STATUS_ERROR_BUSY;
        case BLE_ERROR_NOT_ENABLED:
            return SERIAL_STATUS_ERROR_REJECTED;
        default:
            return SERIAL_STATUS_ERROR_UNKNOWN;
    }
}

serial_state_t serial_state_get(void)
{
    return m_state;
}

void serial_cmd_rsp_send(uint8_t opcode, uint32_t token, uint8_t status, const uint8_t * p_data, uint16_t length)
{
    NRF_MESH_ASSERT((p_data != NULL && length != 0) || (p_data == NULL && length == 0));
    if (m_state != SERIAL_STATE_RUNNING)
    {
        return;
    }

    serial_packet_t * p_rsp;
    uint32_t err_code = serial_packet_buffer_get(SERIAL_EVT_CMD_RSP_LEN_OVERHEAD + length, &p_rsp);
    if (err_code == NRF_SUCCESS)
    {
        p_rsp->opcode = SERIAL_OPCODE_EVT_CMD_RSP;
        p_rsp->payload.evt.cmd_rsp.opcode = opcode;
        p_rsp->payload.evt.cmd_rsp.token = token;
        p_rsp->payload.evt.cmd_rsp.status = status;

        if (p_data != NULL)
        {
            memcpy(&p_rsp->payload.evt.cmd_rsp.data, p_data, length);
        }

        (void) serial_tx(p_rsp);
    }
}
