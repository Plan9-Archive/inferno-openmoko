#include <dat.h>
#include <fns.h>
#include <error.h>
#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <kernel.h>

/*
 * here because Sys_FileIO is not public
 */
extern	int	srvf2c(char*, char*, Sys_FileIO*);

/*
 * System types connected to gc
 */
char	FDmap[] = Sys_FD_map;
char	FileIOmap[] = Sys_FileIO_map;
void	freeFD(Heap*, int);
void	freeFileIO(Heap*, int);
Type*	TFD;
Type*	TFileIO;

static	char	rmap[] = Sys_FileIO_read_map;
static	char	wmap[] = Sys_FileIO_write_map;
static	Type*	FioTread;
static	Type*	FioTwrite;
static	char	dmap[] = Sys_Dir_map;
static	Type*	Tdir;

typedef struct FD FD;
struct FD
{
	Sys_FD	fd;
	Fgrp*	grp;
};

void
sysinit(void)
{
	TFD = dtype(freeFD, sizeof(FD), FDmap, sizeof(FDmap), "Sys->FD");
	TFileIO = dtype(freeFileIO, Sys_FileIO_size, FileIOmap, sizeof(FileIOmap), "Sys->FileIO");

	/* Support for devsrv.c */
	FioTread = dtype(freeheap, Sys_FileIO_read_size, rmap, sizeof(rmap), "Sys->read");
	FioTwrite = dtype(freeheap, Sys_FileIO_write_size, wmap, sizeof(wmap), "Sys->write");

	/* Support for dirread */
	Tdir = dtype(freeheap, Sys_Dir_size, dmap, sizeof(dmap), "Sys->dir");
}

void
freeFD(Heap *h, int swept)
{
	FD *handle;

	USED(swept);

	handle = H2D(FD*, h);

	release();
	if(handle->fd.fd >= 0)
		kfgrpclose(handle->grp, handle->fd.fd);
	closefgrp(handle->grp);
	acquire();
}

void
freeFileIO(Heap *h, int swept)
{
	Sys_FileIO *fio;

	if(swept)
		return;

	fio = H2D(Sys_FileIO*, h);
	ASSIGN(fio->read, H);
	ASSIGN(fio->write, H);
}

Sys_FD*
mkfd(int fd)
{
	Heap *h;
	Fgrp *fg;
	FD *handle;

	h = heap(TFD);
	handle = H2D(FD*, h);
	handle->fd.fd = fd;
	fg = up->env->fgrp;
	handle->grp = fg;
	incref(&fg->r);
	return (Sys_FD*)handle;
}
#define fdchk(x)	((x) == (Sys_FD*)H ? -1 : (x)->fd)

void
seterror(__in_z __format_string const char *err, ...)
{
	char *estr;
	va_list arg;

	estr = up->env->errstr;
	va_start(arg, err);
	vseprint(estr, estr+ERRMAX, err, arg);
	va_end(arg);
}

char*
syserr(char *s, char *es, Prog *p)
{
	Osenv *o;

	o = p->osenv;
	kstrcpy(s, o->errstr, es - s);
	return s + strlen(s);
}

DISAPI(Sys_millisec)
{
	*f->ret = osmillisec();
}

DISAPI(Sys_open)
{
	int fd;

	//destroy(*f->ret);
	//*f->ret = (Sys_FD*)H;
	ASSIGN(*f->ret, (Sys_FD*)H);
	release();
	fd = kopen(string2c(f->s), f->mode);
	acquire();
	if(fd == -1)
		return;

	*f->ret = mkfd(fd);
}

DISAPI(Sys_pipe)
{
	Array *a;
	int fd[2];
	Sys_FD **sfd;

	*f->ret = -1;

	a = f->fds;
	if(a->len < 2)
		return;
	if(kpipe(fd) < 0)
		return;

	sfd = (Sys_FD**)a->data;
	//destroy(sfd[0]);
	//destroy(sfd[1]);
	//sfd[0] = (Sys_FD*)H;
	//sfd[1] = (Sys_FD*)H;
	//sfd[0] = mkfd(fd[0]);
	//sfd[1] = mkfd(fd[1]);
	ASSIGN(sfd[0], mkfd(fd[0]));
	ASSIGN(sfd[1], mkfd(fd[1]));
	*f->ret = 0;
}

