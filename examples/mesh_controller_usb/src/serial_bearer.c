/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 */

#include "serial_bearer.h"
#include "serial_cmd_handler.h"

#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "nrf_error.h"

#include "nrf_mesh_assert.h"
#include "serial.h"
#include "ad_data_proxy.h"
#include "serial_packet.h"
#include "serial_types.h"
#include "packet_buffer.h"
#include "bearer_event.h"
#include "toolchain.h"

#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

/* logging */
#include "nrf5_sdk_log.h"



/*********** Definitions ***********/

#define CDC_ACM_COMM_INTERFACE          0
#define CDC_ACM_COMM_EPIN               NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE          1
#define CDC_ACM_DATA_EPIN               NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT              NRF_DRV_USBD_EPOUT1

/* Buffers must be word-aligned, and also be able to fit two instances of the largest possible
 * packet, to avoid it locking up if the head is in the middle of the buffer. */
#define RX_BUFFER_SIZE                  (2 * ALIGN_VAL(sizeof(serial_packet_t) + sizeof(packet_buffer_packet_t), WORD_SIZE))
#define TX_BUFFER_SIZE                  (64 * ALIGN_VAL(sizeof(serial_packet_t) + sizeof(packet_buffer_packet_t), WORD_SIZE))
#define CMD_RSP_BUFFER_SIZE             (2 * ALIGN_VAL(sizeof(serial_packet_t) + sizeof(packet_buffer_packet_t), WORD_SIZE))
#define SERIAL_PACKET_ENCODED_SIZE      (1UL + (sizeof(serial_packet_t) * 2) + 1UL)

/** Defines for SLIP encoding: see https://tools.ietf.org/html/rfc1055 */
#define SLIP_END                        0xC0
#define SLIP_ESC                        0xDB
#define SLIP_ESC_END                    0xDC
#define SLIP_ESC_ESC                    0xDD



/********** Local typedefs **********/

typedef enum
{
    SERIAL_BEARER_STATE_IDLE,
    SERIAL_BEARER_STATE_TRANSMIT
} serial_bearer_state_t;


/* forward declaration of static functions */

static void usbd_user_ev_handler(app_usbd_event_type_t event);
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

/********** Static variables **********/

static uint8_t m_tx_buffer[TX_BUFFER_SIZE];
static packet_buffer_t m_tx_packet_buf;
static uint8_t m_tx_cmd_rsp_buffer[CMD_RSP_BUFFER_SIZE];
static packet_buffer_t m_tx_cmd_rsp_packet_buf;
static uint8_t m_rx_buffer[RX_BUFFER_SIZE];
static packet_buffer_t m_rx_packet_buf;

static uint8_t m_rx_buffer_encoded[SERIAL_PACKET_ENCODED_SIZE];
int m_rx_idx_encoded = 0;

static serial_bearer_state_t m_serial_bearer_state = SERIAL_BEARER_STATE_IDLE;

static uint32_t m_do_transmit_event_flag;
static bool m_ready;

static const app_usbd_config_t usbd_config = {
    .ev_state_proc = usbd_user_ev_handler
};

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);


/* Serial receive relies on the length field of a serial packet being the first byte and opcode being the second byte */
NRF_MESH_STATIC_ASSERT(offsetof(serial_packet_t, length) == 0);



/********** Static Functions **********/

static size_t slip_encode(const uint8_t *src, ssize_t src_len,
                          uint8_t *dest, size_t dest_len)
{
    int src_i, dest_i = 0;

    dest[dest_i++] = SLIP_END;
    for (src_i = 0; src_i < src_len && dest_i < dest_len; src_i++, dest_i++) {
        if (SLIP_END == src[src_i]) {
            dest[dest_i++] = SLIP_ESC;
            if (dest_i >= dest_len) break;
            dest[dest_i] = SLIP_ESC_END;
        } else if (SLIP_ESC == src[src_i]) {
            dest[dest_i++] = SLIP_ESC;
            if (dest_i >= dest_len) break;
            dest[dest_i] = SLIP_ESC_ESC;
        } else {
            dest[dest_i] = src[src_i];
        }
    }
    dest[dest_i++] = SLIP_END;

    return dest_i;
}


