#include "http.h"

ws_receive_callback_t receive_callback = NULL;

struct espconn *websocket_connections[WEBSOCKET_CONNECTIONS_POOL_SIZE] = { NULL };

void http_start(int16_t port)
{
  struct espconn *http;
  http = (struct espconn *)os_zalloc(sizeof(struct espconn));

	http->type = ESPCONN_TCP;
	http->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	http->proto.tcp->local_port = port;  
	espconn_regist_connectcb(http, http_connect);

  espconn_set_opt(http, ESPCONN_KEEPALIVE);

  espconn_accept(http);
}

char *http_get_file_name(char *pdata, unsigned short len)
{
  char *c = &pdata[4];
  char *str = (char*)os_zalloc(HTTP_FILE_MAX_LENGTH);

  while (*c != 0 && *c != ' ')
  {
    //ets_uart_printf("%c -> %d\n", *c, c-&pdata[4]);
    if (c-&pdata[4] >= HTTP_FILE_MAX_LENGTH || c-&pdata[4] >= len) break;
    str[c-&pdata[4]] = *c;
    c++;
  }

  return str;
}

void http_connect(void *arg)
{
  struct espconn *con = (struct espconn*)arg;

  espconn_set_opt(con, ESPCONN_KEEPALIVE);

  ets_uart_printf("\n connection from %d.%d.%d.%d:%d",
      con->proto.tcp->remote_ip[0], con->proto.tcp->remote_ip[1], 
      con->proto.tcp->remote_ip[2], con->proto.tcp->remote_ip[3], con->proto.tcp->remote_port);
  ets_uart_printf(" to %d.%d.%d.%d:%d\n", 
      con->proto.tcp->local_ip[0], con->proto.tcp->local_ip[1], 
      con->proto.tcp->local_ip[2], con->proto.tcp->local_ip[3], con->proto.tcp->local_port);
  
  espconn_set_opt(con, 0x04); // enable write buffer

	espconn_regist_recvcb(con, http_receive);
  espconn_regist_disconcb(con, http_disconnect);
}

void http_disconnect(void *arg)
{
  //*
  struct espconn *con = (struct espconn*)arg;

  ets_uart_printf("\n disconnected from %d.%d.%d.%d:%d",
      con->proto.tcp->remote_ip[0], con->proto.tcp->remote_ip[1], 
      con->proto.tcp->remote_ip[2], con->proto.tcp->remote_ip[3], con->proto.tcp->remote_port);
  ets_uart_printf(" on %d.%d.%d.%d:%d\n", 
      con->proto.tcp->local_ip[0], con->proto.tcp->local_ip[1], 
      con->proto.tcp->local_ip[2], con->proto.tcp->local_ip[3], con->proto.tcp->local_port);
  // */
}

void http_receive(void *arg, char *pdata, unsigned short len)
{
  struct espconn *con = (struct espconn*)arg;

  /*
  ets_uart_printf("\ndata from %d.%d.%d.%d:%d",
      con->proto.tcp->remote_ip[0], con->proto.tcp->remote_ip[1], 
      con->proto.tcp->remote_ip[2], con->proto.tcp->remote_ip[3], con->proto.tcp->remote_port);
  ets_uart_printf(" to %d.%d.%d.%d:%d\n", 
      con->proto.tcp->local_ip[0], con->proto.tcp->local_ip[1], 
      con->proto.tcp->local_ip[2], con->proto.tcp->local_ip[3], con->proto.tcp->local_port);

  int i;
  for (i = 0; i < len; i++)
  {
    ets_uart_printf("%c", pdata[i]);
  }
  // */

  // TODO: implement proper HTTP request parsing
  if (pdata[0] == 'G'
  ||  pdata[1] == 'E'
  ||  pdata[2] == 'T')
  {
    
    
    char *fileName = http_get_file_name(pdata, len);
    if (fileName)
    {
      ets_uart_printf("requested file: '%s'\n", fileName);

      if (strcmp(fileName, "/") == 0)
      {
        //TODO: get data from file
        http_send_string(con, HTML_INDEX_PAGE, "text/html");
      } else        
      if (strcmp(fileName, "/websocket_test.js") == 0)
      {
        http_send_string(con, WEBSOCKET_TEST_JS, "application/javascript");
      } else
      if (strcmp(fileName, "/socket") == 0)
      {
        char *param = strstr(pdata, "Upgrade: websocket");

        if (param)
        {
          char *protocol = http_get_param(pdata, "Sec-WebSocket-Protocol");
          
          if (strcmp(protocol, WEBSOCKET_PROTOCOL_NAME) == 0) 
          {
            char *key = http_get_param(pdata, "Sec-WebSocket-Key");

            char *accept = websocket_get_accept(key);

            char *header101 = "HTTP/1.1 101 Switching Protocols\nUpgrade: websocket\nConnection: Upgrade";
            char *fullData = (char*)os_zalloc(512);
            os_sprintf(fullData, "%s\nSec-WebSocket-Accept: %s\nSec-WebSocket-Protocol: %s\r\n\r\n", header101, accept, WEBSOCKET_PROTOCOL_NAME);

            espconn_send(con, fullData, strlen(fullData)); // sending with \0

            os_free(fullData);
            os_free(key);
            os_free(accept);

            // add socket to websocket poll
            websocket_add_connection(con);

          }
          else
          {
            char *header400 = "400 Bad Request\nConnection: close\r\n\r\nIncorrect request";
            espconn_send(con, header400, strlen(header400)-1);
            espconn_disconnect(con);
            ets_uart_printf("Wrong protocol (%s)!\n", protocol);
          }
          os_free(protocol);
        }
      } else
      {
        char *header404 = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404</h1><h2>&#x2639;</h2>";
        ets_uart_printf("not found: %s\n", fileName);
        espconn_send(con, header404, strlen(header404));
      }

    }
    
    os_free(fileName);
  }
}

