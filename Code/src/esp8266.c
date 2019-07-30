#include "esp8266.h"
#include "stdio.h"

#define ESP8266_RX_BUFFER_LEN 	512
#define TCP_RX_BUFFER_LEN				128
char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
char tcpRxBuffer[TCP_RX_BUFFER_LEN];
volatile unsigned int bufferHead;
volatile unsigned int tcpBufferHead = 0;
volatile bool buffering = FALSE;

char receiveState = 0x00;

uint8_t buffer2[] = "Host: api.thingspeak.com\r\n";
uint8_t buffer3[] = "Accept: */*\r\n";
uint8_t buffer4[] = "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n";
uint8_t buffer5[] = "\r\n";

extern int count_error;
char text_error[10] = {0};

bool esp8266Begin()
{
	bool test = FALSE;
	
	while(!esp8266Reset());
	
	test = esp8266Test();
	if(test)
	{
		if (esp8266SetMux(0))
			return TRUE;
		return FALSE;
	}
	return FALSE;
}

bool esp8266Test()
{
	//esp8266SendCommand(ESP8266_TEST, ESP8266_CMD_EXECUTE, 0);
	esp8266ClearBuffer();
	usartSendArrar(ESP8266_USART, "AT\r\n");
	if(esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT))
		return TRUE;
	return FALSE;
}

bool esp8266Reset()
{
	esp8266ClearBuffer();
	
	usart_send_string(USART2, "Reset ESP8266\r\n");
	
	bool rc = FALSE;
	esp8266SendCommand(ESP8266_RESET, ESP8266_CMD_EXECUTE, 0);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	delay_ms(500);
	rc = esp8266ReadForResponse(RESPONSE_READY, COMMAND_RESPONSE_TIMEOUT);
	return rc;
}

bool esp8266Disconnect()
{
	esp8266ClearBuffer();
	
	usart_send_string(USART2, "Disconnect ESP8266\r\n");
	
	bool rc = FALSE;
	esp8266SendCommand(ESP8266_DISCONNECT, ESP8266_CMD_EXECUTE, 0);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return rc;
}

int16_t esp8266GetMode()
{
	bool rsp = FALSE;
	char* p, mode;
	esp8266SendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_QUERY, 0);
	rsp = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp)
	{
		// Then get the number after ':':
		p = strchr(esp8266RxBuffer, ':');
		if (p != NULL)
		{
			mode = *(p+1);
			if ((mode >= '1') && (mode <= '3'))
				return (mode - 48); // Convert ASCII to decimal
		}
		
		return ESP8266_RSP_UNKNOWN;
	}
	
	return rsp;
}

bool esp8266SetMode(esp8266_wifi_mode mode)
{
	char modeChar[2] = {0, 0};
	sprintf(modeChar, "%d", mode);
	esp8266SendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_SETUP, modeChar);
	
	return esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

