#include <Arduino.h>
#include "wifi.h"
#include "discovery/scan.h"
#include "things/RoomLightThing.h"
#include "sparql/sparql.h"

using namespace wot;
using namespace std;-ge

RoomLightThing *server = NULL;

void setup() {
    delay(1000);
    Serial.begin(9600); // Set Baudrate

    // Setup WiFi connection:
    WiFiCredentials creds;
    creds.ssid = "G3_8803";
    creds.passphrase = "trivial42";
    connectToWiFi(&creds);

    unsigned char *gpioMap = new unsigned char[max(RoomLightThing::Color::Red, RoomLightThing::Color::White) + 1];
    gpioMap[RoomLightThing::Color::Red] = 4;
    gpioMap[RoomLightThing::Color::White] = 5;

    server = new RoomLightThing(80, gpioMap, false, 250);

    server->begin();
    /*IPAddress onlineHosts[16];
    memset((void*)&onlineHosts, 0, 16 * sizeof(IPAddress));
    Serial.println("Now scanning");
    pingScanSubnet(IPAddress(192, 168, 43, 140), IPAddress(192, 168, 43, 150), (IPAddress*)&onlineHosts, 16);
    for(int i = 0; i < 16 && onlineHosts[i]; i++) {
        Serial.print(onlineHosts[i]);
        Serial.println(" is up!");
    }*/
}

void loop() {
    if(server) {
        server->loop();
    }
}
