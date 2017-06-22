#include <LWiFi.h>
#include <LBLE.h>
#include <LBLECentral.h> 

#include "cli.h"

#define MAX_ADV_LEN 32

char ssid[] = "SSR1100 (2.4G)"; 
char pass[] = "ykdah48545";

int keyIndex = 0;            
int status   = WL_IDLE_STATUS;
WiFiServer server(23);
boolean alreadyConnected = false; 

boolean ble_scan = false;

int cli_scan(int argc, char *argv[])
{
    ble_scan = true;
    
    while(ble_scan) {
        do_scan();
        delay(1000);
        if(Serial.available()){
            char ch = Serial.read();
            Serial.print(ch);
            if(ch == 'Q' || ch == 'q') {
                ble_scan = false;
            }
        }
    }
    
    return 0;
}

void setup() 
{
	Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); 
    
    // Start BLE
    LBLE.begin();
    while (!LBLE.ready()) {
        delay(10);
    }

    // Start WiFi
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		status = WiFi.begin(ssid, pass);
	}

    digitalWrite(LED_BUILTIN, LOW); 
	server.begin();
 
	printWifiStatus();

    // Start Command Line Interface
    int ret;

    cli_init();
    cli_register(cli_scan, "scan", "scan ble devices");
    cli_task();      
}

void loop()
{
    delay(1000);
}

void do_scan() 
{
	WiFiClient client = server.available();

    LBLECentral.stopScan();

    char msg[128];

    for (int i = 0; i < LBLECentral.getPeripheralCount(); ++i) {
        getAdvMsg(i, msg, 128);
        Serial.println(msg);
    
    	if (client) {
    	    if (!alreadyConnected) {
    			client.flush();
    			client.println("BLE/WiFi Gateway Start...");
    			alreadyConnected = true;
    		}
            server.println(msg);        
    	}
    }
    
    LBLECentral.clear();
    LBLECentral.scan();
}

void printWifiStatus() 
{
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.print(rssi);
	Serial.println(" dBm");
}

uint32_t getAdvMsg(int index, char *data, uint32_t data_len)
{
    String  address = LBLECentral.getAddress(index);
    String  name    = LBLECentral.getName(index);
    int32_t rssi    = LBLECentral.getRSSI(index);
    char buf[MAX_ADV_LEN];
    int  ret = 0;
    char hex[16];
    String manufacture;

    ret = LBLECentral.getRawManufacturerData(index, (uint8_t*) buf, MAX_ADV_LEN);
    
    for(int i=0; i<ret; i++) 
    {
        sprintf(hex, "%02X", buf[i]); 
        manufacture += hex;
    } 
    
    if(name.length() == 0) {
        name = "----";
    }
    
    sprintf(data, "%s\t%s\t%d\t%s", 
            address.c_str(), name.c_str(), rssi, manufacture.c_str());
       
    return 0;
}

