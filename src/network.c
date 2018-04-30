#include "network.h"

struct espconn *outConn;

int network_connect(const char *ssid, const char *pass)
{
  int mode = STATION_MODE;

  ets_uart_printf("OPMODE: %d\n", mode);

  wifi_status_led_uninstall();

  wifi_set_opmode(mode);
  wifi_set_broadcast_if(mode);
  wifi_set_user_fixed_rate(FIXED_RATE_MASK_NONE, PHY_RATE_48);
}
