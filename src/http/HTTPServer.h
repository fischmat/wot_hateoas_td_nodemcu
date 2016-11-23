#ifndef __WOT_HTTPSERVER__
#define __WOT_HTTPSERVER__
#include <ESP8266WiFi.h>

#define CONNECT_TIMEOUT_MS 800

namespace wot {
    class HTTPServer {
    private:
        WiFiServer *server;


    protected:
        HTTPServer(unsigned short port);

        ~HTTPServer();

        virtual void handleGET(const char *path, WiFiClient *client) = 0;

        virtual void handlePOST(const char *path, const char *data, const char *mediaType, WiFiClient *client) = 0;

        virtual void handlePUT(const char *path, WiFiClient *client) = 0;

        virtual void handleDELETE(const char *path, WiFiClient *client) = 0;

        void respondBadRequest(WiFiClient *client);

        void respondNotFound(WiFiClient *client);

        void respondUnsupportedMediaType(WiFiClient * client);

        void respondMethodNotAllowed(WiFiClient *Client);

    public:
        virtual void loop();

        void begin();
    };
}

#endif
