#include	"dat.h"
#include	"fns.h"
#include	"error.h"

static Chan *
indirattach(const char *spec)
{
	char *p, spec2[256]; /*TODO*/
	Dev *d;

	if(*spec == 0)
		error(Ebadspec);

	/* spec2, p = spec.split('!') */
	strcpy(spec2, spec);
	p = strrchr(spec2, '!');
	if(p == nil)
		p = "";
	else
		*p++ = 0;

	d = devbyname(spec2);
	if(d == nil || d->dc == '*'){
		snprint(up->env->errstr, ERRMAX, "unknown device: %s", spec2);
		error(up->env->errstr);
	}
	if(up->env->pgrp->nodevs &&
	   (utfrune("|esDa", d->dc) == nil || d->dc == 's' && *p!='\0'))
		error(Enoattach);
	return d->attach(p);
}

Dev indirdevtab = {
	'*',
	"indir",

	devinit,
	indirattach,
};
