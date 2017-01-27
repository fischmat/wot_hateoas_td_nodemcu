#ifndef _BELL_BUTTON_
#define _BELL_BUTTON_
#include "../ajson/aJSON.h"

// URL of the speaker
#define SPEAKER_URL "http://192.168.43.123:8080"
#define RING_ACTION_IRI "http://www.matthias-fisch.de/ontologies/wot#BellRingAction"

namespace wot {

class BellButton {
private:
    const char *speakerHost; // Hostname of the speaker

    unsigned short speakerPort; // The speakers port

    const char *speakerPath; // Path to the speakers TD

public:
    BellButton(const char *speakerHost, unsigned short speakerPort, const char *speakerPath);

    bool onPressed();

private:
    aJsonObject *fetchSpeakerThingDescription();
};

}

#endif
