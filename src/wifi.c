#include "sam.h"
#include <string.h>

#include "gpio.h"
#include "timer.h"
#include "spi.h"
#include "log.h"
#include "winc1500_api.h"
#include "rtc.h"

#include "wifi.h"


struct EventFlags eventFlags;

//stub functions written by user specific for MCU used for winc1500 wifi

void m2mStub_PinSet_CE(t_m2mWifiPinAction action){
	if (action == M2M_WIFI_PIN_HIGH){
		log_trace("Wifi Enable");
		gpio_setPin(WIFI_ENABLE_PORT, WIFI_ENABLE_PIN_bp);
	}
	else if (action == M2M_WIFI_PIN_LOW){
		log_trace("Wifi Disable");
		gpio_clearPin(WIFI_ENABLE_PORT, WIFI_ENABLE_PIN_bp);
	}
}

void m2mStub_PinSet_RESET(t_m2mWifiPinAction action){
	if (action == M2M_WIFI_PIN_HIGH){
		log_trace("Wifi Reset High");
		gpio_setPin(WIFI_RESET_PORT, WIFI_RESET_PIN_bp);
	}
	else if (action == M2M_WIFI_PIN_LOW){
		log_trace("Wifi Reset Low");
		gpio_clearPin(WIFI_RESET_PORT, WIFI_RESET_PIN_bp);
	}
}

void m2mStub_PinSet_SPI_SS(t_m2mWifiPinAction action){
	if (action == M2M_WIFI_PIN_HIGH){
		//log_trace("Wifi SS off");
		gpio_setPin(WIFI_SPI_SS_PORT, WIFI_SPI_SS_PIN_bp);
		//wifi_SPI_slaveDeselect();
		//uint32_t registerContents;
		//registerContents = REG_SPI_MR & ~SPI_MR_PCS_Msk;
		//REG_SPI_MR = registerContents | SPI_MR_PCS(0000);
	}
	else if (action == M2M_WIFI_PIN_LOW){
		//log_trace("Wifi SS Select");
		gpio_clearPin(WIFI_SPI_SS_PORT, WIFI_SPI_SS_PIN_bp);
		//REG_SPI_MR &= ~SPI_MR_PCS_Msk;
		//uint32_t registerContents;
		//registerContents = REG_SPI_MR & ~SPI_MR_PCS_Msk;
		//REG_SPI_MR = registerContents | SPI_MR_PCS(0001);
	}
}

uint32_t m2mStub_GetOneMsTimer(void){
	return timer_getTicks();//timerGetTicks(&timer_1ms);
}

void m2mStub_EintEnable(void){
	//log_trace("Enable IRQ");
	//gpio_getInterruptStatus(WIFI_IRQ_PORT, WIFI_IRQ_PIN_bp);
	gpio_interruptEnable(WIFI_IRQ_PORT, WIFI_IRQ_PIN_bp, INT_FALLING_EDGE);
}

void m2mStub_EintDisable(void){
	//log_trace("Disable IRQ");
	//gpio_getInterruptStatus(WIFI_IRQ_PORT, WIFI_IRQ_PIN_bp);
	gpio_interruptDisable(WIFI_IRQ_PORT, WIFI_IRQ_PIN_bp);
}

//void m2m_EintHandler(void){
//}

