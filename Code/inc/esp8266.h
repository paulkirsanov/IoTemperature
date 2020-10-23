#ifndef __ESP8266__H
#define __ESP8266__H

#include "usart.h"
#include "delay_tim1.h"
#include "stdio.h"
#include "ds18b20.h"
#include "main.h"

#define SOCKET_ERROR 1
#define INVALID_SOCKET 1

#define COMMAND_RESPONSE_TIMEOUT 10000
#define COMMAND_PING_TIMEOUT 3000
#define WIFI_CONNECT_TIMEOUT 30000
#define COMMAND_RESET_TIMEOUT 5000
#define CLIENT_CONNECT_TIMEOUT 5000

#define ESP8266_SOCK_NOT_AVAIL 255

#define ESP8266_USART USART1

typedef enum {
	ESP8266_CMD_BAD = -5,
	ESP8266_RSP_MEMORY_ERR = -4,
	ESP8266_RSP_FAIL = -3,
	ESP8266_RSP_UNKNOWN = -2,
	ESP8266_RSP_TIMEOUT = -1,
	ESP8266_RSP_SUCCESS = 0
}esp8266_cmd_rsp;

typedef enum {
	ESP8266_MODE_STA = 1,
	ESP8266_MODE_AP = 2,
	ESP8266_MODE_STAAP = 3
}esp8266_wifi_mode;

typedef enum {
	ESP8266_SLEEP_DISABLE,
	ESP8266_SLEEP_LIGHT,
	ESP8266_SLEEP_MODEM
}esp8266_sleep_mode;

typedef enum {
	ESP8266_CMD_QUERY,
	ESP8266_CMD_SETUP,
	ESP8266_CMD_EXECUTE
}esp8266_command_type;

typedef enum {
	ESP8266_ECN_OPEN,
	ESP8266_ECN_WPA_PSK,
	ESP8266_ECN_WPA2_PSK,
	ESP8266_ECN_WPA_WPA2_PSK
}esp8266_encryption;

typedef enum {
	ESP8266_STATUS_GOTIP = 2,
	ESP8266_STATUS_CONNECTED = 3,
	ESP8266_STATUS_DISCONNECTED = 4,
	ESP8266_STATUS_NOWIFI = 5	
}esp8266_connect_status;

typedef enum {
	AVAILABLE = 0,
	TAKEN = 1,
}esp8266_socket_state;

typedef enum {
	ESP8266_TCP,
	ESP8266_UDP,
	ESP8266_TYPE_UNDEFINED
}esp8266_connection_type;

typedef enum {
	ESP8266_CLIENT,
	ESP8266_SERVER
}esp8266_tetype;

typedef enum {
	OPEN,
	WEP,
	WPA_PSK,
	WPA2_PSK,
	WPA_WPA2_PSK
} esp8266_ecn;

struct esp8266_ipstatus
{
	uint8_t linkID;
	esp8266_connection_type type;
	char remoteIP[16];
	uint16_t remote_port;
	uint16_t locale_port;
	esp8266_tetype tetype;
};

typedef struct
{
	esp8266_connect_status stat;
	struct esp8266_ipstatus ipstatus;
}esp8266_status;

typedef struct
{
	char *server;
	char *port;
	esp8266_connection_type type;
}esp8266_client;

typedef struct
{
	char *ssid;
	char *pwd;
}esp8266_ap;

typedef struct
{
	esp8266_ecn ecn;
	char ssid[50];
	char rssi[5];
	char mac[17];
	uint8_t chn;
}esp8266_lap;

typedef struct
{
	esp8266_ap nameAP;
	uint8_t channel;
	esp8266_ecn ecn;
}esp8266_sap;

typedef struct
{
	uint8_t *buff;
	uint8_t len;
} buf;

bool esp8266Begin(void);
bool esp8266Reset(void);
bool esp8266Test(void);
bool esp8266DeepSleepMode(uint16_t time);
bool esp8266SleepMode(esp8266_sleep_mode mode);
bool esp8266WakeUpGPIO(void);
int16_t esp8266GetMode(void);
bool esp8266SetMode(esp8266_wifi_mode mode);
bool esp8266Connect(const esp8266_ap *ap, esp8266_wifi_mode wifi_mode);

bool esp8266TcpConnect(const esp8266_client *address_client);
bool esp8266TcpSend(uint8_t *buf, uint16_t size);
bool esp8266TcpClose(void);
bool esp8266SetMux(uint8_t mux);
int tcp_getdata(unsigned char* buf, int count);
bool esp8266TcpStatus(esp8266_status *status);
bool esp8266TcpSend_packet(esp8266_client *address_client, const char *text, buf *rr);

void esp8266SendCommand(const char * cmd, esp8266_command_type type, const char * params);
bool esp8266ReadForResponse(const char * rsp, unsigned int timeout);
bool esp8266ReadForResponses(const char * pass, const char * fail, unsigned int timeout);

