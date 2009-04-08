#include <lib9.h>

size_t
utflen(__in_z const char *s)
{
    size_t n;

    for(n = 0; ; n++) {
        if(CHAR_CAST(*s) < Runeself) {  /* one byte rune */
            if(*s == L'\0')
                break;
            s++;
        }
        else {
            Rune r;
            s += chartorune(&r, s);
            // TODO: if(n==1 && r==Bad)
        }
    }
    return n;
}