#include "HTTPServer.h"
#include "../util.h"

// Maximum number of characters for HTTP method (DELETE + \0):
#define HTTP_METHOD_MAXLEN 7
// Constrain maximum number of characters in path:
#define HTTP_PATH_MAXLEN 150

// Constrain length of payload (in bytes):
#define HTTP_PAYLOAD_MEDIATYPE_MAXLEN 1024

namespace wot {

// Keep these buffers in .data . Don't fragment the heap!
char method[HTTP_METHOD_MAXLEN]; // Buffer for the currently processed HTTP method
char path[HTTP_PATH_MAXLEN]; // Buffer for the currently processed path
char payloadMediaType[HTTP_PAYLOAD_MEDIATYPE_MAXLEN]; // Buffer for the payload

bool extractContentType(WiFiClient *client, char *contentType, unsigned len)
{
    do {
        // Read next header:
        String header = client->readStringUntil('\n');
        // End of HTTP-header already reached?
        if(header.length() == 1 && header[0] == '\r') {
            return false;
        }

        if(strstr(header.c_str(), "Content-Type: ") == header.c_str()) {
            // Number of bytes to copy can be at most the buffer size:
            unsigned ctLen = strcpy_s(contentType, len, header.c_str() + 14);
            // Remove the trailing carriage return:
            contentType[ctLen - 1] = '\0';
            return true;
        }

        // No more data available? Then this is a malformed request:
        if(!client->available()) {
            return false;
        }

    } while(true);
}

void readUntilPayload(WiFiClient *client)
{
    bool payloadFound = false;
    do {
        String line = client->readStringUntil('\n');
        payloadFound = line.length() == 1 && line[0] == '\r';

    } while(!payloadFound);
}

HTTPServer::HTTPServer(unsigned short port)
{
    server = new WiFiServer(port);
}

HTTPServer::~HTTPServer()
{
    if(server) {
        delete server;
    }
}

void HTTPServer::begin()
{
    server->begin();
}

void HTTPServer::loop()
{
    // Accept incoming connection:
    WiFiClient client = server->available();
    // Check if client connected successful:
    if(client) {
        Serial.print(client.remoteIP());
        Serial.println(" connected...");
    } else {
        return;
    }

    // Wait until client sends some data:
    unsigned timeout = CONNECT_TIMEOUT_MS;
    unsigned waitedMs = 0;
    while(!client.available() && waitedMs < timeout) {
        delay(5);
        waitedMs += 5;
    }
    // Cancel routine on timeout:
    if(waitedMs >= timeout) {
        Serial.print("Connection to client ");
        Serial.print(client.remoteIP());
        Serial.printf(" timed out (%d ms)\n", waitedMs);
        return;
    }

    // Read first line of HTTP-request:
    String request = client.readStringUntil('\r');

    // Our iterators:
    char *pMethod = (char*) &method;
    char *pPath = (char*) &path;
    // Clear memory from data read before:
    memset((void*)&method, 0, HTTP_METHOD_MAXLEN * sizeof(char));
    memset((void*)&path, 0, HTTP_PATH_MAXLEN * sizeof(char));

    bool inPath = false; // Flag whether we're currently reading the path
    // Iterate characters in request line:
    for(unsigned i = 0; i < request.length(); i++) {
        // Whitespaces divide the request into method/path/protocol:
        if(request[i] != ' ') {
            // Read into current buffer:
            if(!inPath) {
                *pMethod = request[i];
                pMethod++;
            } else {
                *pPath = request[i];
                pPath++;
            }
        } else {
            // If current character is a whitespace:
            if(!inPath) {
                inPath = true;
            } else {
                break; // End of path reached
            }
        }
    }

    if(method[0] == 'G') { // This is a GET-request
        this->handleGET((const char *)&path, &client);
    } else if(method[0] == 'P' && method[1] == 'O') { // This is a POST request
        // Extract the media type the client provided:
        if(!extractContentType(&client, (char*)&payloadMediaType, HTTP_PAYLOAD_MEDIATYPE_MAXLEN)) {
            // End of header reached, this request is invalid:
            Serial.println("Malformed HTTP-POST request detected!");
            respondBadRequest(&client);
            return;
        }
        readUntilPayload(&client);
        this->handlePOST((const char*)&path, (const char*)&payloadMediaType,  client.readString().c_str(), &client);
    } else if(method[0] == 'P' && method[1] == 'U') {
        this->handlePUT((const char *)&path, &client);
    } else if(method[0] == 'D') {
        this->handleDELETE((const char *)&path, &client);
    } else {
        Serial.printf("Unknown method %s\n", &method);
    }
}

void HTTPServer::respondBadRequest(WiFiClient *client) {
    client->print("HTTP/1.1 400 Bad Request\r\n\r\n");
    client->flush();
    client->stop();
}

void HTTPServer::respondNotFound(WiFiClient *client) {
    client->print("HTTP/1.1 404 Not Found\r\n\r\n");
    client->flush();
    client->stop();
}

void HTTPServer::respondUnsupportedMediaType(WiFiClient * client)
{
    client->print("HTTP/1.1 415 Unsupported Media Type\r\n\r\n");
    client->flush();
    client->stop();
}

void HTTPServer::respondMethodNotAllowed(WiFiClient *client)
{
    client->print("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
    client->flush();
    client->stop();
}

}
