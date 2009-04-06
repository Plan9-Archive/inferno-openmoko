#include <lib9.h>
#include <isa.h>
#include <interp.h>
#include <raise.h>
#include <pool.h>
#include <mathi.h> /* strtod */

#define OP(fn)  void fn(__inout_ecount(1) Disdata*rs, __inout_ecount(1) Disdata*rm, __inout_ecount(1) Disdata*rd, REG*rr)

// indc - Index by character
//
// Syntax:     indc    src1, src2, dst 
// Function:   dst = src1[src2]
//
// The indc instruction indexes UTF strings. The src1 instruction must be a 
// string. The src2 operand must be an integer specifying the Unicode 
// character to store in the dst operand.
OP(indc)
{
    int l;
    DISINT v = rm->disint;
    String *ss = rs->pstring;

    if(ss == H)
        error(exNilref);

    l = ss->len;
    if(l < 0) {
        if(v >= -l)
            error(exBounds);
        l = ss->Srune[v];
    }
    else {
        if(v >= l)
            error(exBounds);
        l = ss->Sascii[v];
    }
    rd->disint = l;
}

// insc - Insert character into string
//
// Syntax:     insc    src1, src2, dst
// Function:   src1[src2] = dst
// 
// The insc instruction inserts a character into an existing string. The index 
// in src2 must be a non-negative integer less than the length of the string 
// plus one. The src1 operand must a string constant created by the newc 
// instruction. The character to insert must be a valid 16-bit unicode value 
// represented as a word.
OP(insc)
{
    ulong v;
    int l, r, expand;
    String *ss, *ns;

    r = rs->disint;
    v = rm->disint;
    ss = rd->pstring;

    expand = r >= Runeself;

    if(ss == H) {
        ss = newstring(0);
        if(expand) {
            l = 0;
            ss->max /= sizeof(Rune);
            goto r;
        }
    }
    else
    if(D2H(ss)->ref > 1 || (expand && ss->len > 0))
        ss = splitc(&rd->pstring, expand);

    l = ss->len;
    if(l < 0 || expand) {
        l = -l;
r:
        if(v < l)
            ss->Srune[v] = r;
        else if(v == l && v < ss->max) {
            ss->len = -(v+1);
            ss->Srune[v] = r;
        }
        else {
            if(v != l)
                error(exBounds);
            ns = newstring((v + 1 + v/4)*sizeof(Rune));
            memmove(ns->Srune, ss->Srune, -ss->len*sizeof(Rune));
            ns->Srune[v] = r;
            ns->len = -(v+1);
            ns->max /= sizeof(Rune);
            ss = ns;
        }
    }
    else {
        if(v < l)
            ss->Sascii[v] = r;
        else if(v == l && v < ss->max) {
            ss->len = v+1;
            ss->Sascii[v] = r;
        } 
        else {
            if(v != l)
                error(exBounds);
            ns = newstring(v + 1 + v/4);
            memmove(ns->Sascii, ss->Sascii, l);
            ns->Sascii[v] = r;
            ns->len = v+1;
            ss = ns;
        }
    }
    if(ss != rd->pstring) {
        ASSIGN(rd->pstring, ss);
    }
}

String*
slicer(ulong start, ulong v, const String *ds)
{
    String *ns;
    int l, nc;

    if(ds == H) {
        if(start == 0 && v == 0)
            return (String*)H;

        error(exBounds);
    }

    nc = v - start;
    if(ds->len < 0) {
        l = -ds->len;
        if(v < start || v > l)
            error(exBounds);
        ns = newrunes(nc);
        memmove(ns->Srune, ds->Srune + start, nc*sizeof(Rune));
    }
    else {
        l = ds->len;
        if(v < start || v > l)
            error(exBounds);
        ns = newstring(nc);
        memmove(ns->Sascii, ds->Sascii + start, nc);
    }

    return ns;
}

