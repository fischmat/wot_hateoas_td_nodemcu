#include "wifi.h"
#include <ESP8266WiFi.h>

const char *radioAnimation[] = {
    "      o      ",
    "    ( o )    ",
    "  ( ( o ) )  ",
    "( ( ( o ) ) )"
};

void connectToWiFi(WiFiCredentials *creds)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(creds->ssid, creds->passphrase);

    // Wait until connected to WiFi:
    unsigned animationStep = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf("\rConnecting to WiFi '%s'...   %s", creds->ssid, radioAnimation[animationStep]);
        delay(500);
        animationStep = (animationStep + 1) % 4;
    }

    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
