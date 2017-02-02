#include "RGBLightThing.h"
#include <Arduino.h>
#include "../ajson/aJSON.h"
#include "../util.h"

using namespace wot;

#define PWM_VALUE(colValue, activeLow) (activeLow ? 255 - colValue : colValue)

inline const char *channelName(uint8_t channel)
{
    switch (channel) {
        case CHANNEL_RED: return "red";
        case CHANNEL_GREEN: return "green";
        case CHANNEL_BLUE: return "blue";
        default: return "unknown";
    }
}

wot::RGBLightThing::RGBLightThing(uint16_t port, uint8_t _redGpio, uint8_t _greenGpio, uint8_t _blueGpio, bool _activeLow)
    : HTTPServer(port), redGpio(_redGpio), greenGpio(_greenGpio), blueGpio(_blueGpio)
{
    // Setup pins:
    pinMode(redGpio, OUTPUT);
    pinMode(greenGpio, OUTPUT);
    pinMode(blueGpio, OUTPUT);

    // Turn off the light:
    color.value = 0x000000;
    analogWrite(redGpio, PWM_VALUE(color.rgb.red, activeLow));
    analogWrite(greenGpio, PWM_VALUE(color.rgb.green, activeLow));
    analogWrite(blueGpio, PWM_VALUE(color.rgb.blue, activeLow));
}


void wot::RGBLightThing::updateChannelMode(uint8_t channel, const char *data, WiFiClient *client)
{
    aJsonObject *root = aJson.parse((char*)data);
    if(root) {
        aJsonObject *stability = aJson.getObjectItem(root, "stability");
        aJsonObject *interval = aJson.getObjectItem(root, "interval");
        if(stability) {
            const char *stabilityStr = stability->valuestring;
            if(!strcmp(stabilityStr, "constant")) {
                modes[channel] = ColorChannelMode::Constant;
                phases.directions[channel] = PHASE_DIRECTION_INCREASE;
                phases.progresses[channel] = 0;
            } else if(!strcmp(stabilityStr, "fade") && interval) {
                modes[channel] = ColorChannelMode::Fade;
                intervals[channel] = max(interval->valueint, 0);
                phases.directions[channel] = PHASE_DIRECTION_INCREASE;
                phases.progresses[channel] = 0;
            } else if(!strcmp(stabilityStr, "blink") && interval) {
                modes[channel] = ColorChannelMode::Blink;
                intervals[channel] = max(interval->valueint, 0);
                phases.directions[channel] = PHASE_DIRECTION_INCREASE;
                phases.progresses[channel] = 0;
            } else {
                respondBadRequest(client);
            }
        } else {
            respondBadRequest(client);
        }
        aJson.deleteItem(root);
    } else {
        respondBadRequest(client);
    }
}

void wot::RGBLightThing::serveChannelMode(uint8_t channel, WiFiClient *client)
{
    const char *stabilityStr;
    if(modes[channel] == ColorChannelMode::Blink) {
        stabilityStr = "blink";
    } else if(modes[channel] == ColorChannelMode::Fade) {
        stabilityStr = "fade";
    } else {
        stabilityStr = "constant";
    }

    client->print("HTTP/1.1 200 OK\r\nContent-Type: application/rgb-light-color-channel-mode+json\r\n\r\n");
    client->printf("{ \
                    	\"stability\": \"%s\", \
                    	\"interval\": %d \
                    }", stabilityStr, intervals[channel]);
}

void wot::RGBLightThing::updateChannelValue(uint8_t channel, const char *data, WiFiClient *client)
{
    // Check if data is valid:
    bool isValid = true;
    for(int i = 0; data[i] != '\0' && isValid; i++) {
        isValid = '0' <= data[i] && data[i] <= '9';
    }
    if(isValid)
    {
        color.rgb[channel] = atoi(data);
        client->print("HTTP/1.1 200 OK\r\n\r\n");
    } else {
        Serial.printf("Payload '%s' is non-numeric!");
        respondBadRequest(client);
    }
}

void wot::RGBLightThing::serveChannelValue(uint8_t channel, WiFiClient *client)
{
    client->printf("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n%d", color.rgb[channel]);
}