// slicec - Slice string
// 
// Syntax:     slicec  src1, src2, dst
// Function:   dst = dst[src1:src2]
// 
// The slicec instruction creates a new string, which contains characters 
// from the index at src1 to the index src2-1. Unlike slicea , the new string
// is a copy of the elements from the initial string.
OP(slicec)
{
    String *ns = slicer(rs->disint, rm->disint, rd->pstring);

    ASSIGN(rd->pstring, ns);
}

static void
cvtup(Rune *r, const String *s)
{
    const char* bp = s->Sascii;
    const char* ep = bp + s->len;

    while(bp < ep)
        *r++ = *bp++;
}

String*
addstring(String *s1, String *s2, int append)
{
    Rune *r;
    String *ns;
    int l, l1, l2;

    if(s1 == H) {
        if(s2 == H)
            return (String*)H;
        return stringdup(s2);
    }
    if(D2H(s1)->ref > 1)
        append = 0;
    if(s2 == H) {
        if(append)
            return s1;
        return stringdup(s1);
    }

    if(s1->len < 0) {
        l1 = -s1->len;
        if(s2->len < 0)
            l = l1 - s2->len;
        else
            l = l1 + s2->len;
        if(append && l <= s1->max)
            ns = s1;
        else {
            ns = newrunes(append? (l+l/4): l);
            memmove(ns->Srune, s1->Srune, l1*sizeof(Rune));
        }
        ns->len = -l;
        r = &ns->Srune[l1];
        if(s2->len < 0)
            memmove(r, s2->Srune, -s2->len*sizeof(Rune));
        else
            cvtup(r, s2);

        return ns;
    }

    if(s2->len < 0) {
        l2 = -s2->len;
        l = s1->len + l2;
        ns = newrunes(append? (l+l/4): l);
        ns->len = -l;
        cvtup(ns->Srune, s1);
        memmove(&ns->Srune[s1->len], s2->Srune, l2*sizeof(Rune));
        return ns;
    }

    l1 = s1->len;
    l = l1 + s2->len;
    if(append && l <= s1->max)
        ns = s1;
    else {
        ns = newstring(append? (l+l/4): l);
        memmove(ns->Sascii, s1->Sascii, l1);
    }
    ns->len = l;
    memmove(ns->Sascii+l1, s2->Sascii, s2->len);

    return ns;
}

// addc - Add strings
//
// Syntax:     addc    src1, src2, dst
// Function:   dst = src1 + src2
//
// The addc instruction concatenates the two UTF strings pointed to by src1 
// and src2; the result is placed in the pointer addressed by dst. If both 
// pointers are H the result will be a zero length string rather than H.
OP(addc)
{
    String *ns = addstring(rm->pstring, rs->pstring, rm == rd);

    if(ns != rd->pstring) {
        ASSIGN(rd->pstring, ns);
    }
}

// cvtca - Convert string to byte array
//
// Syntax:     cvtca   src, dst
// Function:   dst = array(src)
//
// The src operand must be a string which is converted into an array of bytes
// and stored in dst. The new array is a copy of the characters in src. 
OP(cvtca)
{
    int l;
    Rune *r;
    char *p;
    String *ss = rs->pstring;
    Array *a;

    if(ss == H) {
        a = mem2array(nil, 0);
        goto r;
    }
    if(ss->len < 0) {
        l = -ss->len;
        a = mem2array(nil, runenlen(ss->Srune, l));
        p = (char*)a->data;
        r = ss->Srune;
        while(l--)
            p += runetochar(p, r++);
        goto r;
    }
    a = mem2array(ss->Sascii, ss->len);
r:
    ASSIGN(rd->parray, a);
}

// cvtac - Convert byte array to string
//
// Syntax:     cvtac   src, dst
// Function:   dst = string(src)
//
// The src operand must be an array of bytes, which is converted into a 
// character string and stored in dst. The new string is a copy of the bytes
// in src. 
OP(cvtac)
{
    Array *a = rs->parray;
    String *ds = (String*)H;

    if(a != H)
        ds = c2string((char*)a->data, a->len);

    ASSIGN(rd->pstring, ds);
}