void m2mStub_SpiTxRx(uint8_t *p_txBuf, uint16_t txLen, uint8_t *p_rxBuf, uint16_t rxLen){
	//log_trace("TXRX:%d %d", txLen, rxLen);
	uint8_t rxByte; //variable to store received byte
	//log_info("txrx %d %d", txLen, rxLen);
	while (txLen>0 || rxLen>0){ //keep looping until there are no more bytes to send and receive
		if (txLen >0 && rxLen>0){
			rxByte = SPI_byteExchange(*p_txBuf);
			*p_rxBuf = rxByte;
			p_rxBuf++; //increment txBuffer pointer;
			rxLen--; //decrement number of bytes to send
			p_txBuf++; //increment txBuffer pointer;
			txLen--; //decrement number of bytes to send
		}
		else if (txLen > 0){
			rxByte = SPI_byteExchange(*p_txBuf);
			p_txBuf++; //increment txBuffer pointer;
			txLen--; //decrement number of bytes to send
		}
		else if (rxLen > 0){
			rxByte = SPI_byteExchange(DUMMYBYTE);
			*p_rxBuf = rxByte;
			p_rxBuf++; //increment txBuffer pointer;
			rxLen--; //decrement number of bytes to send
		}
		/*
		if (txLen>0){
		//log_info("tx0");
		rxByte = SPI_byteExchange(*p_txBuf);
		//log_info("txrx %d %d", *p_txBuf, rxByte);
		p_txBuf++; //increment txBuffer pointer;
		txLen--; //decrement number of bytes to send
		}
		else{
		//log_info("tx");
		//log_info("txrx %d %d", *p_txBuf, rxByte);
		rxByte = SPI_byteExchange(DUMMYBYTE);
		*p_rxBuf = rxByte; //write rxByte to buffer and increment rxBuffer pointer
		p_rxBuf++;
		rxLen--; //decrement bytes to receive
		}
		if (rxLen>0){
		rxByte = SPI_byteExchange(DUMMYBYTE);
		//log_info("rx");
		*p_rxBuf = rxByte; //write rxByte to buffer and increment rxBuffer pointer
		p_rxBuf++;
		rxLen--; //decrement bytes to receive
		}
		*/
	}
}

void clearWifiEventFlags(){
	eventFlags.driverInitComplete = 0;
	eventFlags.scanComplete = 0;
	eventFlags.scanResultsReady = 0;
	eventFlags.connectionStatusChanged = 0;
	eventFlags.ipAddressAssigned = 0;
	eventFlags.rssiReady = 0;
	eventFlags.provisionInfoReady = 0;
	eventFlags.wpsReady = 0;
	eventFlags.prngReady = 0;
}

