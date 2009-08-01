#include <lib9.h>
#include <isa.h>
#include <interp.h>
#include <raise.h>
#include <pool.h>

//#define DEBUGVM

REG     R = {0};                        /* Virtual Machine registers */ /* BUG: WTF Global R? on multi-processor? */
String  snil = {0};                     /* String known to be zero length */


#define OP(fn)  void fn(__inout_ecount(1) Disdata*rs, __inout_ecount(1) Disdata*rm, __inout_ecount(1) Disdata*rd, REG*rr)


OP(runt) { }
OP(negf) { rd->disreal = -rs->disreal; }
OP(jmp)  { rr->PC = rd->pinst; }
OP(movpc){ rd->pinst = rr->ML->prog + rs->disint; }
OP(movm) { memmove(rd, rs, rm->disint); /* TODO: add some assertions here */ }
OP(lea)  { rd->pdisdata = rs; }
OP(movb) { rd->disbyte = rs->disbyte; }
OP(movw) { rd->disint = rs->disint; }
OP(movf) { rd->disreal = rs->disreal; }
OP(movl) { rd->disbig = rs->disbig; }
OP(cvtbw){ rd->disint = rs->disbyte; }
OP(cvtwb){ rd->disbyte = rs->disint;    /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtrf){ rd->disreal = rs->disreal32; }
OP(cvtfr){ rd->disreal32 = rs->disreal;/* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtws){ rd->disint16 = rs->disint; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtsw){ rd->disint = rs->disint16; }
OP(cvtwf){ rd->disreal = rs->disint;  /* TODO: optional runtime warning when the value is beyond target range */ }
OP(addb) { rd->disbyte = rm->disbyte + rs->disbyte; }
OP(addw) { rd->disint = rm->disint + rs->disint; }
OP(addl) { rd->disbig = rm->disbig + rs->disbig; }
OP(addf) { rd->disreal = rm->disreal + rs->disreal; }
OP(subb) { rd->disbyte = rm->disbyte - rs->disbyte; }
OP(subw) { rd->disint = rm->disint - rs->disint; }
OP(subl) { rd->disbig = rm->disbig - rs->disbig; }
OP(subf) { rd->disreal = rm->disreal - rs->disreal; }
OP(divb) { rd->disbyte = rm->disbyte / rs->disbyte; }
OP(divw) { rd->disint = rm->disint / rs->disint; }
OP(divl) { rd->disbig = rm->disbig / rs->disbig; }
OP(divf) { rd->disreal = rm->disreal / rs->disreal; }
OP(modb) { rd->disbyte = rm->disbyte % rs->disbyte; }
OP(modw) { rd->disint = rm->disint % rs->disint; }
OP(modl) { rd->disbig = rm->disbig % rs->disbig; }
OP(mulb) { rd->disbyte = rm->disbyte * rs->disbyte; }
OP(mulw) { rd->disint = rm->disint * rs->disint; }
OP(mull) { rd->disbig = rm->disbig * rs->disbig; }
OP(mulf) { rd->disreal = rm->disreal * rs->disreal; }
OP(andb) { rd->disbyte = rm->disbyte & rs->disbyte; }
OP(andw) { rd->disint = rm->disint & rs->disint; }
OP(andl) { rd->disbig = rm->disbig & rs->disbig; }
OP(xorb) { rd->disbyte = rm->disbyte ^ rs->disbyte; }
OP(xorw) { rd->disint = rm->disint ^ rs->disint; }
OP(xorl) { rd->disbig = rm->disbig ^ rs->disbig; }
OP(orb)  { rd->disbyte = rm->disbyte | rs->disbyte; }
OP(orw)  { rd->disint = rm->disint | rs->disint; }
OP(orl)  { rd->disbig = rm->disbig | rs->disbig; }
OP(shlb) { rd->disbyte = rm->disbyte << rs->disint; }
OP(shlw) { rd->disint = rm->disint << rs->disint; }
OP(shll) { rd->disbig = rm->disbig << rs->disint; }
OP(shrb) { rd->disbyte = rm->disbyte >> rs->disint; }
OP(shrw) { rd->disint = rm->disint >> rs->disint; }
OP(shrl) { rd->disbig = rm->disbig >> rs->disint; }
OP(lsrw) { rd->disint = (DISUINT)rm->disint >> rs->disint; }
OP(lsrl) { rd->disbig = (DISUBIG)rm->disbig >> rs->disint; }
OP(beqb) { if(rs->disbyte == rm->disbyte) rr->PC = rd->pinst; }
OP(bneb) { if(rs->disbyte != rm->disbyte) rr->PC = rd->pinst; }
OP(bltb) { if(rs->disbyte <  rm->disbyte) rr->PC = rd->pinst; }
OP(bleb) { if(rs->disbyte <= rm->disbyte) rr->PC = rd->pinst; }
OP(bgtb) { if(rs->disbyte >  rm->disbyte) rr->PC = rd->pinst; }
OP(bgeb) { if(rs->disbyte >= rm->disbyte) rr->PC = rd->pinst; }
OP(beqw) { if(rs->disint == rm->disint) rr->PC = rd->pinst; }
OP(bnew) { if(rs->disint != rm->disint) rr->PC = rd->pinst; }
OP(bltw) { if(rs->disint <  rm->disint) rr->PC = rd->pinst; }
OP(blew) { if(rs->disint <= rm->disint) rr->PC = rd->pinst; }
OP(bgtw) { if(rs->disint >  rm->disint) rr->PC = rd->pinst; }
OP(bgew) { if(rs->disint >= rm->disint) rr->PC = rd->pinst; }
OP(beql) { if(rs->disbig == rm->disbig) rr->PC = rd->pinst; }
OP(bnel) { if(rs->disbig != rm->disbig) rr->PC = rd->pinst; }
OP(bltl) { if(rs->disbig <  rm->disbig) rr->PC = rd->pinst; }
OP(blel) { if(rs->disbig <= rm->disbig) rr->PC = rd->pinst; }
OP(bgtl) { if(rs->disbig >  rm->disbig) rr->PC = rd->pinst; }
OP(bgel) { if(rs->disbig >= rm->disbig) rr->PC = rd->pinst; }
OP(beqf) { if(rs->disreal == rm->disreal) rr->PC = rd->pinst; }
OP(bnef) { if(rs->disreal != rm->disreal) rr->PC = rd->pinst; }
OP(bltf) { if(rs->disreal <  rm->disreal) rr->PC = rd->pinst; }
OP(blef) { if(rs->disreal <= rm->disreal) rr->PC = rd->pinst; }
OP(bgtf) { if(rs->disreal >  rm->disreal) rr->PC = rd->pinst; }
OP(bgef) { if(rs->disreal >= rm->disreal) rr->PC = rd->pinst; }
OP(beqc) { if(stringcmp(rs->pstring, rm->pstring) == 0) rr->PC = rd->pinst; }
OP(bnec) { if(stringcmp(rs->pstring, rm->pstring) != 0) rr->PC = rd->pinst; }
OP(bltc) { if(stringcmp(rs->pstring, rm->pstring) <  0) rr->PC = rd->pinst; }
OP(blec) { if(stringcmp(rs->pstring, rm->pstring) <= 0) rr->PC = rd->pinst; }
OP(bgtc) { if(stringcmp(rs->pstring, rm->pstring) >  0) rr->PC = rd->pinst; }
OP(bgec) { if(stringcmp(rs->pstring, rm->pstring) >= 0) rr->PC = rd->pinst; }
OP(iexit){ error(""); }
OP(cvtwl){ rd->disbig = rs->disint; }
OP(cvtlw){ rd->disint = rs->disbig; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtlf){ rd->disreal = rs->disbig; /* TODO: optional runtime warning when the value is beyond target range */ }
OP(cvtfl)
{
        const DISREAL f = rs->disreal;

        rd->disbig = f < 0 ? f - .5 : f + .5;
}
OP(cvtfw)
{
        const DISREAL f = rs->disreal;

        rd->disint = f < 0 ? f - .5 : f + .5;
}
OP(cvtcl)
{
        String *s = rs->pstring;

        if(s == H)
                rd->disbig = 0;
        else
                rd->disbig = strtoll(string2c(s), nil, 10);
}
OP(iexpw)
{
        int inv;
        DISINT x, n, r;

        x = rm->disint;
        n = rs->disint;
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
        rd->disint = r;
}
OP(iexpl)
{
        int inv;
        DISINT n;
        DISBIG x, r;

        x = rm->disbig;
        n = rs->disint;
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
        rd->disbig = r;
}
OP(iexpf)
{
        int inv;
        DISINT n;
        DISREAL x, r;

        x = rm->disreal;
        n = rs->disint;
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
        rd->disreal = r;
}
OP(indx)
{
        Array *a = rs->parray;
        DISINT i = rd->disint;

        if (a == H || i < 0 || i >= a->len)
                error(exBounds);

        rm->pvoid = a->data + i*a->t->size;
}
OP(indw)
{
        Array *a = rs->parray;
        DISINT i = rd->disint;

        if (a == H || i < 0 || i >= a->len)
                error(exBounds);

        rm->pvoid = a->data + i*sizeof(DISINT);
}
OP(indf)
{
        Array *a = rs->parray;
        DISINT i = rd->disint;

        if (a == H || i < 0 || i >= a->len)
                error(exBounds);

        rm->pvoid = a->data + i*sizeof(DISREAL);
}
OP(indl)
{
        Array *a = rs->parray;
        DISINT i = rd->disint;

        if (a == H || i < 0 || i >= a->len)
                error(exBounds);

        rm->pvoid = a->data + i*sizeof(DISBIG);
}
OP(indb)
{
        Array *a = rs->parray;
        DISINT i = rd->disint;

        if (a == H || i < 0 || i >= a->len)
                error(exBounds);

        rm->pvoid = a->data + i*sizeof(DISBYTE);
}
OP(movp)
{
        void *sv = rs->pvoid;

        if(sv != H) {
                ADDREF(sv);
                Setmark(D2H(sv));
        }
        ASSIGN(rd->pvoid, sv);
}
OP(movmp)
{
        Type *t;

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        t = rr->ML->type[rm->disint];

        incmem(rs, t);
        freeptrs(rd, t);
        memcpy(rd, rs, t->size);
}
OP(new)
{
        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rs->disint)
                error(exCctovflw);

        ASSIGN(rd->pvoid, H2D(void*, heap(rr->ML->type[rs->disint])));
}
OP(newz)
{
        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rs->disint)
                error(exCctovflw);

        ASSIGN(rd->pvoid, H2D(void*, heapz(rr->ML->type[rs->disint])));
}
OP(mnewz)
{
        Modlink *ml = rs->pmodlink;

        if(ml == H)
                error(exModule);

        if((DISUINT)ml->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        ASSIGN(rd->pvoid, H2D(void*, heapz(ml->type[rm->disint])));
}
OP(frame) /* == newz */
{
        Frame *f;
        Type *t;

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rs->disint)
                error(exCctovflw);

        t = rr->ML->type[rs->disint];

        /* frame type fix check */
        assert(t->np>=1 && (t->map[0] & 0xF0)==0x60);
        f = H2D(Frame*, heapz(t));
        assert(f->lr == nil);
        assert(f->ml == H);
        assert(f->parent == H);

        rd->pframe = f;
}
OP(mframe)
{
        Type *t;
        Modlink *ml = rs->pmodlink;
        int o = rm->disint;
        Frame *f;

        if(ml == H)
                error(exModule);

        if (o >= 0) {
                if(o >= ml->nlinks)
                        error("invalid mframe");
                t = ml->links[o].frame;
        } else
                t = ml->m->ext[-o-1].frame;   /* TODO: check index range */

        /* frame type fix check */
        assert(t->np>=1 && (t->map[0] & 0xF0)==0x60);
        f = H2D(Frame*,heapz(t));
        assert(f->lr == nil);
        assert(f->ml == H);
        assert(f->parent == H);
        rd->pframe = f;
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
        Type*t;
        const int sz = rs->disint;
        Heap *h;
        Array *a;

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        t = rr->ML->type[rm->disint];

        acheck(t->size, sz);
        h = nheap(sizeof(Array) + (t->size*sz));
        h->t = &Tarray;
        Tarray.ref++; /* meaningless? */
        a = H2D(Array*, h);
        a->t = t;
        a->len = sz;
        a->root = (Array*)H;
        a->data = (char*)(a+1);
        initarray(t, a);

        ASSIGN(rd->parray, a);
}
OP(newaz)
{
        Type*t;
        const int sz = rs->disint;
        Heap *h;
        Array *a;

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        t = rr->ML->type[rm->disint];


        acheck(t->size, sz);
        h = nheap(sizeof(Array) + (t->size*sz));
        h->t = &Tarray;
        Tarray.ref++; /* meaningless? */
        a = H2D(Array*, h);
        a->t = t;
        a->len = sz;
        a->root = (Array*)H;
        a->data = (char*)(a+1);
        memset(a->data, 0, t->size*sz);
        initarray(t, a);

        ASSIGN(rd->parray, a);
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
        c->buf = (Array*)H;
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
newc(Type *t, void (*mover)(void*d, void*s, Channel*c), Disdata*rm, Disdata*rd)
{
        DISINT len = 0;

        if(rm != rd){
                len = rm->disint;
                if(len < 0)
                        error(exNegsize);
        }

        ASSIGN(rd->pchannel, cnewc(t, mover, len));
        return rd->pchannel;
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
        ASSIGN(*(void**)d, sv);
}
void
movertmp(void*d, void*s, Channel*c)             /* Used by send & receive */
{
        Type* t = c->mid.t;
        incmem(s, t);
        freeptrs(d, t);
        memcpy(d, s, t->size);
}
static void
moverm(void*d, void*s, Channel*c)               /* Used by send & receive */
{
        memcpy(d, s, c->mid.w);
}


OP(newcl)  { newc(&Tlong, moverl, rm, rd);  }
OP(newcb)  { newc(&Tbyte, moverb, rm, rd);  }
OP(newcw)  { newc(&Tword, moverw, rm, rd);  }
OP(newcf)  { newc(&Treal, moverf, rm, rd);  }
OP(newcp)  { newc(&Tptr, moverp, rm, rd);  }
OP(newcm)
{
        Channel *c;
        Type *t;

        t = nil;
        if(rm != rd && rs->disint > 0)
                t = dtype(nil, rs->disint, nil, 0, "(newcm)");
        c = newc(t, moverm, rm, rd);
        c->mid.w = rs->disint;
        if(t != nil)
                freetype(t);
}
OP(newcmp)
{
        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rs->disint)
                error(exCctovflw);

        newc(rr->ML->type[rs->disint], movertmp, rm, rd);
}
OP(icase)
{
        DISINT *t, *l, d, n, n2;
        const DISINT v = rs->disint;

        t = rd->disints + 1; /* FIXME: declare struct */
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
        if(rr->ML->compiled) {
                rr->PC = (Inst*)d;
                return;
        }
        rr->PC = rr->ML->prog + d;
}
OP(casel)
{
        DISINT *t, *l, d, n, n2;
        const DISBIG v = rs->disbig;

        t = rd->disints + 2; /* FIXME: declare struct */
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
        if(rr->ML->compiled) {
                rr->PC = (Inst*)d;
                return;
        }
        rr->PC = rr->ML->prog + d;
}
OP(casec)
{
        DISINT *l, *t, *e, n, n2, r;
        String *sl, *sh;
        const String * const sv = rs->pstring;

        t = rd->disints + 1; /* FIXME: declare struct */
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
        if(rr->ML->compiled) {
                rr->PC = (Inst*)t[0];
                return;
        }
        rr->PC = rr->ML->prog + t[0];
}
OP(igoto)
{
        DISINT* t = rd->disints + rs->disint;
        if(rr->ML->compiled) {
                rr->PC = (Inst*)t[0]; /* FIXME: check index and new PC */
        }
        else {
                rr->PC = rr->ML->prog + t[0]; /* FIXME: check index and new PC */
        }
}
OP(call)
{
        Frame *f = rs->pframe;

        assert(f->parent == H); /* virginity check, just curious */
        assert(f->ml == H);
        f->lr = rr->PC;
        f->parent = rr->FP;
        rr->FP = f;
        rr->PC = rd->pinst;
}
OP(mcall)
{
        Prog *p;
        Frame *f = rs->pframe;
        Linkpc *l;
        Modlink *ml = rd->pmodlink;
        int o;

        if(ml == H)
                error(exModule);
        f->lr = rr->PC;
        assert(f->parent == H); /* virginity check, just curious */
        f->parent = rr->FP;
        assert(f->parent != H);
        f->ml = rr->ML;
        assert(f->ml != nil);
        assert(f->ml != H);

        rr->FP = f;
        rr->ML = ml;
        ADDREF(ml);

        o = rm->disint;
        if(o >= 0) {
                if(o >= ml->nlinks)
                        error("invalid mframe");
                l = &ml->links[o].u;
        }
        else {
                l = &ml->m->ext[-o-1].u;   /* TODO: check index range */
        }

        if(ml->prog == nil) { /* build-in module */
                l->runt(f);

                DELREF(ml);
                assert(f->parent!=nil);
                rr->ML = f->ml;
                rr->FP = f->parent;
                ADDREF(f->parent);
                ADDREF(f->ml);
                Setmark(D2H(rr->FP));
                Setmark(D2H(rr->ML));

                ASSIGN(f, H);
                p = currun();
                if(p->kill != nil)
                        error(p->kill);
        }
        else {
                rr->PC = l->pc;

                if(f->ml->compiled != rr->ML->compiled)
                        rr->IC = 1;
        }
}
OP(spawn)
{
        Prog *p = newprog(currun(), rr->ML);

        p->R.PC = rd->pinst;
        p->R.FP = rs->pframe;
}
OP(mspawn)
{
        Prog *p;
        Modlink *ml = rd->pmodlink;
        int o;

        if(ml == H)
                error(exModule);
        if(ml->prog == nil)
                error(exSpawn);
        p = newprog(currun(), ml);
        o = rm->disint;
        if(o >= 0) {
                if(o >= ml->nlinks)
                        error("invalid mframe");
                p->R.PC = ml->links[o].u.pc;
        }
        else {
                p->R.PC = ml->m->ext[-o-1].u.pc;   /* TODO: check index range */
        }
        p->R.FP = rs->pframe;
}
OP(ret)
{
        Frame *f = rr->FP;

        if(f->parent == H) {
                error(""); /* 'stack' underflow, exit */
        }
        rr->PC = f->lr;

        /* return from mcall */
        if(f->ml != H) {
                if(rr->ML->compiled != f->ml->compiled) {
                        rr->IC = 1;
                }
                ADDREF(f->ml);
                Setmark(D2H(f->ml));
                ASSIGN(rr->ML, f->ml);
        }
        ADDREF(f->parent); /* protect parent frame from being destroyed */
        Setmark(D2H(f->parent));
        ASSIGN(rr->FP, f->parent);
}
OP(iload)
{
        char *n = string2c(rs->pstring);
        Module *m = rr->ML->m;
        Import *ldt;
        Modlink *ml;

        if(!(m->rt & HASLDT)) {
                error("obsolete dis");
        }
        ldt = m->ldt[rm->disint]; /* TODO: check index range */

        if(strcmp(n, "$self") == 0) {
                m->ref++;
                ml = linkmod(m, ldt, 0);
                if(ml != H) {
                        ml->MP = rr->ML->MP;
                        ADDREF(ml->MP);
                        Setmark(D2H(ml->MP));
                }
        }
        else {
                m = readmod(n, lookmod(n), 1);
                ml = linkmod(m, ldt, 1);
        }

        ASSIGN(rd->pmodlink, ml);
}
OP(lena)
{
        Array *a = rs->parray;
        DISINT l = 0;

        if(a != H)
                l = a->len;
        rd->disint = l;
}
OP(lenl)
{
        List *a = rs->plist;
        DISINT l = 0;

        while(a != H) {
                l++;
                a = a->tail;
        }
        rd->disint = l;
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
                p->chan = c;    /* for killprog */
                cqadd(&c->send, p);
                return 1;
        }

        if(c->buf != H && c->size > 0)
                print("non-empty buffer in isend\n");

        cqdel(&c->recv);
        if(p->state == Palt)
                altdone(p->aaa, p, c, 1);

        c->mover(p->ptr, v, c);
        p->ptr = nil;
        addrun(p);
        return 0;
}
OP(isend)
{
        Channel *c = rd->pchannel;

        if(c == H)
                error(exNilref);
        if(_isend(c, rs))
                rr->IC = 1;
}
int _irecv(Channel *c, void* v)
{
        Prog *p = c->send->prog;

        if(p == nil) {
                if(c->buf != H && cgetb(c, v))
                        return 0;
                p = delrun(Precv);
                p->ptr = v;
                p->chan = c;    /* for killprog */
                cqadd(&c->recv, p);
                return 1;
        }

        if(c->buf != H && c->size != c->buf->len)
                print("non-full buffer in irecv\n");

        cqdel(&c->send);
        if(p->state == Palt)
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
        Channel *c = rs->pchannel;

        if(c == H)
                error(exNilref);
        if(_irecv(c, rd))
                rr->IC = 1;
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
                R.IC = 1; /*!*/
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
                Setmark(D2H(lv));
        }
        l->tail = lv;
        *lp = l;
        return l;
}
OP(consb)
{
        List *l = cons(sizeof(DISBYTE), &rd->plist);

        l->data.disbyte = rs->disbyte;
}
OP(consw)
{
        List *l = cons(sizeof(DISINT), &rd->plist);

        l->data.disint = rs->disint;
}
OP(consl)
{
        List *l = cons(sizeof(DISBIG), &rd->plist);

        l->data.disbig = rs->disbig;
}
OP(consp)
{
        List *l = cons(sizeof(void*), &rd->plist);
        void *sv = rs->pvoid;

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

        l = cons(sizeof(DISREAL), &rd->plist);
        l->data.disreal = rs->disreal;
}
OP(consm)
{
        DISINT v = rm->disint;
        List *l = cons(v, &rd->plist);
        memcpy(&l->data, rs, v);
}
OP(consmp)
{
        Type *t;
        List *l;

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        t = rr->ML->type[rm->disint];
        l = cons(t->size, &rd->plist);

        incmem(rs, t);
        memcpy(&l->data, rs, t->size);
        l->t = t;
        t->ref++;
}
OP(headb)
{
        List *l = rs->plist;

        if(l == H)
                error(exNilref);
        rd->disbyte = l->data.disbyte;
}
OP(headw)
{
        List *l = rs->plist;

        if(l == H)
                error(exNilref);
        rd->disint = l->data.disint;
}
OP(headl)
{
        List *l = rs->plist;

        if(l == H)
                error(exNilref);
        rd->disbig = l->data.disbig;
}
OP(headp)
{
        List *l = rs->plist;
        void *sv;

        if(l == H)
                error(exNilref);

        //was rs = &l->data; movp();
        sv = l->data.pvoid;
        if(sv != H) {
                ADDREF(sv);
                Setmark(D2H(sv));
        }
        ASSIGN(rd->pvoid, sv);
}
OP(headf)
{
        List *l = rs->plist;

        if(l == H)
                error(exNilref);
        rd->disreal = l->data.disreal;
}
OP(headm)
{
        List *l = rs->plist;

        if(l == H)
                error(exNilref);
        memcpy(rd, &l->data, rm->disint);
}
OP(headmp)
{
        List *l = rs->plist;
        Type *t;

        if(l == H)
                error(exNilref);

        if((DISUINT)rr->ML->m->ntype <= (DISUINT)rm->disint)
                error(exCctovflw);

        t = rr->ML->type[rm->disint];
        incmem(&l->data, t);
        freeptrs(rd, t);
        memcpy(rd, &l->data, t->size);

}
OP(tail)
{
        List *l = rs->plist;
        List *sv;

        if(l == H)
                error(exNilref);

        //was rs = (Disdata*) &l->tail; movp();
        sv = l->tail;
        if(sv != H) {
                ADDREF(sv);
                Setmark(D2H(sv));
        }
        ASSIGN(rd->plist, sv);
}
OP(slicea)
{
        Type *t;
        Heap *h;
        Array *ss, *ds;
        int v, n, start;

        v = rm->disint;
        start = rs->disint;
        n = v - start;
        ds = rd->parray;

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

        if(ds->root != H) {                     /* slicing a slice */
                ds = ds->root;
                ADDREF(ds);
                ASSIGN(rd->parray, ss);
                ss->root = ds;
        }
        else {
                ss->root = ds;
                rd->parray = ss;
        }
        Setmark(D2H(ds));
}
OP(slicela)
{
        Type *t;
        int l, dl;
        Array *ss, *ds;
        char *sp, *dp, *ep;

        ss = rs->parray;
        dl = rm->disint;
        ds = rd->parray;
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
        if(xecalt(1, &rs->alt, &rd->disint))
                rr->IC = 1;
}
OP(nbalt)
{
        if(xecalt(0, &rs->alt, &rd->disint))
                rr->IC = 1;
}
OP(tcmp)
{
        const void * const s = rs->pvoid;
        const void * const d = rd->pvoid;

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
        v = rs->pvoid;
        if(v == H)
                error(exNilref);
        p->exval = (String*)v; /* FIXME */
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

        p = rr->FP->dtmp;
        r = (DISBIG)rm->disint*(DISBIG)rs->disint;
        if(p >= 0)
                r <<= p;
        else
                r >>= (-p);
        rd->disint = (DISINT)r;
}
OP(divx)
{
        DISINT p;
        DISBIG s;

        p = rr->FP->dtmp;
        s = (DISBIG)rm->disint;
        if(p >= 0)
                s <<= p;
        else
                s >>= (-p);
        s /= (DISBIG)rs->disint;
        rd->disint = (DISINT)s;
}
OP(cvtxx)
{
        DISINT p;
        DISBIG r;

        p = rm->disint;
        r = (DISBIG)rs->disint;
        if(p >= 0)
                r <<= p;
        else
                r >>= (-p);
        rd->disint = (DISINT)r;
}
OP(mulx0)
{
        DISINT x, y, p, a;
        DISBIG r;

        x = rm->disint;
        y = rs->disint;
        p = rr->FP->dtmp;
        a = rr->FP->stmp;
        if(x == 0 || y == 0){
                rd->disint = 0;
                return;
        }
        r = (DISBIG)x*(DISBIG)y;
        if(p >= 0)
                r <<= p;
        else
                r >>= (-p);
        r /= (DISBIG)a;
        rd->disint = (DISINT)r;
}
OP(divx0)
{
        DISINT x, y, p, b;
        DISBIG s;

        x = rm->disint;
        y = rs->disint;
        p = rr->FP->dtmp;
        b = rr->FP->stmp;
        if(x == 0){
                rd->disint = 0;
                return;
        }
        s = (DISBIG)b*(DISBIG)x;
        if(p >= 0)
                s <<= p;
        else
                s >>= (-p);
        s /= (DISBIG)y;
        rd->disint = (DISINT)s;
}
OP(cvtxx0)
{
        DISINT x, p, a;
        DISBIG r;

        x = rs->disint;
        p = rm->disint;
        a = rr->FP->stmp;
        if(x == 0){
                rd->disint = 0;
                return;
        }
        r = (DISBIG)x;
        if(p >= 0)
                r <<= p;
        else
                r >>= (-p);
        r /= (DISBIG)a;
        rd->disint = (DISINT)r;
}
OP(mulx1)
{
        DISINT x, y, p, a, v;
        int vnz, wnz;
        DISBIG w, r;

        x = rm->disint;
        y = rs->disint;
        p = rr->FP->dtmp;
        a = rr->FP->stmp;
        if(x == 0 || y == 0){
                rd->disint = 0;
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
        rd->disint = (DISINT)r;
}
OP(divx1)
{
        DISINT x, y, p, b, v;
        int vnz, wnz;
        DISBIG w, s;

        x = rm->disint;
        y = rs->disint;
        p = rr->FP->dtmp;
        b = rr->FP->stmp;
        if(x == 0){
                rd->disint = 0;
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
        rd->disint = (DISINT)s + v;
}
OP(cvtxx1)
{
        DISINT x, p, a, v;
        int vnz, wnz;
        DISBIG w, r;

        x = rs->disint;
        p = rm->disint;
        a = rr->FP->stmp;
        if(x == 0){
                rd->disint = 0;
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
        rd->disint = (DISINT)r;
}
/*
OP(cvtxx)
{
        REAL v;

        v = (REAL)rs->disint*rm->disreal;
        v = v < 0 ? v-0.5: v+0.5;
        rd->disint = (WORD)v;
}
*/
OP(cvtfx)
{
        DISREAL v;

        v = rs->disreal*rm->disreal;
        v = v < 0 ? v-0.5: v+0.5;
        rd->disint = (DISINT)v;
}
OP(cvtxf)
{
        rd->disreal = (DISREAL)rs->disint*rm->disreal;
}

OP(self)
{
        Modlink *ml = rr->ML;

        ADDREF(ml);
        Setmark(D2H(ml));
        ASSIGN(rd->pmodlink, ml);
}

#undef OP
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



extern void cvtca(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtac(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtwc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtcw(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtfc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtcf(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void insc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void indc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void addc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void lenc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void slicec(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);
extern void cvtlc(Disdata*rs, Disdata*rm, Disdata*rd, REG*rr);

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
extern Runtab Sysmodtab[];
// print op operand state before executing
void statebefore(char o[200], char q[200], uchar op, Inst* modprog, Disdata*rs, Disdata*rm, Disdata*rd, REG*rr)
{
#define CURM (rr->ML->m->name)
#define I(r) (r->pinst - modprog)
#define Alen(r) (r->parray==H?0:r->parray->len)

        const n = 200;
        o[0] = '\0';
        q[0] = '\0';
        switch(op)
        {
        case ICALL:
        case IJMP:
        case IBEQB:
        case IBNEB:
        case IBLTB:
        case IBLEB:
        case IBGTB:
        case IBGEB:
        case IBEQW:
        case IBNEW:
        case IBLTW:
        case IBLEW:
        case IBGTW:
        case IBGEW:
        case IBEQL:
        case IBNEL:
        case IBLTL:
        case IBLEL:
        case IBGTL:
        case IBGEL:
        case IBEQF:
        case IBNEF:
        case IBLTF:
        case IBLEF:
        case IBGTF:
        case IBGEF:
        case IBEQC:
        case IBNEC:
        case IBLTC:
        case IBLEC:
        case IBGTC:
        case IBGEC:
                snprint(q,n,"%s_%uX", CURM, I(rd));
                break;
        }
        switch(op)
        {
        case IEXIT:
        case IECLR:
        case ISELF:
        case INOP:
        case IRUNT:
        case IRET:      break;
        case ICVTBW:    // monadic ops
        case IMOVB:     snprint(o,n,"%d", rs->disbyte); break;
        case ICVTSW:    snprint(o,n,"%d", rs->disint16); break;
        case ICVTWB:
        case ICVTWS:
        case ICVTWL:
        case ICVTWF:
        case ICVTWC:
        case IMOVW:     snprint(o,n,"%d", rs->disint); break;
        case ICVTLF:
        case ICVTLW:
        case ICVTLC:
        case IMOVL:     snprint(o,n,"0x%llX", rs->disbig); break;
        case ICVTCL:
        case ICVTCA:
        case ICVTCW:
        case ICVTCF:
        case ILENC:     snprint(o,n,"\"%s\"", string2c(rs->pstring)); break;
        case ICVTFR:
        case ICVTFW:
        case ICVTFL:
        case ICVTFC:
        case INEGF:
        case IMOVF:     snprint(o,n,"%f", rs->disreal); break;
        case ICVTRF:    snprint(o,n,"%f", rs->disreal32); break;
        case ICVTAC:
        case ILENA:     snprint(o,n,"Array[%d]@%p", Alen(rs), rs->parray); break;
        case ILEA:      snprint(o,n,"%p", rs); break;
        case IALT:      /* tbd: show structure bihing the pointer */
        case INBALT:    /* tbd: show structure bihing the pointer */
        case IRAISE:    /* tbd: show structure bihing the pointer */
        case IMOVP:     snprint(o,n,"%p", rs->pvoid); break;
        case IFRAME:    snprint(o,n,"Type_%s_%d", CURM, rs->disint); break;
        case IBEQB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d == %d)",         rs->disbyte, rm->disbyte); break;  // diadic and triadic
        case IBNEB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d != %d)",         rs->disbyte, rm->disbyte); break;
        case IBLTB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d <  %d)",         rs->disbyte, rm->disbyte); break;
        case IBLEB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d <= %d)",         rs->disbyte, rm->disbyte); break;
        case IBGTB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d >  %d)",         rs->disbyte, rm->disbyte); break;
        case IBGEB:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d >= %d)",         rs->disbyte, rm->disbyte); break;
        case IBEQW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d == %d)",         rs->disint,  rm->disint ); break;
        case IBNEW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d != %d)",         rs->disint,  rm->disint ); break;
        case IBLTW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d <  %d)",         rs->disint,  rm->disint ); break;
        case IBLEW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d <= %d)",         rs->disint,  rm->disint ); break;
        case IBGTW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d >  %d)",         rs->disint,  rm->disint ); break;
        case IBGEW:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%d >= %d)",         rs->disint,  rm->disint ); break;
        case IBEQL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX == 0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBNEL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX != 0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBLTL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX <  0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBLEL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX <= 0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBGTL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX >  0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBGEL:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(0x%llX >= 0x%llX)", rs->disbig,  rm->disbig ); break;
        case IBEQF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f == %f)",         rs->disreal, rm->disreal); break;
        case IBNEF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f != %f)",         rs->disreal, rm->disreal); break;
        case IBLTF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f <  %f)",         rs->disreal, rm->disreal); break;
        case IBLEF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f <= %f)",         rs->disreal, rm->disreal); break;
        case IBGTF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f >  %f)",         rs->disreal, rm->disreal); break;
        case IBGEF:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(%f >= %f)",         rs->disreal, rm->disreal); break;
        case IBEQC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" == \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IBNEC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" != \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IBLTC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" <  \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IBLEC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" <= \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IBGTC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" >  \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IBGEC:     snprint(q,n,"%s_%uX", CURM, I(rd)); snprint(o,n,"jmp if(\"%s\" >= \"%s\")", string2c(rs->pstring), string2c(rm->pstring)); break;
        case IADDB:     snprint(o,n,"%d + %d", rm->disbyte, rs->disbyte); break;
        case IADDW:     snprint(o,n,"%d + %d", rm->disint, rs->disint); break;
        case IADDL:     snprint(o,n,"0x%llX + 0x%llX", rm->disbig, rs->disbig); break;
        case IADDF:     snprint(o,n,"%f + %f", rm->disreal, rs->disreal); break;
        case IADDC:     snprint(o,n,"\"%s\" + \"%s\"", string2c(rm->pstring), string2c(rs->pstring)); break;
        case ISUBB:     snprint(o,n,"%d - %d", rm->disbyte, rs->disbyte); break;
        case ISUBW:     snprint(o,n,"%d - %d", rm->disint, rs->disint); break;
        case ISUBL:     snprint(o,n,"0x%llX - 0x%llX", rm->disbig, rs->disbig); break;
        case ISUBF:     snprint(o,n,"%f - %f", rm->disreal, rs->disreal); break;
        case IMULB:     snprint(o,n,"%d * %d", rm->disbyte, rs->disbyte); break;
        case IMULW:     snprint(o,n,"%d * %d", rm->disint, rs->disint); break;
        case IMULL:     snprint(o,n,"0x%llX * 0x%llX", rm->disbig, rs->disbig); break;
        case IMULF:     snprint(o,n,"%f * %f", rm->disreal, rs->disreal); break;
        case IDIVB:     snprint(o,n,"%d / %d", rm->disbyte, rs->disbyte); break;
        case IDIVW:     snprint(o,n,"%d / %d", rm->disint, rs->disint); break;
        case IDIVL:     snprint(o,n,"0x%llX / 0x%llX", rm->disbig, rs->disbig); break;
        case IDIVF:     snprint(o,n,"%f / %f", rm->disreal, rs->disreal); break;
        case IANDB:     snprint(o,n,"%d & %d", rm->disbyte, rs->disbyte); break;
        case IANDW:     snprint(o,n,"%d & %d", rm->disint, rs->disint); break;
        case IANDL:     snprint(o,n,"0x%llX & 0x%llX", rm->disbig, rs->disbig); break;
        case IORB:      snprint(o,n,"%d | %d", rm->disbyte, rs->disbyte); break;
        case IORW:      snprint(o,n,"%d | %d", rm->disint, rs->disint); break;
        case IORL:      snprint(o,n,"0x%llX | 0x%llX", rm->disbig, rs->disbig); break;
        case IXORB:     snprint(o,n,"%d ^ %d", rm->disbyte, rs->disbyte); break;
        case IXORW:     snprint(o,n,"%d ^ %d", rm->disint, rs->disint); break;
        case IXORL:     snprint(o,n,"0x%llX ^ 0x%llX", rm->disbig, rs->disbig); break;
        case IMODB:     snprint(o,n,"%d %% %d", rm->disbyte, rs->disbyte); break;
        case IMODW:     snprint(o,n,"%d %% %d", rm->disint, rs->disint); break;
        case IMODL:     snprint(o,n,"0x%llX %% 0x%llX", rm->disbig, rs->disbig); break;
        case ISHLB:     snprint(o,n,"%d << %d", rm->disbyte, rs->disint); break;
        case ISHLW:     snprint(o,n,"%d << %d", rm->disint, rs->disint); break;
        case ISHLL:     snprint(o,n,"0x%llX << %d", rm->disbig, rs->disint); break;
        case ISHRB:     snprint(o,n,"%d >> %d", rm->disbyte, rs->disint); break;
        case ISHRW:     snprint(o,n,"%d >> %d", rm->disint, rs->disint); break;
        case ISHRL:     snprint(o,n,"0x%llX >> %d", rm->disbig, rs->disint); break;
        case ILSRW:     snprint(o,n,"%ud >>> %d", (DISUINT)rm->disint, rs->disint); break;
        case ILSRL:     snprint(o,n,"0x%lluX >>> %d", (DISUBIG)rm->disbig, rs->disint); break;
        case IEXPW:     snprint(o,n,"%d %d", rm->disint, rs->disint); break;
        case IEXPL:     snprint(o,n,"0x%llX %d", rm->disbig, rs->disint); break;
        case IEXPF:     snprint(o,n,"%f %d", rm->disreal, rs->disint); break;
        case ICVTXF:    snprint(o,n,"%f %d", rm->disreal, rs->disint); break;
        case ICVTFX:    snprint(o,n,"%f %f", rm->disreal, rs->disreal); break;
        case ICVTXX:
        case ICVTXX0:
        case ICVTXX1:
        case IMULX:
        case IMULX0:
        case IMULX1:
        case IDIVX:
        case IDIVX0:
        case IDIVX1:    snprint(o,n,"%d %d", rm->disint, rs->disint); break;
        case IINDF:
        case IINDB:
        case IINDW:
        case IINDL:
        case IINDX:     snprint(o,n,"Array[%d]@%p [%d]", Alen(rs), rs->parray, rd->disint); break;
        case IINDC:     snprint(o,n,"\"%s\" [%d]", string2c(rs->pstring), rm->disint); break;
        case IINSC:     snprint(o,n,"\"%s\" [%d] = %d", string2c(rd->pstring), rm->disint, rs->disint); break;
        case ISLICEA:   snprint(o,n,"Array[%d]@%p [%d:%d]", Alen(rd), rd->parray, rs->disint, rm->disint); break;
        case ISLICEC:   snprint(o,n,"\"%s\" [%d:%d]", string2c(rd->pstring), rs->disint, rm->disint); break;
        case ISLICELA:  snprint(o,n,"Array[%d]@%p [%d:] = Array[%d]@%p", Alen(rd), rd->parray, rm->disint, Alen(rs), rs->parray); break;
        case IMSPAWN:
        case IMCALL:    //if(0==strcmp(rd->pmodlink->m->name, "$Sys"))
                        //    snprint(o,n,"%s.%s Frame@%p FP=Frame@%p", rd->pmodlink->m->name, Sysmodtab[rm->disint].name, rs->pframe, rr->FP);
                        //else
                            snprint(o,n,"%s.%d Frame@%p FP=Frame@%p", rd->pmodlink->m->name, rm->disint, rs->pframe, rr->FP);
                        break;
        case IMFRAME:   snprint(o,n,"%s.%d", rs->pmodlink->m->name, rm->disint); break;
        case ISPAWN:
        case ICALL:     snprint(o,n,"Frame@%p FP=Frame@%p", rs->pframe, rr->FP); break;
        case IJMP:      break;
        case IGOTO:     snprint(o,n,"%s_%uX", CURM, (DISINT*)((DISINT)rd + (rs->disint * sizeof(DISINT)))); break;
        case IMOVPC:    snprint(o,n,"%s_%uX", CURM, I(rs)); break;
        case INEW:
        case INEWZ:     snprint(o,n,"%d", rs->disint); break;
        case IMNEWZ:    snprint(o,n,"Type_%s_%d", rs->pmodlink->m->name, rm->disint); break;
        case INEWA:
        case INEWAZ:    snprint(o,n,"Type_%s_%d [%d]", CURM, rm->disint, rs->disint); break;
        case INEWCB:    // new channel
        case INEWCW:
        case INEWCF:
        case INEWCP:
        case INEWCL:    break;
        case INEWCM:    snprint(o,n,"[%d]", rs->disint); break;
        case INEWCMP:   snprint(o,n,"Type_%s_%d", CURM, rs->disint); break;
        case ISEND:     snprint(o,n,"Channel@%p <-= %p", rd->pchannel, rs->pvoid); break;
        case IRECV:     snprint(o,n,"Channel@%p", rs->pchannel); break;
        case ICONSB:    snprint(o,n,"%d :: List@%p", rs->disbyte, rd->plist); break;    // list
        case ICONSW:    snprint(o,n,"%d :: List@%p", rs->disint, rd->plist); break;
        case ICONSF:    snprint(o,n,"%f :: List@%p", rs->disreal, rd->plist); break;
        case ICONSL:    snprint(o,n,"0x%llx :: List@%p", rs->disbig, rd->plist); break;
        case ICONSP:    snprint(o,n,"%p :: List@%p", rs->pvoid, rd->plist); break;
        case ICONSM:    snprint(o,n,"%p [%d] :: List@%p", rs->pvoid, rm->disint, rd->plist); break;
        case ICONSMP:   snprint(o,n,"Type_%s_%d@%p :: List@%p", CURM, rm->disint, rs->pvoid, rd->plist); break;
        case ITAIL:
        case ILENL:
        case IHEADB:
        case IHEADW:
        case IHEADP:
        case IHEADF:
        case IHEADL:
        case IHEADMP:   snprint(o,n,"List@%p", rs->plist); break;
        case IHEADM:    snprint(o,n,"List@%p [%d]", rs->plist, rm->disint); break;
        case IMOVM:     snprint(o,n,"memmove(%p, %p, %d)", rd, rs, rm->disint); break;
        case IMOVMP:    snprint(o,n,"Type_%s_%d@%p", CURM, rm->disint, rs); break;
        case ITCMP:     snprint(o,n,"%p %p", rm->pvoid, rs->pvoid); break;
        case ILOAD:     snprint(o,n,"\"%s\" %d", string2c(rs->pstring), rm->disint); break;
        case ICASE:     snprint(o,n,"%d %p", rs->disint, rd->pvoid); break;                             /* FIXME: show structure bihing the pointer */
        case ICASEC:    snprint(o,n,"\"%s\" %p", string2c(rs->pstring), rd->pvoid); break;      /* FIXME: show structure bihing the pointer */
        case ICASEL:    snprint(o,n,"0x%llX %p", rs->disbig, rd->pvoid); break;         /* FIXME: show structure bihing the pointer */
        }
}

