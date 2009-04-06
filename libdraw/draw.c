#include <lib9.h>
#include <draw.h>

void
_setdrawop(Display *d, Drawop op)
{
	uchar *a;

	if(op != SoverD){
		a = bufimage(d, 1+1);
		if(a == 0)
			return;
		a[0] = 'O';
		a[1] = op;
	}
}

static void
draw1(__in const Image *dst,
      __in const Rectangle *r,
      __in const Image *src,
      __in const Point *p0,
      __in const Image *mask,
      __in const Point *p1,
      Drawop op)
{
	uchar *a;

	_setdrawop(dst->display, op);

	a = bufimage(dst->display, 1+4+4+4+4*4+2*4+2*4);
	if(a == 0)
		return;
	if(src == nil)
		src = dst->display->black;
	if(mask == nil)
		mask = dst->display->opaque;
	a[0] = 'd';
	BPLONG(a+1, dst->id);
	BPLONG(a+5, src->id);
	BPLONG(a+9, mask->id);
	BPLONG(a+13, r->min.x);
	BPLONG(a+17, r->min.y);
	BPLONG(a+21, r->max.x);
	BPLONG(a+25, r->max.y);
	BPLONG(a+29, p0->x);
	BPLONG(a+33, p0->y);
	BPLONG(a+37, p1->x);
	BPLONG(a+41, p1->y);
}

void
draw(__in const Image *dst, Rectangle r, __in const Image *src, __in const Image *mask, Point p1)
{
	draw1(dst, &r, src, &p1, mask, &p1, SoverD);
}

void
drawop(__in const Image *dst, Rectangle r, __in const Image *src, __in const Image *mask, Point p1, Drawop op)
{
	draw1(dst, &r, src, &p1, mask, &p1, op);
}

void
gendraw(__in const Image *dst, Rectangle r, __in const Image *src, Point p0, __in const Image *mask, Point p1)
{
	draw1(dst, &r, src, &p0, mask, &p1, SoverD);
}

void
gendrawop(__in const Image *dst, Rectangle r, __in const Image *src, Point p0, __in const Image *mask, Point p1, Drawop op)
{
	draw1(dst, &r, src, &p0, mask, &p1, op);
}
