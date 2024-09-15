#ifndef COMMAND_DEFS_H
#define COMMAND_DEFS_H

#define ASK_DEVICE_ID "DEVICE_ID"
#define DEVICE_ID "YARSA123"

#define DEVICE_RESET "RESET"
#define DEVICE_RESTART "RESTART"

/* -- Heartbeat Handling Messages -- */
#define HEARTBEAT_CMD "HEARTBEAT"
#define HEARTBEAT_ACK "HEARTBEAT_ACK"

/* -- Successful NFC Initialization Messages -- */
#define INIT_NFC_CMD "INIT_NFC"
#define INIT_NFC_ACK "INIT_NFC_ACK"

/* -- Error Messages -- */
#define ERROR_NFC_INIT "ERROR_NFC_INIT"
#define ERROR_FORMAT "ERR_FORMAT"
#define ERROR_DECODE "ERR_DECODE"
#define ERROR_AMOUNT "ERR_AMOUNT"
#define ERROR_DQR "ERR_DQR"
#define ERROR_SCANNER_BUSY "ERR_SCANNER_BUSY"

#define TIME_OUT "TIME_OUT"

#define INVALID_COMMAND "INVALID_CMD"

/* -- Offline Payload Transfer Starter Messages -- */
#define DATA_RTS "DATA_RTS"
#define DATA_RTR "DATA_RTR"
#define DATA_ACK "DATA_ACK"

/* -- Payment Result Messages from SDK -- */
#define PAYMENT_SUCCESS "PAYMENT_SUCCESS"
#define PAYMENT_FAILURE "PAYMENT_FAILURE"

/* -- APDU Related Messages -- */
#define SELECT_APDU_SENT "SELECT_APDU_SENT"
#define SELECT_APDU_FAILED "SELECT_APDU_FAILED"

#endif