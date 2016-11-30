#ifndef __WOT_SIMPLE_LIGHT_THING__
#define __WOT_SIMPLE_LIGHT_THING__
#include "../http/HTTPServer.h"

namespace wot {

/*
* Server providing HATEOAS-like access to a simple light thing (the builtin LED).
* The thing can either be turned on/off or set into strobe mode for a certain amount
* of time.
* Used media-types are:
* application/simple-light-thing+json
* Example:
* {
*   "name": "Simple Light Thing",
*   "location": "Somewhere out there",
*   "description": "Try the strobe!"
* }
*
* application/simple-light-thing-state+json
* Example:
* {
*   "mode": "on|off|strobe"
* }
*
* application/simple-light-thing-strobe+json
* Example:
* {
*   "do_strobe": true|false,
*   "duration": 3000 // (optional) Duration of strobe in ms
* }
*/
class SimpleLightThing : public HTTPServer {
public:
    SimpleLightThing(uint16_t port);

private:
    // The possible states of the light:
    enum LEDState {On, Off, Strobe};

    // The current state:
    LEDState state;

    char strobeState;

    int strobeRemainingMs;

    const char *name = "Simple Light Thing";

    const char *location = "";

    const char *description = "";

    void serveRootResource(WiFiClient *client);

    void serveStateResource(WiFiClient *client);

    void updateOnOff(const char *data, WiFiClient *client);

    void updateStrobe(const char *data, WiFiClient *client);

protected:
    virtual void handleGET(const char *path, WiFiClient *client);

    virtual void handlePOST(const char *path, const char *mediaType, const char *data, WiFiClient *client);

    virtual void handlePUT(const char *path, WiFiClient *client);

    virtual void handleDELETE(const char *path, WiFiClient *client);

public:
    virtual void loop();

    const char *getName() { return name; }

    void setName(const char *new_name) { name = new_name ? new_name : ""; }

    const char *getLocation() { return name; }

    void setLocation(const char *new_location) { location = new_location ? new_location : ""; }

    const char *getDescription() { return name; }

    void setDescription(const char *new_desc) { description = new_desc ? new_desc : ""; }
};

}

#endif
