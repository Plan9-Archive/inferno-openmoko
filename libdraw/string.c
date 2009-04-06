#include <lib9.h>
#include <draw.h>

enum
{
	Max = 100
};

Point
string(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_z const char *s)
{
	return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, nil, ZP, SoverD);
}

Point
stringop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_z const char *s, Drawop op)
{
	return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, nil, ZP, op);
}

Point
stringn(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_ecount_z(len) const char *s, int len)
{
	return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, nil, ZP, SoverD);
}

Point
stringnop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_ecount_z(len) const char *s, int len, Drawop op)
{
	return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, nil, ZP, op);
}

Point
runestring(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_z const Rune *r)
{
	return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, nil, ZP, SoverD);
}

Point
runestringop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_z const Rune *r, Drawop op)
{
	return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, nil, ZP, op);
}

Point
runestringn(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_ecount_z(len) const Rune *r, int len)
{
	return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, nil, ZP, SoverD);
}

Point
runestringnop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, __in_ecount_z(len) const Rune *r, int len, Drawop op)
{
	return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, nil, ZP, op);
}

Point
_string(__in_ecount(1) const Image *dst, 
        Point pt, 
        __in_ecount(1) const Image *src, 
        Point sp, 
        Font *f, 
        const char *s, 
        const Rune *r, 
        int len, 
        Rectangle clipr, 
        __in_ecount_opt(1) const Image *bg, 
        Point bgp, 
        Drawop op)
{
	int m, n, wid, max;
	ushort cbuf[Max], *c, *ec;
	uchar *b;
	const char *subfontname;
	const char **sptr;
	const Rune **rptr;
	Font *def;

	if(s == nil){
		s = "";
		sptr = nil;
	}else
		sptr = &s;
	if(r == nil){
		r = (Rune*) L"";
		rptr = nil;
	}else
		rptr = &r;
	while((*s || *r) && len){
		max = Max;
		if(len < max)
			max = len;
		n = cachechars(f, sptr, rptr, cbuf, max, &wid, &subfontname);
		if(n > 0){
			_setdrawop(dst->display, op);

			m = 47+2*n;
			if(bg)
				m += 4+2*4;
			b = bufimage(dst->display, m);
			if(b == 0){
				_drawprint(2, "string: %r\n");
				break;
			}
			if(bg)
				b[0] = 'x';
			else
				b[0] = 's';
			BPLONG(b+1, dst->id);
			BPLONG(b+5, src->id);
			BPLONG(b+9, f->cacheimage->id);
			BPLONG(b+13, pt.x);
			BPLONG(b+17, pt.y+f->ascent);
			BPLONG(b+21, clipr.min.x);
			BPLONG(b+25, clipr.min.y);
			BPLONG(b+29, clipr.max.x);
			BPLONG(b+33, clipr.max.y);
			BPLONG(b+37, sp.x);
			BPLONG(b+41, sp.y);
			BPSHORT(b+45, n);
			b += 47;
			if(bg){
				BPLONG(b, bg->id);
				BPLONG(b+4, bgp.x);
				BPLONG(b+8, bgp.y);
				b += 12;
			}
			ec = &cbuf[n];
			for(c=cbuf; c<ec; c++, b+=2)
				BPSHORT(b, *c);
			pt.x += wid;
			bgp.x += wid;
			agefont(f);
			len -= n;
		}
		if(subfontname){
			if(_getsubfont(f->display, subfontname) == 0){
				def = f->display->defaultfont;
				if(def && f!=def)
					f = def;
				else
					break;
			}
		}
	}
	return pt;
}
