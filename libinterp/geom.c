#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include "draw.h"
#include "runt.h"
#include "raise.h"

DISAPI(Point_add)
{
	Draw_Point *ret;

	ret = f->ret;
	ret->x = f->p.x + f->q.x;
	ret->y = f->p.y + f->q.y;
}

DISAPI(Point_sub)
{
	Draw_Point *ret;

	ret = f->ret;
	ret->x = f->p.x - f->q.x;
	ret->y = f->p.y - f->q.y;
}

DISAPI(Point_mul)
{
	Draw_Point *ret;

	ret = f->ret;
	ret->x = f->p.x * f->i;
	ret->y = f->p.y * f->i;
}

DISAPI(Point_div)
{
	Draw_Point *ret;

	if(f->i == 0)
		error(exZdiv);
	ret = f->ret;
	ret->x = f->p.x / f->i;
	ret->y = f->p.y / f->i;
}

DISAPI(Point_eq)
{
	*f->ret = f->p.x == f->q.x && f->p.y == f->q.y;
}

DISAPI(Point_in)
{
	*f->ret = f->p.x >= f->r.min.x && f->p.x < f->r.max.x &&
	       f->p.y >= f->r.min.y && f->p.y < f->r.max.y;
}

DISAPI(Rect_canon)
{
	Draw_Rect *ret;
	DISINT t;

	ret = f->ret;
	if(f->r.max.x < f->r.min.x){
		t = f->r.max.x;
		ret->max.x = f->r.min.x;
		ret->min.x = t;
	}else{
		t = f->r.max.x;
		ret->min.x = f->r.min.x;
		ret->max.x = t;
	}
	if(f->r.max.y < f->r.min.y){
		t = f->r.max.y;
		ret->max.y = f->r.min.y;
		ret->min.y = t;
	}else{
		t = f->r.max.y;
		ret->min.y = f->r.min.y;
		ret->max.y = t;
	}
}

DISAPI(Rect_combine)
{
	Draw_Rect *ret;

	ret = f->ret;
	*ret = f->r;
	if(f->r.min.x > f->s.min.x)
		ret->min.x = f->s.min.x;
	if(f->r.min.y > f->s.min.y)
		ret->min.y = f->s.min.y;
	if(f->r.max.x < f->s.max.x)
		ret->max.x = f->s.max.x;
	if(f->r.max.y < f->s.max.y)
		ret->max.y = f->s.max.y;
}

DISAPI(Rect_eq)
{
	*f->ret = f->r.min.x == f->s.min.x
		&& f->r.max.x == f->s.max.x
		&& f->r.min.y == f->s.min.y
		&& f->r.max.y == f->s.max.y;
}

DISAPI(Rect_Xrect)
{
	*f->ret = f->r.min.x < f->s.max.x
		&& f->s.min.x < f->r.max.x
		&& f->r.min.y < f->s.max.y
		&& f->s.min.y < f->r.max.y;
}

DISAPI(Rect_clip)
{
	Draw_Rect *r, *s, *ret;

	r = &f->r;
	s = &f->s;
	ret = &f->ret->t0;

	/*
	 * Expand rectXrect() in line for speed
	 */
	if(!(r->min.x<s->max.x && s->min.x<r->max.x
	&& r->min.y<s->max.y && s->min.y<r->max.y)){
		*ret = *r;
		f->ret->t1 = 0;
		return;
	}

	/* They must overlap */
	if(r->min.x < s->min.x)
		ret->min.x = s->min.x;
	else
		ret->min.x = r->min.x;
	if(r->min.y < s->min.y)
		ret->min.y = s->min.y;
	else
		ret->min.y = r->min.y;
	if(r->max.x > s->max.x)
		ret->max.x = s->max.x;
	else
		ret->max.x = r->max.x;
	if(r->max.y > s->max.y)
		ret->max.y = s->max.y;
	else
		ret->max.y = r->max.y;
	f->ret->t1 = 1;
}

DISAPI(Rect_inrect)
{
	*f->ret = f->s.min.x <= f->r.min.x
		&& f->r.max.x <= f->s.max.x
		&& f->s.min.y <= f->r.min.y
		&& f->r.max.y <= f->s.max.y;
}

DISAPI(Rect_contains)
{
	DISINT x = f->p.x;
	DISINT y = f->p.y;

	*f->ret = x >= f->r.min.x && x < f->r.max.x
		&& y >= f->r.min.y && y < f->r.max.y;
}

DISAPI(Rect_addpt)
{
	Draw_Rect *ret = f->ret;
	DISINT n = f->p.x;

	ret->min.x = f->r.min.x + n;
	ret->max.x = f->r.max.x + n;
	n = f->p.y;
	ret->min.y = f->r.min.y + n;
	ret->max.y = f->r.max.y + n;
}

DISAPI(Rect_subpt)
{
	Draw_Rect *ret = f->ret;
	DISINT n = f->p.x;

	ret->min.x = f->r.min.x - n;
	ret->max.x = f->r.max.x - n;
	n = f->p.y;
	ret->min.y = f->r.min.y - n;
	ret->max.y = f->r.max.y - n;
}

DISAPI(Rect_inset)
{
	Draw_Rect *ret = f->ret;
	DISINT n = f->n;

	ret->min.x = f->r.min.x + n;
	ret->min.y = f->r.min.y + n;
	ret->max.x = f->r.max.x - n;
	ret->max.y = f->r.max.y - n;
}

DISAPI(Rect_dx)
{
	*f->ret = f->r.max.x-f->r.min.x;
}

DISAPI(Rect_dy)
{
	*f->ret = f->r.max.y-f->r.min.y;
}

DISAPI(Rect_size)
{
	Draw_Point *ret;

	ret = f->ret;
	ret->x = f->r.max.x-f->r.min.x;
	ret->y = f->r.max.y-f->r.min.y;
}