static size_t slip_decode(const uint8_t *src, ssize_t src_len,
                          uint8_t *dest, size_t dest_len)
{
    int src_i, dest_i = 0;
    uint8_t prev_char = 0;

    for (src_i = 0; src_i < src_len && dest_i < dest_len; src_i++) {
        if (SLIP_END == src[src_i]) {
            break;
        } else if (SLIP_ESC != prev_char) {
            if (SLIP_ESC != src[src_i]) {
                dest[dest_i++] = src[src_i];
            }
        } else {
            switch (src[src_i]) {
            case SLIP_ESC_END:
                dest[dest_i++] = SLIP_END;
                break;
            case SLIP_ESC_ESC:
                dest[dest_i++] = SLIP_ESC;
                break;
            default:
                return dest_i;
            }
        }
        prev_char = src[src_i];
    }

    return dest_i;
}


static void schedule_transmit(void)
{
    bearer_event_flag_set(m_do_transmit_event_flag);
}


static void do_receive(uint8_t *data, size_t length) {
    int i;
    uint8_t packet_length;
    size_t decode_length;
    packet_buffer_packet_t *rx_packet;

    for (i = 0; i < length; i++) {
        if (data[i] == SLIP_END) {
            if (m_rx_idx_encoded > 0) {
                /* first we get the packet size */
                decode_length = slip_decode(m_rx_buffer_encoded, m_rx_idx_encoded,
                                            &packet_length, sizeof(packet_length));
                if (decode_length != sizeof(packet_length)) {
                    /* received packet is too small to even contain a field with the size of the entire packet */
                    serial_bearer_cmd_rsp_send(0, UINT32_MAX, SERIAL_STATUS_ERROR_INTERNAL, NULL, 0);
                    serial_handler_rx_fail_report();
                    NRF_LOG_ERROR("%s(): invalid packet", __func__);
                } else if (NRF_SUCCESS != packet_buffer_reserve(&m_rx_packet_buf, &rx_packet, packet_length + 1)) {
                    /* abnormal situation: we won't even unpack the packet header to form a correct response */
                    serial_bearer_cmd_rsp_send(0, UINT32_MAX, SERIAL_STATUS_ERROR_INTERNAL, NULL, 0);
                    serial_handler_rx_fail_report();
                    NRF_LOG_ERROR("%s(): can't reserve packet %d", __func__, packet_length + 1);
                } else {
                    /* now we can decode the whole packet */
                    decode_length = slip_decode(m_rx_buffer_encoded, m_rx_idx_encoded,
                                                rx_packet->packet, packet_length + 1);
                    if (decode_length != packet_length + 1) {
                        /* the size of the decoded packet does not match the header */
                        uint32_t token =
                            (decode_length < SERIAL_PACKET_LENGTH_OVERHEAD + SERIAL_PACKET_OVERHEAD + SERIAL_CMD_OVERHEAD) ?
                                UINT32_MAX : ((serial_packet_t *)rx_packet->packet)->payload.cmd.token;
                        serial_bearer_cmd_rsp_send(((serial_packet_t *)rx_packet->packet)->opcode, token, SERIAL_STATUS_ERROR_INVALID_LENGTH, NULL, 0);
                        serial_handler_rx_fail_report();
                        packet_buffer_free(&m_rx_packet_buf, rx_packet);
                    } else {
                        /* packet has been successfully received and is ready for processing */
                        packet_buffer_commit(&m_rx_packet_buf, rx_packet, rx_packet->size);
                        serial_process();
                    }
                }

                m_rx_idx_encoded = 0;
            }
        } else {
            if (m_rx_idx_encoded < sizeof(m_rx_buffer_encoded)) {
                m_rx_buffer_encoded[m_rx_idx_encoded++] = data[i];
            }
        }
    }
}


static bool do_transmit(void)
{
    packet_buffer_packet_t *p_current_tx_packet;
    static uint8_t tx_buffer_encoded[SERIAL_PACKET_ENCODED_SIZE];
    size_t tx_buffer_encoded_length;

    if (!m_ready || m_serial_bearer_state != SERIAL_BEARER_STATE_IDLE) {
        return true;
    }

    if (NRF_SUCCESS == packet_buffer_pop(&m_tx_cmd_rsp_packet_buf, &p_current_tx_packet)) {
        /* first, we process the queue of responses to commands */
        tx_buffer_encoded_length = slip_encode(p_current_tx_packet->packet,
                                               p_current_tx_packet->size,
                                               tx_buffer_encoded,
                                               sizeof(tx_buffer_encoded));
        packet_buffer_free(&m_tx_cmd_rsp_packet_buf, p_current_tx_packet);
    } else if (NRF_SUCCESS == packet_buffer_pop(&m_tx_packet_buf, &p_current_tx_packet)) {
        /* the ad data queue has a lower priority */
        tx_buffer_encoded_length = slip_encode(p_current_tx_packet->packet,
                                               p_current_tx_packet->size,
                                               tx_buffer_encoded,
                                               sizeof(tx_buffer_encoded));
        packet_buffer_free(&m_tx_packet_buf, p_current_tx_packet);
    } else {
        return true;
    }

    ret_code_t err_code = app_usbd_cdc_acm_write(&m_app_cdc_acm, tx_buffer_encoded, tx_buffer_encoded_length);
    if (err_code == NRF_SUCCESS) {
        m_serial_bearer_state = SERIAL_BEARER_STATE_TRANSMIT;
    } else {
        NRF_LOG_ERROR("%s(): app_usbd_cdc_acm_write() failed: 0x%x", __func__, err_code);
    }

    return true;
}


