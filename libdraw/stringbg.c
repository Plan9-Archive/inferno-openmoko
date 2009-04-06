#include <lib9.h>
#include <draw.h>

Point
stringbg(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, char *s, __in_ecount(1) const Image *bg, Point bgp)
{
	return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, bg, bgp, SoverD);
}

Point
stringbgop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, char *s, __in_ecount(1) const Image *bg, Point bgp, Drawop op)
{
	return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, bg, bgp, op);
}

Point
stringnbg(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, char *s, int len, __in_ecount(1) const Image *bg, Point bgp)
{
	return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, bg, bgp, SoverD);
}

Point
stringnbgop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, char *s, int len, __in_ecount(1) const Image *bg, Point bgp, Drawop op)
{
	return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, bg, bgp, op);
}

Point
runestringbg(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, Rune *r, __in_ecount(1) const Image *bg, Point bgp)
{
	return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, bg, bgp, SoverD);
}

Point
runestringbgop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, Rune *r, __in_ecount(1) const Image *bg, Point bgp, Drawop op)
{
	return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, bg, bgp, op);
}

Point
runestringnbg(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, Rune *r, int len, __in_ecount(1) const Image *bg, Point bgp)
{
	return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, bg, bgp, SoverD);
}

Point
runestringnbgop(__in_ecount(1) const Image *dst, Point pt, __in_ecount(1) const Image *src, Point sp, Font *f, Rune *r, int len, __in_ecount(1) const Image *bg, Point bgp, Drawop op)
{
	return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, bg, bgp, op);
}
