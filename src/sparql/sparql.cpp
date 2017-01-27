#include "sparql.h"
#include <ESP8266WiFi.h>
#include "../ajson/aJSON.h"
#include "../util.h"
#include <string.h>

using namespace wot;

wot::PrefixResolver::PrefixResolver()
    : numPrefixes(0) {}

wot::PrefixResolver::~PrefixResolver()
{
    for(unsigned char i = 0; i < numPrefixes; i++)
    {
        delete[] prefixes[i].name;
        delete[] prefixes[i].prefix;
    }
}

bool wot::PrefixResolver::addPrefix(const char *name, const char *prefix)
{
    if(numPrefixes < 10)
    {
        unsigned nameLen = strlen(name) + 1;
        unsigned prefixLen = strlen(prefix) + 1;
        prefixes[numPrefixes].name = new char[nameLen];
        prefixes[numPrefixes].prefix = new char[prefixLen];
        strcpy_s(prefixes[numPrefixes].name, nameLen, name);
        strcpy_s(prefixes[numPrefixes].prefix, prefixLen, prefix);
        numPrefixes++;
    }
    else
    {
        return false;
    }
}

unsigned wot::PrefixResolver::resolve(const char *in, char *out, unsigned outLen)
{
    unsigned inLen = strlen(in) + 1;
    char *copy = new char[inLen];
    strcpy_s(copy, inLen, in);

    char *suffix = NULL;
    for(unsigned i = 0; i < inLen; i++) {
        if(copy[i] == ':') {
            copy[i] = '\0';
            suffix = copy + i + 1;
        }
    }
    if(suffix)
    {
        unsigned suffixLen = strlen(suffix);

        char *prefix = NULL;
        for(unsigned char i = 0; i < numPrefixes; i++) {
            if(!strcmp(prefixes[i].name, copy)) {
                prefix = prefixes[i].prefix;
            }
        }

        if(prefix)
        {
            unsigned prefixLen = strlen(prefix);
            if(prefixLen + suffixLen < outLen) {
                strcpy_s(out, outLen, prefix);
                strcpy_s(out + prefixLen, outLen - prefixLen, suffix);
                delete[] copy;
                return prefixLen + suffixLen;
            }
        }
    }
    delete[] copy;
    strcpy_s(out, outLen, in); // In last instance go with the input
    return 0;
}

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

unsigned short wot::queryEquivalentClasses(const char *iri, char **out,
                                    unsigned char entryCount, unsigned entryLen,
                                    const char *endpointHost, const char *endpointPath)
{
    WiFiClient con;
    if(con.connect(endpointHost, 80)) {
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
        con.printf("POST %s HTTP/1.1\r\n", endpointPath);
        con.printf("Host: %s\r\n", endpointHost);
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

bool wot::classesEquivalent(const char *iri1, const char *iri2)
{
    if(!strcmp(iri1, iri2)) {
        return true;
    }
    // Prepare array of equivalent classes:
    char **eq = new char*[10];
    for(unsigned char j = 0; j < 10; j++) {
        eq[j] = new char[200];
    }
    unsigned short numEqClasses = queryEquivalentClasses(iri1, eq, 10, 200);

    // Check if any of those equivalent IRIs is the same as the second:
    bool foundEquivalent = false;
    for(unsigned short j = 0; j < numEqClasses; j++) {
        if(!strcmp(eq[j], iri2)) {
            foundEquivalent = true;
        }
    }
    for(unsigned char j = 0; j < 10; j++) {
        delete[] eq[j];
    }
    delete[] eq;
    return foundEquivalent;
}