DISAPI(Sys_fildes)
{
	int fd;

	//destroy(*f->ret);
	//*f->ret = (Sys_FD*)H;
	ASSIGN(*f->ret, (Sys_FD*)H);
	release();
	fd = kdup(f->fd, -1);
	acquire();
	if(fd == -1)
		return;
	*f->ret = mkfd(fd);
}

DISAPI(Sys_dup)
{
	release();
	*f->ret = kdup(f->old, f->new);
	acquire();
}

DISAPI(Sys_create)
{
	int fd;

	ASSIGN(*f->ret, (Sys_FD*)H);
	release();
	fd = kcreate(string2c(f->s), f->mode, f->perm);
	acquire();
	if(fd == -1)
		return;

	*f->ret = mkfd(fd);
}

DISAPI(Sys_remove)
{
	release();
	*f->ret = kremove(string2c(f->s));
	acquire();
}

DISAPI(Sys_seek)
{
	release();
	*f->ret = kseek(fdchk(f->fd), f->off, f->start);
	acquire();
}

DISAPI(Sys_unmount)
{
	release();
	*f->ret = kunmount(string2c(f->s1), string2c(f->s2));
	acquire();
}

DISAPI(Sys_read)
{
	int n;

	n = f->n;
	if(f->buf == (Array*)H || n < 0) {
		*f->ret = 0;
		return;
	}
	if(n > f->buf->len)
		n = f->buf->len;

	release();
	*f->ret = kread(fdchk(f->fd), f->buf->data, n);
	acquire();
}

DISAPI(Sys_readn)
{
	int fd, m, n, t;

	n = f->n;
	if(f->buf == (Array*)H || n < 0) {
		*f->ret = 0;
		return;
	}
	if(n > f->buf->len)
		n = f->buf->len;
	fd = fdchk(f->fd);

	release();
	for(t = 0; t < n; t += m){
		m = kread(fd, (char*)f->buf->data+t, n-t);
		if(m <= 0){
			if(t == 0)
				t = m;
			break;
		}
	}
	*f->ret = t;
	acquire();
}

DISAPI(Sys_pread)
{
	int n;

	n = f->n;
	if(f->buf == (Array*)H || n < 0) {
		*f->ret = 0;
		return;
	}
	if(n > f->buf->len)
		n = f->buf->len;

	release();
	*f->ret = kpread(fdchk(f->fd), f->buf->data, n, f->off);
	acquire();
}

DISAPI(Sys_chdir)
{
	release();
	*f->ret = kchdir(string2c(f->path));
	acquire();
}

DISAPI(Sys_write)
{
	int n;

	n = f->n;
	if(f->buf == (Array*)H || n < 0) {
		*f->ret = 0;
		return;
	}
	if(n > f->buf->len)
		n = f->buf->len;

	release();
	*f->ret = kwrite(fdchk(f->fd), f->buf->data, n);
	acquire();
}

DISAPI(Sys_pwrite)
{
	int n;

	n = f->n;
	if(f->buf == (Array*)H || n < 0) {
		*f->ret = 0;
		return;
	}
	if(n > f->buf->len)
		n = f->buf->len;

	release();
	*f->ret = kpwrite(fdchk(f->fd), f->buf->data, n, f->off);
	acquire();
}

static void
unpackdir(Dir *d, Sys_Dir *sd)
{
	retstr(d->name, &sd->name);
	retstr(d->uid, &sd->uid);
	retstr(d->gid, &sd->gid);
	retstr(d->muid, &sd->muid);
	sd->qid.path = d->qid.path;
	sd->qid.vers = d->qid.vers;
	sd->qid.qtype = d->qid.type;
	sd->mode = d->mode;
	sd->atime = d->atime;
	sd->mtime = d->mtime;
	sd->length = d->length;
	sd->dtype = d->type;
	sd->dev = d->dev;
}

