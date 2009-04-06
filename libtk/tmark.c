#include <lib9.h>
#include <draw.h>

#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <tk.h>

#include <textw.h>

#define istring u.string
#define iwin u.win
#define imark u.mark
#define iline u.line

static const char* tktmarkgravity(Tk*, __in_z const char*, char**);
static const char* tktmarknames(Tk*, __in_z const char*, char**);
static const char* tktmarknext(Tk*, __in_z const char*, char**);
static const char* tktmarkprevious(Tk*, __in_z const char*, char**);
static const char* tktmarkset(Tk*, __in_z const char*, char**);
static const char* tktmarkunset(Tk*, __in_z const char*, char**);

TkCmdtab
tktmarkcmd[] =
{
	{"gravity",	tktmarkgravity},
	{"names",	tktmarknames},
	{"next",	tktmarknext},
	{"previous",tktmarkprevious},
	{"set",		tktmarkset},
	{"unset",	tktmarkunset},
	{nil}
};

const char*
tktaddmarkinfo(TkText *tkt, __in_z const char *name, TkTmarkinfo **ret)
{
	TkTmarkinfo *mi;

	mi = (TkTmarkinfo *)malloc(sizeof(TkTmarkinfo));
	if(mi == nil)
		return TkNomem;

	mi->name = strdup(name);
	if(mi->name == nil) {
		free(mi);
		return TkNomem;
	}
	mi->gravity = Tkright;
	mi->cur = nil;
	mi->next = tkt->marks;
	tkt->marks = mi;
	*ret = mi;
	return nil;
}

void
tktfreemarks(TkTmarkinfo *m)
{
	TkTmarkinfo *n;

	while(m != nil) {
		n = m->next;
		free(m->name);
		free(m);
		m = n;
	}
}

TkTmarkinfo *
tktfindmark(TkTmarkinfo *m, __in_z const char *name)
{
	while(m != nil) {
		if(strcmp(m->name, name) == 0)
			return m;
		m = m->next;
	}
	return nil;
}

int
tktmarkind(Tk *tk, __in_z const char *name, TkTindex *ans)
{
	TkTmarkinfo *mk;
	TkText *tkt = TKobj(TkText, tk);

	if(strcmp(name, "current") == 0) {
		tktxyind(tk, tkt->current.x, tkt->current.y, ans);
		return 1;
	}

	mk = tktfindmark(tkt->marks, name);
	if(mk == nil || mk->cur == nil)
		return 0;

	ans->item = mk->cur;
	ans->line = tktitemline(ans->item);
	ans->pos = 0;
	return 1;
}

const char*
tktmarkparse(Tk *tk, __inout_ecount_full(1) const char **parg, TkTmarkinfo **ret)
{
	char *buf;
    const char *e;
	TkText *tkt = TKobj(TkText, tk);

	buf = (char*)mallocz(Tkmaxitem, 0);
	if(buf == nil)
		return TkNomem;

	*parg = tkword(tk->env->top, *parg, buf, buf+Tkmaxitem, nil);
	if(*buf == '\0') {
		free(buf);
		return TkOparg;
	}

	*ret = tktfindmark(tkt->marks, buf);
	if(*ret == nil) {
		e = tktaddmarkinfo(tkt, buf, ret);
		if(e != nil) {
			free(buf);
			return e;
		}
	}
	free(buf);

	return nil;
}

/*
 * Insert mark before ixnew, first removing it from old place, if any.
 * Make sure ixnew continues to point after mark.
 */
const char*
tktmarkmove(Tk *tk, TkTmarkinfo *m, TkTindex *ixnew)
{
	const char *e;
	int deleted, split;
	TkTitem *i;
	TkTindex ix, pix;
	TkText *tkt = TKobj(TkText, tk);

	deleted = 0;
	if(m->cur != nil) {
		if(m->cur == ixnew->item)
			return nil;
		ix.item = m->cur;
		ix.line = tktitemline(m->cur);
		ix.pos = 0;
		tktremitem(tkt, &ix);
		deleted = 1;
	}

	/* XXX - Tad: memory leak on 'i' if something fails later? */
	e = tktnewitem(TkTmark, 0, &i);
	if(e != nil)
		return e;

	i->imark = m;
	m->cur = i;

	/* keep adjacent marks sorted: all rights, then all lefts */
	if(m->gravity == Tkright) {
		while(ixnew->item->kind == TkTmark && ixnew->item->imark->gravity == Tkleft)
			if(!tktadjustind(tkt, TkTbyitem, ixnew))
				break;
	}
	else {
		for(;;) {
			pix = *ixnew;
			if(!tktadjustind(tkt, TkTbyitemback, &pix))
				break;
			if(pix.item->kind == TkTmark && pix.item->imark->gravity == Tkright)
				*ixnew = pix;
			else
				break;
		}
	}

	split = (ixnew->pos > 0);
	e = tktsplititem(ixnew);
	if(e != nil)
		return e;

	e = tktiteminsert(tkt, ixnew, i);
	if(e != nil)
		return nil;

	if(strcmp(m->name, "insert") == 0 || split) {
		if(deleted && ix.line != ixnew->line) {
			tktfixgeom(tk, tktprevwrapline(tk, ix.line), ix.line, 0);
			/*
			 * this is ok only because tktfixgeom cannot
			 * free mark items, and we know that i is a mark item.
			 */
			ixnew->item = i;
			ixnew->line = tktitemline(i);
			ixnew->pos = 0;
		}
		tktfixgeom(tk, tktprevwrapline(tk, ixnew->line), ixnew->line, 0);
		tktextsize(tk, 1);
	}

	ixnew->item = i;
	ixnew->line = tktitemline(i);
	ixnew->pos = 0;
	return nil;
}

