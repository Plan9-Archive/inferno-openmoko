#include <lib9.h>

char*
utfrune(__in_z char *s, Rune c)
{
    if(c < Runesync)        /* not part of utf sequence */
        return strchr(s, c);

    for(;;) {
        if(CHAR_CAST(*s) < Runeself) { /* one byte rune */
            if(*s == L'\0')
                return 0;
            //if(*s == c)  /* impossible condition, c is checked above */
            //    return s;
            s++;
        }
        else {
            Rune r;
            // r==Bad, n==1 if s[0] is not a valid first byte of utf-sequence
            int n = chartorune(&r, s);
            if(r == c)
                return s;
            s += n;
        }
    }
    return 0;
}