static Dir*
packdir(Sys_Dir *sd)
{
	char *nm[4], *p;
	int i, n;
	Dir *d;

	nm[0] = string2c(sd->name);
	nm[1] = string2c(sd->uid);
	nm[2] = string2c(sd->gid);
	nm[3] = string2c(sd->muid);
	n = 0;
	for(i=0; i<4; i++)
		n += strlen(nm[i])+1;
	d = (Dir*)smalloc(sizeof(*d)+n);
	p = (char*)d+sizeof(*d);
	for(i=0; i<4; i++){
		n = strlen(nm[i])+1;
		memmove(p, nm[i], n);
		nm[i] = p;
		p += n;
	}
	d->name = nm[0];
	d->uid = nm[1];
	d->gid = nm[2];
	d->muid = nm[3];
	d->qid.path = sd->qid.path;
	d->qid.vers = sd->qid.vers;
	d->qid.type = sd->qid.qtype;
	d->mode = sd->mode;
	d->atime = sd->atime;
	d->mtime = sd->mtime;
	d->length = sd->length;
	d->type = sd->dtype;
	d->dev = sd->dev;
	return d;
}

DISAPI(Sys_fstat)
{
	Dir *d;

	f->ret->t0 = -1;
	release();
	d = kdirfstat(fdchk(f->fd));
	acquire();
	if(d == nil)
		return;
	if(waserror() == 0){
		unpackdir(d, &f->ret->t1);
		f->ret->t0 = 0;
		poperror();
	}
	free(d);
}

DISAPI(Sys_stat)
{
	Dir *d;

	f->ret->t0 = -1;
	release();
	d = kdirstat(string2c(f->s));
	acquire();
	if(d == nil)
		return;
	if(waserror() == 0){
		unpackdir(d, &f->ret->t1);
		f->ret->t0 = 0;
		poperror();
	}
	free(d);
}

DISAPI(Sys_fd2path)
{
	char *s;

	release();
	s = kfd2path(fdchk(f->fd));
	acquire();
	if(waserror() == 0){
		retstr(s, f->ret);
		poperror();
	}
	else {
		ASSIGN(*f->ret, (String*)H);
	}
	free(s);
}

DISAPI(Sys_mount)
{
	release();
	*f->ret = kmount(fdchk(f->fd), fdchk(f->afd), string2c(f->on), f->flags, string2c(f->spec));
	acquire();
}

DISAPI(Sys_bind)
{
	release();
	*f->ret = kbind(string2c(f->s), string2c(f->on), f->flags);
	acquire();
}

DISAPI(Sys_wstat)
{
	Dir *d;

	d = packdir(&f->d);
	release();
	*f->ret = kdirwstat(string2c(f->s), d);
	acquire();
	free(d);
}

DISAPI(Sys_fwstat)
{
	Dir *d;

	d = packdir(&f->d);
	release();
	*f->ret = kdirfwstat(fdchk(f->fd), d);
	acquire();
	free(d);
}

DISAPI(Sys_print)
{
	int n;
	Prog *p;
	Chan *c;
	char buf[1024], *b = buf; /* FIXME: buffer */

	c = up->env->fgrp->fd[1];
	if(c == nil)
		return;
	p = currun();

	release();
	n = xprint(p, f, &f->vargs, f->s, buf, sizeof(buf));
	if (n >= sizeof(buf)-UTFmax-2)
		n = bigxprint(p, f, &f->vargs, f->s, &b, sizeof(buf));
	*f->ret = kwrite(1, b, n);
	if (b != buf)
		free(b);
	acquire();
}

DISAPI(Sys_fprint)
{
	int n;
	Prog *p;
	char buf[1024], *b = buf; /* FIXME: buffer */

	p = currun();
	release();
	n = xprint(p, f, &f->vargs, f->s, buf, sizeof(buf));
	if (n >= sizeof(buf)-UTFmax-2)
		n = bigxprint(p, f, &f->vargs, f->s, &b, sizeof(buf));
	*f->ret = kwrite(fdchk(f->fd), b, n);
	if (b != buf)
		free(b);
	acquire();
}

