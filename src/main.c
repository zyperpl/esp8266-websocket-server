#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "uart.h"
#include "user_interface.h"
#include "espconn.h"

#include <math.h>

#include "http.h"
#include "network.h"

#define HTTP_PORT 80

#ifndef NETWORK_SSID
  #define NETWORK_SSID "WiFi_name_here"
#endif

#ifndef NETWORK_PASS
  #define NETWORK_PASS "WiFi_password_here"
#endif

#define VOLTAGE_TIMER_INTERVAL_S 5

static volatile os_timer_t ws_voltage_timer;

void ICACHE_FLASH_ATTR charrx(uint8_t c);
ws_receive_callback_t my_websocket_receive_callback(struct espconn *con, char *data, uint64_t length);
void ICACHE_FLASH_ATTR start();
void ICACHE_FLASH_ATTR wifi_callback(System_Event_t *evt);
void ICACHE_FLASH_ATTR send_data();

// example of user defined receive callback function
ws_receive_callback_t my_websocket_receive_callback(struct espconn *con, char *data, uint64_t length)
{
  ets_uart_printf("Received data from %p connection: %s\n", con, data); 
}

// example of function that sends data to all websockets
void ICACHE_FLASH_ATTR send_data()
{
  // measure voltage from ADC
  float v = (float)(system_adc_read()/1024.0F);
  char buf[64];
  os_memset(buf, 0, sizeof(buf));
  // save voltage to string
  os_sprintf(buf, "Voltage: %d.%dV", (int)v, (int)(v*1000)%1000);

  // send data to all websockets in the pool
  websocket_write_all(buf, strlen(buf), WEBSOCKET_TEXT);

  // print all connections
  struct espconn **connections = websocket_get_connections();
  os_printf("Websocket connections pool:\n");
  size_t i;
  for (i = 0; i < WEBSOCKET_CONNECTIONS_POOL_SIZE; i++)
  {
    os_printf("%p ", connections[i]);
  }
  os_printf("\n");
}

// called after initliazation is complete
void ICACHE_FLASH_ATTR start()
{ 
  // set wifis callbacks
  wifi_set_event_handler_cb(wifi_callback);

  // connecting to network
  network_connect(NETWORK_SSID, NETWORK_PASS);

  // starting HTTP and WebSocket server
  http_start(HTTP_PORT);

  // set websocket callbacks
  websocket_set_receive_callback((ws_receive_callback_t)my_websocket_receive_callback);

  // set data send timer
  os_timer_disarm(&ws_voltage_timer);
  os_timer_setfn(&ws_voltage_timer, (os_timer_func_t *)send_data, NULL);
  os_timer_arm(&ws_voltage_timer, VOLTAGE_TIMER_INTERVAL_S*1000, 1);
}

// main
void ICACHE_FLASH_ATTR user_init()
{
  // init gpio subsytem
  gpio_init();

  // set initialization done callback
  system_init_done_cb(start);
}


void ICACHE_FLASH_ATTR charrx(uint8_t c)
{ 
  ets_uart_printf("Key %c (%d)\n", c, c);
}

void ICACHE_FLASH_ATTR wifi_callback(System_Event_t *evt)
{
  ets_uart_printf("event %x\n", evt->event);
  switch (evt->event) 
  {
  case EVENT_STAMODE_CONNECTED:
    ets_uart_printf("connected to ssid %s, channel %d\n",
    evt->event_info.connected.ssid,
    evt->event_info.connected.channel);
    break;
  case EVENT_STAMODE_DISCONNECTED:
    ets_uart_printf("disconnected from ssid %s, reason %d\n",
    evt->event_info.disconnected.ssid,
    evt->event_info.disconnected.reason);
    break;
  case EVENT_STAMODE_GOT_IP:

    // print IP when connected to the network
    ets_uart_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n",
    IP2STR(&evt->event_info.got_ip.ip),
    IP2STR(&evt->event_info.got_ip.mask),
    IP2STR(&evt->event_info.got_ip.gw));
    break;
  case EVENT_SOFTAPMODE_STACONNECTED:
    ets_uart_printf("station: " MACSTR "join, AID = %d\n",
    MAC2STR(evt->event_info.sta_connected.mac),
    evt->event_info.sta_connected.aid);
    break;
  case EVENT_SOFTAPMODE_STADISCONNECTED:
    ets_uart_printf("station: " MACSTR "leave, AID = %d\n",
    MAC2STR(evt->event_info.sta_disconnected.mac),
    evt->event_info.sta_disconnected.aid);
    break;
  }
}


