#include "sparql.h"
#include <ESP8266WiFi.h>
#include "../ajson/aJSON.h"
#include "../util.h"

using namespace wot;

const char *equivClassesQuery[] = {
    "PREFIX%20rdfs%3A%20%3Chttp%3A%2F%2Fwww.w3.org%2F2000%2F01%2Frdf-schema%23%3E%0APREFIX%20owl%3A%20%3Chttp%3A%2F%2Fwww.w3.org%2F2002%2F07%2Fowl%23%3E%0ASELECT%20%3Fc%20%7B%0A%20%20%7B%20%3C",
    "%3E%20rdfs%3AseeAlso%20%3Fc%20.%20%7D%20UNION%20%7B%20%3C",
    "%3E%20owl%3AsameAs%20%3Fc%20.%20%7D%0A%20%20%20UNION%20%7B%3Fc%20rdfs%3AseeAlso%20%3C",
    "%3E%20.%20%7D%20UNION%20%7B%20%3Fc%20owl%3AsameAs%20%3C",
    "%3E%20.%20%7D%0A%7D"
};

inline void readUntilPayload(WiFiClient *client)
{
    bool payloadFound = false;
    do {
        String line = client->readStringUntil('\n');
        payloadFound = line.length() == 1 && line[0] == '\r';

    } while(!payloadFound);
}

unsigned short wot::queryEquivalentClasses(const char *iri, char **out, unsigned char entryCount, unsigned entryLen)
{
    WiFiClient con;
    if(con.connect("lov.okfn.org", 80)) {
        // Construct the urlencoded payload of the request:
        String data("query=");
        data += equivClassesQuery[0];
        data += iri;
        data += equivClassesQuery[1];
        data += iri;
        data += equivClassesQuery[2];
        data += iri;
        data += equivClassesQuery[3];
        data += iri;
        data += equivClassesQuery[4];

        // Make a HTTP POST-request:
        con.print("POST /dataset/lov/sparql HTTP/1.1\r\n");
        con.print("Host: lov.okfn.org\r\n");
        con.print("Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n");
        con.print("Accept: application/sparql-results+json,*/*;q=0.9\r\n");
        con.printf("Content-Length: %d\r\n\r\n", data.length());
        con.print(data);

        while(!con.available()) {
            delay(10);
        }

        // Read first line of response:
        String statusLine = con.readStringUntil('\n');
        if(statusLine.indexOf("200") != -1) { // Check response code (200 OK)
            // Read stream until payload:
            readUntilPayload(&con);

            // Parse JSON and write results:
            aJsonObject *root = aJson.parse((char*)con.readString().c_str());
            if(root) {
                aJsonObject *results = aJson.getObjectItem(root, "results");
                if(results) {
                    aJsonObject *bindings = aJson.getObjectItem(results, "bindings");
                    if(bindings) {
                        char **currentEntry = out;
                        unsigned char bindingsCount = aJson.getArraySize(bindings);
                        for(unsigned char i = 0; i < bindingsCount && i < entryCount; i++) {
                            aJsonObject *binding = aJson.getArrayItem(bindings, i);
                            aJsonObject *c = aJson.getObjectItem(binding, "c");
                            aJsonObject *type = aJson.getObjectItem(c, "type");
                            aJsonObject *value = aJson.getObjectItem(c, "value");

                            if(!strcmp(type->valuestring, "uri")) {
                                strcpy_s(*currentEntry, entryLen, value->valuestring);
                                currentEntry++;
                            }
                        }
                        return currentEntry - out;
                    }
                }
            }

            aJson.deleteItem(root);
        }
    }
    return -1;
}
