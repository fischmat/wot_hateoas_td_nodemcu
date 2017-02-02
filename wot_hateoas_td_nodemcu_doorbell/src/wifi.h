#ifndef __WOT_WIFI__
#define __WOT_WIFI__

struct WiFiCredentials {
    const char *ssid; // The SSID of the WiFi
    const char *passphrase; // The passphrase of the WiFi
};

void connectToWiFi(WiFiCredentials *creds);

#endif