// lenc - Length of string
//
// Syntax:     lenc    src, dst
// Function:   dst = utflen(src)
//
// The lenc instruction computes the number of characters in the UTF string 
// addressed by the src operand and stores it in the dst operand. 
OP(lenc)
{
    int l = 0;
    String *ss = rs->pstring;

    if(ss != H) {
        l = ss->len;
        if(l < 0)
            l = -l;
    }
    rd->disint = l;
}

// cvtcw - Convert string to word
//
// Syntax:     cvtcw   src, dst
// Function:   dst = (int)src
//
// The string addressed by the src operand is converted to a word and stored 
// in the dst operand. Initial white space is ignored; after a possible sign, 
// conversion ceases at the first non-digit in the string. 
OP(cvtcw)
{
    String *s = rs->pstring;

    if(s == H)
        rd->disint = 0;
    else if(s->len < 0)
        rd->disint = strtol(string2c(s), nil, 10);
    else {
        s->Sascii[s->len] = '\0';
        rd->disint = strtol(s->Sascii, nil, 10);
    }
}

// cvtcf - Convert string to real
//
// Syntax:     cvtcf   src, dst
// Function:   dst = (float)src
//
// The string addressed by the src operand is converted to a floating point 
// value and stored in the dst operand. Initial white space is ignored; 
// conversion ceases at the first character in the string that is not part 
// of the representation of the floating point value.
OP(cvtcf)
{
    String *s = rs->pstring;

    if(s == H)
        rd->disreal = 0.0;
    else if(s->len < 0)
        rd->disreal = strtod(string2c(s), nil);
    else {
        s->Sascii[s->len] = '\0';
        rd->disreal = strtod(s->Sascii, nil);
    }
}

// cvtwc - Convert word to string
// 
// Syntax:     cvtwc   src, dst
// Function:   dst = string(src)
// 
// The word addressed by the src operand is converted to a string and stored 
// in the dst operand. The string is the decimal representation of the word.
OP(cvtwc)
{
    String *ds = newstring(16);

    ds->len = sprint(ds->Sascii, "%d", rs->disint);

    ASSIGN(rd->pstring, ds);
}

// cvtlc - Convert big to string
// 
// Syntax:     cvtlc   src, dst
// Function:   dst = string(src)
// 
// The big integer addressed by the src operand is converted to a string and 
// stored in the dst operand. The string is the decimal representation of the
// big integer.
OP(cvtlc)
{
    String *ds = newstring(16);

    ds->len = sprint(ds->Sascii, "%lld", rs->disbig);

    ASSIGN(rd->pstring, ds);
}

// cvtfc - Convert real to string
// 
// Syntax:     cvtfc   src, dst
// Function:   dst = string(src)
// 
// The floating point value addressed by the src operand is converted to a 
// string and stored in the dst operand. The string is a floating point 
// representation of the value.
OP(cvtfc)
{
    String *ds = newstring(32);

    ds->len = sprint(ds->Sascii, "%g", rs->disreal);

    ASSIGN(rd->pstring, ds);
}

#undef OP
char*
string2c(String *s)
{
    char *p;
    int c, l, nc;
    Rune *r, *er;

    if(s == H)
        return "";

    if(s->len >= 0) {
        s->Sascii[s->len] = '\0';
        return s->Sascii;
    }

    nc = -s->len;
    l = (nc * UTFmax) + UTFmax;
    if(s->tmp == nil || msize(s->tmp) < l) {
        free(s->tmp);
        s->tmp = (char*)malloc(l);
        if(s->tmp == nil)
            error(exNomem);
    }

    p = s->tmp;
    r = s->Srune;
    er = r + nc;
    while(r < er) {
        c = *r++;
        if(c < Runeself)
            *p++ = c;
        else
            p += runetochar(p, r-1);
    }

    *p = 0;

    return s->tmp;
}