bool esp8266Connect(const esp8266_ap *ap, esp8266_wifi_mode wifi_mode)
{
	// The ESP8266 can be set to one of three modes:
	//  1 - ESP8266_MODE_STA - Station only
	//  2 - ESP8266_MODE_AP - Access point only
	//  3 - ESP8266_MODE_STAAP - Station/AP combo
	bool rc = FALSE;
	
	if(wifi_mode == ESP8266_MODE_STA)
	{
		while(!esp8266Reset());
		
		if(esp8266SetMode(wifi_mode))
		{
			usart_send_string(USART2, "Mode set to station\r\n");
			esp8266ClearBuffer();
			usartSendArrar(ESP8266_USART, "AT");
			usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_CONNECT_AP);
			usartSendArrar(ESP8266_USART, "=\"");
			usartSendArrar(ESP8266_USART, (uint8_t *)ap->ssid);
			usartSendArrar(ESP8266_USART, "\"");
			if (ap->pwd != NULL)
			{
				usartSendArrar(ESP8266_USART, ",");
				usartSendArrar(ESP8266_USART, "\"");
				usartSendArrar(ESP8266_USART, (uint8_t *)ap->pwd);
				usartSendArrar(ESP8266_USART, "\"");
			}
			usartSendArrar(ESP8266_USART, "\r\n");
			rc = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT);
			return rc;
		}
	}
	else if(wifi_mode == ESP8266_MODE_AP)
	{
		while(!esp8266Reset());
		
		while(!esp8266Disconnect());
		
		if(esp8266SetMode(wifi_mode))
		{
			usartSendArrar(USART2, "Mode set to access point\r\n");
			esp8266ClearBuffer();
			
			usartSendArrar(ESP8266_USART, "AT");
			usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_AP_CONFIG);
			usartSendArrar(ESP8266_USART, "=\"");
			usartSendArrar(ESP8266_USART, (uint8_t *)ap->ssid);
			usartSendArrar(ESP8266_USART, "\"");
			if (ap->pwd != NULL)
			{
				usartSendArrar(ESP8266_USART, ",");
				usartSendArrar(ESP8266_USART, "\"");
				usartSendArrar(ESP8266_USART, (uint8_t *)ap->pwd);
				usartSendArrar(ESP8266_USART, "\"");
			}
			usartSendArrar(ESP8266_USART, ",");
			usart_send_data(ESP8266_USART, 0x39);
			usartSendArrar(ESP8266_USART, ",");
			usart_send_data(ESP8266_USART, 0x34);
			usartSendArrar(ESP8266_USART, "\r\n");
			rc = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT);
			return rc;			
		}
	}
	else if(wifi_mode == ESP8266_MODE_STAAP)
	{
		if(esp8266SetMode(wifi_mode))
		{
			usartSendArrar(USART2, "Mode set to access point and station\r\n");
			esp8266ClearBuffer();
		}
	}
	return FALSE;
}

bool esp8266TcpConnect(const esp8266_client *address_client)
{
	bool rsp = FALSE;
	esp8266ClearBuffer();
	usartSendArrar(ESP8266_USART, "AT");
	usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_TCP_CONNECT);
	usartSendArrar(ESP8266_USART, "=");
	
	if(address_client->type == ESP8266_TCP)
		usartSendArrar(ESP8266_USART, "\"TCP\",");
	else if(address_client->type == ESP8266_UDP)
		usartSendArrar(ESP8266_USART, "\"UDP\",");
	
	usartSendArrar(ESP8266_USART, "\"");
	usartSendArrar(ESP8266_USART, (uint8_t *)address_client->server);
	usartSendArrar(ESP8266_USART, "\",");
	usartSendArrar(ESP8266_USART, (uint8_t *)address_client->port);
	usartSendArrar(ESP8266_USART, "\r\n");

	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT);
	
	if(rsp == FALSE)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		rsp = esp8266SearchBuffer("ALREADY");
		if (rsp)
			return TRUE;
		// Otherwise the connection failed. Return the error code:
		return FALSE;
	}
	// Return 1 on successful (new) connection
	return TRUE;
}

bool esp8266TcpSend(uint8_t *buf, uint16_t size)
{
	bool rsp = FALSE;
	uint8_t i = 0;
	uint8_t *p = buf;
	char params[8];
	if (size > 2048)
		return FALSE;
	sprintf(params, "%d", size);
	esp8266SendCommand(ESP8266_TCP_SEND, ESP8266_CMD_SETUP, params);
	
	rsp = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
	
	if(rsp)
	{
		esp8266ClearBuffer();
		//usartSendArrar(ESP8266_USART, buf);
		//usartSendArrar(ESP8266_USART, "\r");
		for(i = 0; i < size; i++)
		{
//			USART_ClearFlag(ESP8266_USART,USART_FLAG_TC);
			usart_send_data(ESP8266_USART, *p);
//			while(USART_GetFlagStatus(ESP8266_USART, USART_FLAG_TC) == RESET);
			p++;
		}
		
		usartSendArrar(USART2, (uint8_t *)" START ");
		
		if(esp8266ReadForResponse("SEND OK", COMMAND_RESPONSE_TIMEOUT))
		{
			usartSendArrar(USART2, (uint8_t *)"SEND OK\t");
			return TRUE;
		}
	}
	return FALSE;
}

