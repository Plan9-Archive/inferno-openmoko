#include <dat.h>
#include <fns.h>
#include <error.h>
#include <isa.h>
#include <interp.h>
#include <ip.h>
#include <srv.h>
#include <srvm.h>

static	QLock	dbq;

DISAPI(Srv_init)
{
	USED(f);
}

DISAPI(Srv_iph2a)
{
	Heap *hpt;
	String *ss;
	int i, n, nhost;
	List **h, *l, *nl;
	char *hostv[10];

	ASSIGN(*f->ret, H);
	release();
	qlock(&dbq);
	if(waserror()){
		qunlock(&dbq);
		acquire();
		return;
	}
	nhost = so_gethostbyname(string2c(f->host), hostv, nelem(hostv));
	poperror();
	qunlock(&dbq);
	acquire();
	if(nhost == 0)
		return;

	l = (List*)H;
	h = &l;
	for(i = 0; i < nhost; i++) {
		n = strlen(hostv[i]);
		ss = newstring(n);
		memmove(ss->Sascii, hostv[i], n);
		free(hostv[i]);

		hpt = nheap(sizeof(List) + sizeof(String*));
		hpt->t = &Tlist;
		hpt->t->ref++;
		nl = H2D(List*, hpt);
		nl->t = &Tptr;
		Tptr.ref++;
		nl->tail = (List*)H;
		nl->data.pstring = ss;

		*h = nl;
		h = &nl->tail;
	}
	*f->ret = l;
}

DISAPI(Srv_ipa2h)
{
	Heap *hpt;
	String *ss;
	int i, n, naliases;
	List **h, *l, *nl;
	char *hostv[10];

	ASSIGN(*f->ret, H);
	release();
	qlock(&dbq);
	if(waserror()){
		qunlock(&dbq);
		acquire();
		return;
	}
	naliases = so_gethostbyaddr(string2c(f->addr), hostv, nelem(hostv));
	poperror();
	qunlock(&dbq);
	acquire();
	if(naliases == 0)
		return;

	l = (List*)H;
	h = &l;
	for(i = 0; i < naliases; i++) {
		n = strlen(hostv[i]);
		ss = newstring(n);
		memmove(ss->Sascii, hostv[i], n);
		free(hostv[i]);

		hpt = nheap(sizeof(List) + sizeof(String*));
		hpt->t = &Tlist;
		hpt->t->ref++;
		nl = H2D(List*, hpt);
		nl->t = &Tptr;
		Tptr.ref++;
		nl->tail = (List*)H;
		nl->data.pstring = ss;

		*h = nl;
		h = &nl->tail;
	}
	*f->ret = l;
}

DISAPI(Srv_ipn2p)
{
	int n;
	char buf[16];

	ASSIGN(*f->ret, (String*)H);
	release();
	qlock(&dbq);
	if(waserror()){
		qunlock(&dbq);
		acquire();
		return;
	}
	n = so_getservbyname(string2c(f->service), string2c(f->net), buf);
	poperror();
	qunlock(&dbq);
	acquire();
	if(n >= 0)
		retstr(buf, f->ret);
}

void
srvmodinit(void)
{
	builtinmod("$Srv", Srvmodtab, Srvmodlen);
}
