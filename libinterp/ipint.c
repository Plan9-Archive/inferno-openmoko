#include <lib9.h>
#include <kernel.h>
#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <mp.h>
#include <libsec.h>
#include <pool.h>
#include <keys.h>
#include <raise.h>

extern Type	*TIPint;
#define	MP(x)	checkIPint((x))

Keyring_IPint*
newIPint(mpint* b)
{
	Heap *h;
	IPint *ip;

	if(b == nil)
		error(exHeap);
	h = heap(TIPint);	/* TO DO: caller might lose other values if heap raises error here */
	ip = H2D(IPint*, h);
	ip->b = b;
	return (Keyring_IPint*)ip;
}

mpint*
checkIPint(Keyring_IPint *v)
{
	IPint *ip;

	ip = (IPint*)v;
	if(ip == H || ip == nil)
		error(exNilref);
	if(D2H(ip)->t != TIPint)
		error(exType);
	return ip->b;
}

void
freeIPint(Heap *h, int swept)
{
	IPint *ip;

	USED(swept);
	ip = H2D(IPint*, h);
	if(ip->b)
		mpfree(ip->b);
	freeheap(h, 0);
}

DISAPI(IPint_iptob64z)
{
	mpint *b;
	char buf[MaxBigBytes];	/* TO DO: should allocate these */
	uchar *p;
	int n, o;

	b = MP(f->i);
	n = (b->top+1)*Dbytes;
	p = (uchar*)malloc(n+1);
	if(p == nil)
		error(exHeap);
	n = mptobe(b, p+1, n, nil);
	if(n < 0){
		free(p);
		ASSIGN(*f->ret, (String*)H);
		return;
	}
	p[0] = 0;
	if(n != 0 && (p[1]&0x80)){
		/* force leading 0 byte for compatibility with older representation */
		o = 0;
		n++;
	}else
		o = 1;
	enc64(buf, sizeof(buf), p+o, n);
	retstr(buf, f->ret);
	free(p);
}

DISAPI(IPint_iptob64)
{
	char buf[MaxBigBytes];

	mptoa(MP(f->i), 64, buf, sizeof(buf));
	retstr(buf, f->ret);
}

DISAPI(IPint_iptobytes)
{
	uchar buf[MaxBigBytes];

	ASSIGN(*f->ret, H);

	/* TO DO: two's complement or have ipmagtobe? */
	*f->ret = mem2array(buf, mptobe(MP(f->i), buf, sizeof(buf), nil));	/* for now we'll ignore sign */
}

DISAPI(IPint_iptobebytes)
{
	uchar buf[MaxBigBytes];

	ASSIGN(*f->ret, H);

	*f->ret = mem2array(buf, mptobe(MP(f->i), buf, sizeof(buf), nil));
}

DISAPI(IPint_iptostr)
{
	char buf[MaxBigBytes];

	mptoa(MP(f->i), f->base, buf, sizeof(buf));
	retstr(buf, f->ret);
}

static Keyring_IPint*
strtoipint(String *s, int base)
{
	char *p, *q;
	mpint *b;

	p = string2c(s);
	b = strtomp(p, &q, base, nil);
	if(b == nil)
		return (Keyring_IPint*)H;
	while(*q == '=')
		q++;
	if(q == p || *q != 0){
		mpfree(b);
		return (Keyring_IPint*)H;
	}
	return newIPint(b);
}

DISAPI(IPint_b64toip)
{
	ASSIGN(*f->ret, H);

	*f->ret = strtoipint(f->str, 64);
}

DISAPI(IPint_bytestoip)
{
	mpint *b;

	ASSIGN(*f->ret, H);

	if(f->buf == H)
		error(exNilref);

	b = betomp((const uchar *)f->buf->data, f->buf->len, nil);	/* for now we'll ignore sign */ /*XXX*/
	*f->ret = newIPint(b);
}

DISAPI(IPint_bebytestoip)
{
	mpint *b;

	ASSIGN(*f->ret, H);

	if(f->mag == H)
		error(exNilref);

	b = betomp((const uchar *)f->mag->data, f->mag->len, nil); /*XXX*/
	*f->ret = newIPint(b);
}

DISAPI(IPint_strtoip)
{
	ASSIGN(*f->ret, H);

	*f->ret = strtoipint(f->str, f->base);
}

/* create a random integer */
DISAPI(IPint_random)
{
	mpint *b;

	ASSIGN(*f->ret, H);

	release();
	b = mprand(f->maxbits, genrandom, nil);
	acquire();
	*f->ret = newIPint(b);
}

/* number of bits in number */
DISAPI(IPint_bits)
{
	int n;

	*f->ret = 0;
	if(f->i == H)
		return;

	n = mpsignif(MP(f->i));
	if(n == 0)
		n = 1;	/* compatibility */
	*f->ret = n;
}

/* create a new IP from an int */
DISAPI(IPint_inttoip)
{
	ASSIGN(*f->ret, H);

	*f->ret = newIPint(itomp(f->i, nil));
}

DISAPI(IPint_iptoint)
{
	*f->ret = 0;
	if(f->i == H)
		return;
	*f->ret = mptoi(MP(f->i));
}

