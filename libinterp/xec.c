#include <lib9.h>
#include "isa.h"
#include "interp.h"
#include "raise.h"
#include "pool.h"

//#define DEBUGVM

REG	R = {0};			/* Virtual Machine registers */ /* BUG: WTF Global R? on multi-processor? */
String	snil = {0};			/* String known to be zero length */


#define OP(fn)	static void fn(void)


OP(runt) { }
OP(negf) { R.d->disreal = -R.s->disreal; }
OP(jmp)  { R.PC = R.d->pinst; }
OP(movpc){ R.d->pinst = R.M->prog + R.s->disint; }
OP(movm) { memmove(R.d, R.s, R.m->disint); /* TODO: add some assertions here */ }
OP(lea)  { R.d->pdisdata = R.s; }
OP(movb) { R.d->disbyte = R.s->disbyte; }
OP(movw) { R.d->disint = R.s->disint; }
OP(movf) { R.d->disreal = R.s->disreal; }
OP(movl) { R.d->disbig = R.s->disbig; }
OP(cvtbw){ R.d->disint = R.s->disbyte; }
OP(cvtwb){ R.d->disbyte = R.s->disint;	/* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtrf){ R.d->disreal = R.s->disreal32; }
OP(cvtfr){ R.d->disreal32 = R.s->disreal;/* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtws){ R.d->disint16 = R.s->disint; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtsw){ R.d->disint = R.s->disint16; }
OP(cvtwf){ R.d->disreal = R.s->disint;  /* TODO: optional runtime warning when the value is beyond target range */ }
OP(addb) { R.d->disbyte = R.m->disbyte + R.s->disbyte; }
OP(addw) { R.d->disint = R.m->disint + R.s->disint; }
OP(addl) { R.d->disbig = R.m->disbig + R.s->disbig; }
OP(addf) { R.d->disreal = R.m->disreal + R.s->disreal; }
OP(subb) { R.d->disbyte = R.m->disbyte - R.s->disbyte; }
OP(subw) { R.d->disint = R.m->disint - R.s->disint; }
OP(subl) { R.d->disbig = R.m->disbig - R.s->disbig; }
OP(subf) { R.d->disreal = R.m->disreal - R.s->disreal; }
OP(divb) { R.d->disbyte = R.m->disbyte / R.s->disbyte; }
OP(divw) { R.d->disint = R.m->disint / R.s->disint; }
OP(divl) { R.d->disbig = R.m->disbig / R.s->disbig; }
OP(divf) { R.d->disreal = R.m->disreal / R.s->disreal; }
OP(modb) { R.d->disbyte = R.m->disbyte % R.s->disbyte; }
OP(modw) { R.d->disint = R.m->disint % R.s->disint; }
OP(modl) { R.d->disbig = R.m->disbig % R.s->disbig; }
OP(mulb) { R.d->disbyte = R.m->disbyte * R.s->disbyte; }
OP(mulw) { R.d->disint = R.m->disint * R.s->disint; }
OP(mull) { R.d->disbig = R.m->disbig * R.s->disbig; }
OP(mulf) { R.d->disreal = R.m->disreal * R.s->disreal; }
OP(andb) { R.d->disbyte = R.m->disbyte & R.s->disbyte; }
OP(andw) { R.d->disint = R.m->disint & R.s->disint; }
OP(andl) { R.d->disbig = R.m->disbig & R.s->disbig; }
OP(xorb) { R.d->disbyte = R.m->disbyte ^ R.s->disbyte; }
OP(xorw) { R.d->disint = R.m->disint ^ R.s->disint; }
OP(xorl) { R.d->disbig = R.m->disbig ^ R.s->disbig; }
OP(orb)  { R.d->disbyte = R.m->disbyte | R.s->disbyte; }
OP(orw)  { R.d->disint = R.m->disint | R.s->disint; }
OP(orl)  { R.d->disbig = R.m->disbig | R.s->disbig; }
OP(shlb) { R.d->disbyte = R.m->disbyte << R.s->disint; }
OP(shlw) { R.d->disint = R.m->disint << R.s->disint; }
OP(shll) { R.d->disbig = R.m->disbig << R.s->disint; }
OP(shrb) { R.d->disbyte = R.m->disbyte >> R.s->disint; }
OP(shrw) { R.d->disint = R.m->disint >> R.s->disint; }
OP(shrl) { R.d->disbig = R.m->disbig >> R.s->disint; }
OP(lsrw) { R.d->disint = (DISUINT)R.m->disint >> R.s->disint; }
OP(lsrl) { R.d->disbig = (DISUBIG)R.m->disbig >> R.s->disint; }
OP(beqb) { if(R.s->disbyte == R.m->disbyte) R.PC = R.d->pinst; }
OP(bneb) { if(R.s->disbyte != R.m->disbyte) R.PC = R.d->pinst; }
OP(bltb) { if(R.s->disbyte <  R.m->disbyte) R.PC = R.d->pinst; }
OP(bleb) { if(R.s->disbyte <= R.m->disbyte) R.PC = R.d->pinst; }
OP(bgtb) { if(R.s->disbyte >  R.m->disbyte) R.PC = R.d->pinst; }
OP(bgeb) { if(R.s->disbyte >= R.m->disbyte) R.PC = R.d->pinst; }
OP(beqw) { if(R.s->disint == R.m->disint) R.PC = R.d->pinst; }
OP(bnew) { if(R.s->disint != R.m->disint) R.PC = R.d->pinst; }
OP(bltw) { if(R.s->disint <  R.m->disint) R.PC = R.d->pinst; }
OP(blew) { if(R.s->disint <= R.m->disint) R.PC = R.d->pinst; }
OP(bgtw) { if(R.s->disint >  R.m->disint) R.PC = R.d->pinst; }
OP(bgew) { if(R.s->disint >= R.m->disint) R.PC = R.d->pinst; }
OP(beql) { if(R.s->disbig == R.m->disbig) R.PC = R.d->pinst; }
OP(bnel) { if(R.s->disbig != R.m->disbig) R.PC = R.d->pinst; }
OP(bltl) { if(R.s->disbig <  R.m->disbig) R.PC = R.d->pinst; }
OP(blel) { if(R.s->disbig <= R.m->disbig) R.PC = R.d->pinst; }
OP(bgtl) { if(R.s->disbig >  R.m->disbig) R.PC = R.d->pinst; }
OP(bgel) { if(R.s->disbig >= R.m->disbig) R.PC = R.d->pinst; }
OP(beqf) { if(R.s->disreal == R.m->disreal) R.PC = R.d->pinst; }
OP(bnef) { if(R.s->disreal != R.m->disreal) R.PC = R.d->pinst; }
OP(bltf) { if(R.s->disreal <  R.m->disreal) R.PC = R.d->pinst; }
OP(blef) { if(R.s->disreal <= R.m->disreal) R.PC = R.d->pinst; }
OP(bgtf) { if(R.s->disreal >  R.m->disreal) R.PC = R.d->pinst; }
OP(bgef) { if(R.s->disreal >= R.m->disreal) R.PC = R.d->pinst; }
OP(beqc) { if(stringcmp(R.s->pstring, R.m->pstring) == 0) R.PC = R.d->pinst; }
OP(bnec) { if(stringcmp(R.s->pstring, R.m->pstring) != 0) R.PC = R.d->pinst; }
OP(bltc) { if(stringcmp(R.s->pstring, R.m->pstring) <  0) R.PC = R.d->pinst; }
OP(blec) { if(stringcmp(R.s->pstring, R.m->pstring) <= 0) R.PC = R.d->pinst; }
OP(bgtc) { if(stringcmp(R.s->pstring, R.m->pstring) >  0) R.PC = R.d->pinst; }
OP(bgec) { if(stringcmp(R.s->pstring, R.m->pstring) >= 0) R.PC = R.d->pinst; }
OP(iexit){ error(""); }
OP(cvtwl){ R.d->disbig = R.s->disint; }
OP(cvtlw){ R.d->disint = R.s->disbig; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtlf){ R.d->disreal = R.s->disbig; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtfl)
{
	const DISREAL f = R.s->disreal;

	R.d->disbig = f < 0 ? f - .5 : f + .5;
}
OP(cvtfw)
{
	const DISREAL f = R.s->disreal;

	R.d->disint = f < 0 ? f - .5 : f + .5;
}
OP(cvtcl)
{
	const String *s = R.s->pstring;

	if(s == H)
		R.d->disbig = 0;
	else
		R.d->disbig = strtoll(string2c(s), nil, 10);
}
OP(iexpw)
{
	int inv;
	DISINT x, n, r;

	x = R.m->disint;
	n = R.s->disint;
	inv = 0;
	if(n < 0){
		n = -n;
		inv = 1;
	}
	r = 1;
	for(;;){
		if(n&1)
			r *= x;
		if((n >>= 1) == 0)
			break;
		x *= x;
	}
	if(inv)
		r = 1/r;
	R.d->disint = r;
}
OP(iexpl)
{
	int inv;
	DISINT n;
	DISBIG x, r;

	x = R.m->disbig;
	n = R.s->disint;
	inv = 0;
	if(n < 0){
		n = -n;
		inv = 1;
	}
	r = 1;
	for(;;){
		if(n&1)
			r *= x;
		if((n >>= 1) == 0)
			break;
		x *= x;
	}
	if(inv)
		r = 1/r;
	R.d->disbig = r;
}
OP(iexpf)
{
	int inv;
	DISINT n;
	DISREAL x, r;

	x = R.m->disreal;
	n = R.s->disint;
	inv = 0;
	if(n < 0){
		n = -n;
		inv = 1;
	}
	r = 1;
	for(;;){
		if(n&1)
			r *= x;
		if((n >>= 1) == 0)
			break;
		x *= x;
	}
	if(inv)
		r = 1/r;
	R.d->disreal = r;
}
OP(indx)
{
	Array *a = R.s->parray;
	DISINT i = R.d->disint;

	if (a == H || i < 0 || i >= a->len)
		error(exBounds);

	R.m->pvoid = a->data + i*a->t->size;
}
OP(indw)
{
	Array *a = R.s->parray;
	DISINT i = R.d->disint;

	if (a == H || i < 0 || i >= a->len)
		error(exBounds);

	R.m->pvoid = a->data + i*sizeof(DISINT);
}
OP(indf)
{
	Array *a = R.s->parray;
	DISINT i = R.d->disint;

	if (a == H || i < 0 || i >= a->len)
		error(exBounds);

	R.m->pvoid = a->data + i*sizeof(DISREAL);
}
OP(indl)
{
	Array *a = R.s->parray;
	DISINT i = R.d->disint;

	if (a == H || i < 0 || i >= a->len)
		error(exBounds);

	R.m->pvoid = a->data + i*sizeof(DISBIG);
}
OP(indb)
{
	Array *a = R.s->parray;
	DISINT i = R.d->disint;

	if (a == H || i < 0 || i >= a->len)
		error(exBounds);

	R.m->pvoid = a->data + i*sizeof(DISBYTE);
}
OP(movp)
{
	void *sv = R.s->pvoid;

	if(sv != H) {
		ADDREF(sv);
		Setmark(D2H(sv));
	}
	destroy(R.d->pvoid); /* FIXME: atomic xchg */
	R.d->pvoid = sv;
}
OP(movmp)
{
	Type *t = R.M->type[R.m->disint];  /* TODO: check index range */

	incmem(R.s, t);
	freeptrs(R.d, t);
	memcpy(R.d, R.s, t->size);
}
OP(new)
{
	destroy(R.d->pvoid);  /* FIXME: atomic xchg */
	R.d->pvoid = H2D(void*, heap(R.M->type[R.s->disint]));  /* TODO: check index range */
}
OP(newz)
{
	destroy(R.d->pvoid); /* FIXME: atomic xchg */
	R.d->pvoid = H2D(void*, heapz(R.M->type[R.s->disint]));  /* TODO: check index range */
}
OP(mnewz)
{
	Modlink *ml = R.s->pmodlink;

	if(ml == H)
		error(exModule);

	destroy(R.d->pvoid); /* FIXME: atomic xchg */
	R.d->pvoid = H2D(void*, heapz(ml->type[R.m->disint]));  /* TODO: check index range */
}
OP(frame) /* == newz */
{
	/*destroy(R.d->pframe); /* ??*/
	R.d->pframe = H2D(Frame*, heapz(R.M->type[R.s->disint]));  /* TODO: check index range */
}
OP(mframe)
{
	Type *t;
	Modlink *ml = R.s->pmodlink;
	int o = R.m->disint;

	if(ml == H)
		error(exModule);

	if (o >= 0) {
		if(o >= ml->nlinks)
			error("invalid mframe");
		t = ml->links[o].frame;
	} else
		t = ml->m->ext[-o-1].frame;   /* TODO: check index range */

	R.d->pframe = H2D(Frame*,heapz(t));
}
void
acheck(int tsz, int sz)
{
	if(sz < 0)
		error(exNegsize);
	/* test for overflow; assumes sz >>> tsz */
	if((int)(sizeof(Array) + sizeof(Heap) + tsz*sz) < sz && tsz != 0)
		error(exHeap);
}
OP(newa)
{
	Type * const t = R.M->type[R.m->disint];  /* TODO: check index range */
	const int sz = R.s->disint;
	Heap *h;
	Array *a;

	acheck(t->size, sz);
	h = nheap(sizeof(Array) + (t->size*sz));
	h->t = &Tarray;
	Tarray.ref++; /* meaningless? */
	a = H2D(Array*, h);
	a->t = t;
	a->len = sz;
	a->root = H;
	a->data = (char*)(a+1);
	initarray(t, a);

	destroy(R.d->parray); /* FIXME: atomic xchg */
	R.d->parray = a;
}
OP(newaz)
{
	Type * const t = R.M->type[R.m->disint];  /* TODO: check index range */
	const int sz = R.s->disint;
	Heap *h;
	Array *a;

	acheck(t->size, sz);
	h = nheap(sizeof(Array) + (t->size*sz));
	h->t = &Tarray;
	Tarray.ref++; /* meaningless? */
	a = H2D(Array*, h);
	a->t = t;
	a->len = sz;
	a->root = H;
	a->data = (char*)(a+1);
	memset(a->data, 0, t->size*sz);
	initarray(t, a);

	destroy(R.d->parray); /* FIXME: atomic xchg */
	R.d->parray = a;
}
Channel*
cnewc(Type *t, void (*mover)(void*d, void*s, Channel*c), int len)
{
	Heap *h = heap(&Tchannel);
	Channel *c = H2D(Channel*, h);

	c->send = (Progq*)malloc(sizeof(Progq));
	c->recv = (Progq*)malloc(sizeof(Progq));
	if(c->send == nil || c->recv == nil){
		free(c->send);
		free(c->recv);
		error(exNomem);
	}
	c->send->prog = c->recv->prog = nil;
	c->send->next = c->recv->next = nil;
	c->mover = mover;
	c->buf = H;
	if(len > 0)
		c->buf = H2D(Array*, heaparray(t, len));
	c->front = 0;
	c->size = 0;
	if(mover == movertmp){
		c->mid.t = t;
		t->ref++;
	}
	return c;
}

