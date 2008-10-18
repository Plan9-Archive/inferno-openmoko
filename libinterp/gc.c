#include "lib9.h"
#include "interp.h"
#include "pool.h"

enum
{
	Quanta		= 50,		/* Allocated blocks to sweep each time slice usually */
	MaxQuanta	= 15*Quanta,
	PTRHASH		= (1<<5)
};

static int quanta = Quanta;
static int gce = 0, gct = 1;

typedef struct Ptrhash Ptrhash;
struct Ptrhash
{
	Heap	*value;
	Ptrhash	*next;
};

	int	nprop = 0;
	int	gchalt = 0;
/*extern	int	mflag;*/
	int	mutator = 0;
	int	gccolor = 3;

	ulong	gcnruns = 0;
	ulong	gcsweeps = 0;
	ulong	gcbroken = 0;
	ulong	gchalted = 0;
	ulong	gcepochs = 0;
	uvlong	gcdestroys = 0;
	uvlong	gcinspects = 0;

static	int	marker  = 1;
static	int	sweeper = 2;
/*
static	Bhdr*	base;
static	Bhdr*	limit;
Bhdr*	ptr;*/
static	int	visit = 0;
static	Ptrhash	*ptrtab[PTRHASH] = {0};
static	Ptrhash	*ptrfree = 0;
enum
{
	MAGIC_A		= 0xa110c,		/* Allocated block */
	MAGIC_F		= 0xbadc0c0a,		/* Free block */
	MAGIC_E		= 0xdeadbabe,		/* End of arena */
	MAGIC_I		= 0xabba		/* Block is immutable (hidden from gc) */
};

#define	HASHPTR(p)	(((ulong)(p) >> 6) & (PTRHASH - 1))

void
ptradd(Heap *v)
{
	int h;
	Ptrhash *p;

	if ((p = ptrfree) != nil)
		ptrfree = p->next;
	else if ((p = (Ptrhash *)malloc(sizeof (Ptrhash))) == nil)
		error("ptradd malloc");
	h = HASHPTR(v);
	p->value = v;
	p->next = ptrtab[h];
	ptrtab[h] = p;
}

void
ptrdel(Heap *v)
{
	Ptrhash	*p, **l;

	for (l = &ptrtab[HASHPTR(v)]; (p = *l) != nil; l = &p->next) {
		if (p->value == v) {
			*l = p->next;
			p->next = ptrfree;
			ptrfree = p;
			return;
		}
	}
	/* ptradd must have failed */
}

static void
ptrmark(void)
{
	int	i;
	Heap	*h;
	Ptrhash	*p;

	for (i = 0; i < PTRHASH; i++) {
		for (p = ptrtab[i]; p != nil; p = p->next) {
			h = p->value;
			Setmark(h);
		}
	}
}

void
noptrs(Type *t, void *vw)
{
	USED(t);
	USED(vw);
}

static int markdepth;

/* code simpler with a depth search compared to a width search*/
void
markheap(Type *t, void *vw)
{
	Heap *h;
	char *p;
	int i, c, m;
	void **w, **q;
	Type *t1;

	if(t == nil || t->np == 0)
		return;

	markdepth++;
	w = (void**)vw;
	p = t->map;
	for(i = 0; i < t->np; i++) {
		c = *p++;
		if(c != 0) {
			q = w;
			for(m = 0x80; m != 0; m >>= 1) {
				if((c & m) && *q != H) {
					h = D2H(*q);
					Setmark(h);
					if(h->color == propagator && --visit >= 0 && markdepth < 64){
						gce--;
						h->color = mutator;
						if((t1 = h->t) != nil)
							t1->fnmark(t1, H2D(void*, h));
					}
				}
				q++;
			}
		}
		w += 8;
	}
	markdepth--;
}

/*
 * TODO This routine should be modified to be incremental, but how?
 */
void
markarray(Type *t, Array *a)
{
	int i;
	Heap *h;
	char *v;

	USED(t);

	t = a->t;
	if(a->root != H) {
		h = D2H(a->root);
		Setmark(h);
	}

	if(t->np == 0)
		return;

	v = a->data;
	for(i = 0; i < a->len; i++) {
		markheap(t, v);
		v += t->size;
	}
	visit -= a->len;
}

void
marklist(Type *t, List *l)
{
	Heap *h;

	USED(t);
	markheap(l->t, &l->data); /*?*/
	while(visit > 0) {
		l = l->tail;
		if(l == H)
			return;
		h = D2H(l);
		Setmark(h);
		markheap(l->t, &l->data);
		visit--;
	}
	l = l->tail;
	if(l != H) {
		D2H(l)->color = propagator;
		nprop = 1;
	}
}

