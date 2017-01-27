#include "BellButton.h"
#include "../sparql/sparql.h"
#include <ESP8266WiFi.h>
#include <string.h>

#define BUFFER_SIZE 1500
char bb_buffer[BUFFER_SIZE]; // Reserve some memory for operations

wot::BellButton::BellButton(const char *host, unsigned short port, const char *path)
    : speakerHost(host), speakerPort(port), speakerPath(path) {}

bool wot::BellButton::onPressed()
{
    // Load thing description:
    aJsonObject *td = fetchSpeakerThingDescription();

    // First we have to register all shorthand IRI notations at a resolver:
    PrefixResolver resolver;
    aJsonObject *context = aJson.getObjectItem(td, "@context");
    for(unsigned char i = 0; i < aJson.getArraySize(context); i++) {
        aJsonObject *item = aJson.getArrayItem(context, i);
        aJsonObject *itemEntry = item->child;
        if(itemEntry) {
            resolver.addPrefix(itemEntry->name, itemEntry->valuestring);
        }
    }

    // Get the list of actions provided by the thing:
    aJsonObject *actions = aJson.getObjectItem(td, "actions");

    // Check all actions for the ring action:
    aJsonObject *ringAction = NULL;
    for(unsigned char i = 0; i < aJson.getArraySize(actions); i++) {
        aJsonObject *action = aJson.getArrayItem(actions, i);

        // Get the @type of the action:
        aJsonObject *type = aJson.getObjectItem(action, "@type");
        if(type) {
            // Query for equivalent classes:
            char *iri = (char*)&bb_buffer;
            resolver.resolve(type->valuestring, iri, BUFFER_SIZE); // Resolve shorthand notations

            if(classesEquivalent(iri, RING_ACTION_IRI)) {
                ringAction = action;
            }
        }
    }

    if(ringAction) {

        // We want to get the HTTP-href of this action:
        // First search the URI with HTTP scheme in the TDs uris definition:
        unsigned char uriHttpIndex = ~0;
        aJsonObject *uris = aJson.getObjectItem(td, "uris");
        for(unsigned char i = 0; i < aJson.getArraySize(uris); i++) {
            char *uri = aJson.getArrayItem(uris, i)->valuestring;
            if(strstr(uri, "http://") == uri) {
                uriHttpIndex = i;
                break;
            }
        }
        if(uriHttpIndex == ~0) { // Exit if not found.
            Serial.println("Thing at %s path %s does not provide HTTP URI.");
            return false;
        }

        // Second get the corresponding href from the action:
        aJsonObject *hrefs = aJson.getObjectItem(ringAction, "hrefs");
        char *href = aJson.getArrayItem(hrefs, uriHttpIndex)->valuestring;

        // Get the input type definition of this action:
        aJsonObject *inputData = aJson.getObjectItem(ringAction, "inputData");

        // Connect to the thing and send POST request:
        WiFiClient con;
        if(con.connect(speakerHost, speakerPort)) {
            bool bodySent = false; // Whether the body was already sent by a input type dispatcher below
            con.printf("POST %s HTTP/1.1\r\n", href);
            con.printf("Host: %s\r\n", speakerHost);

            // If the action requires some input data, we have to check whether we can derive semantics of it:
            if(inputData) {
                char *valueType = aJson.getObjectItem(inputData, "valueType")->valuestring; // The XSD datatype of the inputData
                aJsonObject *item = inputData->child;
                while(item) {
                    if(strcmp(item->name, "valueType")) { // Not the valueType again
                        char *iriRange = (char*)&bb_buffer;
                        char *iriDomain = (char*)&bb_buffer + BUFFER_SIZE/2 + 1;
                        resolver.resolve(item->name, iriRange, BUFFER_SIZE/2); // Resolve shorthand notations
                        resolver.resolve(item->valuestring, iriDomain, BUFFER_SIZE/2 - 1); // Resolve shorthand notations

                        // If there is an option to set the number of times to ring:
                        if(!strcmp(valueType, "integer")
                                && classesEquivalent(iriRange, "http://www.demcare.eu/ontologies/demlab.owl#repetitions")
                                && classesEquivalent(iriDomain, "http://ns.inria.fr/ast/sql#Count")) {
                            con.print("Content-Length: 12\r\n\r\n{\"value\": 5}");
                        }
                    }
                    item = item->next; // Proceed with the next child of inputData
                }
            }

            if(!bodySent) { // Terminate header if no body was sent by a input type dispatcher
                con.print("\r\n");
            }

            // Wait for the things response:
            while(!con.available()) {
                delay(10);
            }

            // Check if the response was successful:
            String responseStatus = con.readStringUntil('\n');
            if(responseStatus.indexOf("200") != -1) {
                return true;
            }
        }
    }
    return false;
}

aJsonObject *wot::BellButton::fetchSpeakerThingDescription()
{
    WiFiClient con;
    if(con.connect(speakerHost, speakerPort)) {
        // Set GET request
        con.printf("GET %s HTTP/1.1\r\n", speakerPath);
        con.printf("Host: %s\r\n\r\n", speakerHost);

        // Wait until speaker has sent something:
        while(!con.available()) {
            delay(10);
        }

        String line = con.readStringUntil('\n');
        if(line.indexOf("200") != -1) {
            // Read until payload reached:
            while(line != "\r\n\r\n") {
                line = con.readStringUntil('\n');
            }

            // Read the payload:
            char *buff = (char*)&bb_buffer;
            char c;
            while((c = con.read()) != -1 && buff < ((char*)&bb_buffer) + BUFFER_SIZE - 1) {
                *buff = c;
                buff++;
            }
            *buff = '\0';

            return aJson.parse((char*)&bb_buffer);
        }
    }
}
