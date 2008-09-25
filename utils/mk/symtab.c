#include	"mk.h"

#define	NHASH	4099
#define	HASHMUL	79L	/* this is a good value */
static Symtab *hash[NHASH];

void
syminit(void)
{
	Symtab **s, *ss, *next;

	for(s = hash; s < &hash[NHASH]; s++){
		for(ss = *s; ss; ss = next){
			next = ss->next;
			free((char *)ss);
		}
		*s = 0;
	}
}

Symtab *
symlook(char *sym, int space, void *install)
{
	long h;
	char *p;
	Symtab *s;
#if 0
	static char*spacename[] = {
	"S_VAR",	/* variable -> value */
	"S_TARGET",	/* target -> rule */
	"S_TIME",	/* file -> time */
	"S_PID",	/* pid -> products */
	"S_NODE",	/* target name -> node */
	"S_AGG",	/* aggregate -> time */
	"S_BITCH",	/* bitched about aggregate not there */
	"S_NOEXPORT",	/* var -> noexport */
	"S_OVERRIDE",	/* can't override */
	"S_OUTOFDATE",	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
	"S_MAKEFILE",	/* target -> node */
	"S_MAKEVAR",	/* dumpable mk variable */
	"S_EXPORTED",	/* var -> current exported value */
	"S_WESET",	/* variable; we set in the mkfile */
	"S_INTERNAL",	/* an internal mk variable (e.g., stem, target) */
#if defined(CASE_INSENSITIVE_ENVIRONMENT)
	"S_LOWCASED",	/* low-cased windows env var name */
#endif
	};

	print("symlook(%s, %s, %x)\n", sym, spacename[space], install);
#endif

	for(p = sym, h = space; *p; h += *p++)
		h *= HASHMUL;
	if(h < 0)
		h = ~h;
	h %= NHASH;
	for(s = hash[h]; s; s = s->next)
		if((s->space == space) && (strcmp(s->name, sym) == 0))
			return(s);
	if(install == 0)
		return(0);
	s = (Symtab *)Malloc(sizeof(Symtab));
	s->space = space;
	s->name = sym;
	s->value = install;
	s->next = hash[h];
	hash[h] = s;
	return(s);
}

void
symdel(char *sym, int space)
{
	long h;
	char *p;
	Symtab *s, *ls;

	/* multiple memory leaks */

	for(p = sym, h = space; *p; h += *p++)
		h *= HASHMUL;
	if(h < 0)
		h = ~h;
	h %= NHASH;
	for(s = hash[h], ls = 0; s; ls = s, s = s->next)
		if((s->space == space) && (strcmp(s->name, sym) == 0)){
			if(ls)
				ls->next = s->next;
			else
				hash[h] = s->next;
			free((char *)s);
		}
}

void
symtraverse(int space, void (*fn)(Symtab*))
{
	Symtab **s, *ss;

	for(s = hash; s < &hash[NHASH]; s++)
		for(ss = *s; ss; ss = ss->next)
			if(ss->space == space)
				(*fn)(ss);
}

void
symstat(void)
{
	Symtab **s, *ss;
	int n;
	int l[1000];

	memset((char *)l, 0, sizeof(l));
	for(s = hash; s < &hash[NHASH]; s++){
		for(ss = *s, n = 0; ss; ss = ss->next)
			n++;
		l[n]++;
	}
	for(n = 0; n < 1000; n++)
		if(l[n]) Bprint(&bout, "%ld of length %d\n", l[n], n);
}