static Channel*
newc(Type *t, void (*mover)(void*d, void*s, Channel*c))
{
	DISINT len = 0;

	if(R.m != R.d){
		len = R.m->disint;
		if(len < 0)
			error(exNegsize);
	}

	destroy(R.d->pchannel); /* FIXME: atomic xchg */
	return R.d->pchannel = cnewc(t, mover, len);
}

static void moverb(void*d, void*s, Channel*c) { *(DISBYTE*)d = *(DISBYTE*)s; }
static void moverw(void*d, void*s, Channel*c) { *(DISINT *)d = *(DISINT *)s; }
static void moverf(void*d, void*s, Channel*c) { *(DISREAL*)d = *(DISREAL*)s; }
static void moverl(void*d, void*s, Channel*c) { *(DISBIG *)d = *(DISBIG *)s; }
void moverp(void*d, void*s, Channel*c)
{
	void *sv = *(void**)s;

	if(sv != H) {
		ADDREF(sv);
		Setmark(D2H(sv));
	}
	destroy(*(void**)d); /* FIXME: atomic xchg */
	*(void**)d = sv;
}
void
movertmp(void*d, void*s, Channel*c)		/* Used by send & receive */
{
	Type* t = c->mid.t;
	incmem(s, t);
	freeptrs(d, t);
	memcpy(d, s, t->size);
}
static void
moverm(void*d, void*s, Channel*c)		/* Used by send & receive */
{
	memcpy(d, s, c->mid.w);
}


