#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include "runt.h"
#include "raise.h"
#include "mathi.h"
#include "mathmod.h"

static union
{
	double x;
	uvlong u;
} bits64;

static union{
	float x;
	unsigned int u;
} bits32;

void
mathmodinit(void)
{
	builtinmod("$Math", Mathmodtab, Mathmodlen);
	fmtinstall('g', gfltconv);
	fmtinstall('G', gfltconv);
	fmtinstall('e', gfltconv);
	/* fmtinstall('E', gfltconv); */	/* avoid clash with ether address */
	fmtinstall(0x00c9, gfltconv);	/* L'É' */
	fmtinstall('f', gfltconv);
}

DISAPI(Math_import_int)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	int *x = (int*)f->x->data;

	if(f->b->len!=4*n)
		error(exMathia);
	for(i=0; i<n; i++){
		x[i] = GBIT32BE(bp);
		bp += 4;
	}
}

DISAPI(Math_import_real32)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	double *x = (double*)f->x->data;

	if(f->b->len!=4*n)
		error(exMathia);
	for(i=0; i<n; i++){
		bits32.u = GBIT32BE(bp);
		x[i] = bits32.x;
		bp += 4;
	}
}

DISAPI(Math_import_real)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	double *x = (double*)f->x->data;

	if(f->b->len!=8*n)
		error(exMathia);
	for(i=0; i<n; i++){
		bits64.u = GBIT64BE(bp);
		x[i] = bits64.x;
		bp += 8;
	}
}

DISAPI(Math_export_int)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	int *x = (int*)f->x->data;

	if(f->b->len!=4*n)
		error(exMathia);
	for(i=0; i<n; i++){
		PBIT32BE(bp, x[i]);
		bp += 4;
	}
}

DISAPI(Math_export_real32)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	double *x = (double*)f->x->data;

	if(f->b->len!=4*n)
		error(exMathia);
	for(i=0; i<n; i++){
		bits32.x = x[i];
		PBIT32BE(bp, bits32.u);
		bp += 4;
	}
}

DISAPI(Math_export_real)
{
	int i, n = f->x->len;
	char *bp = f->b->data;
	double *x = (double*)f->x->data;

	if(f->b->len!=8*n)
		error(exMathia);
	for(i=0; i<n; i++){
		bits64.x = x[i];
		PBIT64BE(bp, bits64.u);
		bp += 8;
	}
}

DISAPI(Math_bits32real)
{
	bits32.u = f->b;
	*f->ret = bits32.x;
}

DISAPI(Math_bits64real)
{
	bits64.u = f->b;
	*f->ret = bits64.x;
}

DISAPI(Math_realbits32)
{
	bits32.x = f->x;
	*f->ret = bits32.u;
}

DISAPI(Math_realbits64)
{
	bits64.x = f->x;
	*f->ret = bits64.u;
}

DISAPI(Math_getFPcontrol)
{
	*f->ret = getFPcontrol();
}

DISAPI(Math_getFPstatus)
{
	*f->ret = getFPstatus();
}

DISAPI(Math_finite)
{
	*f->ret = finite(f->x);
}

DISAPI(Math_ilogb)
{
	*f->ret = ilogb(f->x);
}

DISAPI(Math_isnan)
{
	*f->ret = isnan(f->x);
}

DISAPI(Math_acos)
{
	*f->ret = __ieee754_acos(f->x);
}

DISAPI(Math_acosh)
{
	*f->ret = __ieee754_acosh(f->x);
}

DISAPI(Math_asin)
{
	*f->ret = __ieee754_asin(f->x);
}

DISAPI(Math_asinh)
{
	*f->ret = asinh(f->x);
}

DISAPI(Math_atan)
{
	*f->ret = atan(f->x);
}

DISAPI(Math_atanh)
{
	*f->ret = __ieee754_atanh(f->x);
}

DISAPI(Math_cbrt)
{
	*f->ret = cbrt(f->x);
}

DISAPI(Math_ceil)
{
	*f->ret = ceil(f->x);
}

DISAPI(Math_cos)
{
	*f->ret = cos(f->x);
}

DISAPI(Math_cosh)
{
	*f->ret = __ieee754_cosh(f->x);
}

DISAPI(Math_erf)
{
	*f->ret = erf(f->x);
}

DISAPI(Math_erfc)
{
	*f->ret = erfc(f->x);
}

DISAPI(Math_exp)
{
	*f->ret = __ieee754_exp(f->x);
}

DISAPI(Math_expm1)
{
	*f->ret = expm1(f->x);
}

