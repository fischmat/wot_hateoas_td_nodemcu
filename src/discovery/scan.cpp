#include "scan.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>

using namespace wot;

inline IPAddress nextIPAddress(IPAddress a)
{
    uint32_t tmp = __builtin_bswap32(a);
    tmp++;
    return IPAddress(__builtin_bswap32(tmp));
}

void wot::pingScanSubnet(IPAddress firstIpAddr, IPAddress lastIpAddr, IPAddress *outOnline, unsigned short outOnlineLen, unsigned char retries, uint16_t tout)
{
    uint32_t repliesLen = __builtin_bswap32(lastIpAddr) - __builtin_bswap32(firstIpAddr) + 1;
    Serial.printf("Number of addresses: %d\n", repliesLen);
    delay(500);

    Serial.print("Scanning subnet ");
    Serial.print(firstIpAddr);
    Serial.print(" - ");
    Serial.println(lastIpAddr);

    unsigned short detected = 0;
    for(IPAddress ip = firstIpAddr; __builtin_bswap32(ip) <= __builtin_bswap32(lastIpAddr)
                                                && detected < outOnlineLen; ip = nextIPAddress(ip)) {
        if(Ping.ping(ip)) {
            outOnline[detected] = ip;
            Serial.print(ip);
            Serial.println(" is up");
            detected++;
        }
    }

    Serial.printf("Ping scan ended. Found %d hosts up.", detected);

}