OP(newcl)  { newc(&Tlong, moverl);  }
OP(newcb)  { newc(&Tbyte, moverb);  }
OP(newcw)  { newc(&Tword, moverw);  }
OP(newcf)  { newc(&Treal, moverf);  }
OP(newcp)  { newc(&Tptr, moverp);  }
OP(newcm)
{
	Channel *c;
	Type *t;

	t = nil;
	if(R.m != R.d && R.s->disint > 0)
		t = dtype(nil, R.s->disint, nil, 0, "(newcm)");
	c = newc(t, moverm);
	c->mid.w = R.s->disint;
	if(t != nil)
		freetype(t);
}
OP(newcmp)
{
	newc(R.M->type[R.s->disint], movertmp);  /* TODO: check index range */
}
OP(icase)
{
	DISINT *t, *l, d, n, n2;
	const DISINT v = R.s->disint;

	t = R.d->disints + 1; /* FIXME: declare struct */
	n = t[-1];
	d = t[n*3];

	while(n > 0) {
		n2 = n >> 1;
		l = t + n2*3;
		if(v < l[0]) {
			n = n2;
			continue;
		}
		if(v >= l[1]) {
			t = l+3;
			n -= n2 + 1;
			continue;
		}
		d = l[2];
		break;
	}
	if(R.M->compiled) {
		R.PC = (Inst*)d;
		return;
	}
	R.PC = R.M->prog + d;
}
OP(casel)
{
	DISINT *t, *l, d, n, n2;
	const DISBIG v = R.s->disbig;

	t = R.d->disints + 2; /* FIXME: declare struct */
	n = t[-2];
	d = t[n*6];

	while(n > 0) {
		n2 = n >> 1;
		l = t + n2*6;
		if(v < ((DISBIG*)l)[0]) {
			n = n2;
			continue;
		}
		if(v >= ((DISBIG*)l)[1]) {
			t = l+6;
			n -= n2 + 1;
			continue;
		}
		d = l[4];
		break;
	}
	if(R.M->compiled) {
		R.PC = (Inst*)d;
		return;
	}
	R.PC = R.M->prog + d;
}
OP(casec)
{
	DISINT *l, *t, *e, n, n2, r;
	String *sl, *sh;
	const String * const sv = R.s->pstring;

	t = R.d->disints + 1; /* FIXME: declare struct */
	n = t[-1];
	e = t + n*3;
	if(n > 2){
		while(n > 0){
			n2 = n>>1;
			l = t + n2*3;
			sl = (String*)l[0];
			r = stringcmp(sv, sl);
			if(r == 0){
				e = l + 2;
				break;
			}
			if(r < 0){
				n = n2;
				continue;
			}
			sh = (String*)l[1];
			if(sh == H || stringcmp(sv, sh) > 0){
				t = l+3;
				n -= n2+1;
				continue;
			}
			e = l + 2;
			break;
		}
		t = e;
	}
	else{
		while(t < e) {
			sl = (String*)t[0];
			sh = (String*)t[1];
			if(sh == H) {
				if(stringcmp(sl, sv) == 0) {
					t += 2;
					goto found;
				}
			}
			else
			if(stringcmp(sl, sv) <= 0 && stringcmp(sh, sv) >= 0) {
				t += 2;
				goto found;
			}
			t += 3;
		}
	}
found:
	if(R.M->compiled) {
		R.PC = (Inst*)t[0];
		return;
	}
	R.PC = R.M->prog + t[0];
}
OP(igoto)
{
	DISINT* t = R.d->disints + R.s->disint;
	if(R.M->compiled) {
		R.PC = (Inst*)t[0]; /* FIXME: check index and new PC */
	}
	else {
		R.PC = R.M->prog + t[0]; /* FIXME: check index and new PC */
	}
}
OP(call)
{
	Frame *f = R.s->pframe;

	f->lr = R.PC;
	f->fp = R.FP;
	R.FP = f;
	R.PC = R.d->pinst;
}
OP(spawn)
{
	Prog *p = newprog(currun(), R.M);

	p->R.PC = R.d->pinst;
	p->R.FP = R.s->pframe;
}
OP(mspawn)
{
	Prog *p;
	Modlink *ml = R.d->pmodlink;
	int o;

	if(ml == H)
		error(exModule);
	if(ml->prog == nil)
		error(exSpawn);
	p = newprog(currun(), ml);
	o = R.m->disint;
	if(o >= 0)
		p->R.PC = ml->links[o].u.pc;   /* TODO: check index range */
	else
		p->R.PC = ml->m->ext[-o-1].u.pc;   /* TODO: check index range */
	p->R.FP = R.s->pframe;
}
OP(ret)
{
	Frame *f = R.FP;
	Modlink *m;

	R.FP = f->fp;
	if(R.FP == nil) {
		R.FP = f;
		error(""); /* 'stack' underflow */
	}
	R.PC = f->lr;
	m = f->mr;

	//? destroy(f)
	// FIXME: huh, what if the return value is array or adt ?
	assert(D2H(f)->t != nil);
	freeptrs(f, D2H(f)->t);

	/* return from mcall */
	if(m != nil) {
		if(R.M->compiled != m->compiled) {
			R.IC = 1;
			/*R.t = 1; write only*/
		}
		destroy(R.M); /* FIXME: atomic xchg */
		R.M = m;
		R.MP = m->MP;
	}
}
OP(iload)
{
	char *n = string2c(R.s->pstring);
	Module *m = R.M->m;
	Import *ldt;
	Modlink *ml;

	if(!(m->rt & HASLDT)) {
		error("obsolete dis");
	}
	ldt = m->ldt[R.m->disint]; /* TODO: check index range */

	if(strcmp(n, "$self") == 0) {
		m->ref++;
		ml = linkmod(m, ldt, 0);
		if(ml != H) {
			ml->MP = R.M->MP;
			ADDREF(ml->MP);
		}
	}
	else {
		m = readmod(n, lookmod(n), 1);
		ml = linkmod(m, ldt, 1);
	}

	destroy(R.d->pmodlink); /* FIXME: atomic xchg */
	R.d->pmodlink = ml;
}
OP(mcall)
{
	Prog *p;
	Frame *f = R.s->pframe;
	Linkpc *l;
	Modlink *ml = R.d->pmodlink;
	int o;

	if(ml == H)
		error(exModule);

	f->lr = R.PC;
	f->fp = R.FP;
	f->mr = R.M;

	R.FP = f;
	R.M = ml;
	ADDREF(ml);

	o = R.m->disint;
	if(o >= 0)
		l = &ml->links[o].u;   /* TODO: check index range */
	else
		l = &ml->m->ext[-o-1].u;   /* TODO: check index range */
	if(ml->prog == nil) {
		l->runt(f);
		D2H(ml)->ref--;
		R.M = f->mr;
		R.FP = f->fp;

		//? destroy(f)
		assert(D2H(f)->t != nil);
		freeptrs(f, D2H(f)->t);

		p = currun();
		if(p->kill != nil)
			error(p->kill);
		/*R.t = 0; write only*/
		return;
	}
	R.MP = R.M->MP;
	R.PC = l->pc;
	/*R.t = 1; write only*/

	if(f->mr->compiled != R.M->compiled)
		R.IC = 1;
}
OP(lena)
{
	Array *a = R.s->parray;
	DISINT l = 0;

	if(a != H)
		l = a->len;
	R.d->disint = l;
}
OP(lenl)
{
	List *a = R.s->plist;
	DISINT l = 0;

	while(a != H) {
		l++;
		a = a->tail;
	}
	R.d->disint = l;
}
static int
cgetb(Channel *c, void *v)
{
	Array *a = c->buf;
	void *w;

	if(a == H)
		return 0;
	if(c->size > 0){
		w = a->data + c->front*a->t->size;
		c->front++;
		if(c->front == c->buf->len)
			c->front = 0;
		c->size--;
		c->mover(v, w, c);
		freeptrs(w, a->t);
		initmem(a->t, w);
		return 1;
	}
	return 0;
}
static int
cputb(Channel *c, void *v)
{
	Array *a = c->buf;
	DISINT len, r;

	if(a == H)
		return 0;
	len = c->buf->len;
	if(c->size < len){
		r = c->front+c->size;
		if(r >= len)
			r -= len;
		c->size++;

		c->mover(a->data + r * a->t->size, v, c);
		return 1;
	}
	return 0;
}
/*
int
cqsize(Progq *q)
{
	int n;

	n = 0;
	for( ; q != nil; q = q->next)
		if(q->prog != nil)
			n++;
	return n;
}
*/

