#include <dat.h>
#include <fns.h>
#include <isa.h>
#include <interp.h>

#define	QP(l)	(Prog**)((char*)(l)+sizeof(QLock))

void*
libqlowner(QLock *q)
{
	return *QP(q);
}

void
libqlock(QLock *q)
{
	Prog *p;
	//QLock *q;

	//q = l;
	p = currun();
	if(p == nil)
		abort();

	if(!canqlock(q)) {
		release();
		qlock(q);
		acquire();
	}
	*QP(q) = p;
}

void
libqunlock(QLock *q)
{
	Prog *p;
	//QLock *q;

	//q = l;
	p = currun();
	if(p == nil)
		abort();
	if(*QP(q) != p)
		abort();

	*QP(q) = nil;
	qunlock(q);
}

QLock*
libqlalloc(void)
{
	QLock *q;

	q = (QLock *)mallocz(sizeof(QLock)+sizeof(Prog*), 1);
	return q;
}

void
libqlfree(QLock *q)
{
	free(q);
}

vlong
libseek(int fd, vlong off, int whence)
{
	release();
	off = kseek(fd, off, whence);
	acquire();
	return off;
}

int
libread(int fd, char *buf, int n)
{
	release();
	n = kread(fd, buf, n);
	acquire();
	return n;
}

int
libreadn(int fd, char *a, long n)
{
	//char *a;
	long m, t;

	//a = av;
	t = 0;
	release();
	while(t < n){
		m = kread(fd, a+t, n-t);
		if(m <= 0){
			if(t == 0){
				acquire();
				return m;
			}
			break;
		}
		t += m;
	}
	acquire();
	return t;
}

int
libwrite(int fd, void *buf, int n)
{
	release();
	n = kwrite(fd, buf, n);
	acquire();
	return n;
}

int
libopen(const char *name, int omode)
{
	int fd;

	release();
	fd = kopen(name, omode);
	acquire();
	return fd;
}

int
libclose(int fd)
{
	release();
	fd = kclose(fd);
	acquire();
	return fd;
}

Dir*
libdirfstat(int fd)
{
	Dir *d;

	release();
	d = kdirfstat(fd);
	acquire();
	return d;
}

int
libbind(char *s, char *t, int f)
{
	int n;

	release();
	n = kbind(s, t, f);
	acquire();
	return n;
}

void
libchanclose(Chan *chan)
{
	release();
	cclose(chan);
	acquire();
}

void*
libfdtochan(int fd, int mode)
{
	Chan *c;

	release();
	if(waserror()) {
		acquire();
		return nil;
	}
	c = fdtochan(up->env->fgrp, fd, mode, 0, 1);
	poperror();
	acquire();
	return c;
}