DISAPI(Sys_werrstr)
{
	*f->ret = 0;
	kstrcpy(up->env->errstr, string2c(f->s), ERRMAX);
}

DISAPI(Sys_dial)
{
	int cfd;
	char dir[NETPATHLEN], *a, *l;

	a = string2c(f->addr);
	l = string2c(f->local);
	release();
	f->ret->t0 = kdial(a, l, dir, &cfd);
	acquire();
	//destroy(f->ret->t1.dfd);
	//f->ret->t1.dfd = (Sys_FD*)H;
	//destroy(f->ret->t1.cfd);
	//f->ret->t1.cfd = (Sys_FD*)H;
	if(f->ret->t0 == -1)
	{
		ASSIGN(f->ret->t1.dfd, (Sys_FD*)H);
		ASSIGN(f->ret->t1.cfd, (Sys_FD*)H);
		ASSIGN(f->ret->t1.dir, (String*)H);
		return;
	}

	ASSIGN(f->ret->t1.dfd, mkfd(f->ret->t0));
	f->ret->t0 = 0;
	ASSIGN(f->ret->t1.cfd, mkfd(cfd));
	retstr(dir, &f->ret->t1.dir);
}

DISAPI(Sys_announce)
{
	char dir[NETPATHLEN], *a;

	a = string2c(f->addr);
	release();
	f->ret->t0 = kannounce(a, dir);
	acquire();
	//destroy(f->ret->t1.dfd);
	//f->ret->t1.dfd = (Sys_FD*)H;
	//destroy(f->ret->t1.cfd);
	//f->ret->t1.cfd = (Sys_FD*)H;
	ASSIGN(f->ret->t1.dfd, (Sys_FD*)H);
	if(f->ret->t0 == -1)
	{
		ASSIGN(f->ret->t1.cfd, (Sys_FD*)H);
		ASSIGN(f->ret->t1.dir, (String*)H);
		return;   /* BUG: f->ret->t1.dir was unitialized */
	}

	ASSIGN(f->ret->t1.cfd, mkfd(f->ret->t0));
	f->ret->t0 = 0;
	retstr(dir, &f->ret->t1.dir);
}

DISAPI(Sys_listen)
{
	char dir[NETPATHLEN], *d;

	d = string2c(f->c.dir);
	release();
	f->ret->t0 = klisten(d, dir);
	acquire();

	//destroy(f->ret->t1.dfd);
	//f->ret->t1.dfd = (Sys_FD*)H;
	//destroy(f->ret->t1.cfd);
	//f->ret->t1.cfd = (Sys_FD*)H;
	ASSIGN(f->ret->t1.dfd, (Sys_FD*)H);
	if(f->ret->t0 == -1)
	{
		ASSIGN(f->ret->t1.cfd, (Sys_FD*)H);
		ASSIGN(f->ret->t1.dir, (String*)H);
		return;
	}

	//f->ret->t1.cfd = mkfd(f->ret->t0);
	ASSIGN(f->ret->t1.cfd, mkfd(f->ret->t0));

	f->ret->t0 = 0;
	retstr(dir, &f->ret->t1.dir);
}

DISAPI(Sys_sleep)
{
	release();
	if(f->period > 0){
		if(waserror()){
			acquire();
			error("");
		}
		osenter();
		*f->ret = limbosleep(f->period);
		osleave();
		poperror();
	}
	acquire();
}