//Called to notify the application of Wi-Fi events. See Wi-Fi Events in Section 7.1.
void m2m_wifi_handle_events(t_m2mWifiEventType eventCode, t_wifiEventData *p_eventData){
	log_trace ("CHECKING EVENTS");
	switch (eventCode)
	{
		case M2M_WIFI_DRIVER_INIT_EVENT:
		//g_driverInitComplete = true;
		log_info ("Event: Init Complete");
		eventFlags.driverInitComplete = TRUE;
		break;
		
		case M2M_WIFI_CONN_STATE_CHANGED_EVENT:
		// event data in p_eventData->connState
		//g_connectionStateChanged = true;
		log_info ("Event: Connection Changed");
		eventFlags.connectionStatusChanged = TRUE;
		break;
		
		case M2M_WIFI_SYS_TIME_EVENT:{
			log_info("EVENT: M2M_WIFI_SYS_TIME_EVENT");
			struct RTC_DateTime dateTime;
			if (rtc_configureNewTime(&dateTime, p_eventData->sysTime.u8Month,
			p_eventData->sysTime.u8Day,
			p_eventData->sysTime.u16Year,
			p_eventData->sysTime.u8Hour,
			p_eventData->sysTime.u8Minute,
			p_eventData->sysTime.u8Second, 1) < 0){
				log_error("Bad Date/Time Configuration");
			}
			
			int8_t result = rtc_setTime(&dateTime);
			if (result < 0){
				log_error("Could not set Date/Time: %d",result);
			}
			else{
				log_info("RTC Time Updated");
			}
			break;
		}
		case M2M_WIFI_CONN_INFO_RESPONSE_EVENT:
		// event data in p_eventData->connInfo
		//dprintf("   EVENT: M2M_WIFI_CONN_INFO_RESPONSE_EVENT\n");
		log_info("EVENT: M2M_WIFI_CONN_INFO_RESPONSE_EVENT");
		break;
		
		case M2M_WIFI_IP_ADDRESS_ASSIGNED_EVENT:
		// Occurs in STA mode when an IP address is assigned to the host MCU
		// Occurs in SoftAP mode when a client joins the AP network
		
		// event data in p_eventData->ipConfig
		//dprintf("   EVENT: M2M_WIFI_IP_ADDRESS_ASSIGNED_EVENT\n");
		eventFlags.ipAddressAssigned = TRUE;
		uint8_t ipAddress[4];
		uint8_t gateway[4];
		uint8_t subnet[4];
		uint8_t dns[4];
		
		ipAddress[0] = p_eventData->ipConfig.u32StaticIp >> 24 & 0xff;
		ipAddress[1] = p_eventData->ipConfig.u32StaticIp >> 16 & 0xff;
		ipAddress[2] = p_eventData->ipConfig.u32StaticIp >> 8 & 0xff;
		ipAddress[3] = p_eventData->ipConfig.u32StaticIp & 0xff;
		
		gateway[0] = p_eventData->ipConfig.u32Gateway >> 24 & 0xff;
		gateway[1] = p_eventData->ipConfig.u32Gateway >> 16 & 0xff;
		gateway[2] = p_eventData->ipConfig.u32Gateway >> 8 & 0xff;
		gateway[3] = p_eventData->ipConfig.u32Gateway & 0xff;
		
		subnet[0] = p_eventData->ipConfig.u32SubnetMask >> 24 & 0xff;
		subnet[1] = p_eventData->ipConfig.u32SubnetMask >> 16 & 0xff;
		subnet[2] = p_eventData->ipConfig.u32SubnetMask >> 8 & 0xff;
		subnet[3] = p_eventData->ipConfig.u32SubnetMask & 0xff;
		
		dns[0] = p_eventData->ipConfig.u32DNS >> 24 & 0xff;
		dns[1] = p_eventData->ipConfig.u32DNS >> 16 & 0xff;
		dns[2] = p_eventData->ipConfig.u32DNS >> 8 & 0xff;
		dns[3] = p_eventData->ipConfig.u32DNS & 0xff;
		
		log_info ("Event: Connected to Wifi Access Point");
		log_info ("IP Address: %d.%d.%d.%d", ipAddress[3], ipAddress[2], ipAddress[1], ipAddress[0]);
		log_info ("Gateway: %d.%d.%d.%d", gateway[3], gateway[2], gateway[1], gateway[0]);
		log_info ("Subnet: %d.%d.%d.%d", subnet[3], subnet[2], subnet[1], subnet[0]);
		log_info ("DNS: %d.%d.%d.%d", dns[3], dns[2], dns[1], dns[0]);
		
		wifiConnectionInfo.dns = p_eventData->ipConfig.u32DNS;
		wifiConnectionInfo.ipAddress = p_eventData->ipConfig.u32StaticIp;
		wifiConnectionInfo.gateway = p_eventData->ipConfig.u32Gateway;
		wifiConnectionInfo.subnetMask = p_eventData->ipConfig.u32SubnetMask;
		
		//g_ipAddressAssigned = true;
		break;
		
		case M2M_WIFI_WPS_EVENT:
		// event data in p_eventData->wpsInfo
		//dprintf("   EVENT: M2M_WIFI_WPS_EVENT\n");
		log_info("");
		//g_wpsReady = true;
		eventFlags.wpsReady = TRUE;
		break;
		
		case M2M_WIFI_IP_CONFLICT_EVENT:
		// event data in p_eventData->conflictedIpAddress
		//dprintf("   EVENT: M2M_WIFI_IP_CONFLICT_EVENT\n");
		log_info("EVENT: M2M_WIFI_IP_CONFLICT_EVENT");
		break;
		
		case M2M_WIFI_SCAN_DONE_EVENT:
		// event data in p_eventData->scanDone
		//dprintf("   EVENT: M2M_WIFI_SCAN_DONE_EVENT\n");
		//g_scanComplete = true;
		eventFlags.scanComplete = TRUE;
		break;
		
		case M2M_WIFI_SCAN_RESULT_EVENT:
		// event data in p_eventData->scanResult
		//dprintf("   EVENT: M2M_WIFI_SCAN_RESULT_EVENT\n");
		//g_scanResultReady = true;
		eventFlags.scanResultsReady = TRUE;
		break;
		
		case M2M_WIFI_RSSI_EVENT:
		// event data in p_eventData->rssi
		//g_rssiReady = true;
		eventFlags.rssiReady = TRUE;
		break;
		
		case M2M_WIFI_PROVISION_INFO_EVENT:
		// event data in p_eventData->provisionInfo
		log_info("EVENT: M2M_WIFI_PROVISION_INFO_EVENT");
		
		if (p_eventData->provisionInfo.u8Status == M2M_SUCCESS){
			log_trace("SSID: %s", p_eventData->provisionInfo.au8SSID);
			log_trace("PASS: %s", p_eventData->provisionInfo.au8Password);
			log_trace("SECT: %d", p_eventData->provisionInfo.u8SecType);
			
			m2m_wifi_connect ((char*)p_eventData->provisionInfo.au8SSID, strlen((char*)p_eventData->provisionInfo.au8SSID), p_eventData->provisionInfo.u8SecType, p_eventData->provisionInfo.au8Password,M2M_WIFI_CH_ALL);
			log_info("Connecting to WIFI");
			eventFlags.ipAddressAssigned = 0;
			wifiConnectionInfo.connected = 1;
		}
		else{
			m2m_wifi_stop_provision_mode(); //stop provisioning server
			log_info("Provision Error");
			//restart provisioning?
		}
		
		eventFlags.provisionInfoReady = TRUE;
		break;
		
		case M2M_WIFI_DEFAULT_CONNNECT_EVENT:{
			log_info("EVENT: M2M_WIFI_DEFAULT_CONNNECT_EVENT");
			wifiConnectionInfo.defaultConnectionResult = p_eventData->defaultConnInfo.s8ErrorCode;
			break;
		}
		case M2M_WIFI_PRNG_EVENT:
		//dprintf("   EVENT: M2M_WIFI_PRNG_EVENT\n");
		log_info("EVENT: M2M_WIFI_PRNG_EVENT");
		// event data in p_eventData->prng
		//g_prngReady = true;
		eventFlags.prngReady = TRUE;
		break;
		
		default:
		//dprintf("!!!! Unknown Wi-Fi event (%d)\r\n", eventCode);
		log_info("Unknown Wifi event (%d)", eventCode);
		break;
	}
}

