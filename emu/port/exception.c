#include "dat.h"
#include "fns.h"
#include "error.h"
#include "isa.h"
#include "interp.h"
#include "runt.h"
#include "kernel.h"
#include "raise.h"

static int
ematch(char *pat, char *exp)
{
	int l;

	if(strcmp(pat, exp) == 0)
		return 1;

	l = strlen(pat);
	if(l == 0)
		return 0;
	if(pat[l-1] == '*') {
		if(l == 1)
			return 1;
		if(strncmp(pat, exp, l-1) == 0)
			return 1;
	}
	return 0;
}

static void
setstr(String *s, char *p)
{
	if(s == H)
		return;
	if(s->len < 0 || s->max < 4)
		return;
	kstrcpy(s->Sascii, p, s->max);	/* TO DO: we are assuming they aren't runes */
	s->len = strlen(s->Sascii);
}

static String *exstr;

void
excinit(void)
{
	exstr = newstring(ERRMAX);
	/* BUG poolimmutable(D2H(exstr)); */
}

static String*
newestring(char *estr)
{
	String *s;

	if(waserror()){
		setstr(exstr, estr);
		ADDREF(exstr);
		return exstr;
	}
	s = c2string(estr, strlen(estr));
	poperror();
	return s;
}

#define NOPC	0xffffffff
#if STACK
#define FRTYPE(f)	((f)->t == nil ? SEXTYPE(f)->reg.TR : (f)->t)

/*
 * clear up an uncalled frame
 */
static void
freeframe(uchar *fp, int setsp)
{
	Frame *f;

	f = (Frame*)fp;
	if(f->t == nil)
		unextend(f);
	else if(f->t->np)
		freeptrs(f, f->t);
	if(setsp)
		R.SP = fp;
}
#endif
extern REG R;
int
handler(char *estr)
{
	Prog *p;
	Modlink *m, *ml;
	int str, ne;
	ulong pc, newpc;
	long eoff;
	Frame *f;
	String ** eadr;
	Type *zt;
	Handler *h;
	Except *e;
	void *v;

	p = currun();
#if STACK
#else
	print("handler:");
	print(" |%s|%p %d\n", estr, p, p?p->pid:0);
	/*panic("handler");*/
#endif
	if(*estr == 0 || p == nil)
		return 0;

	print("handler go on\n");
	str = p->exval == H || D2H(p->exval)->t == &Tstring;
	m = R.ML;
	if(m->compiled)
		pc = (char*)R.PC-(char*)m->prog;
	else
		pc = R.PC-m->prog;
	pc--;

	for(f = R.FP; f != H; f = f->parent) {		/* look for a handler */
		if((h = m->m->htab) != nil){
			for( ; h->etab != nil; h++){
				if(pc < h->pc1 || pc >= h->pc2)
					continue;
				eoff = h->eoff;
				zt = h->t;
				for(e = h->etab, ne = h->ne; e->s != nil; e++, ne--){
					if(ematch(e->s, estr) && (str && ne <= 0 || !str && ne > 0)){
						newpc = e->pc;
						goto found;
					}
				}
				newpc = e->pc;
				if(newpc != NOPC)
					goto found;
			}
		}
		if(!str && f != R.FP){		/* becomes a string exception in immediate caller */
			/*
			v = p->exval;
			p->exval = *(String**)v;
			ADDREF(p->exval);
			ASSIGN(v, H);
			*/
			ADDREF(*(String**)p->exval);
			ASSIGN(p->exval, *(String**)p->exval);
			/**/
			str = 1;
			continue;
		}
		//f = (Frame*)fp;
		if(f->ml != H)
			m = f->ml;
		if(m->compiled)
			pc = (char*)f->lr - (char*)m->prog;
		else
			pc = f->lr - m->prog;
		pc--;
	}
	ASSIGN(p->exval, H);
	return 0;
found:
	print("exc:found\n");
	{
		int n;
		char name[3*KNAMELEN];

		pc = modstatus(&R, name, sizeof(name));
		n = 10+1+strlen(name)+1+strlen(estr)+1;
		p->exstr = (char*)realloc(p->exstr, n);
		if(p->exstr != nil)
			snprint(p->exstr, n, "%lud %s %s", pc, name, estr);
	}

	/*
	 * there may be an uncalled frame at the top of the stack
	 */
	f = R.FP;
	/* BUG */
#if STACK
	t = FRTYPE(f);
	if(R.FP < R.EX || R.FP >= R.TS)
		freeframe(R.EX+OA(Stkext, reg.tos.fr), 0);
	else if(R.FP+t->size < R.SP)
		freeframe(R.FP+t->size, 1);
#endif
	m = R.ML;
	while(R.FP != f){
		f = R.FP;
		R.PC = f->lr;
		R.FP = f->parent;

		ml = f->ml;

		//? destroy(f)
		assert(D2H(f)->t != nil);
		freeptrs(f, D2H(f)->t);   /* FIXME */

		if(ml != H){
			m = ml;
			ASSIGN(R.ML, m);
		}
	}
	if(zt != nil){
		freeptrs(f, zt);
		initmem(zt, f);
	}
	eadr = (String **)((char*)f+eoff);
	ASSIGN(*eadr, H);
	if(p->exval == H)
		*eadr = newestring(estr);	/* might fail */
	else{
		D2H(p->exval)->ref++;
		*eadr = p->exval;
	}
	if(m->compiled)
		R.PC = (Inst*)((char*)m->prog+newpc);
	else
		R.PC = m->prog+newpc;
	memmove(&p->R, &R, sizeof(R));
	p->kill = nil;
	ASSIGN(p->exval, H);
	return 1;
}
