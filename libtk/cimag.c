#include <lib9.h>
#include <draw.h>

#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <tk.h>

#include <canvs.h>


/* Image Options (+ means implemented)
	+anchor
	+image
*/

typedef struct TkCimag TkCimag;
struct TkCimag
{
	int	anchor;
	Point	anchorp;
	TkImg*	tki;
};

static
TkOption imgopts[] =
{
	{"anchor",	OPTstab,	offsetof(TkCimag, anchor),	{tkanchor}},
	{"image",	OPTimag,	offsetof(TkCimag, tki)		},
	{nil}
};

static
TkOption itemopts_cimag[] =
{
	{"tags",	OPTctag,	offsetof(TkCitem, tags)		},
	{nil}
};

void
tkcvsimgsize(TkCitem *i)
{
	Point o;
	int dx, dy;
	TkCimag *t;

	t = TKobj(TkCimag, i);
	i->p.bb = bbnil;
	if(t->tki == nil)
		return;

	dx = t->tki->w;
	dy = t->tki->h;

	o = tkcvsanchor(i->p.drawpt[0], dx, dy, t->anchor);

	i->p.bb.min.x = o.x;
	i->p.bb.min.y = o.y;
	i->p.bb.max.x = o.x + dx;
	i->p.bb.max.y = o.y + dy;
	t->anchorp = subpt(o, i->p.drawpt[0]);
}

TH(tkcvsimgcreat)
{
	const char* e;
	TkCimag *t;
	TkCitem *i;
	TkCanvas *c;
	TkOptab tko[3];

	c = TKobj(TkCanvas, tk);

	i = tkcnewitem(tk, TkCVimage, sizeof(TkCitem)+sizeof(TkCimag));
	if(i == nil)
		return TkNomem;

	t = TKobj(TkCimag, i);

	e = tkparsepts(tk->env->top, &i->p, &arg, 0);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}
	if(i->p.npoint != 1) {
		tkcvsfreeitem(i);
		return TkFewpt;
	}

	tko[0].ptr = t;
	tko[0].optab = imgopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts_cimag;
	tko[2].ptr = nil;
	e = tkparse(tk->env->top, arg, tko, nil);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}

	e = tkcaddtag(tk, i, 1);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}

	tkcvsimgsize(i);

	e = tkvalue(val, "%d", i->id);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}

	tkcvsappend(c, i);
	tkbbmax(&c->update, &i->p.bb);
	tkcvssetdirty(tk);
	return nil;
}

const char*
tkcvsimgcget(TkCitem *i, __in_z const char *arg, char **val)
{
	TkOptab tko[3];
	TkCimag *t = TKobj(TkCimag, i);

	tko[0].ptr = t;
	tko[0].optab = imgopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts_cimag;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val, i->env->top);
}

const char*
tkcvsimgconf(Tk *tk, TkCitem *i, __in_z const char *arg)
{
	const char *e;
	TkOptab tko[3];
	TkCimag *t = TKobj(TkCimag, i);

	tko[0].ptr = t;
	tko[0].optab = imgopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts_cimag;
	tko[2].ptr = nil;

	e = tkparse(tk->env->top, arg, tko, nil);
	tkcvsimgsize(i);
	return e;
}

void
tkcvsimgfree(TkCitem *i)
{
	TkCimag *t;

	t = TKobj(TkCimag, i);
	if(t->tki)
		tkimgput(t->tki);
}

void
tkcvsimgdraw(__in_ecount(1) const Image *img, TkCitem *i, TkEnv *pe)
{
	TkCimag *t;
	TkImg *tki;
	Rectangle r;
	Image *fg;

	USED(pe);

	t = TKobj(TkCimag, i);
	tki = t->tki;
	if(tki == nil)
		return;
	fg = tki->img;
	if(fg == nil)
		return;

	r.min = addpt(t->anchorp, i->p.drawpt[0]);
	r.max = r.min;
	r.max.x += tki->w;
	r.max.y += tki->h;

	draw(img, r, fg, nil, ZP);
}

const char*
tkcvsimgcoord(TkCitem *i, __in_z const char *arg, int x, int y)
{
	const char* e;
	TkCpoints p;

	if(arg == nil) {
		tkxlatepts(i->p.parampt, i->p.npoint, x, y);
		tkxlatepts(i->p.drawpt, i->p.npoint, TKF2I(x), TKF2I(y));
		i->p.bb = rectaddpt(i->p.bb, Pt(TKF2I(x), TKF2I(y)));
	}
	else {
		e = tkparsepts(i->env->top, &p, &arg, 0);
		if(e != nil)
			return e;
		if(p.npoint != 1) {
			tkfreepoint(&p);
			return TkFewpt;
		}
		tkfreepoint(&i->p);
		i->p = p;
		tkcvsimgsize(i);
	}
	return nil;
}
