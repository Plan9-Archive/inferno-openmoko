#include <lib9.h>

char*
utfrrune(__in_z char *s, Rune c)
{
    char *s1;

    if(c < Runesync)        /* not part of utf sequence */
        return strrchr(s, c);

    s1 = 0;
    for(;;) {
        if(CHAR_CAST(*s) < Runeself) { /* one byte rune */
            if(*s == L'\0')
                return s1;
            //if(*s == c)  /* impossible condition, c is checked above */
            //    return s;
            s++;
        }
        else {
            // r==Bad, n==1 if s[0] is not a valid first byte of utf-sequence
            Rune r;
            int n = chartorune(&r, s);
            if(r == c)
                s1 = s;
            s += n;
        }
    }
    return 0;
}