/* Text Mark Commands (+ means implemented)
	+gravity
	+names
	+next
	+previous
	+set
	+unset
*/

static TH(tktmarkgravity)
{
	const char *e;
	TkTmarkinfo *m;
	char *buf;

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;

	if(*arg == '\0')
		return tkvalue(val, (m->gravity & Tkleft)? "left" : "right");
	else {
		buf = (char*)mallocz(Tkmaxitem, 0);
		if(buf == nil)
			return TkNomem;
		tkword(tk->env->top, arg, buf, buf+Tkmaxitem, nil);
		if(strcmp(buf, "left") == 0)
			m->gravity = Tkleft;
		else
		if(strcmp(buf, "right") == 0)
			m->gravity = Tkright;
		else {
			free(buf);
			return TkBadcm;
		}
		free(buf);
	}
	return nil;
}

static TH(tktmarknames)
{
	const char *r, *fmt;
	TkTmarkinfo *m;
	TkText *tkt = TKobj(TkText, tk);

	USED(arg);

	fmt = "%s";
	for(m = tkt->marks; m != nil; m = m->next) {
		r = tkvalue(val, fmt, m->name);
		if(r != nil)
			return r;
		fmt = " %s";
	}
	return nil;
}

static TH(tktmarknext)
{
	const char *e;
	TkTmarkinfo *mix;
	TkTindex ix, ixend;
	TkText *tkt = TKobj(TkText, tk);

	/* special behavior if specified index is a mark name */
	mix = tktfindmark(tkt->marks, arg);

	e = tktindparse(tk, &arg, &ix);
	if(e != nil)
		return e;

	if(mix != nil)
		tktadjustind(tkt, TkTbyitem, &ix);

	/* special behavior if index is 'end' */
	tktendind(tkt, &ixend);
	if(tktindcompare(tkt, &ix, TkEq, &ixend)) {
		do {
			tktadjustind(tkt, TkTbyitemback, &ix);
		} while(ix.item->kind == TkTmark);
	}

	do {
		if(ix.item->kind == TkTmark)
			return tkvalue(val, "%s", ix.item->imark->name);

	} while(tktadjustind(tkt, TkTbyitem, &ix));

	return nil;
}

static TH(tktmarkprevious)
{
	const char *e;
	TkTindex ix;
	TkText *tkt = TKobj(TkText, tk);

	e = tktindparse(tk, &arg, &ix);
	if(e != nil)
		return e;

	while(tktadjustind(tkt, TkTbyitemback, &ix)) {
		if(ix.item->kind == TkTmark)
			return tkvalue(val, "%s", ix.item->imark->name);
	}

	return nil;
}

/* XXX - Tad: possible memory leak here */
static TH(tktmarkset)
{
	const char *e;
	TkTmarkinfo *m;
	TkTindex ixnew;

	USED(val);

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;
	e = tktindparse(tk, &arg, &ixnew);
	if(e != nil)
		return e;

	return tktmarkmove(tk, m, &ixnew);
}

static TH(tktmarkunset)
{
	TkText *tkt;
	TkTmarkinfo *m, **p;
	TkTindex ix;
	const char *e;
	int resize;

	USED(val);

	tkt = TKobj(TkText, tk);

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;

	resize = 0;
	while(m != nil) {
		if(strcmp(m->name, "insert") == 0 || strcmp(m->name, "current") == 0)
			return TkBadvl;

		if(m->cur != nil) {
			ix.item = m->cur;
			ix.line = tktitemline(m->cur);
			ix.pos = 0;
			tktremitem(tkt, &ix);
			tktfixgeom(tk, tktprevwrapline(tk, ix.line), ix.line, 0);
			resize = 1;
		}

		for(p = &tkt->marks; *p != nil; p = &(*p)->next) {
			if(*p == m) {
				*p = m->next;
				break;
			}
		}
		m->next = nil;
		tktfreemarks(m);

		if(*arg != '\0') {
			e = tktmarkparse(tk, &arg, &m);
			if(e != nil)
				return e;
		}
		else
			m = nil;
	}
	if (resize)
		tktextsize(tk, 1);
	return nil;
}