void
cqadd(Progq **q, Prog *p)
{
	Progq *n;

	if((*q)->prog == nil){
		(*q)->prog = p;
		return;
	}
	n = (Progq*)malloc(sizeof(Progq));
	if(n == nil)
		error(exNomem);
	n->prog = p;
	n->next = nil;
	for( ; *q != nil; q = &(*q)->next)
		;
	*q = n;
}
void
cqdel(Progq **q)
{
	Progq *f;

	if((*q)->next == nil){
		(*q)->prog = nil;
		return;
	}
	f = *q;
	*q = f->next;
	free(f);
}
void
cqdelp(Progq **q, Prog *p)
{
	Progq *f;

	if((*q)->next == nil){
		if((*q)->prog == p)
			(*q)->prog = nil;
		return;
	}
	for( ; *q != nil; ){
		if((*q)->prog == p){
			f = *q;
			*q = (*q)->next;
			free(f);
		}
		else
			q = &(*q)->next;
	}
}

int _isend(Channel *c, void *v)
{
 	Prog *p = c->recv->prog;

	if(p == nil) {
		if(c->buf != H && cputb(c, v))
			return 1;
		p = delrun(Psend);
		p->ptr = v;
		p->chan = c;	/* for killprog */
		cqadd(&c->send, p);
		return 1;
	}

	if(c->buf != H && c->size > 0)
		print("non-empty buffer in isend\n");

	cqdel(&c->recv);
	if(p->state == Palt)
		/*altdone(&p->R.s->alt, p, c, 1);*/
		altdone(p->aaa, p, c, 1);

	c->mover(p->ptr, v, c);
	p->ptr = nil;
	addrun(p);
	return 0;
}
OP(isend)
{
	Channel *c = R.d->pchannel;

	if(c == H)
		error(exNilref);
	if(_isend(c, R.s))
		R.IC = 1;
}
int _irecv(Channel *c, void* v)
{
	Prog *p = c->send->prog;

	if(p == nil) {
		if(c->buf != H && cgetb(c, v))
			return 0;
		p = delrun(Precv);
		p->ptr = v;
		p->chan = c;	/* for killprog */
		cqadd(&c->recv, p);
		return 1;
	}

	if(c->buf != H && c->size != c->buf->len)
		print("non-full buffer in irecv\n");

	cqdel(&c->send);
	if(p->state == Palt)
		/*altdone(&p->R.s->alt, p, c, 0);*/
		altdone(p->aaa, p, c, 0);

	if(c->buf != H){
		cgetb(c, v);
		cputb(c, p->ptr);
		p->ptr = nil;
	}
	else{
		c->mover(v, p->ptr, c);
		p->ptr = nil;
	}
	addrun(p);
	return 0;
}
OP(irecv)
{
	Channel *c = R.s->pchannel;

	if(c == H)
		error(exNilref);
	if(_irecv(c, R.d))
		R.IC = 1;
}

int
csendalt(Channel *c, void *ip, Type *t, int len)
{
	if(c == H)
		error(exNilref);

	if(c->recv->prog == nil && (c->buf == H || c->size == c->buf->len)){
		if(c->buf != H){
			print("csendalt failed\n");
			freeptrs(ip, t);
			return 0;
		}
		c->buf = H2D(Array*, heaparray(t, len));
	}
	if(_isend(c, ip))
		R.IC = 1;
	freeptrs(ip, t);
	return 1;
}

