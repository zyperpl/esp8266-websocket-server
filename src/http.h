#pragma once

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "uart.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

#include "sha1.h"
#include "b64.h"

#include "network.h"

// parameters from WebSocket specification RFC 6455
#define WEBSOCKET_MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define WEBSOCKET_CONT  0x00
#define WEBSOCKET_TEXT  0x01
#define WEBSOCKET_BIN   0x02
#define WEBSOCKET_EXIT  0x08
#define WEBSOCKET_PING  0x9
#define WEBSOCKET_PONG  0xA

// ping magic
#define WEBSOCKET_PING_MAGIC  "zPNGz"__DATE__"_"__TIME__"zPNGz"

// maximum number of connections remebered
#define WEBSOCKET_CONNECTIONS_POOL_SIZE 16

// name of custom websocket protocol
#define WEBSOCKET_PROTOCOL_NAME "ZPROTO2"

#define HTTP_FILE_MAX_LENGTH  32
#define HTTP_PAYLOAD_SIZE     256
#define HTTP_PAYLOAD_INTERVAL 1000
#define HTTP_BUFFER_SIZE      8192

/*
 * Type of WebSocket callback function that is called on every decoded websocket packet.
 */
typedef void (*ws_receive_callback_t)(struct espconn *con, char *data, uint64_t length);

// WebSocket related functions
/*
 * Generates websocket accept key based on client key.
 */
char ICACHE_FLASH_ATTR *websocket_get_accept(char *key);

/*
 * Sets connection parameters and registers its callbacks. Adds connection to pool.
 */
void ICACHE_FLASH_ATTR websocket_add_connection(struct espconn *con);

/*
 * Get all websocket connections in pool
 */
struct espconn ICACHE_FLASH_ATTR **websocket_get_connections();

/*
 * Decodes websocket packet.
 */
char ICACHE_FLASH_ATTR *websocket_decode(char *pdata, unsigned short len);

/*
 * Invokes when websocket data is received.
 */
void ICACHE_FLASH_ATTR websocket_receive(void *arg, char *pdata, unsigned short len);

/*
 * Sets user-defined websocket receive callback.
 */
void ICACHE_FLASH_ATTR websocket_set_receive_callback(ws_receive_callback_t callback);

/*
 * Sends websocket data to specified client.
 */
void ICACHE_FLASH_ATTR websocket_write(struct espconn *con, const void *data, uint64_t len, uint8_t mode);

/*
 * Sends websocket data to all clients in the pool
 */
void ICACHE_FLASH_ATTR websocket_write_all(const void *data, uint64_t len, uint8_t mode);

/*
 * Closes connection with client
 */
void ICACHE_FLASH_ATTR websocket_close(struct espconn *con);



// Simple HTTP server functions
/*
 * Initialize HTTP server listening on port 
 */
void ICACHE_FLASH_ATTR http_start(int16_t port);

/*
 * Invokes when new connecting to the server is made.
 */
void ICACHE_FLASH_ATTR http_connect(void *arg);

/*
 * Invokes when connected client disconnects from the server.
 */
void ICACHE_FLASH_ATTR http_disconnect(void *arg);

/*
 * Sends data to the server. 
 */
void ICACHE_FLASH_ATTR http_send(struct espconn *con, char *data); 
void ICACHE_FLASH_ATTR http_send_file(struct espconn *con, const char *str, const char *type);
void ICACHE_FLASH_ATTR http_send_string(struct espconn *con, const char *str, const char *type);

/*
 * Invokes when data is received. 
 */
void ICACHE_FLASH_ATTR http_receive(void *arg, char *pdata, unsigned short len);

/*
 * Extracts file name from HTTP request string.
 */
char ICACHE_FLASH_ATTR *http_get_file_name(char *pdata, unsigned short len);

/*
 * Extracts parameter param from HTTP request.
 */
char ICACHE_FLASH_ATTR *http_get_param(char *req, char *param);


// example files
#define HTML_INDEX_PAGE "<!DOCTYPE HTML><html><head>OK</head><body><div style='width: 400px; height: 800px; overflow:auto;' id='wdata'></div><script src='websocket_test.js'></script></body></html>"
#define WEBSOCKET_TEST_JS "window.onload = function() { "\
                          "socket = new WebSocket('ws://' + window.location.host + '/socket', '"WEBSOCKET_PROTOCOL_NAME"');"\
                          "socket.onmessage = function(e) { console.info(e); wd = document.getElementById('wdata'); wd.innerHTML += e.data + '<br>'; wd.scrollTop = wd.scrollHeight; }"\
                          "}"