static void
rootset(Prog *root)
{
	Heap *h;
	Type *t;
	Frame *f;
	Module *m;
	Stkext *sx;
	Modlink *ml;
	char *fp, *sp, *ex, *mp;

	mutator = gccolor % 3;
	marker = (gccolor-1)%3;
	sweeper = (gccolor-2)%3;

	while(root != nil) {
		ml = root->R.ML;
		h = D2H(ml);
		Setmark(h);
		mp = ml->MP;
		if(mp != H) {
			h = D2H(mp);
			Setmark(h);
		}
/* TODO */
#if STACK
		sp = root->R.SP;
		ex = root->R.EX;
		while(ex != nil) {
			sx = (Stkext*)ex;
			fp = sx->reg.tos.fu;
			while(fp != sp) {
				f = (Frame*)fp;
				t = f->t;
				if(t == nil)
					t = sx->reg.TR;
				fp += t->size;
				t->mark(t, f);
				ml = f->mr;
				if(ml != nil) {
					h = D2H(ml);
					Setmark(h);
					mp = ml->MP;
					if(mp != H) {
						h = D2H(mp);
						Setmark(h);
					}
				}
			}
			ex = sx->reg.EX;
			sp = sx->reg.SP;
		}
#else
	print("rootset\n");
#endif
		root = root->next;
	}

	for(m = modules; m != nil; m = m->link) {
		if(m->origmp != H) {
			h = D2H(m->origmp);
			Setmark(h);
		}
	}

	ptrmark();
}

static int
oktag(int tag)
{
	switch(tag) {
	case MAGIC_A:
	case MAGIC_F:
	case MAGIC_E:
	case MAGIC_I:
		return 1;
	}
	return 0;
}
/*
static void
domflag(Heap *h)
{
	int i;
	Module *m;

	print("sweep h=0x%lux t=0x%lux c=%d", (ulong)h, (ulong)h->t, h->color);
	for(m = modules; m != nil; m = m->link) {
		for(i = 0; i < m->ntype; i++) {
			if(m->type[i] == h->t) {
				print(" module %s desc %d", m->name, i);
				break;
			}
		}
	}
	print("\n");
	if(mflag > 1)
		abort();
}*/

void
rungc(Prog *p)
{
/*print("rungc pid=%d\n", p->pid);/**/
/*
	Type *t;
	Heap *h;
	Bhdr *b;

	return;

	gcnruns++;
	if(gchalt) {
		gchalted++;
		return;
	}
	if(base == nil) {
		gcsweeps++;
		b = poolchain(heapmem);
		base = b;
		ptr = b;
		limit = B2LIMIT(b);
	}

	//* Chain broken ? *
	if(!okbhdr(ptr)) {
		base = nil;
		gcbroken++;
		return;
	}

	for(visit = quanta; visit > 0; ) {
		if(ptr->magic == MAGIC_A) {
			visit--;
			gct++;
			gcinspects++;
			h = B2D(ptr);
			t = h->t;
			if(h->color == propagator) {
				gce--;
				h->color = mutator;
				if(t != nil)
					t->mark(t, H2D(void*, h));
			}
			else
			if(h->color == sweeper) {
				gce++;
				//*if(0 && mflag)
				//	domflag(h);*
				if(heapmonitor != nil)
					heapmonitor(2, h, 0);
				if(t != nil) {
					gclock();
					t->free(h, 1);
					gcunlock();
					freetype(t);
				}
				gcdestroys++;
				poolfree(heapmem, h);
			}
		}
		ptr = B2NB(ptr);
		if(ptr >= limit) {
			base = base->clink;
			if(base == nil)
				break;
			ptr = base;
			limit = B2LIMIT(base);
		}
	}

	quanta = (MaxQuanta+Quanta)/2 + ((MaxQuanta-Quanta)/20)*((100*gce)/gct);
	if(quanta < Quanta)
		quanta = Quanta;
	if(quanta > MaxQuanta)
		quanta = MaxQuanta;

	if(base != nil)		/* Completed this iteration ? *
		return;
	if(nprop == 0) {	/* Completed the epoch ? *
		gcepochs++;
		gccolor++;
		rootset(p);
		gce = 0;
		gct = 1;
		return;
	}
	nprop = 0;
*/
}
