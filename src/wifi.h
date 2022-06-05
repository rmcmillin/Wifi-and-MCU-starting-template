
#define TRUE	1
#define FALSE	0

typedef enum wifi_states{
	WIFI_STATE_ERROR,
	WIFI_INITIALIZE,
	WIFI_WAIT_FOR_INIT,
	WIFI_CONNECT_AP,
	WIFI_CONNECT_SERVER,
	WIFI_WAIT_FOR_DATA,
	WIFI_TXRX,
}wifi_state_t;

struct EventFlags{
	uint8_t driverInitComplete:1;
	uint8_t scanComplete:1;
	uint8_t scanResultsReady:1;
	uint8_t connectionStatusChanged:1;
	uint8_t ipAddressAssigned:1;
	uint8_t rssiReady:1;
	uint8_t provisionInfoReady:1;
	uint8_t wpsReady:1;
	uint8_t prngReady:1;
};

void clearWifiEventFlags();

/////////////////
//State Machine//
/////////////////

struct WifiConnectionInfo{
	//32bit or 8bit array of 4?
	uint32_t ipAddress;
	uint32_t gateway;
	uint32_t subnetMask;
	uint32_t dns;
	uint8_t connected;
	uint8_t defaultConnectionResult;
}wifiConnectionInfo;

wifi_state_t wifi_currentState;
wifi_state_t (*wifi_tasks)(); //state machine pointer to current state

//these needs tx and rx buffer or parameters, circular buffer of events and associated data
wifi_state_t wifi_state_initialize(); //setup hardware -> set up gpio, set ouputs for en, wake, reset, Init Buffers, set LED, set ssid, passphrase
wifi_state_t wifi_state_waitForBoot(); //wait for M2M_WIFI_DRIVER_INIT_EVENT
wifi_state_t wifi_state_connectToWifi(); //connect manual
wifi_state_t wifi_state_waitToConnect(); //wait for connecction or error event
wifi_state_t wifi_state_provisionServer(); //run provisioning server???? or check for stored ssid/passphrase -> button on main board to reset?
wifi_state_t wifi_state_connectToWifi(); //connect to WAP
wifi_state_t wifi_state_waitForWLAN(); //wait for M2M_WIFI_CONN_STATE_CHANGED_EVENT
wifi_state_t wifi_state_waitForLAN(); //wait for IP address M2M_WIFI_IP_ADDRESS_ASSIGNED_EVENT
wifi_state_t wifi_state_connectToServer(); //initiate connection to server
wifi_state_t wifi_state_readyToTxRx(); //read tx buffer, check event for rx data and read
//wifi_state_t wifi_state_dataTxRx();