static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event) {
    case APP_USBD_EVT_DRV_SUSPEND:
        NRF_LOG_DEBUG("USB driver suspend");
        break;
    case APP_USBD_EVT_DRV_RESUME:
        NRF_LOG_DEBUG("USB driver resume");
        break;
    case APP_USBD_EVT_STARTED:
        NRF_LOG_DEBUG("USB started");
        break;
    case APP_USBD_EVT_STOPPED:
        NRF_LOG_INFO("USB ready");
        app_usbd_disable();
        break;
    case APP_USBD_EVT_POWER_DETECTED:
        NRF_LOG_INFO("USB power detected");
        if (!nrf_drv_usbd_is_enabled()) {
            app_usbd_enable();
        }
        break;
    case APP_USBD_EVT_POWER_REMOVED:
        NRF_LOG_INFO("USB power removed");
        app_usbd_stop();
        break;
    case APP_USBD_EVT_POWER_READY:
        NRF_LOG_INFO("USB ready");
        app_usbd_start();
        break;
    default:
        break;
    }
}


static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    static uint8_t rx_buffer[NRF_DRV_USBD_EPSIZE];

    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        NRF_LOG_DEBUG("ACM port open, prev_ready=%d", m_ready);

        UNUSED_VARIABLE(proxy_stop());

        packet_buffer_flush(&m_tx_packet_buf);
        packet_buffer_flush(&m_tx_cmd_rsp_packet_buf);
        packet_buffer_flush(&m_rx_packet_buf);

        m_serial_bearer_state = SERIAL_BEARER_STATE_IDLE;
        m_ready = true;

        /* setup first transfer */
        UNUSED_VARIABLE(app_usbd_cdc_acm_read_any(&m_app_cdc_acm,
                                                   rx_buffer,
                                                   sizeof(rx_buffer)));

        /* Send device started event. */
//        serial_evt_device_started_t evt_started;
//        evt_started.operating_mode = SERIAL_DEVICE_OPERATING_MODE_APPLICATION;
//        evt_started.hw_error = NRF_POWER->RESETREAS & RESET_REASONS_HW_ERROR;
//        evt_started.data_credit_available = SERIAL_PACKET_PAYLOAD_MAXLEN;

//        uint32_t serial_bearer_cmd_rsp_send(SERIAL_OPCODE_EVT_DEVICE_STARTED, uint32_t token, uint8_t status,
//                                    const uint8_t *p_data, uint16_t length)

//        p_start_packet->opcode = SERIAL_OPCODE_EVT_DEVICE_STARTED;
//        serial_tx(p_start_packet);

        break;

    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
        UNUSED_VARIABLE(proxy_stop());
        m_ready = false;
        break;

    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
        m_serial_bearer_state = SERIAL_BEARER_STATE_IDLE;
        schedule_transmit();
        break;

    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t err_code;

            do {
                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
                do_receive(rx_buffer, size);

                /* fetch data until internal buffer is empty */
                err_code = app_usbd_cdc_acm_read_any(&m_app_cdc_acm,
                                                     rx_buffer,
                                                     sizeof(rx_buffer));
            } while (err_code == NRF_SUCCESS);
        }
        break;

    default:
        break;
    }
}



/********** Interface Functions **********/

