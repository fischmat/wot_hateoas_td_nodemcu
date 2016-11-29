#ifndef __WOT_UTIL__
#define __WOT_UTIL__

namespace wot {

int firstDiff(const char *s1, unsigned s1Len, const char *s2, unsigned s2Len);

unsigned strcpy_s(char *dest, unsigned len, const char *src);

}

#endif