void http_send_string(struct espconn *con, const char *str, const char *type)
{
  char *fullData = (char*)os_zalloc(HTTP_BUFFER_SIZE);
  size_t length = strlen(str);
  os_sprintf(fullData, "HTTP/1.1 200 OK\nContent-Type: %s;charset=utf-8\n\
Content-Length: %d\r\n\r\n%s", type, length, str);

  //espconn_send(con, fullData, strlen(fullData)-1);
  http_send(con, fullData);
  os_free(fullData);
  os_delay_us(HTTP_PAYLOAD_INTERVAL);
}

void http_send(struct espconn *con, char *data)
{
  size_t sizeToSend = strlen(data);
  size_t sizeSent = 0;

  while (sizeToSend)
  {
    // maximium size is size of payload
    size_t size = (sizeToSend > HTTP_PAYLOAD_SIZE) ? HTTP_PAYLOAD_SIZE : sizeToSend;

    system_soft_wdt_feed();    

    // send data
    int ret = espconn_send(con, data + (sizeSent), size);

    if (ret != 0)
    {
      ets_uart_printf("Sending %d bytes (from %d to %d) failed! ret=%d\n", 
          size, sizeSent, size+sizeSent, ret);
      /*switch (ret)
      {
        case ESPCONN_MEM:     ets_uart_printf("espconn: out of memory\n"); break;
        case ESPCONN_ARG:     ets_uart_printf("espconn: illegal argument\n"); break;
        case ESPCONN_MAXNUM:  ets_uart_printf("espconn: buffer of sending data is full\n"); break;
        case ESPCONN_IF:      ets_uart_printf("espconn: udp fail\n"); break;                        
      }
      break;
      // */
    }
    sizeToSend -= size;
    sizeSent += size;
    
    os_delay_us(HTTP_PAYLOAD_INTERVAL);
  }
 
  char buf[1];
  buf[0] = 0;
  espconn_send(con, buf, 1*sizeof(char));
}

char *http_get_param(char *req, char *param)
{
  size_t paramLen = strlen(param);

  char *valBeginPtr;
  valBeginPtr = strstr(req, param);
  valBeginPtr += paramLen;
  if (param[paramLen-1] != ' ')
  {
    if (param[paramLen-1] != ':') valBeginPtr += 2; // move pointer after string: ": "
    else valBeginPtr += 1;
  }

  char *ret = (char*)os_zalloc(128);
  memcpy(ret, valBeginPtr, strcspn(valBeginPtr, "\r\n"));

  return ret;
}

char *websocket_get_accept(char *key)
{
  strcat(key, WEBSOCKET_MAGIC); //concat with magic

  char *sha1key = (char*)os_malloc(20);
  SHA1(sha1key, key, strlen(key)); //sha1

  char *res = b64_encode(sha1key, strlen(sha1key)); //base64

  os_free(sha1key);  
  
  return res;
}

void websocket_add_connection(struct espconn *con)
{
  // set extra parameters
  espconn_set_opt(con, ESPCONN_KEEPALIVE);
  espconn_set_opt(con, ESPCONN_NODELAY);
  espconn_set_opt(con, ESPCONN_COPY);

  espconn_regist_recvcb(con, websocket_receive);

  // ping client
  websocket_write(con, WEBSOCKET_PING_MAGIC, strlen(WEBSOCKET_PING_MAGIC), WEBSOCKET_PING);

  //TODO: remove sending of test string
  websocket_write(con, "Hello", 5+1, WEBSOCKET_TEXT);

  // add to pool if space available
  size_t i;
  for (i = 0; i < WEBSOCKET_CONNECTIONS_POOL_SIZE; i++) {
    if (websocket_connections[i] == NULL) {
      websocket_connections[i] = con;
      break;
    }
  }
}

