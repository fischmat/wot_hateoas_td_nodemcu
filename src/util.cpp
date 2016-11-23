#include "util.h"
#include <cctype>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace std;

int wot::firstDiff(const char *s1, unsigned s1Len, const char *s2, unsigned s2Len) {
    for(int i = 0; i < s1Len && i < s2Len; i++)
    {
        if(s1[i] != s2[i]) {
            return i;
        }
    }
    if(s1Len < s2Len) {
        return s1Len;
    } else if(s2Len < s1Len) {
        return s2Len;
    }
    return -1;
}

unsigned wot::strcpy_s(char *dest, unsigned len, const char *src)
{
    int i = 0;
    for(    ; i < len - 1 && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return i;
}


string wot::url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

std::string wot::getHostnameFromUrl(const char *url)
{
    string hname;

    unsigned slashes = 0;
    for(unsigned i = 0; url[i] != '\0'; i++) {
        // Already encountered the two slashes after the protocol?
        if(slashes == 2) {
            // If this is the first slash of path, seperator from port or the end, we're done:
            if(url[i] == '/' || url[i] == ':' || url[i] == '\0') {
                return hname;
            }
            hname += url[i];
        }
        if(url[i] == '/') {
            slashes++;
        }
    }
    return string();
}

unsigned short wot::getPortFromUrl(const char *url)
{
    static char portStr[6];
    memset((void*)&portStr, 0, 6 * sizeof(char));
    char *portStrPtr = (char*)&portStr;

    bool inPort = false;
    for(unsigned i = 0; url[i] != '\0'; i++) {
        if(inPort) {
            if(url[i] != '/' || url[i] != '/') {
                *portStrPtr = url[i];
                portStrPtr++;
            } else {
                return atoi((char*)&portStr);
            }
        } else if(url[i] == ':') {
            inPort = true;
        }
    }
    return 0;
}

string wot::getPathFromUrl(const char *url)
{
    unsigned slashes = 0;
    const char *p = url;
    while(*p != '\0') {
        if(*p == '/') {
            slashes++;
            if(slashes == 3) {
                return p;
            }
        }
        p++;
    }
    return string();
}

string wot::urljoin(const char *host, const char *path, unsigned short port) {
    stringstream s;
    s << host << ":" << port << path;
    return s.str();
}