bool esp8266TcpClose()
{
	bool rc = FALSE;
	esp8266SendCommand(ESP8266_TCP_CLOSE, ESP8266_CMD_EXECUTE, 0);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return rc;
}

bool esp8266TcpStatus(esp8266_status *status)
{
	bool rc = FALSE;
	char *p;
	uint8_t i = 0;
	char temp_buffer[20];
	
	esp8266SendCommand(ESP8266_TCP_STATUS, ESP8266_CMD_EXECUTE, 0);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if(rc)
	{
		p = (char *)strstr((const char *)esp8266RxBuffer, "STATUS:");
		if (p)
		{
			p += 7;

			while (*(p) != '\r')
			{
				temp_buffer[i++] = *p++;
			}

			temp_buffer[i] = '\0';
			int result = atoi(temp_buffer);
			switch (result)
			{
			case ESP8266_STATUS_GOTIP:
				status->stat = ESP8266_STATUS_GOTIP;
				break;
			case ESP8266_STATUS_CONNECTED:
				status->stat = ESP8266_STATUS_CONNECTED;
				break;
			case ESP8266_STATUS_DISCONNECTED:
				status->stat = ESP8266_STATUS_DISCONNECTED;
				break;
			case ESP8266_STATUS_NOWIFI:
				status->stat = ESP8266_STATUS_NOWIFI;
				break;
			}

			/* ------------------------------------------ */

			memset(temp_buffer, 0, 20);
			i = 0;
			p = (char *)strstr((const char *)esp8266RxBuffer, "+CIPSTATUS:");
			if (p)
			{
				p += 11;
				while (*p != ',')
				{
					temp_buffer[i++] = *p;
					p++;
				}

				temp_buffer[i] = '\0';
				result = atoi(temp_buffer);
				status->ipstatus.linkID = result;

				/* ------------------------------------------ */

				memset(temp_buffer, 0, 20);
				i = 0;

				p += 2;
				while (*p != '\"')
				{
					temp_buffer[i++] = *p++;
				}

				temp_buffer[i] = '\0';

				if (strcmp((const char *)temp_buffer, "TCP") == 0)
				{
					status->ipstatus.type = ESP8266_TCP;
				}
				else if (strcmp((const char *)temp_buffer, "UDP") == 0)
				{
					status->ipstatus.type = ESP8266_UDP;
				}
				else
					status->ipstatus.type = ESP8266_TYPE_UNDEFINED;

				/* ------------------------------------------ */
				memset(temp_buffer, 0, 20);
				i = 0;
				result = 0;
				p += 3;
				while (*p != '\"')
				{
					status->ipstatus.remoteIP[i++] = *p++;
				}

				temp_buffer[i] = '\0';

				/* ------------------------------------------ */

				memset(temp_buffer, 0, 20);
				i = 0;
				result = 0;

				p += 2;

				while (*p != ',')
				{
					temp_buffer[i++] = *p;
					p++;
				}

				temp_buffer[i] = '\0';
				result = atoi(temp_buffer);
				status->ipstatus.remote_port = result;

				/* ------------------------------------------ */

				memset(temp_buffer, 0, 20);
				i = 0;
				result = 0;

				p += 1;

				while (*p != '\r')
				{
					temp_buffer[i++] = *p;
					p++;
				}
				result = atoi(temp_buffer);
				if (result == ESP8266_CLIENT)
				{
					status->ipstatus.tetype = ESP8266_CLIENT;
				}
				else if (result == ESP8266_SERVER)
				{
					status->ipstatus.tetype = ESP8266_SERVER;
				}
				return TRUE;
			}
			return TRUE;
		}
		else return FALSE;
	}
	else
		return FALSE;
}