DISAPI(Math_fabs)
{
	*f->ret = fabs(f->x);
}

DISAPI(Math_floor)
{
	*f->ret = floor(f->x);
}

DISAPI(Math_j0)
{
	*f->ret = __ieee754_j0(f->x);
}

DISAPI(Math_j1)
{
	*f->ret = __ieee754_j1(f->x);
}

DISAPI(Math_log)
{
	*f->ret = __ieee754_log(f->x);
}

DISAPI(Math_log10)
{
	*f->ret = __ieee754_log10(f->x);
}

DISAPI(Math_log1p)
{
	*f->ret = log1p(f->x);
}

DISAPI(Math_rint)
{
	*f->ret = rint(f->x);
}

DISAPI(Math_sin)
{
	*f->ret = sin(f->x);
}

DISAPI(Math_sinh)
{
	*f->ret = __ieee754_sinh(f->x);
}

DISAPI(Math_sqrt)
{
	*f->ret = __ieee754_sqrt(f->x);
}

DISAPI(Math_tan)
{
	*f->ret = tan(f->x);
}

DISAPI(Math_tanh)
{
	*f->ret = tanh(f->x);
}

DISAPI(Math_y0)
{
	*f->ret = __ieee754_y0(f->x);
}

DISAPI(Math_y1)
{
	*f->ret = __ieee754_y1(f->x);
}

DISAPI(Math_fdim)
{
	*f->ret = fdim(f->x, f->y);
}

DISAPI(Math_fmax)
{
	*f->ret = fmax(f->x, f->y);
}

DISAPI(Math_fmin)
{
	*f->ret = fmin(f->x, f->y);
}

DISAPI(Math_fmod)
{
	*f->ret = __ieee754_fmod(f->x, f->y);
}

DISAPI(Math_hypot)
{
	*f->ret = __ieee754_hypot(f->x, f->y);
}

DISAPI(Math_nextafter)
{
	*f->ret = nextafter(f->x, f->y);
}

DISAPI(Math_pow)
{
	*f->ret = __ieee754_pow(f->x, f->y);
}

DISAPI(Math_FPcontrol)
{
	*f->ret = FPcontrol(f->r, f->mask);
}

DISAPI(Math_FPstatus)
{
	*f->ret = FPstatus(f->r, f->mask);
}

DISAPI(Math_atan2)
{
	*f->ret = __ieee754_atan2(f->y, f->x);
}

DISAPI(Math_copysign)
{
	*f->ret = copysign(f->x, f->s);
}

DISAPI(Math_jn)
{
	*f->ret = __ieee754_jn(f->n, f->x);
}

DISAPI(Math_lgamma)
{
	f->ret->t1 = __ieee754_lgamma_r(f->x, &f->ret->t0);
}

DISAPI(Math_modf)
{
	double ipart;

	f->ret->t1 = modf(f->x, &ipart);
	f->ret->t0 = ipart;
}

DISAPI(Math_pow10)
{
	*f->ret = ipow10(f->p);
}

DISAPI(Math_remainder)
{
	*f->ret = __ieee754_remainder(f->x, f->p);
}

DISAPI(Math_scalbn)
{
	*f->ret = scalbn(f->x, f->n);
}

DISAPI(Math_yn)
{
	*f->ret = __ieee754_yn(f->n, f->x);
}


/**** sorting real vectors through permutation vector ****/
/* qsort from coma:/usr/jlb/qsort/qsort.dir/qsort.c on 28 Sep '92
 char* has been changed to uchar*, static internal functions.
 specialized to swapping ints (which are 32-bit anyway in limbo).
 converted uchar* to int* (and substituted 1 for es).
*/

static int
cmp(int *u, int *v, double *x)
{
	return ((x[*u]==x[*v])? 0 : ((x[*u]<x[*v])? -1 : 1));
}

#define swap(u, v) {int t = *(u); *(u) = *(v); *(v) = t;}

#define vecswap(u, v, n) if(n>0){	\
    int i = n;				\
    register int *pi = u;		\
    register int *pj = v;		\
    do {				\
        register int t = *pi;		\
        *pi++ = *pj;			\
        *pj++ = t;			\
    } while (--i > 0);			\
}

#define minimum(x, y) ((x)<=(y) ? (x) : (y))

static int *
med3(int *a, int *b, int *c, double *x)
{	return cmp(a, b, x) < 0 ?
		  (cmp(b, c, x) < 0 ? b : (cmp(a, c, x) < 0 ? c : a ) )
		: (cmp(b, c, x) > 0 ? b : (cmp(a, c, x) < 0 ? a : c ) );
}