/* modular exponentiation */
DISAPI(IPint_expmod)
{
	mpint *ret, *mod, *base, *exp;

	ASSIGN(*f->ret, H);

	base = MP(f->base);
	exp = MP(f->exp);
	if(f->mod != H)
		mod = MP(f->mod);
	else
		mod = nil;
	ret = mpnew(0);
	if(ret != nil)
		mpexp(base, exp, mod, ret);
	*f->ret = newIPint(ret);
}

/* multiplicative inverse */
DISAPI(IPint_invert)
{
	mpint *ret;

	ASSIGN(*f->ret, H);

	ret = mpnew(0);
	if(ret != nil)
		mpinvert(MP(f->base), MP(f->mod), ret);
	*f->ret = newIPint(ret);
}

/* basic math */
DISAPI(IPint_add)
{
	mpint *i1, *i2, *ret;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpadd(i1, i2, ret);

	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_sub)
{
	mpint *i1, *i2, *ret;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpsub(i1, i2, ret);

	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_mul)
{
	mpint *i1, *i2, *ret;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpmul(i1, i2, ret);

	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_div)
{
	mpint *i1, *i2, *quo, *rem;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	quo = mpnew(0);
	if(quo == nil)
		error(exHeap);
	rem = mpnew(0);
	if(rem == nil){
		mpfree(quo);
		error(exHeap);
	}
	mpdiv(i1, i2, quo, rem);

	ASSIGN(f->ret->t0, newIPint(quo));
	ASSIGN(f->ret->t1, newIPint(rem));
}

DISAPI(IPint_mod)
{
	mpint *i1, *i2, *ret;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpmod(i1, i2, ret);

	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_neg)
{
	mpint *ret;

	ret = mpcopy(MP(f->i));
	if(ret == nil)
		error(exHeap);
	ret->sign = -ret->sign;

	ASSIGN(*f->ret, newIPint(ret));
}

/* copy */
DISAPI(IPint_copy)
{
	ASSIGN(*f->ret, newIPint(mpcopy(MP(f->i))));
}


/* equality */
DISAPI(IPint_eq)
{
	*f->ret = mpcmp(MP(f->i1), MP(f->i2)) == 0;
}

/* compare */
DISAPI(IPint_cmp)
{
	*f->ret = mpcmp(MP(f->i1), MP(f->i2));
}

/* shifts */
DISAPI(IPint_shl)
{
	mpint *ret, *i;

	i = MP(f->i);
	ret = mpnew(0);
	if(ret != nil)
		mpleft(i, f->n, ret);
	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_shr)
{
	mpint *ret, *i;

	i = MP(f->i);
	ret = mpnew(0);
	if(ret != nil)
		mpright(i, f->n, ret);
	ASSIGN(*f->ret, newIPint(ret));
}

static void
mpand(mpint *b, mpint *m, mpint *res)
{
	int i;

	res->sign = b->sign;
	if(b->top == 0 || m->top == 0){
		res->top = 0;
		return;
	}
	mpbits(res, b->top*Dbits);
	res->top = b->top;
	for(i = b->top; --i >= 0;){
		if(i < m->top)
			res->p[i] = b->p[i] & m->p[i];
		else
			res->p[i] = 0;
	}
	mpnorm(res);
}

static void
mpor(mpint *b1, mpint *b2, mpint *res)
{
	mpint *t;
	int i;

	if(b2->top > b1->top){
		t = b1;
		b1 = b2;
		b2 = t;
	}
	if(b1->top == 0){
		mpassign(b2, res);
		return;
	}
	if(b2->top == 0){
		mpassign(b1, res);
		return;
	}
	mpassign(b1, res);
	for(i = b2->top; --i >= 0;)
		res->p[i] |= b2->p[i];
	mpnorm(res);
}

static void
mpxor(mpint *b1, mpint *b2, mpint *res)
{
	mpint *t;
	int i;

	if(b2->top > b1->top){
		t = b1;
		b1 = b2;
		b2 = t;
	}
	if(b1->top == 0){
		mpassign(b2, res);
		return;
	}
	if(b2->top == 0){
		mpassign(b1, res);
		return;
	}
	mpassign(b1, res);
	for(i = b2->top; --i >= 0;)
		res->p[i] ^= b2->p[i];
	mpnorm(res);
}

static void
mpnot(mpint *b1, mpint *res)
{
	int i;

	mpbits(res, Dbits*b1->top);
	res->sign = 1;
	res->top = b1->top;
	for(i = res->top; --i >= 0;)
		res->p[i] = ~b1->p[i];
	mpnorm(res);
}

/* bits */
DISAPI(IPint_and)
{
	mpint *ret, *i1, *i2;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpand(i1, i2, ret);
	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_ori)
{
	mpint *ret, *i1, *i2;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpor(i1, i2, ret);
	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_xor)
{
	mpint *ret, *i1, *i2;

	i1 = MP(f->i1);
	i2 = MP(f->i2);
	ret = mpnew(0);
	if(ret != nil)
		mpxor(i1, i2, ret);
	ASSIGN(*f->ret, newIPint(ret));
}

DISAPI(IPint_not)
{
	mpint *ret, *i1;

	i1 = MP(f->i1);
	ret = mpnew(0);
	if(ret != nil)
		mpnot(i1, ret);
	ASSIGN(*f->ret, newIPint(ret));
}