void wot::RGBLightThing::serveChannel(uint8_t channel, WiFiClient *client)
{
    const char *cn = channelName(channel);
    client->print("HTTP/1.1 200 OK\r\nContent-Type: application/rgb-light-color-channel+json\r\n\r\n");
    client->printf("{ \
	\"_forms\": { \
    		\"value\": { \
    			\"href\": \"/%s/value\", \
    			\"method\": \"POST\", \
    			\"accept\": \"text/plain\" \
    		}, \
    		\"mode\": { \
    			\"href\": \"/%s/mode\", \
    			\"method\": \"POST\", \
    			\"accept\": \"application/rgb-light-color-channel-mode+json\" \
    		} \
    	}, \
    	\"_links\": { \
    		\"value\": { \
    			\"href\": \"/%s/value\", \
    			\"type\": \"text/plain\" \
    		}, \
    		\"mode\": { \
    			\"href\": \"/%s/mode\", \
    			\"type\": \"application/rgb-light-color-channel-mode+json\" \
    		} \
    	} \
    }", cn, cn, cn, cn);
}

void wot::RGBLightThing::serveRootResource(WiFiClient *client)
{
    client->print("HTTP/1.1 200 OK\r\nContent-Type: application/rgb-light+json\r\n\r\n");
    client->printf("{ \
                	\"_links\": { \
                		\"red\": { \
                			\"href\": \"/red\", \
                			\"type\": \"application/rgb-light-color-channel+json\" \
                		},\
                		\"green\": { \
                			\"href\": \"/green\", \
                			\"type\": \"application/rgb-light-color-channel+json\" \
                		}, \
                		\"blue\": { \
                			\"href\": \"/blue\", \
                			\"type\": \"application/rgb-light-color-channel+json\" \
                		} \
                	}, \
                	\"name\": \"%s\", \
                	\"location\": \"%s\", \
                	\"description\": \"%s\" \
                }", name, location, description);
}

inline bool pathForChannel(const char *cName, const char *path, const char **continuation = NULL)
{
    unsigned cNameLen = strlen(cName);
    char *prefix = new char[cNameLen + 2];
    prefix[0] = '/';
    strcpy(prefix + 1, cName);
    prefix[1 + cNameLen] = '\0';

    int firstDiffPos = firstDiff(path, strlen(path), prefix, 1 + cNameLen);

    if(firstDiffPos == -1) { // Exact match
        if(continuation) {
            *continuation = NULL;
        }
        return true;
    } else if(firstDiffPos >= cNameLen + 1 && path[cNameLen + 1] == '/') { // Matches and path has format /cName/...
        if(continuation) {
            *continuation = path + 1 + cNameLen;
        }
        return true;
    } else {
        return false;
    }
}

void wot::RGBLightThing::handleGET(const char *path, WiFiClient *client)
{
    Serial.print(client->remoteIP());
    Serial.printf(" requested %s (GET)\n", path);

    const char *continuation = path;
    if(!strcmp(path, "/")) {
        serveRootResource(client);
    } else if(pathForChannel("red", path, &continuation)) {
        if(!continuation) {
            serveChannel(CHANNEL_RED, client);
        } else if(!strcmp(continuation, "/value")) {
            serveChannelValue(CHANNEL_RED, client);
        } else if(!strcmp(continuation, "/mode")) {
            serveChannelMode(CHANNEL_RED, client);
        } else {
            respondNotFound(client);
        }
    } else if(pathForChannel("green", path, &continuation)) {
        if(!continuation) {
            serveChannel(CHANNEL_GREEN, client);
        } else if(!strcmp(continuation, "/value")) {
            serveChannelValue(CHANNEL_GREEN, client);
        } else if(!strcmp(continuation, "/mode")) {
            serveChannelMode(CHANNEL_GREEN, client);
        } else {
            respondNotFound(client);
        }
    } else if(pathForChannel("blue", path, &continuation)) {
        if(!continuation) {
            serveChannel(CHANNEL_BLUE, client);
        } else if(!strcmp(continuation, "/value")) {
            serveChannelValue(CHANNEL_BLUE, client);
        } else if(!strcmp(continuation, "/mode")) {
            serveChannelMode(CHANNEL_BLUE, client);
        } else {
            respondNotFound(client);
        }
    } else {
        respondNotFound(client);
    }
    // Flush buffers and disconnect:
    client->flush();
    client->stop();
}