void
rqsort(int *a, int n, double *x)
{
	int *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int  d, r;

	if (n < 7) { /* Insertion sort on small arrays */
		for (pm = a + 1; pm < a + n; pm++)
			for (pl = pm; pl > a && cmp(pl-1, pl, x) > 0; pl--)
				swap(pl, pl-1);
		return;
	}
	pm = a + (n/2);
	if (n > 7) {
		pl = a;
		pn = a + (n-1);
		if (n > 40) { /* On big arrays, pseudomedian of 9 */
			d = (n/8);
			pl = med3(pl, pl+d, pl+2*d, x);
			pm = med3(pm-d, pm, pm+d, x);
			pn = med3(pn-2*d, pn-d, pn, x);
		}
		pm = med3(pl, pm, pn, x); /* On mid arrays, med of 3 */
	}
	swap(a, pm); /* On tiny arrays, partition around middle */
	pa = pb = a + 1;
	pc = pd = a + (n-1);
	for (;;) {
		while (pb <= pc && (r = cmp(pb, a, x)) <= 0) {
			if (r == 0) { swap(pa, pb); pa++; }
			pb++;
		}
		while (pb <= pc && (r = cmp(pc, a, x)) >= 0) {
			if (r == 0) { swap(pc, pd); pd--; }
			pc--;
		}
		if (pb > pc) break;
		swap(pb, pc);
		pb++;
		pc--;
	}
	pn = a + n;
	r = minimum(pa-a,  pb-pa);   vecswap(a,  pb-r, r);
	r = minimum(pd-pc, pn-pd-1); vecswap(pb, pn-r, r);
	if ((r = pb-pa) > 1) rqsort(a, r, x);
	if ((r = pd-pc) > 1) rqsort(pn-r, r, x);
}

DISAPI(Math_sort)
{
	int	i, pilen, xlen, *p;

	/* check that permutation contents are in [0,n-1] !!! */
	p = (int*) (f->pi->data);
	pilen = f->pi->len;
	xlen = f->x->len - 1;

	for(i = 0; i < pilen; i++) {
		if((*p < 0) || (xlen < *p))
			error(exMathia);
		p++;
	}

	rqsort( (int*)(f->pi->data), f->pi->len, (double*)(f->x->data));
}


/************ BLAS ***************/

DISAPI(Math_dot)
{
	if(f->x->len!=f->y->len)
		error(exMathia);	/* incompatible lengths */
	*f->ret = dot(f->x->len, (double*)(f->x->data), (double*)(f->y->data));
}

DISAPI(Math_iamax)
{
	*f->ret = iamax(f->x->len, (double*)(f->x->data));
}

DISAPI(Math_norm2)
{
	*f->ret = norm2(f->x->len, (double*)(f->x->data));
}

DISAPI(Math_norm1)
{
	*f->ret = norm1(f->x->len, (double*)(f->x->data));
}

DISAPI(Math_gemm)
{
	int nrowa, ncola, nrowb, ncolb, mn, ld, m, n;
	double *adata = 0, *bdata = 0, *cdata;
	int nota = f->transa=='N';
	int notb = f->transb=='N';
	if(nota){
		nrowa = f->m;
		ncola = f->k;
	}else{
		nrowa = f->k;
		ncola = f->m;
	}
	if(notb){
		nrowb = f->k;
		ncolb = f->n;
	}else{
		nrowb = f->n;
		ncolb = f->k;
	}
	if(     (!nota && f->transa!='C' && f->transa!='T') ||
		(!notb && f->transb!='C' && f->transb!='T') ||
		(f->m < 0 || f->n < 0 || f->k < 0) ){
		error(exMathia);
	}
	if(f->a != H){
		mn = f->a->len;
		adata = (double*)(f->a->data);
		ld = f->lda;
		if(ld<nrowa || ld*(ncola-1)>mn)
			error(exBounds);
	}
	if(f->b != H){
		mn = f->b->len;
		ld = f->ldb;
		bdata = (double*)(f->b->data);
		if(ld<nrowb || ld*(ncolb-1)>mn)
			error(exBounds);
	}
	m = f->m;
	n = f->n;
	mn = f->c->len;
	cdata = (double*)(f->c->data);
	ld = f->ldc;
	if(ld<m || ld*(n-1)>mn)
		error(exBounds);

	gemm(f->transa, f->transb, f->m, f->n, f->k, f->alpha,
		adata, f->lda, bdata, f->ldb, f->beta, cdata, f->ldc);
}