String*
c2string(const char *cs, int len)
{
    const char *p;
    const char *ecs;
    String *s;
    Rune *r, junk;
    int c, nc, isrune;

    isrune = 0;
    ecs = cs+len;
    p = cs;
    while(len--) {
        c = (unsigned char)*p++;
        if(c >= Runeself) {
            isrune = 1;
            break;
        }
    }

    if(isrune == 0) {
        nc = ecs - cs;
        s = newstring(nc);
        memmove(s->Sascii, cs, nc);
        return s;
    }

    p--;
    nc = p - cs;
    while(p < ecs) {
        c = (unsigned char)*p;
        if(c < Runeself)
            p++;
        else if(p+UTFmax<=ecs || fullrune(p, ecs-p))
            p += chartorune(&junk, p);
        else
            break;
        nc++;
    }
    s = newrunes(nc);
    r = s->Srune;
    while(nc--)
        cs += chartorune(r++, cs);

    return s;
}

String*
newstring(int nb)
{
    Heap *h = nheap(sizeof(String)+nb);
    String *s = H2D(String*, h);

    h->t = &Tstring;
    Tstring.ref++; /*???*/
    s->tmp = nil;
    s->len = nb;
    s->max = hmsize(h) - (sizeof(String)+sizeof(Heap));
    return s;
}

String*
newrunes(int nr)
{
    Heap *h;
    String *s;

    if(nr == 0)
        return newstring(nr);
    if(nr < 0)
        nr = -nr;
    h = nheap(sizeof(String)+nr*sizeof(Rune));
    h->t = &Tstring;
    Tstring.ref++;
    s = H2D(String*, h);
    s->tmp = nil;
    s->len = -nr;
    s->max = (hmsize(h) - (sizeof(String)+sizeof(Heap)))/sizeof(Rune);
    return s;
}

String*
stringdup(const String *s)
{
    String *ns;

    if(s == H)
        return (String*)H;

    if(s->len >= 0) {
        ns = newstring(s->len);
        memmove(ns->Sascii, s->Sascii, s->len);
        return ns;
    }

    ns = newrunes(-s->len);
    memmove(ns->Srune, s->Srune,-s->len*sizeof(Rune));

    return ns;
}

int
stringcmp(const String *s1, const String *s2)
{
    const Rune *r1, *r2;
    const char *a1, *a2;
    int v, n, n1, n2, c1, c2;
    static String snil = { 0, 0, nil };

    if(s1 == H)
        s1 = &snil;
    if(s2 == H)
        s2 = &snil;

    if(s1 == s2)
        return 0;

    v = 0;
    n1 = s1->len;
    if(n1 < 0) {
        n1 = -n1;
        v |= 1;
    }
    n2 = s2->len;
    if(n2 < 0) {
        n2 = -n2;
        v |= 2;
    }

    n = n1;
    if(n2 < n)
        n = n2;

    switch(v) {
    case 0:     /* Ascii Ascii */
        n = memcmp(s1->Sascii, s2->Sascii, n);
        if(n == 0)
            n = n1 - n2;
        return n;
    case 1:     /* Rune Ascii */
        r1 = s1->Srune;
        a2 = s2->Sascii;
        while(n > 0) {
            c1 = *r1++;
            c2 = *a2++;
            if(c1 != c2)
                goto ne;
            n--;
        }
        break;
    case 2:     /* Ascii Rune */
        a1 = s1->Sascii;
        r2 = s2->Srune;
        while(n > 0) {
            c1 = *a1++;
            c2 = *r2++;
            if(c1 != c2)
                goto ne;
            n--;
        }
        break;
    case 3:     /* Rune Rune */
        r1 = s1->Srune;
        r2 = s2->Srune;
        while(n > 0) {
            c1 = *r1++;
            c2 = *r2++;
            if(c1 != c2)
                goto ne;
            n--;
        }
        break;
    }
    return n1 - n2;

ne: if(c1 < c2)
        return -1;
    return 1;
}

String*
splitc(String **s, int expand)
{
    const String * const ss = *s;
    String *ns;

    if(expand && ss->len > 0) {
        ns = newrunes(ss->len);
        cvtup(ns->Srune, ss);
    }
    else
        ns = stringdup(ss);

    ASSIGN(*s, ns); /*XXX*/
    return ns;
}