//Called to notify the application of Socket events. See Socket Events in Section 7.2.
void m2m_socket_handle_events(SOCKET sock, t_m2mSocketEventType eventCode, t_socketEventData *p_eventData){
	switch (eventCode){
		case M2M_SOCKET_CONNECT_EVENT:{
			log_trace ("M2M_SOCKET_CONNECT_EVENT");
			if (p_eventData->connectResponse.error < 0){
				//close(sock); //do i need to close?
				//sock = -1;
				log_error ("Could not connect");
			}
			if (p_eventData->connectResponse.error == 0){
				log_info("Connected");
			}
			break;
		}
		case M2M_SOCKET_ACCEPT_EVENT:{
			log_info ("Connection From: IP");
			break;
		}
		case M2M_SOCKET_BIND_EVENT:{
			log_info ("Socket Bind");
			break;
		}
		case M2M_SOCKET_DNS_RESOLVE_EVENT:{
			log_info ("DNS Resolve");
			break;
		}
		case M2M_SOCKET_LISTEN_EVENT:{
			log_info ("Socket listening on port");
			break;
		}
		case M2M_SOCKET_PING_RESPONSE_EVENT:{
			log_info ("ping");
			break;
		}
		case M2M_SOCKET_RECVFROM_EVENT:{
			log_info ("rx udp");
			break;
		}
		case M2M_SOCKET_RECV_EVENT:{
			log_info ("rx tcp");
			break;
		}
		case M2M_SOCKET_SENDTO_EVENT:{
			log_info ("tx udp");
			break;
		}
		case M2M_SOCKET_SEND_EVENT:{
			log_info ("tx tcp");
			break;
		}
	}
}

//Called to notify the application of OTA events. See OTA Events in Section 7.3.
void m2m_ota_handle_events(t_m2mOtaEventType eventCode, t_m2mOtaEventData *p_eventData){

}