bool esp8266TcpStartServer()
{
	bool rc = FALSE;
	
	usartSendArrar(USART2, "Configure ESP8266 server\r\n");
	
	esp8266ClearBuffer();
	esp8266SetMux(1);
	esp8266ClearBuffer();
	
	usartSendArrar(ESP8266_USART, "AT");
	usartSendArrar(ESP8266_USART, (uint8_t *)ESP8266_SERVER_CONFIG);
	usartSendArrar(ESP8266_USART, "=");
	usart_send_data(ESP8266_USART, 0x31);
	usartSendArrar(ESP8266_USART, ",");
	usart_send_data(ESP8266_USART, 0x38);
	usart_send_data(ESP8266_USART, 0x30);
	usartSendArrar(ESP8266_USART, "\r\n");
	rc = esp8266ReadForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT);
	return rc;
}

bool esp8266SetMux(uint8_t mux)
{
	bool rc = FALSE;
	char params[2] = {0, 0};
	params[0] = (mux > 0) ? '1' : '0';
	esp8266SendCommand(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, params);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	return rc;
}

int tcp_getdata(unsigned char* buf, int count)
{
	int i;
	if(count <= TCP_RX_BUFFER_LEN)
	{
		for(i = 0; i < count; i++)
		{
			*(buf + i) = tcpRxBuffer[i];
		}
		for(i = 0; i < TCP_RX_BUFFER_LEN - count; i++)
		{
			tcpRxBuffer[i] = tcpRxBuffer[i+count];
		}
		return count;
	}
	else
	{
		return -1;
	}
}

void esp8266TcpSend_packet(esp8266_client *address_client, const char *data_packet)
{
	while(!esp8266TcpConnect(address_client))
	{
		usartSendArrar(USART2, (uint8_t *)"Waiting for connection...\r\n");
		/*if(esp8266StatusPrint().stat == ESP8266_STATUS_DISCONNECTED)
		{
			usartSendArrar(USART2, (uint8_t *)"STATUS DISCONNECTED\t");
			sprintf(text_error, "%d", ++count_error);
			LCDI2C_setCursor(5, 1);
			LCDI2C_write_String(text_error);
		}*/
	}
	
	usartSendArrar(USART2, (uint8_t *)"Tcp connection is open\r\n");
	
	if(esp8266StatusPrint().stat == ESP8266_STATUS_CONNECTED)
	{
		uint8_t buffer1_part1[] = "GET /update?api_key=UHFL1R04OC12Y812&field1=";
		uint8_t buffer1_part2[] = " HTTP/1.1\r\n";
		
		uint8_t length_part1 = strlen((char *)buffer1_part1);
		uint8_t length_part2 = strlen((char *)buffer1_part2);
		uint8_t length_data = strlen((char *)data_packet);
		uint8_t legth_total = length_part1 + length_data + length_part2;
		
		uint8_t length_buffer2 = strlen((char *)buffer2);
		uint8_t length_buffer3 = strlen((char *)buffer3);
		uint8_t length_buffer4 = strlen((char *)buffer4);
		uint8_t length_buffer5 = strlen((char *)buffer5);
		
		uint8_t *total_buffer = 0;
		
		uint8_t *buffer1 = (uint8_t*)malloc(legth_total);
		strcpy((char *)buffer1, (char *)buffer1_part1);
		strncat((char *)buffer1, (char *)data_packet, length_data);
		strncat((char *)buffer1, (char *)buffer1_part2, length_part2);
		esp8266TcpSend(buffer1, legth_total);
		free(buffer1);
		delay_ms(20);
		total_buffer = (uint8_t*)malloc(length_buffer2);
		strcpy((char *)total_buffer, (char *)buffer2);
		esp8266TcpSend(total_buffer, length_buffer2);
		free(total_buffer);
		delay_ms(20);
		total_buffer = (uint8_t*)malloc(length_buffer3);
		strcpy((char *)total_buffer, (char *)buffer3);
		esp8266TcpSend(total_buffer, length_buffer3);
		free(total_buffer);
		delay_ms(20);
		total_buffer = (uint8_t*)malloc(length_buffer4);
		strcpy((char *)total_buffer, (char *)buffer4);
		esp8266TcpSend(total_buffer, length_buffer4);
		free(total_buffer);
		delay_ms(20);
		total_buffer = (uint8_t*)malloc(length_buffer5);
		strcpy((char *)total_buffer, (char *)buffer5);
		esp8266TcpSend(total_buffer, length_buffer5);
		free(total_buffer);
		delay_ms(20);
		
		usartSendArrar(USART2, (uint8_t *)"\r\nPacket sent\r\n");
		
		if(esp8266TcpClose())
		{
			usartSendArrar(USART2, (uint8_t *)"TCP connection is closed\r\n");
		}
	}
}

