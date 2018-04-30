# ESP8266 WebSocket server

Implementation and example of WebSocket (and simple HTTP) servers for ESP8266 microcontrollers in native C SDK from Espressif.

## WebSocket server

Server functions are declared in [http.h](src/http.h) file along with HTTP server and defined in [http.c](src/http.c) file.


### Important functions

```c
void http_start(int16_t port)
```
Starts HTTP and WebSocket server listening on the ```port```.

<br/>

```c
void ICACHE_FLASH_ATTR websocket_set_receive_callback(ws_receive_callback_t callback);
```
Users can define their own receive callback that gets called when websocket received new data. Callback must be of void type. 
Callbacks parameters are:
```struct espconn *con``` - with is current connection that sent data, ```char *data``` - pointer to the data and ```uint64_t length``` - length of that data.
Example of receive callback is presented here: [src/main.c](src/main.c#L35).
<br/>

```c
void websocket_write(struct espconn *con, const void *data, uint64_t len, uint8_t mode);
```
Writes ```data``` for specified connection ```con``` of length ```len``` in websocket ```mode```. Available modes are:<br/>
```WEBSOCKET_BIN``` - for binary data<br/>
```WEBSOCKET_TEXT``` - for strings<br/>


<br/>

```c
void websocket_write_all(const void *data, uint64_t len, uint8_t mode);
```
Writes to all websocket clients existing in the connections pool. Parameters are the same as for ```websocket_write```.

<br/>

```c
struct espconn **websocket_get_connections();
```
Returns espconn connections of websockets in the pool. 
This is not equal to all espconn connections made to the server or even all websocket clients, but only pool of clients that we can send data spontaneously. 
We can still respond to every connected client via receive callback.

<br/>

```c
void websocket_close(struct espconn *con);
```
Closes connection to websocket and removes it from the pool.

<br/>

```c
char *websocket_get_accept(char *key);
```
Generates ```Sec-WebSocket-Accept``` key based on key ```Sec-WebSocket-Key``` sent by the client and WEBSOCKET_MAGIC.

<br/>

```c
void websocket_add_connection(struct espconn *con);
```
Sets espconn connection parameters (such as KEEPALIVE and NODELAY) and adds connection to websocket pool connection.

<br/>

```c
char *websocket_decode(char *pdata, unsigned short len);
```
Unmasks data received from websocket.


<br/>

```c
void websocket_receive(void *arg, char *pdata, unsigned short len);
```
Gets called when raw data is received from websocket connection.



### Example
Project consist of [example web page and JS script](src/http.h#L133) and [main function](src/main.c#L85) that sends voltage on ADC every few seconds to all websockets in pool.


## License
Published under MIT license.
```
Copyright (c) 2018 Kacper Zybała

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