List*
cons(ulong size, List **lp)
{
	Heap *h = nheap(offsetof(List,data) + size);
	List *lv, *l;

	h->t = &Tlist;
	Tlist.ref++;
	l = H2D(List*, h);
	l->t = nil;

	lv = *lp;
	if(lv != H) {
		h = D2H(lv);
		Setmark(h);
	}
	l->tail = lv;
	*lp = l;
	return l;
}
OP(consb)
{
	List *l = cons(sizeof(DISBYTE), &R.d->plist);

	l->data.disbyte = R.s->disbyte;
}
OP(consw)
{
	List *l = cons(sizeof(DISINT), &R.d->plist);

	l->data.disint = R.s->disint;
}
OP(consl)
{
	List *l = cons(sizeof(DISBIG), &R.d->plist);

	l->data.disbig = R.s->disbig;
}
OP(consp)
{
	List *l = cons(sizeof(void*), &R.d->plist);
	void *sv = R.s->pvoid;

	if(sv != H) {
		ADDREF(sv);
		Setmark(D2H(sv));
	}
	l->t = &Tptr;
	Tptr.ref++;
	l->data.pvoid = sv;
}
OP(consf)
{
	List *l;

	l = cons(sizeof(DISREAL), &R.d->plist);
	l->data.disreal = R.s->disreal;
}
OP(consm)
{
	DISINT v = R.m->disint;
	List *l = cons(v, &R.d->plist);
	memcpy(&l->data, R.s, v);
}
OP(consmp)
{
	Type *t = R.M->type[R.m->disint];  /* TODO: check index range */
	List *l = cons(t->size, &R.d->plist);

	incmem(R.s, t);
	memcpy(&l->data, R.s, t->size);
	l->t = t;
	t->ref++;
}
OP(headb)
{
	List *l = R.s->plist;

	if(l == H)
		error(exNilref);
	R.d->disbyte = l->data.disbyte;
}
OP(headw)
{
	List *l = R.s->plist;

	if(l == H)
		error(exNilref);
	R.d->disint = l->data.disint;
}
OP(headl)
{
	List *l = R.s->plist;

	if(l == H)
		error(exNilref);
	R.d->disbig = l->data.disbig;
}
OP(headp)
{
	List *l = R.s->plist;
	void *sv;

	if(l == H)
		error(exNilref);

	//was R.s = &l->data; movp();
	sv = l->data.pvoid;
	if(sv != H) {
		ADDREF(sv);
		Setmark(D2H(sv));
	}
	destroy(R.d->pvoid); /* FIXME: atomic xchg */
	R.d->pvoid = sv;
}
OP(headf)
{
	List *l = R.s->plist;

	if(l == H)
		error(exNilref);
	R.d->disreal = l->data.disreal;
}
OP(headm)
{
	List *l = R.s->plist;

	if(l == H)
		error(exNilref);
	memcpy(R.d, &l->data, R.m->disint);
}
OP(headmp)
{
	List *l = R.s->plist;
	Type *t;

	if(l == H)
		error(exNilref);

	//was R.s = &l->data; movmp();
	t = R.M->type[R.m->disint];  /* TODO: check index range */
	incmem(&l->data, t);
	freeptrs(R.d, t);
	memcpy(R.d, &l->data, t->size);

}
OP(tail)
{
	List *l = R.s->plist;
	List *sv;

	if(l == H)
		error(exNilref);

	//was R.s = (Disdata*) &l->tail; movp();
	sv = l->tail;
	if(sv != H) {
		ADDREF(sv);
		Setmark(D2H(sv));
	}
	destroy(R.d->plist); /* FIXME: atomic xchg */
	R.d->plist = sv;
}
OP(slicea)
{
	Type *t;
	Heap *h;
	Array *ss, *ds;
	int v, n, start;

	v = R.m->disint;
	start = R.s->disint;
	n = v - start;
	ds = R.d->parray;

	if(ds == H) {
		if(n == 0)
			return;
		error(exNilref);
	}
	if(n < 0 || (ulong)start > ds->len || (ulong)v > ds->len)
		error(exBounds);

	t = ds->t;
	h = heap(&Tarray);
	ss = H2D(Array*, h);
	ss->len = n;
	ss->data = ds->data + start*t->size;
	ss->t = t;
	t->ref++;

	if(ds->root != H) {			/* slicing a slice */
		ds = ds->root;
		ADDREF(ds);
		destroy(R.d->parray); /* FIXME: atomic xchg */
		R.d->parray = ss;
		ss->root = ds;
	}
	else {
		ss->root = ds;
		R.d->parray = ss;
	}
	Setmark(D2H(ds));
}
OP(slicela)
{
	Type *t;
	int l, dl;
	Array *ss, *ds;
	uchar *sp, *dp, *ep;

	ss = R.s->parray;
	dl = R.m->disint;
	ds = R.d->parray;
	if(ss == H)
		return;
	if(ds == H)
		error(exNilref);
	if(dl < 0 || dl+ss->len > ds->len)
		error(exBounds);

	t = ds->t;
	if(t->np == 0) {
		memmove(ds->data+dl*t->size, ss->data, ss->len*t->size);
		return;
	}
	sp = ss->data;
	dp = ds->data+dl*t->size;

	if(dp > sp) {
		l = ss->len * t->size;
		sp = ss->data + l;
		ep = dp + l;
		while(ep > dp) {
			ep -= t->size;
			sp -= t->size;
			incmem(sp, t);
			freeptrs(ep, t);
		}
	}
	else {
		ep = dp + ss->len*t->size;
		while(dp < ep) {
			incmem(sp, t);
			freeptrs(dp, t);
			dp += t->size;
			sp += t->size;
		}
	}
	memmove(ds->data+dl*t->size, ss->data, ss->len*t->size);
}
OP(alt)
{
	/*R.t = 0; write only*/
	if(xecalt(1, &R.s->alt, &R.d->disint))
		R.IC = 1;
}
OP(nbalt)
{
	if(xecalt(0, &R.s->alt, &R.d->disint))
		R.IC = 1;
}
OP(tcmp)
{
	const void * const s = R.s->pvoid;
	const void * const d = R.d->pvoid;

	if(s != H && (d == H || D2H(s)->t != D2H(d)->t)) /* wow */
		error(exTcheck);
}
OP(eclr)
{
	/* spare slot */
}
OP(badop)
{
	error(exOp);
}
OP(iraise)
{
	void *v;
	Prog *p;


	p = currun();
	v = R.s->pvoid;
	if(v == H)
		error(exNilref);
	p->exval = v;
	ADDREF(v);
	if(D2H(v)->t == &Tstring)
		error(string2c((String*)v));
	else
		error(string2c(*(String**)v)); /* FIXME: OMG */
}
OP(mulx)
{
	DISINT p;
	DISBIG r;

	p = R.FP->dtmp;
	r = (DISBIG)R.m->disint*(DISBIG)R.s->disint;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	R.d->disint = (DISINT)r;
}
OP(divx)
{
	DISINT p;
	DISBIG s;

	p = R.FP->dtmp;
	s = (DISBIG)R.m->disint;
	if(p >= 0)
		s <<= p;
	else
		s >>= (-p);
	s /= (DISBIG)R.s->disint;
	R.d->disint = (DISINT)s;
}
OP(cvtxx)
{
	DISINT p;
	DISBIG r;

	p = R.m->disint;
	r = (DISBIG)R.s->disint;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	R.d->disint = (DISINT)r;
}
OP(mulx0)
{
	DISINT x, y, p, a;
	DISBIG r;

	x = R.m->disint;
	y = R.s->disint;
	p = R.FP->dtmp;
	a = R.FP->stmp;
	if(x == 0 || y == 0){
		R.d->disint = 0;
		return;
	}
	r = (DISBIG)x*(DISBIG)y;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	r /= (DISBIG)a;
	R.d->disint = (DISINT)r;
}
OP(divx0)
{
	DISINT x, y, p, b;
	DISBIG s;

	x = R.m->disint;
	y = R.s->disint;
	p = R.FP->dtmp;
	b = R.FP->stmp;
	if(x == 0){
		R.d->disint = 0;
		return;
	}
	s = (DISBIG)b*(DISBIG)x;
	if(p >= 0)
		s <<= p;
	else
		s >>= (-p);
	s /= (DISBIG)y;
	R.d->disint = (DISINT)s;
}
OP(cvtxx0)
{
	DISINT x, p, a;
	DISBIG r;

	x = R.s->disint;
	p = R.m->disint;
	a = R.FP->stmp;
	if(x == 0){
		R.d->disint = 0;
		return;
	}
	r = (DISBIG)x;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	r /= (DISBIG)a;
	R.d->disint = (DISINT)r;
}
OP(mulx1)
{
	DISINT x, y, p, a, v;
	int vnz, wnz;
	DISBIG w, r;

	x = R.m->disint;
	y = R.s->disint;
	p = R.FP->dtmp;
	a = R.FP->stmp;
	if(x == 0 || y == 0){
		R.d->disint = 0;
		return;
	}
	vnz = p&2;
	wnz = p&1;
	p >>= 2;
	v = 0;
	w = 0;
	if(vnz){
		v = a-1;
		if(x >= 0 && y < 0 || x < 0 && y >= 0)
			v = -v;
	}
	if(wnz){
		if((!vnz && (x > 0 && y < 0 || x < 0 && y > 0)) ||
		(vnz && (x > 0 && y > 0 || x < 0 && y < 0)))
			w = ((DISBIG)1<<(-p)) - 1;
	}
	r = (DISBIG)x*(DISBIG)y + w;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	r += (DISBIG)v;
	r /= (DISBIG)a;
	R.d->disint = (DISINT)r;
}
OP(divx1)
{
	DISINT x, y, p, b, v;
	int vnz, wnz;
	DISBIG w, s;

	x = R.m->disint;
	y = R.s->disint;
	p = R.FP->dtmp;
	b = R.FP->stmp;
	if(x == 0){
		R.d->disint = 0;
		return;
	}
	vnz = p&2;
	wnz = p&1;
	p >>= 2;
	v = 0;
	w = 0;
	if(vnz){
		v = 1;
		if(x >= 0 && y < 0 || x < 0 && y >= 0)
			v = -v;
	}
	if(wnz){
		if(x <= 0)
			w = ((DISBIG)1<<(-p)) - 1;
	}
	s = (DISBIG)b*(DISBIG)x + w;
	if(p >= 0)
		s <<= p;
	else
		s >>= (-p);
	s /= (DISBIG)y;
	R.d->disint = (DISINT)s + v;
}
OP(cvtxx1)
{
	DISINT x, p, a, v;
	int vnz, wnz;
	DISBIG w, r;

	x = R.s->disint;
	p = R.m->disint;
	a = R.FP->stmp;
	if(x == 0){
		R.d->disint = 0;
		return;
	}
	vnz = p&2;
	wnz = p&1;
	p >>= 2;
	v = 0;
	w = 0;
	if(vnz){
		v = a-1;
		if(x < 0)
			v = -v;
	}
	if(wnz){
		if(!vnz && x < 0 || vnz && x > 0)
			w = ((DISBIG)1<<(-p)) - 1;
	}
	r = (DISBIG)x + w;
	if(p >= 0)
		r <<= p;
	else
		r >>= (-p);
	r += (DISBIG)v;
	r /= (DISBIG)a;
	R.d->disint = (DISINT)r;
}
/*
OP(cvtxx)
{
	REAL v;

	v = (REAL)R.s->disint*R.m->disreal;
	v = v < 0 ? v-0.5: v+0.5;
	R.d->disint = (WORD)v;
}
*/
OP(cvtfx)
{
	DISREAL v;

	v = R.s->disreal*R.m->disreal;
	v = v < 0 ? v-0.5: v+0.5;
	R.d->disint = (DISINT)v;
}
OP(cvtxf)
{
	R.d->disreal = (DISREAL)R.s->disint*R.m->disreal;
}