//Called to notify the application of error events. See OTA Events in Section 7.3. Error codes are described in wf_errors.h (see t_m2mWifiErrorCodes).
void m2m_error_handle_events(uint32_t errorCode){
	switch (errorCode){
		case M2M_SUCCESS:{
			log_error("M2M_SUCCESS");
			break;
		}
		case M2M_WIFI_INVALID_CHIP_REV_ERROR:{
			log_error("M2M_WIFI_INVALID_CHIP_REV_ERROR");
			break;
		}
		case M2M_WIFI_SET_CUST_INFO_ERROR:{
			log_error("M2M_WIFI_SET_CUST_INFO_ERROR");
			break;
		}
		case M2M_WIFI_SET_CUST_INFO_LEN_ERROR:{
			log_error("M2M_WIFI_SET_CUST_INFO_LEN_ERROR");
			break;
		}
		case M2M_WIFI_SCAN_OPTIONS_ERROR:{
			log_error("M2M_WIFI_SCAN_OPTIONS_ERROR");
			break;
		}
		case M2M_WIFI_SCAN_REGION_ERROR:{
			log_error("M2M_WIFI_SCAN_REGION_ERROR");
			break;
		}
		case M2M_WIFI_SCAN_IN_PROGRESS_ERROR:{
			log_error("M2M_WIFI_SCAN_IN_PROGRESS_ERROR");
			break;
		}
		case M2M_WIFI_SCAN_CHANNEL_ERROR:{
			log_error("M2M_WIFI_SCAN_CHANNEL_ERROR");
			break;
		}
		case M2M_WIFI_P2P_CHANNEL_ERROR:{
			log_error("M2M_WIFI_P2P_CHANNEL_ERROR");
			break;
		}
		case M2M_WIFI_AP_CONFIG_ERROR:{
			log_error("M2M_WIFI_AP_CONFIG_ERROR");
			break;
		}
		case M2M_WIFI_REQ_SLEEP_ERROR:{
			log_error("M2M_WIFI_REQ_SLEEP_ERROR");
			break;
		}
		case M2M_WIFI_DEVICE_NAME_TO_LONG_ERROR:{
			log_error("M2M_WIFI_DEVICE_NAME_TO_LONG_ERROR");
			break;
		}
		case M2M_WIFI_PROVISION_MODE_ERROR:{
			log_error("M2M_WIFI_PROVISION_MODE_ERROR");
			break;
		}
		case M2M_WIFI_CONNECT_ERROR:{
			log_error("M2M_WIFI_CONNECT_ERROR");
			break;
		}
		case M2M_WIFI_FIRMWARE_READ_ERROR:{
			log_error("M2M_WIFI_FIRMWARE_READ_ERROR");
			break;
		}
		case M2M_WIFI_FIRMWARE_MISMATCH_ERROR:{
			log_error("M2M_WIFI_FIRMWARE_MISMATCH_ERROR");
			break;
		}
		case M2M_WIFI_FIRMWARE_VERS_ZERO_ERROR:{
			log_error("M2M_WIFI_FIRMWARE_VERS_ZERO_ERROR");
			break;
		}
		case M2M_WIFI_FIRMWARE_REG_READ_2_ERROR:{
			log_error("M2M_WIFI_FIRMWARE_REG_READ_2_ERROR");
			break;
		}
		case M2M_WIFI_PRNG_GET_ERROR:{
			log_error("M2M_WIFI_PRNG_GET_ERROR");
			break;
		}
		case M2M_WIFI_BOOTROM_LOAD_FAIL_ERROR:{
			log_error("M2M_WIFI_BOOTROM_LOAD_FAIL_ERROR");
			break;
		}
		case M2M_WIFI_FIRMWARE_START_ERROR:{
			log_error("M2M_WIFI_FIRMWARE_START_ERROR");
			break;
		}
		case M2M_WIFI_WAKEUP_FAILED_ERROR:{
			log_error("M2M_WIFI_WAKEUP_FAILED_ERROR");
			break;
		}
		case M2M_WIFI_FALSE_INTERRUPT_ERROR:{
			log_error("M2M_WIFI_FALSE_INTERRUPT_ERROR");
			break;
		}
		case M2M_WIFI_INVALID_SIZE_ERROR:{
			log_error("M2M_WIFI_INVALID_SIZE_ERROR");
			break;
		}
		case M2M_WIFI_INVALID_GROUP_ERROR:{
			log_error("M2M_WIFI_INVALID_GROUP_ERROR");
			break;
		}
		case M2M_WIFI_INVALID_PACKET_SIZE_ERROR:{
			log_error("M2M_WIFI_INVALID_PACKET_SIZE_ERROR");
			break;
		}
		case M2M_WIFI_CHIP_REV_ERROR:{
			log_error("M2M_WIFI_CHIP_REV_ERROR");
			break;
		}
		case M2M_WIFI_INVALID_WIFI_EVENT_ERROR:{
			log_error("M2M_WIFI_INVALID_WIFI_EVENT_ERROR");
			break;
		}
		case M2M_WIFI_EVENT_READ_ERROR:{
			log_error("M2M_WIFI_EVENT_READ_ERROR");
			break;
		}
		case M2M_WIFI_HIF_RECEIVE_1_ERROR:{
			log_error("M2M_WIFI_HIF_RECEIVE_1_ERROR");
			break;
		}
		case M2M_WIFI_HIF_RECEIVE_2_ERROR:{
			log_error("M2M_WIFI_HIF_RECEIVE_2_ERROR");
			break;
		}
		case M2M_WIFI_FAILED_TO_WAKE_CHIP_ERROR:{
			log_error("M2M_WIFI_FAILED_TO_WAKE_CHIP_ERROR");
			break;
		}
		case M2M_WIFI_HIF_RECEIVE_3_ERROR:{
			log_error("M2M_WIFI_HIF_RECEIVE_3_ERROR");
			break;
		}
		case M2M_WIFI_HIF_RECEIVE_4_ERROR:{
			log_error("M2M_WIFI_HIF_RECEIVE_4_ERROR");
			break;
		}
		case M2M_WIFI_INVALID_OTA_RESPONSE_ERROR:{
			log_error("M2M_WIFI_INVALID_OTA_RESPONSE_ERROR");
			break;
		}
		case M2M_WIFI_MISMATCH_SESSION_ID_ERROR:{
			log_error("M2M_WIFI_MISMATCH_SESSION_ID_ERROR");
			break;
		}
		case M2M_WIFI_FLASH_WRITE_1_ERROR:{
			log_error("M2M_WIFI_FLASH_WRITE_1_ERROR");
			break;
		}
		case M2M_WIFI_FLASH_WRITE_2_ERROR:{
			log_error("M2M_WIFI_FLASH_WRITE_2_ERROR");
			break;
		}
		case M2M_WIFI_FLASH_WRITE_3_ERROR:{
			log_error("M2M_WIFI_FLASH_WRITE_3_ERROR");
			break;
		}
		case M2M_WIFI_FLASH_READ_ERROR:{
			log_error("M2M_WIFI_FLASH_READ_ERROR");
			break;
		}
	}
	
	
}

