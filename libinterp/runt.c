#include <lib9.h>
#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <sysmod.h>
#include <raise.h>


static	int		utfnleng(const char*, int, int*);

void
sysmodinit(void)
{
	sysinit();
	builtinmod("$Sys", Sysmodtab, Sysmodlen);
}

int
xprint(Prog *xp, const void* vfp, const void* vva, String *s1, char *buf, int n)
{
	DISINT i;
	void *p;
	DISBIG bg;
	Type *t;
	double d;
	String *ss;
	ulong *ptr;
	int nc, c, isbig, isr, sip;
	char *b, *eb, *f, fmt[32];
	Rune r;

	const char *fp = vfp;
	const char *va = vva;

	sip = 0;
	isr = 0;
	if(s1 == H)
		return 0;
	nc = s1->len;
	if(nc < 0) {
		nc = -nc;
		isr = 1;
	}

	b = buf;
	eb = buf+n-1;
	while(nc--) {
		c = isr ? s1->Srune[sip] : s1->Sascii[sip];
		sip++;
		if(c != '%') {
			if(b < eb) {
				if(c < Runeself)
					*b++ = c;
				else
					b += snprint(b, eb-b, "%C", c);
			}
			continue;
		}
		f = fmt;
		*f++ = c;
		isbig = 0;
		while(nc--) {
			c = isr ? s1->Srune[sip] : s1->Sascii[sip];
			sip++;
			*f++ = c;
			*f = '\0';
			switch(c) {
			default:
				continue;
			case '*':
				i = *(DISINT*)va;
				f--;
				f += snprint(f, sizeof(fmt)-(f-fmt), "%d", i);
				va += sizeof(DISINT);
				continue;
			case 'b':
				f[-1] = 'l';
				*f++ = 'l';
				*f = '\0';
				isbig = 1;
				continue;
			case '%':
				if(b < eb)
					*b++ = '%';
				break;
			case 'q':
			case 's':
				ss = *(String**)va;
				va += sizeof(String*);
				if(ss == H)
					p = "";
				else
				if(ss->len < 0) {
					f[-1] += 'A'-'a';
					ss->Srune[-ss->len] = L'\0';
					p = ss->Srune;
				}
				else {
					ss->Sascii[ss->len] = '\0';
					p = ss->Sascii;
				}
				b += snprint(b, eb-b, fmt, p);
				break;
			case 'E':
				f--;
				r = 0x00c9;	/* L'É' */
				f += runetochar(f, &r);	/* avoid clash with ether address */
				*f = '\0';
				/* fall through */
			case 'e':
			case 'f':
			case 'g':
			case 'G':
				while((va - fp) & (sizeof(DISREAL)-1))
					va++;
				d = *(DISREAL*)va;
				b += snprint(b, eb-b, fmt, d);
				va += sizeof(DISREAL);
				break;
			case 'd':
			case 'o':
			case 'x':
			case 'X':
			case 'c':
				if(isbig) {
					while((va - fp) & (sizeof(DISBIG)-1)) /* WTF */
						va++;
					bg = *(DISBIG*)va;
					b += snprint(b, eb-b, fmt, bg);
					va += sizeof(DISBIG);
				}
				else {
					i = *(DISINT*)va;
					/* always a unicode character */
					if(c == 'c')
						f[-1] = 'C';
					b += snprint(b, eb-b, fmt, i);
					va += sizeof(DISINT);
				}
				break;
			case 'r':
				b = syserr(b, eb, xp);
				break;
/* Debugging formats - may disappear */
			case 'H':
				ptr = *(ulong**)va;
				c = -1;
				t = nil;
				if(ptr != H) {
					c = D2H(ptr)->ref;
					t = D2H(ptr)->t;
				}
				b += snprint(b, eb-b, "%d.%.8p", c, t);
				va += sizeof(void*);
				break;
			}
			break;
		}
	}
	return b - buf;
}

int
bigxprint(Prog *xp, const void *vfp, const void *vva, String *s1, char **buf, int s)
{
	char *b;
	int m, n;

	m = s;
	for (;;) {
		m *= 2;
		b = (char*)malloc(m);
		if (b == nil)
			error(exNomem);
		n = xprint(xp, vfp, vva, s1, b, m);
		if (n < m-UTFmax-2)
			break;
		free(b);
	}
	*buf = b;
	return n;
}

DISAPI(Sys_sprint)
{
	int n;
	char buf[256], *b = buf; /* FIXME: small buffer */

	n = xprint(currun(), f, &f->vargs, f->s, buf, sizeof(buf));
	if (n >= sizeof(buf)-UTFmax-2)
		n = bigxprint(currun(), f, &f->vargs, f->s, &b, sizeof(buf));
	b[n] = '\0';
	retstr(b, f->ret);
	if (b != buf)
		free(b);
}

DISAPI(Sys_aprint)
{
	int n;
	char buf[256], *b = buf; /* FIXME: small buffer */

	n = xprint(currun(), f, &f->vargs, f->s, buf, sizeof(buf));
	if (n >= sizeof(buf)-UTFmax-2)
		n = bigxprint(currun(), f, &f->vargs, f->s, &b, sizeof(buf));
	ASSIGN(*f->ret, mem2array(b, n));
	if (b != buf)
		free(b);
}

static int
tokdelim(int c, const String *d)
{
	int l;
	const char *p;
	const Rune *r;

	l = d->len;
	if(l < 0) {
		l = -l;
		for(r = d->Srune; l != 0; l--)
			if(*r++ == c)
				return 1;
		return 0;
	}
	for(p = d->Sascii; l != 0; l--)
		if(*p++ == c)
			return 1;
	return 0;
}

