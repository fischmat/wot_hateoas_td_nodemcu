#ifndef __WOT_RGBLightThing__
#define __WOT_RGBLightThing__

#include "../http/HTTPServer.h"

#define CHANNEL_RED 0
#define CHANNEL_GREEN 1
#define CHANNEL_BLUE 2
#define CHANNEL_INVALID (~0)

#define PHASE_DIRECTION_DECREASE 0
#define PHASE_DIRECTION_INCREASE 1

namespace wot {

/*
* A rgb light thing has a LED with three color channels.
* Furthermore it supports for each color a stability  (constant, fade, blink).
* Media-Types:
* application/rgb-light+json
*{
*	"_links": {
*		"red": {
*			"href": "/red",
*			"type": "application/rgb-light-color-channel+json"
*		},
*		"green": {
*			"href": "/green",
*			"type": "application/rgb-light-color-channel+json"
*		},
*		"blue": {
*			"href": "/blue",
*			"type": "application/rgb-light-color-channel+json"
*		}
*	},
*	"name": "%s",
*	"location": "%s",
*	"description": "%s"
*}
*
*application/rgb-light-color-channel+json
*{
*	"_forms": {
*		"value": {
*			"href": "/<COLOR>/value",
*			"method": "POST",
*			"accept": "text/plain"
*		},
*		"mode": {
*			"href": "/<COLOR>/mode",
*			"method": "POST",
*			"accept": "application/rgb-light-color-channel-mode+json"
*		},
*	},
*	"_links": {
*		"value": {
*			"href": "/<COLOR>/value",
*			"type": "text/plain"
*		},
*		"mode": {
*			"href": "/<COLOR>/mode",
*			"type": "application/rgb-light-color-channel-mode+json"
*		},
*	}
*}
*application/rgb-light-color-channel-mode+json
*{
*	"stability": "on|off|fading",
*	"fade_duration": 3000
*}
*/
class RGBLightThing : public HTTPServer {

public:
    RGBLightThing(uint16_t port, uint8_t redGpio = 5, uint8_t greenGpio = 4, uint8_t blueGpio = 1, bool activeLow = true);

private:
    uint8_t redGpio;
    uint8_t greenGpio;
    uint8_t blueGpio;
    bool    activeLow;

    union color_t {
        struct rgb_t {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t &operator[](int i) {
                switch(i) {
                    case CHANNEL_RED: return this->red;
                    case CHANNEL_GREEN: return this->green;
                    case CHANNEL_BLUE: return this->blue;
                    default: return red;
                }
            }
        } rgb;
        uint32_t value : 24;
    } color;

    enum ColorChannelMode {Constant, Fade, Blink};

    ColorChannelMode modes[3];

    unsigned intervals[3];

    struct phase_t {
        struct phase_direction_t {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t &operator[](int i)
            {
                switch (i) {
                    case CHANNEL_RED: return red;
                    case CHANNEL_GREEN: return green;
                    case CHANNEL_BLUE: return blue;
                    default: return red;
                }
            }
        } directions;
        struct phase_progress_t {
            unsigned red;
            unsigned green;
            unsigned blue;
            unsigned &operator[](int i)
            {
                switch (i) {
                    case CHANNEL_RED: return red;
                    case CHANNEL_GREEN: return green;
                    case CHANNEL_BLUE: return blue;
                    default: return red;
                }
            }
        } progresses;
    } phases;

    void updateChannelMode(uint8_t channel, const char *data, WiFiClient *client);

    void serveChannelMode(uint8_t channel, WiFiClient *client);

    void updateChannelValue(uint8_t channel, const char *data, WiFiClient *client);

    void serveChannelValue(uint8_t channel, WiFiClient *client);

    void serveChannel(uint8_t channel, WiFiClient *client);

    void serveRootResource(WiFiClient *client);

    const char *name = "RGB Light Thing";

    const char *location = "";

    const char *description = "";

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