struct espconn **websocket_get_connections()
{
  return websocket_connections; 
}

void websocket_receive(void *arg, char *pdata, unsigned short len)
{
  struct espconn *con = (struct espconn*)arg;

  /*
  ets_uart_printf("\nWEBSOCKET data from %d.%d.%d.%d:%d",
      con->proto.tcp->remote_ip[0], con->proto.tcp->remote_ip[1], 
      con->proto.tcp->remote_ip[2], con->proto.tcp->remote_ip[3], con->proto.tcp->remote_port);
  ets_uart_printf(" to %d.%d.%d.%d:%d\n", 
      con->proto.tcp->local_ip[0], con->proto.tcp->local_ip[1], 
      con->proto.tcp->local_ip[2], con->proto.tcp->local_ip[3], con->proto.tcp->local_port);

  int i;
  for (i = 0; i < len; i++)
  {
    ets_uart_printf("%c (%d)", pdata[i], pdata[i]);
  }
  // */
  
  const unsigned int PACKET_HEADER_SIZE = 6;
  
  uint8_t opCode = pdata[0] & 0x0F;
  switch (opCode) 
  {
    case WEBSOCKET_CONT:
    case WEBSOCKET_TEXT:
    case WEBSOCKET_BIN: 
        pdata = websocket_decode(pdata, len);

        ets_uart_printf("Websocket. Received %d bytes. ", len);
        ets_uart_printf("Data:\n%s\n\n", pdata+PACKET_HEADER_SIZE);

        if (receive_callback != NULL) {
          //call user-defined callback
          receive_callback(con, pdata+PACKET_HEADER_SIZE, len);
        }

        // send the same data
        //websocket_write(con, pdata, len, WEBSOCKET_BIN);
        break;
    case WEBSOCKET_EXIT:
        ets_uart_printf("Exit received!\n");
        websocket_close(con);
        return;
        break;
    case WEBSOCKET_PING:
        ets_uart_printf("Ping received!\n");
        pdata = websocket_decode(pdata, len);
        websocket_write(con, pdata+PACKET_HEADER_SIZE, strlen(pdata+PACKET_HEADER_SIZE), WEBSOCKET_PONG);
        break;
    case WEBSOCKET_PONG:
        ets_uart_printf("Pong received.\n");
        pdata = websocket_decode(pdata, len);
        //ets_uart_printf("Pong data: %s\n", pdata+6);
        break;    
  }
}

void websocket_set_receive_callback(ws_receive_callback_t callback)
{
  receive_callback = callback;
}

void websocket_write(struct espconn *con, const void *data, uint64_t len, uint8_t mode)
{
  uint8_t *buf;

  if (len < 125)
  {
    buf = (uint8_t*)os_malloc(len+1+1);
    buf[0] = 0x80 | mode;
    buf[1] = len;
    memcpy(&buf[2], data, len);
    len += 2;

  } else
  if (len < 66536)
  {
    buf = (uint8_t*)os_malloc(len+1+1+2);
    buf[0] = 0x80 | mode;
    buf[1] = 126;
    buf[2] = (uint16_t)(len) >> 8;
    buf[3] = (uint16_t)(len);

    memcpy(&buf[4], data, len);
    len += 4;
  } else
  {
    buf = (uint8_t*)os_malloc(len+1+1+8);
    buf[0] = 0x80 | mode;
    buf[1] = 127;
    buf[2] = (len) >> 56;
    buf[3] = (len) >> 48;
    buf[4] = (len) >> 40;
    buf[5] = (len) >> 32;
    buf[6] = (len) >> 24;
    buf[7] = (len) >> 16;
    buf[8] = (len) >> 8;    
    buf[9] = (len);

    memcpy(&buf[10], data, len);
    len += 10;
  }

  espconn_send(con, buf, len);
  os_free(buf);
}

void websocket_write_all(const void *data, uint64_t len, uint8_t mode)
{
  size_t i;
  for (i = 0; i < WEBSOCKET_CONNECTIONS_POOL_SIZE; i++)
  {
    if (websocket_connections[i] != NULL) {
      websocket_write(websocket_connections[i], data, len, mode);
    }
  }
}

void websocket_close(struct espconn *con)
{
  char buf[] = {0x88, 0x02, 0x03, 0xe8};
  uint16_t len = sizeof(buf);
  espconn_send(con, buf, len);

  size_t i;
  for (i = 0; i < WEBSOCKET_CONNECTIONS_POOL_SIZE; i++)
  {
    // remove from the pool if found
    if (websocket_connections[i] == con) {
      websocket_connections[i] = NULL;
    }
  }
}

char *websocket_decode(char *pdata, unsigned short len)
{
  if (len > 6) len -= 6;

  int i;
  for (i = 0; i < len; i++)
      pdata[i + 6] ^= pdata[2 + i % 4];

  pdata[6+len] = 0;

  return pdata;
}