DISAPI(Sys_tokenize)
{
	const String * const s = f->s;
	const String * const d = f->delim;
	List **h, *l, *nl;
	int n, c, nc, first, last, srune;

	if(s == H || d == H) {
		f->ret->t0 = 0;
		ASSIGN(f->ret->t1, (List*)H);
		return;
	}

	n = 0;
	l = (List*)H;
	h = &l;
	first = 0;
	srune = 0;

	nc = s->len;
	if(nc < 0) {
		nc = -nc;
		srune = 1;
	}

	while(first < nc) {
		while(first < nc) {
			c = srune ? s->Srune[first] : s->Sascii[first];
			if(tokdelim(c, d) == 0)
				break;
			first++;
		}

		last = first;

		while(last < nc) {
			c = srune ? s->Srune[last] : s->Sascii[last];
			if(tokdelim(c, d) != 0)
				break;
			last++;
		}

		if(first == last)
			break;

		nl = cons(sizeof(String*), h);
		nl->tail = (List*)H;
		nl->t = &Tptr;
		Tptr.ref++;
		nl->data.pstring = slicer(first, last, s);
		h = &nl->tail;

		first = last;
		n++;
	}

	f->ret->t0 = n;
	ASSIGN(f->ret->t1, l);
}

DISAPI(Sys_utfbytes)
{
	Array * const a = f->buf;
	int nbyte;

	if(a == H || f->n<0 || f->n >= a->len)
		error(exBounds);

	utfnleng(a->data, f->n, &nbyte);
	*f->ret = nbyte;
}

DISAPI(Sys_byte2char)
{
	Array * const a = f->buf;
	const int n = f->n;
	Rune r;
	const char *p;
	int w;

	if(a == H || n<0 || n >= a->len)
		error(exBounds);

	r = a->data[n];
	if(r < Runeself){
		f->ret->t0 = r;
		f->ret->t1 = 1;
		f->ret->t2 = 1;
		return;
	}
	p = a->data + n;
	if(n+UTFmax <= a->len || fullrune(p, a->len-n))
		w = chartorune(&r, p);
	else {
		/* insufficient data */
		f->ret->t0 = Runeerror;
		f->ret->t1 = 0;
		f->ret->t2 = 0;
		return;
	}
	if(r == Runeerror && w==1){	/* encoding error */
		f->ret->t0 = Runeerror;
		f->ret->t1 = 1;
		f->ret->t2 = 0;
		return;
	}
	f->ret->t0 = r;
	f->ret->t1 = w;
	f->ret->t2 = 1;
}

DISAPI(Sys_char2byte)
{
	Array * const a = f->buf;
	const int n = f->n;
	int c = f->c;
	Rune r;

	if(a == H || n<0 || n>=a->len)
		error(exBounds);

	if(c<0 || c>=(1<<16))
		c = Runeerror;
	if(c < Runeself){
		a->data[n] = c;
		*f->ret = 1;
		return;
	}
	r = c;
	if(n+UTFmax<=a->len || runelen(c)<=a->len-n){
		*f->ret = runetochar((char*)a->data+n, &r);
		return;
	}
	*f->ret = 0;
}

extern void frame_type_fix(Type* t);

Module *
builtinmod(const char *name, const Runtab *vr, int rlen)
{
	const Runtab *r = vr;
	Type *t;
	Module *m;
	Link *l;

	m = newmod(name);
	if(rlen == 0){
		while(r->name){
			rlen++;
			r++;
		}
		r = vr;
	}
	l = m->ext = (Link*)malloc((rlen+1)*sizeof(Link));
	if(l == nil){
		freemod(m);
		return nil;
	}
	while(r->name) {
		t = dtype(freeheap, r->size, r->map, r->np, r->name); /* Frame of function r->name */

		/* BUG: t may be nil? */
                frame_type_fix(t);

		runtime(m, l, r->name, r->sig, r->fn, t);
		r++;
		l++;
	}
	l->name = nil;
	return m;
}

void
retnstr(const char *s, int n, String **d)
{
	String *s1 = (String *)H;

	if(n != 0)
		s1 = c2string(s, n);
	ASSIGN(*d, s1);
}

void
retstr(const char *s, String **d)
{
	String *s1 = (String *)H;

	if(s != nil)
		s1 = c2string(s, strlen(s));
	ASSIGN(*d, s1);
}

Array*
mem2array(const void *va, int n)
{
	Heap *h;
	Array *a;

	if(n < 0)
		n = 0;
	h = nheap(sizeof(Array)+n); /* FIXME: make function createarray() */
	h->t = &Tarray;
	h->t->ref++;
	a = H2D(Array*, h);
	a->t = &Tbyte;
	Tbyte.ref++;
	a->len = n;
	a->root = (Array*)H;
	a->data = (char*)a+sizeof(Array);
	if(va != 0)
		memmove(a->data, va, n);

	return a;
}

static int
utfnleng(const char *s, int nb, int *ngood)
{
	int c;
	long n;
	Rune rune;
	const char *es, *starts;

	starts = s;
	es = s+nb;
	for(n = 0; s < es; n++) {
		c = *(uchar*)s;
		if(c < Runeself)
			s++;
		else {
			if(s+UTFmax<=es || fullrune(s, es-s))
				s += chartorune(&rune, s);
			else
				break;
		}
	}
	if(ngood)
		*ngood = s-starts;
	return n;
}
