#include <Arduino.h>
#include "wifi.h"
#include "discovery/scan.h"
#include "things/RGBLightThing.h"
#include "logic/ld.h"

using namespace wot;
using namespace std;

RGBLightThing server(80);

void setup() {
    delay(1000);
    Serial.begin(9600); // Set Baudrate

    // Setup WiFi connection:
    WiFiCredentials creds;
    creds.ssid = "G3_8803";
    creds.passphrase = "trivial42";
    connectToWiFi(&creds);

    SPARQLEndpoint ep("lov.okfn.org", "/dataset/lov/sparql");
    vector<string> eq = ep.equivalent("http://purl.org/iot/vocab/m3-lite#Thermometer");
    vector<string>::iterator i = eq.begin();
    while(i != eq.end()) {
        Serial.println(i->c_str());
        i++;
    }

    //server.begin();
    /*IPAddress onlineHosts[16];
    memset((void*)&onlineHosts, 0, 16 * sizeof(IPAddress));
    Serial.println("Now scanning");
    pingScanSubnet(IPAddress(192, 168, 43, 140), IPAddress(192, 168, 43, 150), (IPAddress*)&onlineHosts, 16);
    for(int i = 0; i < 16 && onlineHosts[i]; i++) {
        Serial.print(onlineHosts[i]);
        Serial.println(" is up!");
    }*/
}

float x = 0.0;

void loop() {
	//server.loop();
    delay(10);
}