/////////////////
//STATE MACHINE// //change this to a case switch style state machine
/////////////////


wifi_state_t wifi_state_initialize(){//this needs tx and rx buffer
	//set LED indicators on gpio
	//set gpio pins (bring in from main)
	clearWifiEventFlags();
	m2m_wifi_init();
	wifi_tasks = wifi_state_waitForBoot;
	//initialize struct here
	wifiConnectionInfo.connected = 0;
	wifiConnectionInfo.defaultConnectionResult = -1;
	return WIFI_INITIALIZE;
}


wifi_state_t wifi_state_waitForBoot(){
	if (eventFlags.driverInitComplete == TRUE){
		log_trace("wifi ready");
		
		tstrM2mRev rev;
		nm_get_firmware_info(&rev);
		log_info("Chip ID: %d", rev.u32Chipid);
		log_info("Firmware: %d.%d patch %d",rev.u8DriverMajor, rev.u8DriverMinor, rev.u8FirmwarePatch);
		
		//get MAC address
		uint8_t mac_addr[6];
		m2m_wifi_get_mac_address(mac_addr);
		log_info("MAC: %h:%h:%h:%h:%h:%h",mac_addr[0], mac_addr[1], mac_addr[2],mac_addr[3], mac_addr[4], mac_addr[5]);
		
		wifi_tasks = wifi_state_connectToWifi;
	}
	
	return WIFI_WAIT_FOR_INIT;
}

wifi_state_t wifi_state_checkSavedConnection(){
	m2m_wifi_default_connect();
	wifi_tasks = wifi_state_connectToWifi;
	return 0; //state
}

wifi_state_t wifi_state_waitSavedConnection(){
	//wait for event result of default connection
	//failures
	if (wifiConnectionInfo.defaultConnectionResult > 0){
		if (wifiConnectionInfo.defaultConnectionResult == M2M_DEFAULT_CONN_FAIL){
			log_trace("Could not connect");
			//retry every minute
		}
		else if (wifiConnectionInfo.defaultConnectionResult == M2M_DEFAULT_CONN_SCAN_MISMATCH){
			log_trace("Scan mismatch");
			//fail
		}
		else if (wifiConnectionInfo.defaultConnectionResult == M2M_DEFAULT_CONN_EMPTY_LIST){
			log_trace("WAP list empty, starting provisioning");
			//start provisioning server
			
		}
	}
	//success
	if (wifiConnectionInfo.connected == 1){
		//connected
		
		//save connection using m2m_wifi_connect_sc()
		
		//go to waiting for data
	}
	
	return 0; //state
}

