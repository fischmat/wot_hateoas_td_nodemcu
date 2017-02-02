#ifndef __WOT_SCAN__
#define __WOT_SCAN__

#include <ESP8266WiFi.h>

namespace wot {

void pingScanSubnet(IPAddress firstIpAddr, IPAddress lastIpAddr, IPAddress *outOnline, unsigned short outOnlineLen, unsigned char retries = 1, uint16_t tout = 500);

}

#endif
