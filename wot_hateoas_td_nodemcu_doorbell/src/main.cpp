#include <Arduino.h>
#include "wifi.h"
#include "discovery/scan.h"

using namespace wot;
using namespace std;



void setup() {
    delay(1000);
    Serial.begin(9600); // Set Baudrate

    // Setup WiFi connection:
    WiFiCredentials creds;
    creds.ssid = "G3_8803";
    creds.passphrase = "trivial42";
    connectToWiFi(&creds);

    pinMode(16, INPUT);

    /*
    IPAddress onlineHosts[16];
    memset((void*)&onlineHosts, 0, 16 * sizeof(IPAddress));
    Serial.println("Now scanning");
    pingScanSubnet(IPAddress(192, 168, 43, 140), IPAddress(192, 168, 43, 150), (IPAddress*)&onlineHosts, 16);
    for(int i = 0; i < 16 && onlineHosts[i]; i++) {
        Serial.print(onlineHosts[i]);
        Serial.println(" is up!");
    }
    */
}

void loop() {
    bool button_pressed = false;
    int buttonPin = 16;
    button_pressed = digitalRead(buttonPin) == HIGH;

    if(button_pressed) {
      Serial.println("Button pressed");
      button_pressed = false;

      // TODO: Call to controller
    }
}