void esp8266SendCommand(const char * cmd, esp8266_command_type type, const char * params)
{
	esp8266ClearBuffer();
	usartSendArrar(ESP8266_USART, "AT");
	usartSendArrar(ESP8266_USART, (uint8_t *)cmd);
	if (type == ESP8266_CMD_QUERY)
		usartSendArrar(ESP8266_USART, "?");
	else if (type == ESP8266_CMD_SETUP)
	{
		usartSendArrar(ESP8266_USART, "=");
		usartSendArrar(ESP8266_USART, (uint8_t *)params);
	}
	usartSendArrar(ESP8266_USART, "\r\n");
}

bool esp8266ReadForResponse(const char * rsp, unsigned int timeout)
{
	unsigned long timeIn = millis();	// Timestamp coming into function
	while (timeIn + timeout > millis()) // While we haven't timed out
	{
		if (esp8266RxBufferAvailable()) // If data is available on ESP8266_USART RX
		{
			if (esp8266SearchBuffer(rsp))	// Search the buffer for goodRsp
				return TRUE;
		}
	}
	return FALSE; // Return the timeout error code
}

bool esp8266ReadForResponses(const char * pass, const char * fail, unsigned int timeout)
{
	bool rc = FALSE;
	unsigned long timeIn = millis();	// Timestamp coming into function
	while (timeIn + timeout > millis()) // While we haven't timed out
	{
		if (esp8266RxBufferAvailable()) // If data is available on UART RX
		{
			rc = esp8266SearchBuffer(pass);
			if (rc)	// Search the buffer for goodRsp
				return TRUE;	// Return how number of chars read
			rc = esp8266SearchBuffer(fail);
			if (rc)
				return FALSE;
		}
	}
	return FALSE;
}

void esp8266ClearBuffer()
{
	memset(esp8266RxBuffer, '\0', ESP8266_RX_BUFFER_LEN);
	bufferHead = 0;
}