// print values affected by op
void stateafter(char* o, int n, uchar op, Disdata*rs, Disdata*rm, Disdata*rd, REG*rr)
{
        o[0] = '\0';
        switch(op)
        {
//      case IRET: {
//      Frame*f = (Frame*)rr->FP;
//      snprint(o,n," => %s_%uX", (f&&f->mr&&f->mr->m)?f->mr->m->name:"", f?f->lr:-1 /*- f->mr->m->prog*/);
//      }
//      break;
        case IINDF:     snprint(o,n," => %f", rm->disreal); break; // result in middle
        case IINDB:     snprint(o,n," => %d", rm->disbyte); break;
        case IINDW:     snprint(o,n," => %d", rm->disint); break;
        case IINDL:     snprint(o,n," => 0x%llx", rm->disbig); break;
        case IINDX:     snprint(o,n," => %p", rm->pvoid); break;
        case ILOAD:     // module
        case ISELF:     snprint(o,n," => Module(%s)", rd->pmodlink==H?"nil":rd->pmodlink->m->name); break;
        case INEW:      // pointer
        case INEWZ:
        case IMNEWZ:
        case IMOVP:
        case IMOVMP:
        case ILEA:
        case IHEADP:
        case IHEADMP:
        case IHEADM:
        case IRECV:
        case IMOVPC:    snprint(o,n," => %p", rd->pvoid); break;
        case ICONSB:    // list
        case ICONSW:
        case ICONSF:
        case ICONSL:
        case ICONSP:
        case ICONSM:
        case ICONSMP:
        case ITAIL:     snprint(o,n," => List@%p", rd->plist); break;
        case INEWA:     // array
        case INEWAZ:
        case ICVTCA:
        case ISLICEA:   snprint(o,n," => Array[%d]@%p", Alen(rd), rd->parray); break;
        case INEWCB:    // channel
        case INEWCW:
        case INEWCF:
        case INEWCP:
        case INEWCL:
        case INEWCM:
        case INEWCMP:   snprint(o,n," => Channel@%p", rd->pchannel); break;
        case ICVTFR:    snprint(o,n," => %f", rd->disreal32); break; // short float
        case ICVTRF:    // float
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
        case IHEADF:    snprint(o,n," => %f", rd->disreal);     break;
        case ICVTWB:    // byte
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
        case IHEADB:    snprint(o,n," => %d", rd->disbyte);     break;
        case ICVTWS:    snprint(o,n," => %d", rd->disint16);    break; // short
        case IINDC:     // int
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
        case INBALT:    snprint(o,n," => %d", rd->disint);      break;
        case ICVTFX:    // X?
        case IMULX:
        case IMULX0:
        case IMULX1:
        case IDIVX:
        case IDIVX0:
        case IDIVX1:
        case ICVTXX:
        case ICVTXX0:
        case ICVTXX1:   snprint(o,n," => %d", rd->disint);      break;
        case ICVTFL:    // big
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
        case IHEADL:    snprint(o,n," => 0x%llx", rd->disbig);break;
        case ICVTLC:    // string
        case ICVTAC:
        case ICVTWC:
        case ICVTFC:
        case ISLICEC:
        case IADDC:     snprint(o,n," => \"%s\"", string2c(rd->pstring));       break;
        case IMFRAME:
        case IFRAME:    snprint(o,n," => Frame@%p", rd->pframe);        break;
        }
}
#endif

