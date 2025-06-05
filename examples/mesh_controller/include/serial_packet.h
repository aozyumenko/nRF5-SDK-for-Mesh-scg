/* Copyright (c) 2025, Alexander Ozuenko
 */

#ifndef SERIAL_PACKET_H__
#define SERIAL_PACKET_H__

#include <stdint.h>
#include <limits.h>
#include "nrf_mesh_assert.h"



/* serial protocol version */
#define SERIAL_API_VERSION              (0x0001)

/* packet length in addition to parameter */
#define SERIAL_PACKET_LENGTH_OVERHEAD   (1UL)
#define SERIAL_PACKET_OVERHEAD          (2 - SERIAL_PACKET_LENGTH_OVERHEAD)
#define SERIAL_PACKET_PAYLOAD_MAXLEN    (UINT8_MAX - SERIAL_PACKET_OVERHEAD)

#define SERIAL_CMD_OVERHEAD             (4)
#define SERIAL_CMD_PAYLOAD_MAXLEN       (SERIAL_PACKET_PAYLOAD_MAXLEN - SERIAL_CMD_OVERHEAD)

#define SERIAL_EVT_CMD_RSP_OVERHEAD     (6)
#define SERIAL_EVT_CMD_RSP_LEN_OVERHEAD (SERIAL_PACKET_OVERHEAD + SERIAL_EVT_CMD_RSP_OVERHEAD)

#define BLE_AD_DATA_LENGTH_OVERHEAD     (1)
#define BLE_AD_DATA_HEADER_LENGTH       (2)
#define BLE_AD_DATA_PAYLOAD_MAXLEN      (SERIAL_CMD_PAYLOAD_MAXLEN - BLE_AD_DATA_HEADER_LENGTH)


/** commands section */

/* BLE Advertising Data Send command packet */
typedef struct __attribute((packed))
{
    uint8_t data[SERIAL_CMD_PAYLOAD_MAXLEN];
} serial_cmd_ad_data_t;


/* union of all serial command parameters */
typedef struct __attribute((packed))
{
    uint32_t token;     /* unique ID of the command */
    union __attribute((packed)) {
        serial_cmd_ad_data_t ad_data;
    } payload;
} serial_cmd_t;


/** command responses section */

/* command response data with version information */
typedef struct __attribute((packed))
{
    uint16_t serial_ver;
} serial_evt_cmd_rsp_data_serial_version_t;

/* serial interface housekeeping data */
typedef struct __attribute((packed))
{
    uint32_t alloc_fail_count;
} serial_evt_cmd_rsp_data_housekeeping_t;


/* command response packet */
typedef struct __attribute((packed))
{
    uint8_t opcode;     /* opcode of original command */
    uint32_t token;     /* unique ID of the original command */
    uint8_t status;     /* return status of the command: serial_status_t  */
    union __attribute((packed))
    {
        serial_evt_cmd_rsp_data_serial_version_t serial_version;        /* serial protocol version */
        serial_evt_cmd_rsp_data_housekeeping_t hk_data;                 /* housekeeping data response */
    } data;
} serial_evt_cmd_rsp_t;



/** events section */

/* started event packet. */
typedef struct __attribute((packed))
{
    uint8_t operating_mode;             /* operating mode of the device: serial_device_operating_mode_t */
    uint8_t hw_error;                   /* hardware error code, or 0 if no error occurred */
    uint8_t data_credit_available;      /* number of bytes available in each of the tx and rx buffers */
} serial_evt_device_started_t;


/** BLE Advertising event packet. */
typedef struct __attribute((packed))
{
    uint8_t data[SERIAL_CMD_PAYLOAD_MAXLEN];    /* BLE Advertising Data (AdvData) */
} serial_evt_ad_data_t;


/** Union of all serial event parameters */
typedef union __attribute((packed))
{
    serial_evt_cmd_rsp_t        cmd_rsp;        /* command response parameters. */
    serial_evt_device_started_t started;        /* dDevice started parameters. */
    serial_evt_ad_data_t        ad_data;        /* BLE Advertising parameters. */
} serial_evt_t;



/* serial interface packet */
typedef struct __attribute((packed))
{
    uint8_t length;             /* length of the packet in bytes */
    uint8_t opcode;             /* opcode of the packet */
#define SERIAL_OPCODE_CMD_RESET                         (0x01)
#define SERIAL_OPCODE_CMD_SERIAL_VERSION_GET            (0x02)
#define SERIAL_OPCODE_CMD_START                         (0x03)
#define SERIAL_OPCODE_CMD_STOP                          (0x04)
#define SERIAL_OPCODE_CMD_AD_DATA_SEND                  (0x05)  /* serial_cmd_ad_data_t */
#define SERIAL_OPCODE_CMD_HOUSEKEEPING_DATA_GET         (0x7e)
#define SERIAL_OPCODE_CMD_HOUSEKEEPING_DATA_CLEAR       (0x7f)

#define SERIAL_OPCODE_EVT_DEVICE_STARTED                (0x81)  /* serial_evt_device_started_t */
#define SERIAL_OPCODE_EVT_CMD_RSP                       (0x82)  /* serial_evt_cmd_rsp_t */
#define SERIAL_OPCODE_EVT_AD_DATA_RECEIVED              (0x83)  /* serial_evt_ad_data_t */

    /* union of the various payload structures for all serial packets */
    union __attribute((packed))
    {
        serial_cmd_t cmd;       /* command packet parameters */
        serial_evt_t evt;       /* event packet parameters */
    } payload;
} serial_packet_t;



/* check that the length of the packet fits in a uint8_t */
NRF_MESH_STATIC_ASSERT(sizeof(serial_packet_t) <= (UINT8_MAX + 1));



#endif