bool esp8266ReadTcpData()
{
	bool rc;
	int len = 0;
	uint8_t i, byteOfLen = 0;
	char *p;
	rc = esp8266ReadForResponse(RESPONSE_RECEIVED, COMMAND_RESPONSE_TIMEOUT);
	if(rc)
	{
		p = strstr((const char *)esp8266RxBuffer, "+IPD,");
		if(p)
		{
			p += 5;
			while(*p != ':')
			{
				if(byteOfLen == 0)
					len += (int)(*p - '0');
				else if(byteOfLen == 1)
					len = (int)(*p - '0') + len * 10;
				else if(byteOfLen == 2)
					len += (int)(*p - '0') + len * 10;
				p++;
				if(byteOfLen >= 3)
				{
					byteOfLen = 0;
					return FALSE;
				}
				byteOfLen++;
			}
			p++;
			for(i = 0; i < len; i++)
			{
				tcpRxBuffer[i] = *p;
				p++;
			}
			esp8266ClearBuffer();
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
		return FALSE;
	}
}

bool esp8266ReadTcpData_(char *data)
{
	bool rc;
	int len = 0;
	uint8_t i = 0;
	char *p;
	
	rc = esp8266ReadForResponse(RESPONSE_RECEIVED, COMMAND_RESPONSE_TIMEOUT);
	if(rc)
	{
		p = strstr((const char *)esp8266RxBuffer, "+IPD,");
		if(p)
		{
			delay_ms(50);
			
			p += 5;
			len = atoi(p);
			
			while(*p != ':')
			{
				p++;
			}
			p++;
			
			for(i = 0; i < len; i++)
			{
				tcpRxBuffer[i] = *p;
				p++;
			}
			
			strcpy(data, tcpRxBuffer);
			memset(tcpRxBuffer, 0, 128);
			esp8266ClearBuffer();
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
		return FALSE;
	}
}

bool esp8266RxBufferAvailable()
{
	return (bufferHead > 0) ? TRUE:FALSE;
}

bool esp8266SearchBuffer(const char * test)
{
	int i =0;
	int bufferLen = strlen((const char *)esp8266RxBuffer);
	// If our buffer isn't full, just do an strstr
	if (bufferLen < ESP8266_RX_BUFFER_LEN)
	{
		if(strstr((const char *)esp8266RxBuffer, test))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{	//! TODO
		// If the buffer is full, we need to search from the end of the 
		// buffer back to the beginning.
		int testLen = strlen(test);
		for (i=0; i<ESP8266_RX_BUFFER_LEN; i++)
		{
			
		}
	}
	return FALSE;
}

void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_SR_RXNE)
	{
		USART1->SR &= ~USART_SR_RXNE;
		
		esp8266RxBuffer[bufferHead++] = ESP8266_USART->DR;
	}
	
	if(bufferHead >= ESP8266_RX_BUFFER_LEN)
	{
		bufferHead = 0;
	}
}

esp8266_status esp8266StatusPrint(void)
{
	esp8266_status cur_status = { ESP8266_STATUS_NOWIFI, {0} };
	char str_b[50];
	
	if(esp8266TcpStatus(&cur_status))
	{
		if(cur_status.stat == ESP8266_STATUS_GOTIP)
		{
			usartSendArrar(USART2, (uint8_t *)"STATUS GOTIP\t");
		}
		if(cur_status.stat == ESP8266_STATUS_CONNECTED)
		{
			usartSendArrar(USART2, (uint8_t *)"STATUS CONNECTED\t");
		}
		if(cur_status.stat == ESP8266_STATUS_DISCONNECTED)
		{
			usartSendArrar(USART2, (uint8_t *)"STATUS DISCONNECTED\t");
		}
		if(cur_status.stat == ESP8266_STATUS_NOWIFI)
		{
			usartSendArrar(USART2, (uint8_t *)"STATUS NOWIFI\r\n");
		}
		
		if(cur_status.stat == ESP8266_STATUS_CONNECTED)
		{
			sprintf(str_b, "%d\t", cur_status.ipstatus.linkID);
			usartSendArrar(USART2, (uint8_t *)str_b);
			
			if(cur_status.ipstatus.type == ESP8266_TCP)
				usartSendArrar(USART2, (uint8_t *)"TCP\t");
			else if(cur_status.ipstatus.type == ESP8266_UDP)
				usartSendArrar(USART2, (uint8_t *)"UDP\t");
			else
				usartSendArrar(USART2, (uint8_t *)"TYPE UNDEFINED\t");
			
			memset(str_b, 0, 50);
			sprintf(str_b, "%s\t", cur_status.ipstatus.remoteIP);
			usartSendArrar(USART2, (uint8_t *)str_b);
			
			memset(str_b, 0, 50);
			sprintf(str_b, "%d\t", cur_status.ipstatus.remote_port);
			usartSendArrar(USART2, (uint8_t *)str_b);
			
			if(cur_status.ipstatus.tetype == ESP8266_CLIENT)
				usartSendArrar(USART2, (uint8_t *)"CLIENT\r\n");
			else if(cur_status.ipstatus.tetype == ESP8266_SERVER)
				usartSendArrar(USART2, (uint8_t *)"SERVER\r\n");
		}
	}
	return cur_status;
}

bool esp8266ListAP(esp8266_lap *dataAP)
{
	char *p;
	bool rc = FALSE;
	char temp_buffer[20] = {0};
	char temp_ecn = 0;
	uint8_t i = 0, x = 0;
	
	char data_str[10];
	
	esp8266SendCommand(ESP8266_LIST_AP, ESP8266_CMD_EXECUTE, 0);
	rc = esp8266ReadForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if(rc)
	{
		p = (char *)strstr((const char *)esp8266RxBuffer, "+CWLAP:");
		while (p)
		{
			if (p)
			{
				p += 8;
				temp_ecn = *p;

				switch (temp_ecn)
				{
				case '0':
					dataAP[i].ecn = OPEN;
					break;
				case '1':
					dataAP[i].ecn = WEP;
					break;
				case '2':
					dataAP[i].ecn = WPA_PSK;
					break;
				case '3':
					dataAP[i].ecn = WPA2_PSK;
					break;
				case '4':
					dataAP[i].ecn = WPA_WPA2_PSK;
					break;
				default:
					break;
				}

				p += 3;
				while (*p != '\"')
				{
					dataAP[i].ssid[x++] = *p++;
				}

				p += 2;
				x = 0;
				while (*p != ',')
				{
					temp_buffer[x++] = *p++;
				}
				
				strcpy(dataAP[i].rssi, temp_buffer);

				p += 2;
				x = 0;
				while (*p != ',')
				{
					dataAP[i].mac[x++] = *p++;
				}

				p += 1;
				x = 0;
				memset(temp_buffer, 0, 20);
				while (*p != ')')
				{
					temp_buffer[x++] = *p++;
				}
				
				dataAP[i].chn = atoi(temp_buffer);
				
				x = 0;

				if ((char *)strstr(p, "+CWLAP:"))
				{
					while (*p != '+')
						*p++;
					i++;
				}
				else
				{
					for (uint8_t y = 0; y != i + 1; y++)
					{
						sprintf(data_str, "%d %s", y + 1, ". ");
						usartSendArrar(USART2, (uint8_t *)data_str);
						
						switch (dataAP[y].ecn)
						{
						case 0:
							usartSendArrar(USART2, (uint8_t *)"OPEN\t");
							break;
						case 1:
							usartSendArrar(USART2, (uint8_t *)"WEP\t");
							break;
						case 2:
							usartSendArrar(USART2, (uint8_t *)"WPA_PSK\t");
							break;
						case 3:
							usartSendArrar(USART2, (uint8_t *)"WPA2_PSK\t");
							break;
						case 4:
							usartSendArrar(USART2, (uint8_t *)"WPA_WPA2_PSK\t");
							break;
						}
						
						usartSendArrar(USART2, (uint8_t *)dataAP[y].ssid);
						usartSendArrar(USART2, (uint8_t *)"\t");
						usartSendArrar(USART2, (uint8_t *)dataAP[y].rssi);
						usartSendArrar(USART2, (uint8_t *)"\t");
						usartSendArrar(USART2, (uint8_t *)dataAP[y].mac);
						usartSendArrar(USART2, (uint8_t *)"\t");
						sprintf(data_str, "%d\r\n", dataAP[y].chn);
						usartSendArrar(USART2, (uint8_t *)data_str);
					}
					
					return TRUE;
				}
			}
			
		}
	}
	
	return FALSE;
}