OP(self)
{
	Modlink *ml = R.M;

	ADDREF(ml);

	destroy(R.d->pmodlink); /* FIXME: atomic xchg */
	R.d->pmodlink = ml;
}

void
destroystack(REG *reg)
{
	/* BUG: free frames as well */
	/* TODO mark Frame::mp and Frame::fp in Type::map */
	Frame *f;
	Type* t;
	print("destroystack begin:\n");
	for(f = reg->FP; f != nil; f = f->fp)
	{
		assert(D2H(f)->t!=nil);
		assert(D2H(f)->t->destructor == &freeheap);

		print("Frame:\t");PRINT_TYPE(D2H(f)->t); print("\n");
		/*{Type*t=D2H(f)->t; int i;
		print("<%02X:", t->np); for(i=0; i<t->np; i++) print("%02X", t->map[i]);
		print(">\n");
		}*/

		freeptrs(f, D2H(f)->t);
		if(f->mr != nil) {
			destroy(reg->M); /* FIXME: atomic xchg */
			reg->M = f->mr;
		}
	}
	destroy(reg->M); /* FIXME: atomic xchg */
	reg->M = H;	/* for devprof */
	print(":destroystack end\n");
}
Prog*
isave(void)
{
	Prog *p;

	p = delrun(Prelease);
	p->R = R;
	return p;
}

void
irestore(Prog *p)
{
	R = p->R;
	R.IC = 1;
}



extern void cvtca(void);
extern void cvtac(void);
extern void cvtwc(void);
extern void cvtcw(void);
extern void cvtfc(void);
extern void cvtcf(void);
extern void insc(void);
extern void indc(void);
extern void addc(void);
extern void lenc(void);
extern void slicec(void);
extern void cvtlc(void);

#include "optab.h"

void
opinit(void)
{
	int i;

	for(i = 0; i < 256; i++)
		if(optab[i] == nil)
			optab[i] = badop;
}

