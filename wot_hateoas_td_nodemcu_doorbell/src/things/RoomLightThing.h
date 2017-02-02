#ifndef __WOT_ROOMLIGHT_THING__
#define __WOT_ROOMLIGHT_THING__

#include "../http/HTTPServer.h"

namespace wot {

class RoomLightThing : public HTTPServer
{
public:
    // Available colors for this thing:
    enum Color {White, Red};
private:
    bool powerOn; // Whether the light is currently on

    Color currentColor; // The current color of the thing

    unsigned flashingIntervalMs; // Milliseconds between flashes

    unsigned flashingMsRemaining; // Milleseconds remaining in flashing mode. 0 if currently not flashing

    unsigned flashingPhaseProgressMs; // Progress of the current flashing interval in ms

    bool flashingPhasePowerOn; // Whether power is on in the current flashing phase

    unsigned char gpioMap[2]; // Mapping from Color to GPIO numbers.

    bool activeLow; // Whether the lights are controlled active-low.

public:
    /**
    * port: The port the server will listen on.
    * gpioMap: Array of GPIO numbers for the different colors.
    *          The entry for each value of Color must be the positive GPIO number
    *          of the corresponding light or is not used if the number is 0.
    * activeLow: Whether the lights are controlled active-low.
    * flashingIntervalMs: Amount of milliseconds between two flashes.
    */
    RoomLightThing(unsigned short port, unsigned char *gpioMap, bool activeLow = true, unsigned flashingIntervalMs = 100);

protected:
    virtual void handleGET(const char *path, WiFiClient *client);

    virtual void handlePOST(const char *path, const char *mediaType, const char *data, WiFiClient *client);

    virtual void handlePUT(const char *path, WiFiClient *client);

    virtual void handleDELETE(const char *path, WiFiClient *client);

public:
    void loop();

private:
    /**
    * Serves the root, i.e. GET /
    */
    void serveRoot(WiFiClient *client);

    /**
    * Respond to queries to the onoff state.
    */
    void serveOnOffState(WiFiClient *client);

    /**
    * Respond to queries to the colour state. (Yes, british!)
    */
    void serveColourState(WiFiClient *client);

    /**
    * Update the onoff state. data must be boolean.
    */
    void updateOnOffState(WiFiClient *client, const char *data);

    /**
    * Update the colour state. data must be one of the allowed colours.
    */
    void updateColourState(WiFiClient *client, const char *data);

    /**
    * Turn on the strobe mode. data must contain the duration in seconds.
    */
    void updateStrobeOn(WiFiClient *client, const char *data);

    /**
    * Turn the strobe mode off.
    */
    void updateStrobeOff(WiFiClient *client);
};

}

#endif