DISAPI(Sys_stream)
{
	Prog *p;
	char *buf;
	int src, dst;
	int nbytes, t, n;

	buf = (char *)malloc(f->bufsiz);
	if(buf == nil) {
		kwerrstr(Enomem);
		*f->ret = -1;
		return;
	}

	src = fdchk(f->src);
	dst = fdchk(f->dst);

	p = currun();

	release();
	t = 0;
	nbytes = 0;
	while(p->kill == nil) {
		n = kread(src, buf+t, f->bufsiz-t);
		if(n <= 0)
			break;
		t += n;
		if(t >= f->bufsiz) {
			if(kwrite(dst, buf, t) != t) {
				t = 0;
				break;
			}

			nbytes += t;
			t = 0;
		}
	}
	if(t != 0) {
		kwrite(dst, buf, t);
		nbytes += t;
	}
	acquire();
	free(buf);
	*f->ret = nbytes;
}

DISAPI(Sys_export)
{
	release();
	*f->ret = export(fdchk(f->c), string2c(f->dir), f->flag&Sys_EXPASYNC);
	acquire();
}

DISAPI(Sys_file2chan)
{
	int r;
	Heap *h;
	Channel *c;
	Sys_FileIO *fio;
	//void *sv;

	h = heap(TFileIO);

	fio = H2D(Sys_FileIO*, h);

	c = cnewc(FioTread, movertmp, 16);
	fio->read = c;

	c = cnewc(FioTwrite, movertmp, 16);
	fio->write = c;

	//sv = *f->ret;
	//*f->ret = fio;
	//destroy(sv);
	ASSIGN(*f->ret, fio);

	release();
	r = srvf2c(string2c(f->dir), string2c(f->file), fio);
	acquire();
	if(r == -1) {
		//*f->ret = (Sys_FileIO*)H;
		ASSIGN(*f->ret, H); /*kills fio */
	}
}

enum
{
	/* the following pctl calls can block and must release the virtual machine */
	BlockingPctl=	Sys_NEWFD|Sys_FORKFD|Sys_NEWNS|Sys_FORKNS|Sys_NEWENV|Sys_FORKENV
};

DISAPI(Sys_pctl)
{
	int fd;
	Prog *p;
	List *l;
	Chan *c;
	volatile struct {Pgrp *np;} np;
	Pgrp *opg;
	Chan *dot;
	Osenv *o;
	Fgrp *fg, *ofg, *nfg;
	volatile struct {Egrp *ne;} ne;
	Egrp *oe;

	p = currun();
	if(f->flags & BlockingPctl)
		release();

	np.np = nil;
	ne.ne = nil;
	if(waserror()) {
		closepgrp(np.np);
		closeegrp(ne.ne);
		if(f->flags & BlockingPctl)
			acquire();
		*f->ret = -1;
		return;
	}

	o = p->osenv;
	if(f->flags & Sys_NEWFD) {
		ofg = o->fgrp;
		nfg = newfgrp(ofg);
		lock(&ofg->l);
		/* file descriptors to preserve */
		for(l = f->movefd; l != H; l = l->tail) {
			fd = l->data.disint;
			if(fd >= 0 && fd <= ofg->maxfd) {
				c = ofg->fd[fd];
				if(c != nil && fd < nfg->nfd && nfg->fd[fd] == nil) {
					incref(&c->r);
					nfg->fd[fd] = c;
					if(nfg->maxfd < fd)
						nfg->maxfd = fd;
				}
			}
		}
		unlock(&ofg->l);
		o->fgrp = nfg;
		closefgrp(ofg);
	}
	else
	if(f->flags & Sys_FORKFD) {
		ofg = o->fgrp;
		fg = dupfgrp(ofg);
		/* file descriptors to close */
		for(l = f->movefd; l != H; l = l->tail)
			kclose(l->data.disint);
		o->fgrp = fg;
		closefgrp(ofg);
	}

	if(f->flags & Sys_NEWNS) {
		np.np = newpgrp();
		dot = o->pgrp->dot;
		np.np->dot = cclone(dot);
		np.np->slash = cclone(dot);
		cnameclose(np.np->slash->name);
		np.np->slash->name = newcname("/");
		np.np->pin = o->pgrp->pin;		/* pin is ALWAYS inherited */
		np.np->nodevs = o->pgrp->nodevs;
		opg = o->pgrp;
		o->pgrp = np.np;
		np.np = nil;
		closepgrp(opg);
	}
	else
	if(f->flags & Sys_FORKNS) {
		np.np = newpgrp();
		pgrpcpy(np.np, o->pgrp);
		opg = o->pgrp;
		o->pgrp = np.np;
		np.np = nil;
		closepgrp(opg);
	}

	if(f->flags & Sys_NEWENV) {
		oe = o->egrp;
		o->egrp = newegrp();
		closeegrp(oe);
	}
	else
	if(f->flags & Sys_FORKENV) {
		ne.ne = newegrp();
		egrpcpy(ne.ne, o->egrp);
		oe = o->egrp;
		o->egrp = ne.ne;
		ne.ne = nil;
		closeegrp(oe);
	}

	if(f->flags & Sys_NEWPGRP)
		newgrp(p);

	if(f->flags & Sys_NODEVS)
		o->pgrp->nodevs = 1;

	poperror();

	if(f->flags & BlockingPctl)
		acquire();

	*f->ret = p->pid;
}

