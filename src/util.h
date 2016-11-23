#ifndef __WOT_UTIL__
#define __WOT_UTIL__

#include <string>

namespace wot {

int firstDiff(const char *s1, unsigned s1Len, const char *s2, unsigned s2Len);

unsigned strcpy_s(char *dest, unsigned len, const char *src);

std::string url_encode(const std::string &value);

std::string getHostnameFromUrl(const char *url);

unsigned short getPortFromUrl(const char *url);

std::string getPathFromUrl(const char *url);

std::string urljoin(const char *host, const char *path = "/", unsigned short port = 80);
}

#endif