uint32_t serial_bearer_init(void)
{
    uint32_t err_code = app_usbd_init(&usbd_config);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    err_code = app_usbd_class_append(class_cdc_acm);
    if (NRF_SUCCESS != err_code) {
        return err_code;
    }

    packet_buffer_init(&m_tx_packet_buf, (void *)m_tx_buffer, sizeof(m_tx_buffer));
    packet_buffer_init(&m_tx_cmd_rsp_packet_buf, (void *)m_tx_cmd_rsp_buffer, sizeof(m_tx_cmd_rsp_buffer));
    packet_buffer_init(&m_rx_packet_buf, (void *)m_rx_buffer, sizeof(m_rx_buffer));

    m_do_transmit_event_flag = bearer_event_flag_add(do_transmit);

    m_serial_bearer_state = SERIAL_BEARER_STATE_IDLE;
    m_ready = false;

    return NRF_SUCCESS;
}


uint32_t serial_bearer_start(void)
{
    if (m_ready) {
        return NRF_ERROR_INVALID_STATE;
    }

    app_usbd_enable();
    app_usbd_start();

    return NRF_SUCCESS;
}


uint32_t serial_bearer_ad_data_send(uint8_t ad_type, const uint8_t *p_data, uint16_t length)
{
    packet_buffer_packet_t *p_buf_packet;
    serial_packet_t *p_packet;
    size_t packet_len = SERIAL_PACKET_OVERHEAD + BLE_AD_DATA_HEADER_LENGTH + length;

    NRF_MESH_ASSERT(p_data != NULL && length > 0);

    if (!m_ready) {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t err_code = packet_buffer_reserve(&m_tx_packet_buf, &p_buf_packet,
                                              packet_len + SERIAL_PACKET_LENGTH_OVERHEAD);
    if (err_code != NRF_SUCCESS)  {
        NRF_LOG_ERROR("%s(): can't reserve packet ad data: 0x%x", __func__, err_code);
        serial_handler_alloc_fail_report();
        return err_code;
    }

    p_packet = (serial_packet_t *)p_buf_packet->packet;
    p_packet->length = packet_len;
    p_packet->opcode = SERIAL_OPCODE_EVT_AD_DATA_RECEIVED;
    p_packet->payload.evt.ad_data.data[0] = length + BLE_AD_DATA_OVERHEAD;
    p_packet->payload.evt.ad_data.data[1] = ad_type;
    memcpy(p_packet->payload.evt.ad_data.data + 2, p_data, length);

    bearer_event_critical_section_begin();
    schedule_transmit();
    packet_buffer_commit(&m_tx_packet_buf, p_buf_packet, p_buf_packet->size);
    bearer_event_critical_section_end();

    return NRF_SUCCESS;
}


uint32_t serial_bearer_cmd_rsp_send(uint8_t opcode, uint32_t token, uint8_t status,
                                    const uint8_t *p_data, uint16_t length)
{
    packet_buffer_packet_t *p_buf_packet;
    serial_packet_t *p_rsp;
    size_t packet_len = SERIAL_EVT_CMD_RSP_LEN_OVERHEAD + length;

    NRF_MESH_ASSERT((p_data == NULL && length == 0) || (p_data != NULL && length > 0));

    if (!m_ready) {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t err_code = packet_buffer_reserve(&m_tx_cmd_rsp_packet_buf, &p_buf_packet,
                                               packet_len + SERIAL_PACKET_LENGTH_OVERHEAD);
    if (err_code != NRF_SUCCESS)  {
        NRF_LOG_ERROR("%s(): can't reserve packet for command response: 0x%x", __func__, err_code);
        serial_handler_alloc_fail_report();
        return err_code;
    }

    p_rsp = (serial_packet_t *)p_buf_packet->packet;
    p_rsp->length = packet_len;
    p_rsp->opcode = SERIAL_OPCODE_EVT_CMD_RSP;
    p_rsp->payload.evt.cmd_rsp.opcode = opcode;
    p_rsp->payload.evt.cmd_rsp.status = status;
    p_rsp->payload.evt.cmd_rsp.token = token;
    memcpy(&p_rsp->payload.evt.cmd_rsp.data, p_data, length);

    bearer_event_critical_section_begin();
    schedule_transmit();
    packet_buffer_commit(&m_tx_cmd_rsp_packet_buf, p_buf_packet, p_buf_packet->size);
    bearer_event_critical_section_end();

    return NRF_SUCCESS;
}


bool serial_bearer_rx_get(serial_packet_t *p_packet)
{
    packet_buffer_packet_t *p_buf_packet;

    if (!m_ready) {
        return false;
    }

    if (packet_buffer_pop(&m_rx_packet_buf, &p_buf_packet) != NRF_SUCCESS) {
        return false;
    }

    memcpy(p_packet, p_buf_packet->packet, p_buf_packet->size);
    packet_buffer_free(&m_rx_packet_buf, p_buf_packet);
    return true;
}
