#pragma once

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "uart.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

/*
 * Connects to the network
 */
int ICACHE_FLASH_ATTR network_connect(const char *ssid, const char *pass);