struct timerdata {
    vlong start, num, tmin, tmax, ttotal;
};

void timer_begin(struct timerdata* t)
{
    assert(t->start==0);
    t->start = rdtsc();
}
vlong timer_end(struct timerdata* t)
{
    vlong stop, delta;
    assert(t->start!=0);
    stop = rdtsc();
    delta = stop - t->start;
    if(t->tmin==0 || delta<t->tmin) t->tmin = delta;
    if(delta>t->tmax) t->tmax = delta;
    t->ttotal += delta;
    t->num ++;
    t->start = 0;
    return delta;
}
void timer_print(struct timerdata* t)
{
    print("num=%lld total=%lld ave=%lld min=%lld max=%lld\n", t->num, t->ttotal, t->num?t->ttotal/t->num:0, t->tmin, t->tmax);
}

void
xec(Prog *p)
{
    struct timerdata tcmd = {0};
        int op;

        R = p->R;
        R.IC = p->quanta;

        if(p->kill != nil) {
                char *m;
                m = p->kill;
                p->kill = nil;
                error(m);
        }

// print("%lux %lux %lux %lux %lux\n", (ulong)&R, R.xpc, R.FP, R.MP, R.PC);

        if(R.ML->compiled)
        {
                printf("%s %d\n", __FUNCTION__, __LINE__);

                /* BUG */
#if JIT
                comvec();
#endif
        } else do {

                Dec d;

                dec(&d, R.PC, R.ML->MP, R.FP); /* TODO: inline here? */

                op = R.PC->op;
#ifdef DEBUGVM
                {
                char sz[100];
                char szinst[100];
                char sz2[200], szdest[200];
                if(op==IMCALL)
                {
                    snprint(sz, sizeof(sz), "%s_%uX:", R.ML->m->name, R.PC - R.ML->m->prog );

                    statebefore(sz2, szdest, op, R.ML->m->prog, d.s, d.m, d.d, &R);
                    sprint(szinst, "%D", R.PC);
                    if(szdest[0])
                    {
                            char *p = strrchr(szinst, '$');
                            assert(p);
                            strcpy(p, szdest);
                    }
/*
                    print("%-16s %02ux %02ux %04ux %08ux",
                            sz,
                            R.PC->op, R.PC->add, R.PC->reg, R.PC->s.imm);
                    if(szdest[0])
                            print(" ________");
                    else
                            print(" %08ux", R.PC->d.imm);
                    print(" %-40s\t%s",
                            szinst, sz2);
*/
                }
#endif
                R.PC++;
#ifdef DEBUGVM
                if(op==IMCALL){
                    timer_begin(&tcmd);
                    if(0==strcmp(d.d->pmodlink->m->name, "$Sys") && d.m->disint==4)
                    {
                        DebugBreak();
                    }
                }
#endif
                optab[op](d.s, d.m, d.d, &R); /* TODO: inline here? */
#ifdef DEBUGVM
                if(op==IMCALL){

                    print("%-16s %02ux %02ux %04ux %08ux",
                            sz,
                            R.PC[-1].op, R.PC[-1].add, R.PC[-1].reg, R.PC[-1].s.imm);
                    if(szdest[0])
                            print(" ________");
                    else
                            print(" %08ux", R.PC[-1].d.imm);
                    print(" %-40s\t%s",
                            szinst, sz2);

                    {
                    vlong delta;
                    delta = timer_end(&tcmd);
                    if(delta>100000000)
                        print(" **[%lld] ", delta);
                    else
                        print(" [%lld] ", delta);
                    }
                }
#endif
#ifdef DEBUGVM
                if(op==IMCALL)
                {
                stateafter(sz2, sizeof(sz2), op, d.s, d.m, d.d, &R);
                print("%s\n", sz2);
                }
                }
#endif
        } while(--R.IC != 0);

        p->R = R;
        //timer_print(&tcmd);
}
