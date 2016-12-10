#include "RoomLightThing.h"

// TODO: Change for HATEOAS approach
#define WOT_IFACE_TD

#ifdef WOT_IFACE_TD
#warning Compiling RoomLightThing in TD mode
#else
#warning Compiling RoomLightThing in HATEOAS mode
#endif

#define WORKING_BUFFER_SIZE 2048

// General purpose buffer in .data segment. Avoid heap fragmentation.
char buffer[WORKING_BUFFER_SIZE];

wot::RoomLightThing::RoomLightThing(unsigned short port, unsigned char *gm, bool al, unsigned fims)
: HTTPServer(port), flashingIntervalMs(fims), activeLow(al), powerOn(false),
                    currentColor(Color::White), flashingMsRemaining(0)
{
    #ifdef WOT_IFACE_TD
    Serial.printf("Room light thing online at http://%s:%d/ with Thing Description...\n", getHostname(), getPort());
    #else
    Serial.printf("Room light thing online at http://%s:%d/ with HATEOAS approach...\n", getHostname(), getPort());
    #endif
    gpioMap[Color::White] = gm[Color::White];
    gpioMap[Color::Red] = gm[Color::Red];
    pinMode(gpioMap[Color::White], OUTPUT);
    digitalWrite(gpioMap[Color::White], activeLow ? HIGH : LOW);
    pinMode(gpioMap[Color::Red], OUTPUT);
    digitalWrite(gpioMap[Color::Red], activeLow ? HIGH :  LOW);
}