bool esp8266ReadTcpData(void);
bool esp8266ReadTcpData_(char *data);
void esp8266ClearBuffer(void);
bool esp8266RxBufferAvailable(void); 
bool esp8266SearchBuffer(const char * test);

esp8266_status esp8266StatusPrint(void);
bool esp8266ListAP(esp8266_lap *dataAP);
bool esp8266Disconnect(void);
bool esp8266TcpStartServer(void);

static const char RESPONSE_OK[] = "OK\r\n";
static const char RESPONSE_ERROR[] = "ERROR\r\n";
static const char RESPONSE_FAIL[] = "FAIL";
static const char RESPONSE_READY[] = "ready";			//WIFI CONNECTED
static const char RESPONSE_RECEIVED[] = "+IPD,";
static const char RESPONSE_STATUS[] = "+CIPSTATUS";

static const char ESP8266_TEST[] = "";	// Test AT startup
static const char ESP8266_RESET[] = "+RST"; // Restart module
static const char ESP8266_VERSION[] = "+GMR"; // View version info
static const char ESP8266_GSLP[] = "+GSLP"; // Enter deep-sleep mode
static const char ESP8266_SLEEP_MODE[] = "+SLEEP"; // Enter sleep mode
static const char ESP8266_WAKEUPGPIO[] = "+WAKEUPGPIO"; // Set Wake Up GPIO
static const char ESP8266_ECHO_ENABLE[] = "E1"; // AT commands echo
static const char ESP8266_ECHO_DISABLE[] = "E0"; // AT commands echo
//!const char ESP8266_RESTORE[] = "+RESTORE"; // Factory reset
static const char ESP8266_UART[] = "+UART"; // UART configuration

static const char ESP8266_WIFI_MODE[] = "+CWMODE"; // WiFi mode (sta/AP/sta+AP)
static const char ESP8266_CONNECT_AP[] = "+CWJAP"; // Connect to AP
static const char ESP8266_LIST_AP[] = "+CWLAP"; // List available AP's
static const char ESP8266_DISCONNECT[] = "+CWQAP"; // Disconnect from AP
static const char ESP8266_AP_CONFIG[] = "+CWSAP"; // Set softAP configuration
static const char ESP8266_STATION_IP[] = "+CWLIF"; // List station IP's connected to softAP
//!const char ESP8266_DHCP_EN[] = "+CWDHCP"; // Enable/disable DHCP
//!const char ESP8266_AUTO_CONNECT[] = "+CWAUTOCONN"; // Connect to AP automatically
//!const char ESP8266_SET_STA_MAC[] = "+CIPSTAMAC"; // Set MAC address of station
static const char ESP8266_GET_STA_MAC[] = "+CIPSTAMAC"; // Get MAC address of station
//!const char ESP8266_SET_AP_MAC[] = "+CIPAPMAC"; // Set MAC address of softAP
//!const char ESP8266_SET_STA_IP[] = "+CIPSTA"; // Set IP address of ESP8266 station
//!const char ESP8266_SET_AP_IP[] = "+CIPAP"; // Set IP address of ESP8266 softAP

static const char ESP8266_TCP_STATUS[] = "+CIPSTATUS"; // Get connection status
static const char ESP8266_TCP_CONNECT[] = "+CIPSTART"; // Establish TCP connection or register UDP port
static const char ESP8266_TCP_SEND[] = "+CIPSEND"; // Send Data
static const char ESP8266_TCP_CLOSE[] = "+CIPCLOSE"; // Close TCP/UDP connection
static const char ESP8266_GET_LOCAL_IP[] = "+CIFSR"; // Get local IP address
static const char ESP8266_TCP_MULTIPLE[] = "+CIPMUX"; // Set multiple connections mode
static const char ESP8266_SERVER_CONFIG[] = "+CIPSERVER"; // Configure as server
static const char ESP8266_TRANSMISSION_MODE[] = "+CIPMODE"; // Set transmission mode
//!const char ESP8266_SET_SERVER_TIMEOUT[] = "+CIPSTO"; // Set timeout when ESP8266 runs as TCP server
static const char ESP8266_PING[] = "+PING"; // Function PING

static const char STATUS_CONNECTED[] = "STATUS:2";
static const char STATUS_TCP_CONNECTED[] = "STATUS:3";
static const char STATUS_TCP_DISCONNECT[] = "STATUS:4";
static const char STATUS_DISCONNECT[] = "STATUS:5";

//static uint8_t *buff1[] = { "GET /update?api_key=UHFL1R04OC12Y812&field1=", " HTTP/1.1\r\n", "Host: api.thingspeak.com\r\n", "Accept: */*\r\n", "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n", "\r\n" };

static esp8266_client tp_client = { "184.106.153.149", "80", ESP8266_TCP };
static esp8266_ap ap_client = { "My_WIFI", "pass0000"};
//static esp8266_ap ap_client = { "NikolGuest", "777555333"};

static esp8266_client ow_client = { "184.106.153.149", "80", ESP8266_TCP };
//lat=59.89
//lon=30.26
//appid=7404f7ffd0cddc73dcd0bff8e1540a2e


#endif
