#include <lib9.h>
#include <draw.h>

static void
doellipse(int cmd,
          __in_ecount(1) const Image *dst,
          __in_ecount(1) const Point *c,
          int xr,
          int yr,
          int thick,
          __in_ecount(1) const Image *src,
          __in_ecount(1) const Point *sp,
          int alpha,
          int phi,
          Drawop op)
{
	uchar *a;

	_setdrawop(dst->display, op);

	a = bufimage(dst->display, 1+4+4+2*4+4+4+4+2*4+2*4);
	if(a == 0){
		_drawprint(2, "image ellipse: %r\n");
		return;
	}
	a[0] = cmd;
	BPLONG(a+1, dst->id);
	BPLONG(a+5, src->id);
	BPLONG(a+9, c->x);
	BPLONG(a+13, c->y);
	BPLONG(a+17, xr);
	BPLONG(a+21, yr);
	BPLONG(a+25, thick);
	BPLONG(a+29, sp->x);
	BPLONG(a+33, sp->y);
	BPLONG(a+37, alpha);
	BPLONG(a+41, phi);
}

void
ellipse(__in_ecount(1) const Image *dst, Point c, int a, int b, int thick, __in_ecount(1) const Image *src, Point sp)
{
	doellipse('e', dst, &c, a, b, thick, src, &sp, 0, 0, SoverD);
}

void
ellipseop(__in_ecount(1) const Image *dst, Point c, int a, int b, int thick, __in_ecount(1) const Image *src, Point sp, Drawop op)
{
	doellipse('e', dst, &c, a, b, thick, src, &sp, 0, 0, op);
}

void
fillellipse(__in_ecount(1) const Image *dst, Point c, int a, int b, __in_ecount(1) const Image *src, Point sp)
{
	doellipse('E', dst, &c, a, b, 0, src, &sp, 0, 0, SoverD);
}

void
fillellipseop(__in_ecount(1) const Image *dst, Point c, int a, int b, __in_ecount(1) const Image *src, Point sp, Drawop op)
{
	doellipse('E', dst, &c, a, b, 0, src, &sp, 0, 0, op);
}

void
arc(__in_ecount(1) const Image *dst, Point c, int a, int b, int thick, __in_ecount(1) const Image *src, Point sp, int alpha, int phi)
{
	alpha |= 1<<31;
	doellipse('e', dst, &c, a, b, thick, src, &sp, alpha, phi, SoverD);
}

void
arcop(__in_ecount(1) const Image *dst, Point c, int a, int b, int thick, __in_ecount(1) const Image *src, Point sp, int alpha, int phi, Drawop op)
{
	alpha |= 1<<31;
	doellipse('e', dst, &c, a, b, thick, src, &sp, alpha, phi, op);
}

void
fillarc(__in_ecount(1) const Image *dst, Point c, int a, int b, __in_ecount(1) const Image *src, Point sp, int alpha, int phi)
{
	alpha |= 1<<31;
	doellipse('E', dst, &c, a, b, 0, src, &sp, alpha, phi, SoverD);
}

void
fillarcop(__in_ecount(1) const Image *dst, Point c, int a, int b, __in_ecount(1) const Image *src, Point sp, int alpha, int phi, Drawop op)
{
	alpha |= 1<<31;
	doellipse('E', dst, &c, a, b, 0, src, &sp, alpha, phi, op);
}
