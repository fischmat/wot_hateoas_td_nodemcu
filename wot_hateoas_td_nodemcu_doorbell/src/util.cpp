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
    unsigned i = 0;
    for(    ; i < len - 1 && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return i;
}