DISAPI(Sys_dirread)
{
	Dir *b;
	int i, n;
	Heap *h;
	char *d;

	f->ret->t0 = -1;
	ASSIGN(f->ret->t1, H);

	release();
	n = kdirread(fdchk(f->fd), &b);
	acquire();
	if(n <= 0) {
		f->ret->t0 = n;
		free(b);
		return;
	}
	if(waserror()){
		free(b);
		return;
	}
	h = heaparray(Tdir, n);
	poperror();
	d = H2D(Array*, h)->data;
	for(i = 0; i < n; i++) {
		unpackdir(b+i, (Sys_Dir*)d);
		d += Sys_Dir_size;
	}
	f->ret->t0 = n;
	f->ret->t1 = H2D(Array*, h);
	free(b);
}

DISAPI(Sys_fauth)
{
	int fd;

	ASSIGN(*f->ret, H);
	release();
	fd = kfauth(fdchk(f->fd), string2c(f->aname));
	acquire();
	if(fd >= 0)
		*f->ret = mkfd(fd);
}

DISAPI(Sys_fversion)
{
	//void *r;
	int n;
	char buf[20], *s;

	f->ret->t0 = -1;
	//r = f->ret->t1;
	//f->ret->t1 = (String*)H;
	//destroy(r);
	ASSIGN(f->ret->t1, (String*)H);
	s = string2c(f->version);
	n = strlen(s);
	if(n >= sizeof(buf)-1)
		n = sizeof(buf)-1;
	memmove(buf, s, n);
	buf[n] = 0;
	release();
	n = kfversion(fdchk(f->fd), f->msize, buf, sizeof(buf));
	acquire();
	if(n >= 0){
		f->ret->t0 = f->msize;
		retnstr(buf, n, &f->ret->t1);
	}
}

DISAPI(Sys_iounit)
{
	release();
	*f->ret = kiounit(fdchk(f->fd));
	acquire();
}

void
ccom(Progq **cl, Prog *p)
{
	volatile struct {Progq **cl;} vcl;

	cqadd(cl, p);
	vcl.cl = cl;
	if(waserror()) {
		if(p->ptr != nil) {	/* no killcomm */
			cqdelp(vcl.cl, p);
			p->ptr = nil;
		}
		nexterror();
	}
	cblock(p);
	poperror();
}
extern REG R;
void
crecv(Channel *c, void *ip)
{
	Prog *p;

	if(c->send->prog == nil && c->size == 0) {
		p = currun();
		p->ptr = ip;
		ccom(&c->recv, p);
		return;
	}

	if(_irecv(c, ip)) /* FIXME: args passed via R  */
		R.IC = 1;
}

void
csend(Channel *c, void *ip)
{
 	Prog *p;

	if(c->recv->prog == nil && (c->buf == H || c->size == c->buf->len)) {
		p = currun();
		p->ptr = ip;
		ccom(&c->send, p);
		return;
	}

	if(_isend(c, ip)) /* FIXME: args passed via R  */
		R.IC = 1;
}
