#include <lib9.h>

size_t
utfnlen(__in_z const char *s, size_t m)
{
    size_t n;
    const char * const es = s + m;

    for(n = 0; s < es; n++) {
        if(CHAR_CAST(*s) < Runeself) { /* one byte rune */
            if(*s == L'\0')
                break;
            s++;
        }
        else {
            Rune r;
            if(!fullrune(s, es-s))
                break;
            s += chartorune(&r, s);
            // TODO: if(n==1 && r==Bad)
        }
    }
    return n;
}