wifi_state_t wifi_state_connectToWifi(){
	//provisioning server
	tstrM2MAPConfig wifiConfig;
	strcpy((char*)wifiConfig.au8SSID, "WINC_SSID");
	wifiConfig.u8ListenChannel = 1;
	wifiConfig.u8SsidHide = SSID_MODE_VISIBLE;
	//wifiConfig.u8KeyIndx = 1;
	//wifiConfig.u8KeySz = WEP_104_KEY_STRING_SIZE;
	//strcpy((char*)wifiConfig.au8WepKey, "1234567890");
	wifiConfig.u8SecType = M2M_WIFI_SEC_WPA_PSK;
	strcpy((char*)wifiConfig.au8Key, "1234567890");
	wifiConfig.u8KeySz = 10;
	
	wifiConfig.au8DHCPServerIP[0] = 192;
	wifiConfig.au8DHCPServerIP[1] = 168;
	wifiConfig.au8DHCPServerIP[2] = 10;
	wifiConfig.au8DHCPServerIP[3] = 1;
	//m2m_wifi_enable_ap(&wifiConfig);
	log_trace("Starting Provisioning Server");
	m2m_wifi_start_provision_mode(&wifiConfig, "wincprov.com", 1);
	wifi_tasks = wifi_state_waitToConnect;
	
	/* //manual connect
	char *ssid = "JDM";
	char *auth = "@oxbow1182";
	m2m_wifi_connect (ssid, 3, M2M_WIFI_SEC_WPA_PSK, auth,M2M_WIFI_CH_ALL);
	log_info("Connecting to WIFI");
	wifi_tasks = wifi_state_waitToConnect;
	*/
	
	return 2;
}

wifi_state_t wifi_state_waitToConnect(){
	//change to this: if (wifi_connected == M2M_WIFI_CONNECTED)
	//m2m_wifi_get_connection_info();
	//if (eventFlags.ipAddressAssigned == TRUE){ //TRUE //connected to wifi with ip address
	if (wifiConnectionInfo.connected == 1 && eventFlags.ipAddressAssigned == TRUE){
		m2m_wifi_enable_sntp(1)	;
		//m2m_wifi_get_sytem_time();
		
		log_trace ("creating socket");
		/** Socket for client */
		static SOCKET tcp_client_socket = -1;
		/* Open client socket. */
		if (tcp_client_socket < 0) {
			if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				log_error("main: failed to create TCP client socket error!\r\n");
			}		}
		//configure & bind local IP
		//BIND is only for server listeing socket
		//struct sockaddr_in my_addr;
		//my_addr.sin_addr.s_addr = wifiConnectionInfo.ipAddress;
		//if (bind(tcp_client_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0){
		//	log_error ("Could not bind address");
		//}
		
		//configure connection
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = 8080;
		addr.sin_addr.s_addr = 0x0201A8C0;//0x738C6B61;//0x0A00A8C0; //ip address 10 0 168 192
		
		/* Initialize socket address structure. */
		addr.sin_family = AF_INET;
		addr.sin_port = _htons(8080); //puts into big endian
		//addr.sin_addr.s_addr = _htonl(0x0201A8C0);
		
		/* Connect server */
		int8_t ret;
		log_trace ("Connecting to server %d:%d",addr.sin_addr.s_addr, addr.sin_port);
		ret = connect(tcp_client_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		log_trace ("ret value %d",ret);
		if (ret < 0){
			close(tcp_client_socket);
			tcp_client_socket = -1;
			log_error("Could not connect to server");
		}
		wifi_tasks = wifi_state_connectToServer;
	}
	return 3;
}

wifi_state_t wifi_state_connectToServer(){
	
	
	return 4;
}

wifi_state_t wifi_state_waitForData(){
	return 0;
}

wifi_state_t wifi_state_dataTxRx(){
	return 0;
}
