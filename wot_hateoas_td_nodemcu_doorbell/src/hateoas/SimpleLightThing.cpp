#include "SimpleLightThing.h"
#include <Arduino.h>
#include "../ajson/aJSON.h"

namespace wot {

SimpleLightThing::SimpleLightThing(unsigned short port)
    : HTTPServer(port), state(LEDState::Off)
{
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, HIGH);
}

void SimpleLightThing::serveRootResource(WiFiClient *client)
{
    client->print("HTTP/1.1 200 OK\r\nContent-Type: application/simple-light-thing+json\r\n\r\n");
    client->printf("{ \
    	\"_links\": { \
    		\"state\": { \
    			\"href\": \"/state\", \
    			\"type\": \"application/simple-light-thing-state+json\" \
    		} \
    	}, \
        \"name\": \"%s\", \
        \"location\": \"%s\", \
        \"description\": \"%s\" \
    }", name, location, description);
}

void SimpleLightThing::serveStateResource(WiFiClient *client)
{
    const char *modeStr;
    switch (state) {
        case LEDState::Off: modeStr = "off"; break;
        case LEDState::On: modeStr = "on"; break;
        case LEDState::Strobe: modeStr = "strobe"; break;
    }

    client->print("HTTP/1.1 200 OK\r\nContent-Type: application/simple-light-thing-state+json\r\n\r\n");
    client->printf("{ \
            	\"_forms\": { \
            		\"onoff\": { \
            			\"href\": \"/onoff\", \
            			\"method\": \"POST\", \
            			\"accept\": \"text/plain\" \
            		}, \
            		\"strobe\": { \
            			\"href\": \"/strobe\", \
            			\"method\": \"POST\", \
            			\"accept\": \"application/simple-light-thing-strobe+json\" \
            		} \
            	}, \
            	\"mode\": \"%s\" \
            }", modeStr);
}

void SimpleLightThing::updateOnOff(const char *data, WiFiClient *client)
{
    if(!strcmp(data, "on")) {
        state = LEDState::On;
        client->print("HTTP/1.1 200 OK\r\n\r\n");
    } else if(!strcmp(data, "off")) {
        state = LEDState::Off;
        client->print("HTTP/1.1 200 OK\r\n\r\n");
    } else {
        respondBadRequest(client);
    }
}

void SimpleLightThing::updateStrobe(const char *data, WiFiClient *client)
{
    /*
        Process application/simple-light-thing-strobe+json:
    */
    aJsonObject *root = aJson.parse((char*)data);

    if(root) {
        aJsonObject *mode = aJson.getObjectItem(root, "do_strobe");

        if(mode) {
            if(mode->valuebool) {
                state = LEDState::Strobe;
                aJsonObject *duration = aJson.getObjectItem(root, "duration");
                if(duration) {
                    strobeRemainingMs = duration->valueint;
                    client->print("HTTP/1.1 200 OK\r\n\r\n");
                } else {
                    Serial.println("Malformed media-type provided. Missing duration in application/simple-light-thing-strobe+json");
                    respondBadRequest(client);
                }
            } else {
                state = LEDState::Off;
            }
        } else {
            Serial.println("Malformed media-type provided. Missing do_strobe in application/simple-light-thing-strobe+json");
            respondBadRequest(client);
        }
        // Clean up memory:
        aJson.deleteItem(root);

    } else {
        Serial.println("Malformed media-type provided (application/simple-light-thing-strobe+json).");
        respondBadRequest(client);
    }
}

void SimpleLightThing::handleGET(const char *path, WiFiClient *client)
{
    if(!strcmp(path, "/")) {
        serveRootResource(client);
    } else if(!strcmp(path, "/state")) {
        serveStateResource(client);
    } else {
        respondNotFound(client);
    }
    // Flush buffers and disconnect:
    client->flush();
    client->stop();
}

void SimpleLightThing::handlePOST(const char *path, const char *mediaType, const char *data, WiFiClient *client)
{
    if(!strcmp(path, "/onoff")) {
        if(!strcmp(mediaType, "text/plain")) {
            updateOnOff(data, client);
        } else {
            respondUnsupportedMediaType(client);
        }
    } else if(!strcmp(path, "/strobe")) {
        if(!strcmp(mediaType, "application/simple-light-thing-strobe+json")) {
            updateStrobe(data, client);
        } else {
            respondUnsupportedMediaType(client);
        }
    } else {
        respondNotFound(client);
    }
    // Flush buffers and disconnect:
    client->flush();
    client->stop();
}

void SimpleLightThing::handlePUT(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void SimpleLightThing::handleDELETE(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void SimpleLightThing::loop()
{
    HTTPServer::loop();
    if(state == LEDState::On) {
        digitalWrite(BUILTIN_LED, LOW);
    } else if(state == LEDState::Off) {
        digitalWrite(BUILTIN_LED, HIGH);
    } else if(state == LEDState::Strobe)     {
        digitalWrite(BUILTIN_LED, (strobeState = (strobeState + 1) % 2) ? LOW : HIGH);
        delay(500);
        strobeRemainingMs -= 500;
        if(strobeRemainingMs < 0) {
            state = LEDState::Off;
        }
    }
}

}