void wot::RGBLightThing::handlePOST(const char *path, const char *mediaType, const char *data, WiFiClient *client)
{
    Serial.printf("POST request for '%s' with media type '%s' and data '%s'\n", path, mediaType, data);
    const char *continuation = path;
    if(!strcmp(path, "/")) {
        serveRootResource(client);
    } else if(pathForChannel("red", path, &continuation)) {
        if(!continuation) {
            // POSTing to /<COLOR> is not allowed:
            respondMethodNotAllowed(client);
        } else if(!strcmp(continuation, "/value")) {
            if(!strcmp(mediaType, "text/plain")) {
                updateChannelValue(CHANNEL_RED, data, client);
            } else {
                respondUnsupportedMediaType(client);
            }
        } else if(!strcmp(continuation, "/mode")) {
            if(!strcmp(mediaType, "application/rgb-light-color-channel-mode+json")) {
                updateChannelMode(CHANNEL_RED, data, client);
            } else {
                respondUnsupportedMediaType(client);
            };
        } else {
            respondNotFound(client);
        }
    } else if(pathForChannel("green", path, &continuation)) {
        if(!continuation) {
            // POSTing to /<COLOR> is not allowed:
            respondMethodNotAllowed(client);
        } else if(!strcmp(continuation, "/value")) {
            if(!strcmp(mediaType, "text/plain")) {
                updateChannelValue(CHANNEL_GREEN, data, client);
            } else {
                respondUnsupportedMediaType(client);
            }
        } else if(!strcmp(continuation, "/mode")) {
            if(!strcmp(mediaType, "application/rgb-light-color-channel-mode+json")) {
                updateChannelMode(CHANNEL_GREEN, data, client);
            } else {
                respondUnsupportedMediaType(client);
            };
        } else {
            respondNotFound(client);
        }
    } else if(pathForChannel("blue", path, &continuation)) {
        Serial.println("Channel is blue...");
        if(!continuation) {
            // POSTing to /<COLOR> is not allowed:
            respondMethodNotAllowed(client);
        } else if(!strcmp(continuation, "/value")) {
            if(!strcmp(mediaType, "text/plain")) {
                updateChannelValue(CHANNEL_BLUE, data, client);
            } else {
                respondUnsupportedMediaType(client);
            }
        } else if(!strcmp(continuation, "/mode")) {
            if(!strcmp(mediaType, "application/rgb-light-color-channel-mode+json")) {
                updateChannelMode(CHANNEL_BLUE, data, client);
            } else {
                respondUnsupportedMediaType(client);
            };
        } else {
            respondNotFound(client);
        }
    }  else {
        respondNotFound(client);
    }
    // Flush buffers and disconnect:
    client->flush();
    client->stop();
}

void wot::RGBLightThing::handlePUT(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void wot::RGBLightThing::handleDELETE(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void wot::RGBLightThing::loop()
{
    HTTPServer::loop();
    delay(10);
    uint8_t channels[] = {CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE};
    uint8_t channelGpios[] = {redGpio, greenGpio, blueGpio};
    for(int i = 0; i < 3; i++)
    {
        phases.progresses[channels[i]] += 10;
        if(modes[channels[i]] == ColorChannelMode::Blink) {
            if(phases.progresses[channels[i]] > intervals[channels[i]])
            {
                phases.progresses[channels[i]] = 0;
                phases.directions[channels[i]] = !phases.directions[channels[i]];
                if(phases.directions[channels[i]] == PHASE_DIRECTION_INCREASE)
                {
                    analogWrite(channelGpios[i], PWM_VALUE(color.rgb[channels[i]], activeLow));
                } else {
                    analogWrite(channelGpios[i], PWM_VALUE(0x00, activeLow));
                }
            }
        } else if(modes[channels[i]] == ColorChannelMode::Fade) {
            uint8_t colValue;
            uint8_t colDist = ((uint8_t) (phases.progresses[channels[i]]/((float)intervals[channels[i]])) * 255.0);
            if(phases.directions[channels[i]] == PHASE_DIRECTION_INCREASE) {
                colValue = colDist;
            } else {
                colValue = 255 - colDist;
            }
            analogWrite(channelGpios[i], PWM_VALUE(colValue, activeLow));

            if(intervals[channels[i]] <= phases.progresses[channels[i]]) {
                phases.progresses[channels[i]] = 0;
                phases.directions[channels[i]] = !phases.directions[channels[i]];
            }

        } else if(modes[channels[i]] == ColorChannelMode::Constant) {
            analogWrite(channelGpios[i], PWM_VALUE(color.rgb[channels[i]], activeLow));
        }
    }
}