#ifdef DEBUGVM
// print op operand state before executing
void statebefore(char* o, int n, uchar op, Inst* modprog)
{
#define CURM (R.M->m->name)
#define I(r) (R.r->pinst - modprog)
#define	Alen(r)	(R.r->parray==H?0:R.r->parray->len)

	o[0] = '\0';
	switch(op)
	{
	case IEXIT:
	case IECLR:
	case ISELF:
	case INOP:
	case IRUNT:
	case IRET:	break;
	case ICVTBW:   	// monadic ops
	case IMOVB: 	snprint(o,n,"%d", R.s->disbyte); break;
	case ICVTSW:	snprint(o,n,"%d", R.s->disint16); break;
	case ICVTWB:
	case ICVTWS:
	case ICVTWL:
	case ICVTWF:
	case ICVTWC:
	case IMOVW: 	snprint(o,n,"%d", R.s->disint); break;
	case ICVTLF:
	case ICVTLW:
	case ICVTLC:
	case IMOVL: 	snprint(o,n,"0x%llX", R.s->disbig); break;
	case ICVTCL:
	case ICVTCA:
	case ICVTCW:
	case ICVTCF:
	case ILENC: 	snprint(o,n,"\"%s\"", string2c(R.s->pstring)); break;
	case ICVTFR:
	case ICVTFW:
	case ICVTFL:
	case ICVTFC:
	case INEGF:
	case IMOVF: 	snprint(o,n,"%f", R.s->disreal); break;
	case ICVTRF:	snprint(o,n,"%f", R.s->disreal32); break;
	case ICVTAC:
	case ILENA: 	snprint(o,n,"Array[%d]@%p", Alen(s), R.s->parray); break;
	case ILEA:	snprint(o,n,"%p", R.s); break;
	case IALT:	/* tbd: show structure bihing the pointer */
	case INBALT:	/* tbd: show structure bihing the pointer */
	case IRAISE:    /* tbd: show structure bihing the pointer */
	case IMOVP: 	snprint(o,n,"%p", R.s->pvoid); break;
	case IFRAME:	snprint(o,n,"Type_%s_%d", CURM, R.s->disint); break;
	case IBEQB:	snprint(o,n,"if(%d == %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;  // diadic and triadic
	case IBNEB:	snprint(o,n,"if(%d != %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;
	case IBLTB:	snprint(o,n,"if(%d <  %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;
	case IBLEB:	snprint(o,n,"if(%d <= %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;
	case IBGTB:	snprint(o,n,"if(%d >  %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;
	case IBGEB:	snprint(o,n,"if(%d >= %d) jmp %s_%uX", R.s->disbyte, R.m->disbyte, CURM, I(d)); break;
	case IBEQW:	snprint(o,n,"if(%d == %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBNEW:	snprint(o,n,"if(%d != %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBLTW:	snprint(o,n,"if(%d <  %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBLEW:	snprint(o,n,"if(%d <= %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBGTW:	snprint(o,n,"if(%d >  %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBGEW:	snprint(o,n,"if(%d >= %d) jmp %s_%uX", R.s->disint, R.m->disint, CURM, I(d)); break;
	case IBEQL:	snprint(o,n,"if(0x%llX == 0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBNEL:	snprint(o,n,"if(0x%llX != 0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBLTL:	snprint(o,n,"if(0x%llX <  0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBLEL:	snprint(o,n,"if(0x%llX <= 0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBGTL:	snprint(o,n,"if(0x%llX >  0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBGEL:	snprint(o,n,"if(0x%llX >= 0x%llX) jmp %s_%uX", R.s->disbig, R.m->disbig, CURM, I(d)); break;
	case IBEQF:	snprint(o,n,"if(%f == %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBNEF:	snprint(o,n,"if(%f != %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBLTF:	snprint(o,n,"if(%f <  %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBLEF:	snprint(o,n,"if(%f <= %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBGTF:	snprint(o,n,"if(%f >  %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBGEF:	snprint(o,n,"if(%f >= %f) jmp %s_%uX", R.s->disreal, R.m->disreal, CURM, I(d)); break;
	case IBEQC:	snprint(o,n,"if(\"%s\" == \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IBNEC:	snprint(o,n,"if(\"%s\" != \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IBLTC:	snprint(o,n,"if(\"%s\" <  \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IBLEC:	snprint(o,n,"if(\"%s\" <= \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IBGTC:	snprint(o,n,"if(\"%s\" >  \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IBGEC:	snprint(o,n,"if(\"%s\" >= \"%s\") jmp %s_%uX", string2c(R.s->pstring), string2c(R.m->pstring), CURM, I(d)); break;
	case IADDB:	snprint(o,n,"%d + %d", R.m->disbyte, R.s->disbyte); break;
	case IADDW:	snprint(o,n,"%d + %d", R.m->disint, R.s->disint); break;
	case IADDL:	snprint(o,n,"0x%llX + 0x%llX", R.m->disbig, R.s->disbig); break;
	case IADDF:	snprint(o,n,"%f + %f", R.m->disreal, R.s->disreal); break;
	case IADDC: 	snprint(o,n,"\"%s\" + \"%s\"", string2c(R.m->pstring), string2c(R.s->pstring)); break;
	case ISUBB:	snprint(o,n,"%d - %d", R.m->disbyte, R.s->disbyte); break;
	case ISUBW:	snprint(o,n,"%d - %d", R.m->disint, R.s->disint); break;
	case ISUBL:	snprint(o,n,"0x%llX - 0x%llX", R.m->disbig, R.s->disbig); break;
	case ISUBF:	snprint(o,n,"%f - %f", R.m->disreal, R.s->disreal); break;
	case IMULB:	snprint(o,n,"%d * %d", R.m->disbyte, R.s->disbyte); break;
	case IMULW:	snprint(o,n,"%d * %d", R.m->disint, R.s->disint); break;
	case IMULL:	snprint(o,n,"0x%llX * 0x%llX", R.m->disbig, R.s->disbig); break;
	case IMULF:	snprint(o,n,"%f * %f", R.m->disreal, R.s->disreal); break;
	case IDIVB:	snprint(o,n,"%d / %d", R.m->disbyte, R.s->disbyte); break;
	case IDIVW:	snprint(o,n,"%d / %d", R.m->disint, R.s->disint); break;
	case IDIVL:	snprint(o,n,"0x%llX / 0x%llX", R.m->disbig, R.s->disbig); break;
	case IDIVF:	snprint(o,n,"%f / %f", R.m->disreal, R.s->disreal); break;
	case IANDB:	snprint(o,n,"%d & %d", R.m->disbyte, R.s->disbyte); break;
	case IANDW:	snprint(o,n,"%d & %d", R.m->disint, R.s->disint); break;
	case IANDL:	snprint(o,n,"0x%llX & 0x%llX", R.m->disbig, R.s->disbig); break;
	case IORB:	snprint(o,n,"%d | %d", R.m->disbyte, R.s->disbyte); break;
	case IORW:	snprint(o,n,"%d | %d", R.m->disint, R.s->disint); break;
	case IORL:	snprint(o,n,"0x%llX | 0x%llX", R.m->disbig, R.s->disbig); break;
	case IXORB:	snprint(o,n,"%d ^ %d", R.m->disbyte, R.s->disbyte); break;
	case IXORW:	snprint(o,n,"%d ^ %d", R.m->disint, R.s->disint); break;
	case IXORL:	snprint(o,n,"0x%llX ^ 0x%llX", R.m->disbig, R.s->disbig); break;
	case IMODB:	snprint(o,n,"%d %% %d", R.m->disbyte, R.s->disbyte); break;
	case IMODW:	snprint(o,n,"%d %% %d", R.m->disint, R.s->disint); break;
	case IMODL:	snprint(o,n,"0x%llX %% 0x%llX", R.m->disbig, R.s->disbig); break;
	case ISHLB:	snprint(o,n,"%d << %d", R.m->disbyte, R.s->disint); break;
	case ISHLW:	snprint(o,n,"%d << %d", R.m->disint, R.s->disint); break;
	case ISHLL:	snprint(o,n,"0x%llX << %d", R.m->disbig, R.s->disint); break;
	case ISHRB:	snprint(o,n,"%d >> %d", R.m->disbyte, R.s->disint); break;
	case ISHRW:	snprint(o,n,"%d >> %d", R.m->disint, R.s->disint); break;
	case ISHRL:	snprint(o,n,"0x%llX >> %d", R.m->disbig, R.s->disint); break;
	case ILSRW:	snprint(o,n,"%ud >>> %d", (DISUINT)R.m->disint, R.s->disint); break;
	case ILSRL:	snprint(o,n,"0x%lluX >>> %d", (DISUBIG)R.m->disbig, R.s->disint); break;
	case IEXPW:	snprint(o,n,"%d %d", R.m->disint, R.s->disint); break;
	case IEXPL:	snprint(o,n,"0x%llX %d", R.m->disbig, R.s->disint); break;
	case IEXPF:	snprint(o,n,"%f %d", R.m->disreal, R.s->disint); break;
	case ICVTXF:	snprint(o,n,"%f %d", R.m->disreal, R.s->disint); break;
	case ICVTFX:	snprint(o,n,"%f %f", R.m->disreal, R.s->disreal); break;
	case ICVTXX:
	case ICVTXX0:
	case ICVTXX1:
	case IMULX:
	case IMULX0:
	case IMULX1:
	case IDIVX:
	case IDIVX0:
	case IDIVX1:	snprint(o,n,"%d %d", R.m->disint, R.s->disint); break;
	case IINDF:
	case IINDB:
	case IINDW:
	case IINDL:
	case IINDX:	snprint(o,n,"Array[%d]@%p [%d]", Alen(s), R.s->parray, R.d->disint); break;
	case IINDC:     snprint(o,n,"\"%s\" [%d]", string2c(R.s->pstring), R.m->disint); break;
	case IINSC:	snprint(o,n,"\"%s\" [%d] = %d", string2c(R.d->pstring), R.m->disint, R.s->disint); break;
	case ISLICEA: 	snprint(o,n,"Array[%d]@%p [%d:%d]", Alen(d), R.d->parray, R.s->disint, R.m->disint); break;
	case ISLICEC: 	snprint(o,n,"\"%s\" [%d:%d]", string2c(R.d->pstring), R.s->disint, R.m->disint); break;
	case ISLICELA: 	snprint(o,n,"Array[%d]@%p [%d:] = Array[%d]@%p", Alen(d), R.d->parray, R.m->disint, Alen(s), R.s->parray); break;
	case IMSPAWN:
	case IMCALL:	snprint(o,n,"%s.%d Frame@%p FP=Frame@%p", R.d->pmodlink->m->name, R.m->disint, R.s->pframe, R.FP); break;
	case IMFRAME:	snprint(o,n,"%s.%d", R.s->pmodlink->m->name, R.m->disint); break;
	case ISPAWN:
	case ICALL: 	snprint(o,n,"%s_%uX Frame@%p FP=Frame@%p", CURM, I(d), R.s->pframe, R.FP); break;
	case IJMP: 	snprint(o,n,"%s_%uX", CURM, I(d)); break;
	case IGOTO:	snprint(o,n,"%s_%uX", CURM, (DISINT*)((DISINT)R.d + (R.s->disint * sizeof(DISINT)))); break;
	case IMOVPC:	snprint(o,n,"%s_%uX", CURM, I(s)); break;
	case INEW:
	case INEWZ:	snprint(o,n,"%d", R.s->disint); break;
	case IMNEWZ:	snprint(o,n,"Type_%s_%d", R.s->pmodlink->m->name, R.m->disint); break;
	case INEWA:
	case INEWAZ:	snprint(o,n,"Type_%s_%d [%d]", CURM, R.m->disint, R.s->disint); break;
	case INEWCB:	// new channel
	case INEWCW:
	case INEWCF:
	case INEWCP:
	case INEWCL:
	case INEWCM: 	snprint(o,n,"[%d]", R.s->disint); break;
	case INEWCMP:	snprint(o,n,"Type_%s_%d", CURM, R.s->disint); break;
	case ISEND:	snprint(o,n,"Channel@%p <-= %p", R.d->pchannel, R.s->pvoid); break;
	case IRECV:	snprint(o,n,"Channel@%p", R.s->pchannel); break;
	case ICONSB:	snprint(o,n,"%d :: List@%p", R.s->disbyte, R.d->plist); break;	// list
	case ICONSW:	snprint(o,n,"%d :: List@%p", R.s->disint, R.d->plist); break;
	case ICONSF:	snprint(o,n,"%f :: List@%p", R.s->disreal, R.d->plist); break;
	case ICONSL:	snprint(o,n,"0x%llx :: List@%p", R.s->disbig, R.d->plist); break;
	case ICONSP:	snprint(o,n,"%p :: List@%p", R.s->pvoid, R.d->plist); break;
	case ICONSM:	snprint(o,n,"%p [%d] :: List@%p", R.s->pvoid, R.m->disint, R.d->plist); break;
	case ICONSMP:	snprint(o,n,"Type_%s_%d@%p :: List@%p", CURM, R.m->disint, R.s->pvoid, R.d->plist); break;
	case ITAIL:
	case ILENL:
	case IHEADB:
	case IHEADW:
	case IHEADP:
	case IHEADF:
	case IHEADL:
	case IHEADMP:   snprint(o,n,"List@%p", R.s->plist); break;
	case IHEADM:	snprint(o,n,"List@%p [%d]", R.s->plist, R.m->disint); break;
	case IMOVM:	snprint(o,n,"memmove(%p, %p, %d)", R.d, R.s, R.m->disint); break;
	case IMOVMP:	snprint(o,n,"Type_%s_%d@%p", CURM, R.m->disint, R.s); break;
	case ITCMP:	snprint(o,n,"%p %p", R.m->pvoid, R.s->pvoid); break;
	case ILOAD:	snprint(o,n,"\"%s\" %d", string2c(R.s->pstring), R.m->disint); break;
	case ICASE:	snprint(o,n,"%d %p", R.s->disint, R.d->pvoid); break;               		/* FIXME: show structure bihing the pointer */
	case ICASEC:	snprint(o,n,"\"%s\" %p", string2c(R.s->pstring), R.d->pvoid); break;	/* FIXME: show structure bihing the pointer */
	case ICASEL:	snprint(o,n,"0x%llX %p", R.s->disbig, R.d->pvoid); break;		/* FIXME: show structure bihing the pointer */
	}
}

// print values affected by op
void stateafter(char* o, int n, uchar op)
{
	o[0] = '\0';
	switch(op)
	{
//	case IRET: {
//	Frame*f = (Frame*)R.FP;
//	snprint(o,n," => %s_%uX", (f&&f->mr&&f->mr->m)?f->mr->m->name:"", f?f->lr:-1 /*- f->mr->m->prog*/);
//	}
//	break;
	case IINDF:	snprint(o,n," => %f", R.m->disreal); break; // result in middle
	case IINDB:     snprint(o,n," => %d", R.m->disbyte); break;
	case IINDW:     snprint(o,n," => %d", R.m->disint); break;
	case IINDL:	snprint(o,n," => 0x%llx", R.m->disbig); break;
	case IINDX:	snprint(o,n," => %p", R.m->pvoid); break;
	case ILOAD:	// module
	case ISELF:	snprint(o,n," => Module(%s)", R.d->pmodlink==H?"nil":R.d->pmodlink->m->name); break;
	case INEW:	// pointer
	case INEWZ:
	case IMNEWZ:
	case IMOVP:
	case IMOVMP:
	case ILEA:
	case IHEADP:
	case IHEADMP:
	case IHEADM:
	case IRECV:
	case IMOVPC:	snprint(o,n," => %p", R.d->pvoid); break;
	case ICONSB:	// list
	case ICONSW:
	case ICONSF:
	case ICONSL:
	case ICONSP:
	case ICONSM:
	case ICONSMP:
	case ITAIL:	snprint(o,n," => List@%p", R.d->plist); break;
	case INEWA:	// array
	case INEWAZ:
	case ICVTCA:
	case ISLICEA: 	snprint(o,n," => Array[%d]@%p", Alen(d), R.d->parray); break;
	case INEWCB:    // channel
	case INEWCW:
	case INEWCF:
	case INEWCP:
	case INEWCL:
	case INEWCM:
	case INEWCMP:	snprint(o,n," => Channel@%p", R.d->pchannel); break;
	case ICVTFR:	snprint(o,n," => %f", R.d->disreal32); break; // short float
	case ICVTRF:	// float
	case ICVTWF:
	case ICVTLF:
	case ICVTCF:
	case INEGF:
	case IMOVF:
	case IADDF:
	case ISUBF:
	case IMULF:
	case IDIVF:
	case ICVTXF:
	case IEXPF:
	case IHEADF:	snprint(o,n," => %f", R.d->disreal); 	break;
	case ICVTWB:	// byte
	case IMOVB:
	case IADDB:
	case ISUBB:
	case IMULB:
	case IDIVB:
	case IANDB:
	case IORB:
	case IXORB:
	case IMODB:
	case ISHLB:
	case ISHRB:
	case IHEADB:	snprint(o,n," => %d", R.d->disbyte); 	break;
	case ICVTWS:	snprint(o,n," => %d", R.d->disint16); 	break; // short
	case IINDC:	// int
	case ILENC:
	case ILENL:
	case ILENA:
	case ICVTBW:
	case ICVTFW:
	case ICVTLW:
	case ICVTCW:
	case IMOVW:
	case IADDW:
	case ISUBW:
	case IMULW:
	case IDIVW:
	case IANDW:
	case IORW:
	case IXORW:
	case IMODW:
	case ISHLW:
	case ISHRW:
	case ILSRW:
	case IEXPW:
	case IHEADW:
	case IALT:
	case INBALT:	snprint(o,n," => %d", R.d->disint);	break;
	case ICVTFX:	// X?
	case IMULX:
	case IMULX0:
	case IMULX1:
	case IDIVX:
	case IDIVX0:
	case IDIVX1:
	case ICVTXX:
	case ICVTXX0:
	case ICVTXX1:	snprint(o,n," => %d", R.d->disint);	break;
	case ICVTFL:	// big
	case ICVTWL:
	case ICVTCL:
	case IMOVL:
	case IADDL:
	case ISUBL:
	case IMULL:
	case IDIVL:
	case IANDL:
	case IORL:
	case IXORL:
	case IMODL:
	case ISHLL:
	case ISHRL:
	case ILSRL:
	case IEXPL:
	case IHEADL:	snprint(o,n," => 0x%llx", R.d->disbig);break;
	case ICVTLC:	// string
	case ICVTAC:
	case ICVTWC:
	case ICVTFC:
	case ISLICEC:
	case IADDC:	snprint(o,n," => \"%s\"", string2c(R.d->pstring)); 	break;
	case IMFRAME:
	case IFRAME:	snprint(o,n," => Frame@%p", R.d->pframe); 	break;
	}
}
#endif

void
xec(Prog *p)
{
	int op;

	R = p->R;
	R.MP = R.M->MP;
	R.IC = p->quanta;

	if(p->kill != nil) {
		char *m;
		m = p->kill;
		p->kill = nil;
		error(m);
	}

// print("%lux %lux %lux %lux %lux\n", (ulong)&R, R.xpc, R.FP, R.MP, R.PC);

	if(R.M->compiled)
	{
		/* BUG */
#if STACK
		comvec();
#endif
	} else do {
		dec[R.PC->add]();
		op = R.PC->op;
#ifdef DEBUGVM
		{
		char sz[100], sz2[200];
		snprint(sz, sizeof(sz), "%s_%uX:", CURM, R.PC - R.M->m->prog );
		statebefore(sz2, sizeof(sz2), op, R.M->m->prog);
		print("%-16s", sz);
		print("\t%02ux %02ux %04ux %08ux %08ux", R.PC->op, R.PC->add, R.PC->reg, R.PC->s.imm, R.PC->d.imm);
		print("\t%-40D", R.PC);
		print("\t%s", sz2);
#endif
		R.PC++;
		optab[op]();
#ifdef DEBUGVM
		stateafter(sz2, sizeof(sz2), op);
		print("%s\n", sz2);
		}
#endif
	} while(--R.IC != 0);

	p->R = R;
}