void wot::RoomLightThing::serveRoot(WiFiClient *client)
{
#ifdef WOT_IFACE_TD
    client->print("HTTP/1.1 200 OK\r\n");
    client->print("Content-Type: application/thing-description+json\r\n");

    unsigned currentColourStability = flashingIntervalMs > 0 ? flashingIntervalMs : 0;
    sprintf((char*)&buffer, "{\
    	\"@context\": [\
    		\"http://w3c.github.io/wot/w3c-wot-td-context.jsonld\",\
    		{\"dogont\": \"http://elite.polito.it/ontologies/dogont.owl#\"},\
    		{\"dbr\": \"dbpedia.org/resource/\"},\
    		{\"dbo\": \"dbpedia.org/ontology/\"}\
    	],\
    	\"@type\": \"dogont:Lighting\",\
    	\"name\": \"Room Light\",\
    	\"vendor\": \"WoT Experts Group\",\
    	\"uris\": [\"http://%s:%d/\"],\
    	\"encodings\": [\"JSON\"],\
    	\"properties\": [\
    		{\
    			\"@type\": \"dogont:OnOffState\",\
    			\"name\": \"Powerstate\",\
    			\"valueType\": {\"type\": \"string\", \"enum\": [\"on\", \"off\"]},\
    			\"writeable\": true,\
    			\"hrefs\": [\"onoff\"],\
    			\"stability\": -1\
    		},\
    		{\
    			\"@type\": \"dogont:ColorState\",\
    			\"name\": \"Color\",\
    			\"valueType\": {\
    				\"type\": \"string\",\
    				\"options\": [\
    					{\
    						\"value\": \"white\",\
    						\"dbo:Colour\": \"dbr:White\"\
    					},\
    					{\
    						\"value\": \"red\",\
    						\"dbo:Colour\": \"dbr:Red\"\
    					}\
    				]\
    			},\
    			\"writeable\": true,\
    			\"hrefs\": [\"colour\"],\
    			\"stability\": \"%d\"\
    		}\
    	],\
    	\"actions\": [\
    		{\
    			\"@type\": \"dogont:OnFlashingCommand\",\
    			\"name\": \"Turn Strobe On\",\
    			\"inputData\": {\
    				\"valueType\": \"number\",\
    				\"dogont:flashingTime\": \"dbr:Second\"\
    			},\
    			\"hrefs\": [\"strobeon\"]\
    		},\
    		{\
    			\"@type\": \"dogont:OffFlashingCommand\",\
    			\"name\": \"Turn Strobe Off\",\
    			\"hrefs\": [\"strobeoff\"]\
    		}\
    	]\
    }", getHostname(), getPort(), currentColourStability);

    client->printf("Content-Length: %d\r\n\r\n", strlen((char*)&buffer));
    client->print((char*)&buffer);
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::serveOnOffState(WiFiClient *client)
{
#ifdef WOT_IFACE_TD
    client->print("HTTP/1.1 200 OK\r\n");
    client->print("Content-Type: text/plain\r\n");

    if(powerOn) {
        client->print("Content-Length: 2\r\n\r\non");
    } else {
        client->print("Content-Length: 3\r\n\r\noff");
    }
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::serveColourState(WiFiClient *client)
{
#ifdef WOT_IFACE_TD
    const char *colorStr;
    if(currentColor == Color::White) {
        colorStr = "white";
    } else {
        colorStr = "red";
    }

    client->print("HTTP/1.1 200 OK\r\n");
    client->print("Content-Type: text/plain\r\n");
    client->printf("Content-Length: %d\r\n\r\n", strlen(colorStr));
    client->print(colorStr);
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::updateOnOffState(WiFiClient *client, const char *data)
{
#ifdef WOT_IFACE_TD
    // Set the new power state:
    if(!strcmp(data, "on")) {
        powerOn = true;
    } else if(!strcmp(data, "off")) {
        powerOn = false;
    } else {
        respondBadRequest(client);
    }
    client->print("HTTP/1.1 200 OK\r\n\r\n");
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::updateColourState(WiFiClient *client, const char *data)
{
#ifdef WOT_IFACE_TD
    if(!strcmp(data, "white")) {
        currentColor = Color::White;
    } else if(!strcmp(data, "red")) {
        currentColor = Color::Red;
    } else {
        respondBadRequest(client);
    }
    client->print("HTTP/1.1 200 OK\r\n\r\n");
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::updateStrobeOn(WiFiClient *client, const char *data)
{
#ifdef WOT_IFACE_TD
    double secs = atof(data);
    if(secs > 0.0) {
        powerOn = true; // Turn power on if someone wishes the strobe mode
        flashingMsRemaining = secs * 1000.0;
        client->print("HTTP/1.1 200 OK\r\n\r\n");

    } else {
        respondBadRequest(client);
    }
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::updateStrobeOff(WiFiClient *client)
{
#ifdef WOT_IFACE_TD
    flashingMsRemaining = 0;
    client->print("HTTP/1.1 200 OK\r\n\r\n");
#else
    #warning Method not implemented for HATEOAS
#endif
}

void wot::RoomLightThing::handleGET(const char *path, WiFiClient *client)
{
    if(!strcmp(path, "/")) {
        serveRoot(client);
    } else if(!strcmp(path, "/onoff")) {
        serveOnOffState(client);
    } else if(!strcmp(path, "/colour")) {
        serveColourState(client);
    } else {
        respondNotFound(client);
    }
    client->stop();
}

void wot::RoomLightThing::handlePOST(const char *path, const char *data, const char *mediaType, WiFiClient *client)
{
    if(!strcmp(path, "/onoff")) {
        updateOnOffState(client, data);
    } else if(!strcmp(path, "/colour")) {
        updateColourState(client, data);
    } else if(!strcmp(path, "/")) {
        respondMethodNotAllowed(client);
    } else if(!strcmp(path, "/strobeon")) {
        updateStrobeOn(client, data);
    } else if(!strcmp(path, "/strobeoff")) {
        updateStrobeOff(client);
    } else {
        respondNotFound(client);
    }
    client->stop();
}

void wot::RoomLightThing::handlePUT(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void wot::RoomLightThing::handleDELETE(const char *path, WiFiClient *client)
{
    respondMethodNotAllowed(client);
}

void wot::RoomLightThing::loop()
{
    HTTPServer::loop();

    delay(10);

    if(powerOn) {
        for(int col = Color::White; col <= Color::Red; col++) {
            int voltage;
            if(col == currentColor) {
                if(flashingMsRemaining > 0) { // Currently in strobe mode?
                    flashingMsRemaining -= 10;
                    flashingPhaseProgressMs += 10;
                    if(flashingPhaseProgressMs > flashingIntervalMs) {
                        flashingPhasePowerOn = !flashingPhasePowerOn;
                        flashingPhaseProgressMs = 0;
                    }
                    voltage = flashingPhasePowerOn ? (activeLow ? LOW : HIGH) : (activeLow ? HIGH : LOW);
                } else {
                    voltage = activeLow ? LOW : HIGH;
                }
            } else {
                voltage = activeLow ? HIGH : LOW;
            }

            digitalWrite(gpioMap[col], voltage);
        }
    } else {
        for(int col = Color::White; col <= Color::Red; col++) {
            digitalWrite(gpioMap[col], activeLow ? HIGH : LOW);
        }
    }
}
