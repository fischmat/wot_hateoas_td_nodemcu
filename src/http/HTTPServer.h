#ifndef __WOT_HTTPSERVER__
#define __WOT_HTTPSERVER__
#include <ESP8266WiFi.h>

// Amount of ms in which a client must respond
#define CONNECT_TIMEOUT_MS 800

namespace wot {

    /**
    * Abstract baseclass for HTTP webservers.
    */
    class HTTPServer {
    private:
        WiFiServer *server; // Internally used TCP-server


    protected:
        /**
        * Creates and binds the server to a port.
        * port: The port the server should be bound to.
        */
        HTTPServer(unsigned short port);

        ~HTTPServer();

        /**
        * Handles GET-requests. Response must be written directly to client.
        * path: The path requested.
        * client: The client that filed the request.
        */
        virtual void handleGET(const char *path, WiFiClient *client) = 0;

        /**
        * Handles POST-requests. Response must be written directly to client.
        * path: The path requested.
        * data: The body of the request.
        * mediaType: The media type of the request body.
        * client: The client that filed the request.
        */
        virtual void handlePOST(const char *path, const char *data, const char *mediaType, WiFiClient *client) = 0;

        /**
        * Handles PUT-requests. Response must be written directly to client.
        * path: The path requested.
        * client: The client that filed the request.
        */
        virtual void handlePUT(const char *path, WiFiClient *client) = 0;

        /**
        * Handles DELETE-requests. Response must be written directly to client.
        * path: The path requested.
        * client: The client that filed the request.
        */
        virtual void handleDELETE(const char *path, WiFiClient *client) = 0;

        /**
        * Sends a '400 Bad request' response to the client and closes the connection.
        * client: The client the response should be sent to.
        */
        void respondBadRequest(WiFiClient *client);

        /**
        * Sends a '404 Not Found' response to the client and closes the connection.
        * client: The client the response should be sent to.
        */
        void respondNotFound(WiFiClient *client);

        /**
        * Sends a '415 Unsupported Media Type' response to the client and closes the connection.
        * client: The client the response should be sent to.
        */
        void respondUnsupportedMediaType(WiFiClient * client);

        /**
        * Sends a '405 Method Not Allowed' response to the client and closes the connection.
        * client: The client the response should be sent to.
        */
        void respondMethodNotAllowed(WiFiClient *Client);

    public:
        /**
        * Accepts and dispatches incoming requests.
        * IMPORTANT: Periodically call this method (e.g. in arduinos loop())
        * and also call it first in overriding implementations.
        */
        virtual void loop();

        /**
        * Makes the server listen for incoming connections.
        * Call this method before the first call of loop() (e.g. in arduinos setup()).
        */
        void begin();
    };
}

#endif
