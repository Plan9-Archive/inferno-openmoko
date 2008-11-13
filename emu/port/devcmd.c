#include <dat.h>
#include <fns.h>
#include <error.h>

enum
{
	Qcmd_topdir,	/* top level directory */
	Qcmd_cmd,
	Qcmd_clonus,
	Qcmd_convdir,
	Qcmd_convbase,
	Qcmd_data = Qcmd_convbase,
	Qcmd_stderr,
	Qcmd_ctl,
	Qcmd_status,
	Qcmd_wait,

	Debug=0	/* to help debug os.c */
};
#define DEVCMDTYPE(x) 	((ulong)(x).path & 0xf)
#define DEVCMDCONV(x) 	(((ulong)(x).path >> 4)&0xfff)
#define DEVCMDQID(c, y) 	(((c)<<4) | (y))

typedef struct CmdConv CmdConv;
struct CmdConv
{
	int	x;
	int	inuse;
	int	fd[3];	/* stdin, stdout, and stderr */
	int	count[3];	/* number of readers on stdin/stdout/stderr */
	int	perm;
	char*	owner;
	char*	state;
	Cmdbuf*	cmd;
	char*	dir;
	QLock	l;	/* protects state changes */
	Queue*	waitq;
	void*	child;
	char*	error;	/* on start up */
	int	nice;
	short	killonclose;
	short	killed;
	Rendez	startr;
};

static struct
{
	QLock	l;
	int	nc;
	int	maxconv;
	CmdConv**	conv;
} cmd;

static	CmdConv*	cmdclone(char*);
static	void	cmdproc(void*);

static int
cmd3gen(Chan *c, int i, Dir *dp)
{
	Qid q;
	CmdConv *cv;

	cv = cmd.conv[DEVCMDCONV(c->qid)];
	switch(i){
	default:
		return -1;
	case Qcmd_data:
		mkqid(&q, DEVCMDQID(DEVCMDCONV(c->qid), Qcmd_data), 0, QTFILE);
		devdir(c, q, "data", 0, cv->owner, cv->perm, dp);
		return 1;
	case Qcmd_stderr:
		mkqid(&q, DEVCMDQID(DEVCMDCONV(c->qid), Qcmd_stderr), 0, QTFILE);
		devdir(c, q, "stderr", 0, cv->owner, 0444, dp);
		return 1;
	case Qcmd_ctl:
		mkqid(&q, DEVCMDQID(DEVCMDCONV(c->qid), Qcmd_ctl), 0, QTFILE);
		devdir(c, q, "ctl", 0, cv->owner, cv->perm, dp);
		return 1;
	case Qcmd_status:
		mkqid(&q, DEVCMDQID(DEVCMDCONV(c->qid), Qcmd_status), 0, QTFILE);
		devdir(c, q, "status", 0, cv->owner, 0444, dp);
		return 1;
	case Qcmd_wait:
		mkqid(&q, DEVCMDQID(DEVCMDCONV(c->qid), Qcmd_wait), 0, QTFILE);
		devdir(c, q, "wait", 0, cv->owner, 0444, dp);
		return 1;
	}
}

static int
cmdgen(Chan *c, const char *name, Dirtab *d, int nd, int s, Dir *dp)
{
	Qid q;
	CmdConv *cv;

	USED(name);
	USED(nd);
	USED(d);

	if(s == DEVDOTDOT){
		switch(DEVCMDTYPE(c->qid)){
		case Qcmd_topdir:
		case Qcmd_cmd:
			mkqid(&q, DEVCMDQID(0, Qcmd_topdir), 0, QTDIR);
			devdir(c, q, "#C", 0, eve, DMDIR|0555, dp);
			break;
		case Qcmd_convdir:
			mkqid(&q, DEVCMDQID(0, Qcmd_cmd), 0, QTDIR);
			devdir(c, q, "cmd", 0, eve, DMDIR|0555, dp);
			break;
		default:
			panic("cmdgen %llux", c->qid.path);
		}
		return 1;
	}

	switch(DEVCMDTYPE(c->qid)) {
	case Qcmd_topdir:
		if(s >= 1)
			return -1;
		mkqid(&q, DEVCMDQID(0, Qcmd_cmd), 0, QTDIR);
		devdir(c, q, "cmd", 0, "cmd", DMDIR|0555, dp);
		return 1;
	case Qcmd_cmd:
		if(s < cmd.nc) {
			cv = cmd.conv[s];
			mkqid(&q, DEVCMDQID(s, Qcmd_convdir), 0, QTDIR);
			sprint(up->genbuf, "%d", s);
			devdir(c, q, up->genbuf, 0, cv->owner, DMDIR|0555, dp);
			return 1;
		}
		s -= cmd.nc;
		if(s == 0){
			mkqid(&q, DEVCMDQID(0, Qcmd_clonus), 0, QTFILE);
			devdir(c, q, "clone", 0, "cmd", 0666, dp);
			return 1;
		}
		return -1;
	case Qcmd_clonus:
		if(s == 0){
			mkqid(&q, DEVCMDQID(0, Qcmd_clonus), 0, QTFILE);
			devdir(c, q, "clone", 0, "cmd", 0666, dp);
			return 1;
		}
		return -1;
	case Qcmd_convdir:
		return cmd3gen(c, Qcmd_convbase+s, dp);
	case Qcmd_data:
	case Qcmd_stderr:
	case Qcmd_ctl:
	case Qcmd_status:
	case Qcmd_wait:
		return cmd3gen(c, DEVCMDTYPE(c->qid), dp);
	}
	return -1;
}

static void
cmdinit(void)
{
	cmd.maxconv = 1000;
	cmd.conv = (CmdConv**)mallocz(sizeof(CmdConv*)*(cmd.maxconv+1), 1);
	/* cmd.conv is checked by cmdattach, below */
}

static Chan *
cmdattach(const char *spec)
{
	Chan *c;

	if(cmd.conv == nil)
		error(Enomem);
	c = devattach('C', spec);
	mkqid(&c->qid, DEVCMDQID(0, Qcmd_topdir), 0, QTDIR);
	return c;
}

static Walkqid*
cmdwalk(Chan *c, Chan *nc, const char **name, int nname)
{
	return devwalk(c, nc, name, nname, 0, 0, cmdgen);
}

static int
cmdstat(Chan *c, char *db, int n)
{
	return devstat(c, db, n, 0, 0, cmdgen);
}

static Chan *
cmdopen(Chan *c, int omode)
{
	int perm;
	CmdConv *cv;
	char *user;

	perm = 0;
	omode = openmode(omode);
	switch(omode) {
	case OREAD:
		perm = 4;
		break;
	case OWRITE:
		perm = 2;
		break;
	case ORDWR:
		perm = 6;
		break;
	}

	switch(DEVCMDTYPE(c->qid)) {
	default:
		break;
	case Qcmd_topdir:
	case Qcmd_cmd:
	case Qcmd_convdir:
	case Qcmd_status:
		if(omode != OREAD)
			error(Eperm);
		break;
	case Qcmd_clonus:
		qlock(&cmd.l);
		if(waserror()){
			qunlock(&cmd.l);
			nexterror();
		}
		cv = cmdclone(up->env->user);
		poperror();
		qunlock(&cmd.l);
		if(cv == 0)
			error(Enodev);
		mkqid(&c->qid, DEVCMDQID(cv->x, Qcmd_ctl), 0, QTFILE);
		break;
	case Qcmd_data:
	case Qcmd_stderr:
	case Qcmd_ctl:
	case Qcmd_wait:
		qlock(&cmd.l);
		cv = cmd.conv[DEVCMDCONV(c->qid)];
		qlock(&cv->l);
		if(waserror()){
			qunlock(&cv->l);
			qunlock(&cmd.l);
			nexterror();
		}
		user = up->env->user;
		if((perm & (cv->perm>>6)) != perm) {
			if(strcmp(user, cv->owner) != 0 ||
		 	  (perm & cv->perm) != perm)
				error(Eperm);
		}
		switch(DEVCMDTYPE(c->qid)){
		case Qcmd_data:
			if(omode == OWRITE || omode == ORDWR)
				cv->count[0]++;
			if(omode == OREAD || omode == ORDWR)
				cv->count[1]++;
			break;
		case Qcmd_stderr:
			if(omode != OREAD)
				error(Eperm);
			cv->count[2]++;
			break;
		case Qcmd_wait:
			if(cv->waitq == nil)
				cv->waitq = qopen(1024, Qmsg, nil, 0);
			break;
		}
		cv->inuse++;
		if(cv->inuse == 1) {
			cv->state = "Open";
			kstrdup(&cv->owner, user);
			cv->perm = 0660;
			cv->nice = 0;
		}
		poperror();
		qunlock(&cv->l);
		qunlock(&cmd.l);
		break;
	}
	c->mode = omode;
	c->flag |= COPEN;
	c->offset = 0;
	return c;
}

static void
closecmdconv(CmdConv *c)
{
	kstrdup(&c->owner, "cmd");
	kstrdup(&c->dir, rootdir);
	c->perm = 0666;
	c->state = "Closed";
	c->killonclose = 0;
	c->killed = 0;
	c->nice = 0;
	free(c->cmd);
	c->cmd = nil;
	if(c->waitq != nil){
		qfree(c->waitq);
		c->waitq = nil;
	}
	free(c->error);
	c->error = nil;
}

static void
cmdfdclose(CmdConv *c, int fd)
{
	if(--c->count[fd] == 0 && c->fd[fd] != -1){
		close(c->fd[fd]);
		c->fd[fd] = -1;
	}
}

static void
cmdclose(Chan *c)
{
	CmdConv *cc;
	int r;

	if((c->flag & COPEN) == 0)
		return;

	switch(DEVCMDTYPE(c->qid)) {
	case Qcmd_ctl:
	case Qcmd_data:
	case Qcmd_stderr:
	case Qcmd_wait:
		cc = cmd.conv[DEVCMDCONV(c->qid)];
		qlock(&cc->l);
		if(DEVCMDTYPE(c->qid) == Qcmd_data){
			if(c->mode == OWRITE || c->mode == ORDWR)
				cmdfdclose(cc, 0);
			if(c->mode == OREAD || c->mode == ORDWR)
				cmdfdclose(cc, 1);
		}else if(DEVCMDTYPE(c->qid) == Qcmd_stderr)
			cmdfdclose(cc, 2);

		r = --cc->inuse;
		if(cc->child != nil){
			if(!cc->killed)
			if(r == 0 || (cc->killonclose && DEVCMDTYPE(c->qid) == Qcmd_ctl)){
				oscmdkill(cc->child);
				cc->killed = 1;
			}
		}else if(r == 0)
			closecmdconv(cc);

		qunlock(&cc->l);
		break;
	}
}

static long
cmdread(Chan *ch, char *a, long n, vlong offset)
{
	CmdConv *c;
	char *cmds;
	int fd;

	USED(offset);

	switch(DEVCMDTYPE(ch->qid)) {
	default:
		error(Eperm);
	case Qcmd_cmd:
	case Qcmd_topdir:
	case Qcmd_convdir:
		return devdirread(ch, a, n, 0, 0, cmdgen);
	case Qcmd_ctl:
		sprint(up->genbuf, "%ld", DEVCMDCONV(ch->qid));
		return readstr(offset, a, n, up->genbuf);
	case Qcmd_status:
		c = cmd.conv[DEVCMDCONV(ch->qid)];
		cmds = "";
		if(c->cmd != nil)
			cmds = c->cmd->f[1];
		snprint(up->genbuf, sizeof(up->genbuf), "cmd/%d %d %s %q %q\n",
			c->x, c->inuse, c->state, c->dir, cmds);
		return readstr(offset, a, n, up->genbuf);
	case Qcmd_data:
	case Qcmd_stderr:
		fd = 1;
		if(DEVCMDTYPE(ch->qid) == Qcmd_stderr)
			fd = 2;
		c = cmd.conv[DEVCMDCONV(ch->qid)];
		qlock(&c->l);
		if(c->fd[fd] == -1){
			qunlock(&c->l);
			return 0;
		}
		qunlock(&c->l);
		osenter();
		n = read(c->fd[fd], a, n);
		osleave();
		if(n < 0)
			oserror();
		return n;
	case Qcmd_wait:
		c = cmd.conv[DEVCMDCONV(ch->qid)];
		return qread(c->waitq, a, n);
	}
}

static int
cmdstarted(void *a)
{
	CmdConv *c;

	c = (CmdConv *)a;
	return c->child != nil || c->error != nil || strcmp(c->state, "Execute") != 0;
}

enum
{
	CM_dir,
	CM_exec,
	CM_kill,
	CM_nice,
	CM_killonclose
};

static
Cmdtab cmdtab[] = {
	{CM_dir,		"dir",		2},
	{CM_exec,	"exec",		0},
	{CM_kill,	"kill",		1},
	{CM_nice,	"nice",		0},
	{CM_killonclose, "killonclose",	0},
};

static long
cmdwrite(Chan *ch, const char *a, long n, vlong offset)
{
	int i, r;
	CmdConv *c;
	Cmdbuf *cb;
	Cmdtab *ct;

	USED(offset);

	switch(DEVCMDTYPE(ch->qid)) {
	default:
		error(Eperm);
	case Qcmd_ctl:
		c = cmd.conv[DEVCMDCONV(ch->qid)];
		cb = parsecmd(a, n);
		if(waserror()){
			free(cb);
			nexterror();
		}
		ct = lookupcmd(cb, cmdtab, nelem(cmdtab));
		switch(ct->index){
		case CM_dir:
			kstrdup(&c->dir, cb->f[1]);
			break;
		case CM_exec:
			poperror();	/* cb */
			qlock(&c->l);
			if(waserror()){
				qunlock(&c->l);
				free(cb);
				nexterror();
			}
			if(c->child != nil || c->cmd != nil)
				error(Einuse);
			for(i = 0; i < nelem(c->fd); i++)
				if(c->fd[i] != -1)
					error(Einuse);
			if(cb->nf < 1)
				error(Etoosmall);
			kproc("cmdproc", cmdproc, c, 0);  /* BUG: check return value */	/* cmdproc held back until unlock below */
			free(c->cmd);
			c->cmd = cb;	/* don't free cb */
			c->state = "Execute";
			poperror();
			qunlock(&c->l);
			while(waserror())
				;
			sleep9(&c->startr, cmdstarted, c);
			poperror();
			if(c->error)
				error(c->error);
			return n;	/* avoid free(cb) below */
		case CM_kill:
			qlock(&c->l);
			if(waserror()){
				qunlock(&c->l);
				nexterror();
			}
			if(c->child == nil)
				error("not started");
			if(oscmdkill(c->child) < 0)
				oserror();
			poperror();
			qunlock(&c->l);
			break;
		case CM_nice:
			c->nice = cb->nf > 1? atoi(cb->f[1]): 1;
			break;
		case CM_killonclose:
			c->killonclose = 1;
			break;
		}
		poperror();
		free(cb);
		break;
	case Qcmd_data:
		c = cmd.conv[DEVCMDCONV(ch->qid)];
		qlock(&c->l);
		if(c->fd[0] == -1){
			qunlock(&c->l);
			error(Ehungup);
		}
		qunlock(&c->l);
		osenter();
		r = write(c->fd[0], a, n);
		osleave();
		if(r == 0)
			error(Ehungup);
		if(r < 0) {
			/* XXX perhaps should kill writer "write on closed pipe" here, 2nd time around? */
			oserror();
		}
		return r;
	}
	return n;
}

static int
cmdwstat(Chan *c, char *dp, int n)
{
	Dir *d;
	CmdConv *cv;

	switch(DEVCMDTYPE(c->qid)){
	default:
		error(Eperm);
	case Qcmd_ctl:
	case Qcmd_data:
	case Qcmd_stderr:
		d = (Dir*)malloc(sizeof(*d)+n);
		if(d == nil)
			error(Enomem);
		if(waserror()){
			free(d);
			nexterror();
		}
		n = convM2D(dp, n, d, (char*)&d[1]);
		if(n == 0)
			error(Eshortstat);
		cv = cmd.conv[DEVCMDCONV(c->qid)];
		if(!iseve() && strcmp(up->env->user, cv->owner) != 0)
			error(Eperm);
		if(!emptystr(d->uid))
			kstrdup(&cv->owner, d->uid);
		if(d->mode != ~0UL)
			cv->perm = d->mode & 0777;
		poperror();
		free(d);
		break;
	}
	return n;
}

static CmdConv*
cmdclone(char *user)
{
	CmdConv *c, **pp, **ep;
	int i;

	c = nil;
	ep = &cmd.conv[cmd.maxconv];
	for(pp = cmd.conv; pp < ep; pp++) {
		c = *pp;
		if(c == nil) {
			c = (CmdConv*)malloc(sizeof(CmdConv));
			if(c == nil)
				error(Enomem);
			qlock(&c->l);
			c->inuse = 1;
			c->x = pp - cmd.conv;
			cmd.nc++;
			*pp = c;
			break;
		}
		if(canqlock(&c->l)){
			if(c->inuse == 0 && c->child == nil)
				break;
			qunlock(&c->l);
		}
	}
	if(pp >= ep)
		return nil;

	c->inuse = 1;
	kstrdup(&c->owner, user);
	kstrdup(&c->dir, rootdir);
	c->perm = 0660;
	c->state = "Closed";
	for(i=0; i<nelem(c->fd); i++)
		c->fd[i] = -1;

	qunlock(&c->l);
	return c;
}

static void
cmdproc(void *a)
{
	CmdConv *c = (CmdConv *)a;
	int n;
	char status[ERRMAX];
	void *t;

	qlock(&c->l);
	if(Debug)
		print("f[0]=%q f[1]=%q\n", c->cmd->f[0], c->cmd->f[1]);
	if(waserror()){
		if(Debug)
			print("failed: %q\n", up->env->errstr);
		kstrdup(&c->error, up->env->errstr);
		c->state = "Done";
		qunlock(&c->l);
		wakeup9(&c->startr);
		pexit("cmdproc", 0);
	}
	t = oscmd(c->cmd->f+1, c->nice, c->dir, c->fd);
	if(t == nil)
		oserror();
	c->child = t;	/* to allow oscmdkill */
	poperror();
	qunlock(&c->l);
	wakeup9(&c->startr);
	if(Debug)
		print("started\n");
	while(waserror())
		oscmdkill(t);
	osenter();
	n = oscmdwait(t, status, sizeof(status));
	osleave();
	if(n < 0){
		oserrstr(up->genbuf, sizeof(up->genbuf));
		n = snprint(status, sizeof(status), "0 0 0 0 %q", up->genbuf);
	}
	qlock(&c->l);
	c->child = nil;
	oscmdfree(t);
	if(Debug){
		status[n]=0;
		print("done %d %d %d: %q\n", c->fd[0], c->fd[1], c->fd[2], status);
	}
	if(c->inuse > 0){
		c->state = "Done";
		if(c->waitq != nil)
			qproduce(c->waitq, status, n);
	}else
		closecmdconv(c);
	qunlock(&c->l);
	pexit("", 0);
}

Dev cmddevtab = {
	'C',
	"cmd",

	cmdinit,
	cmdattach,
	cmdwalk,
	cmdstat,
	cmdopen,
	devcreate,
	cmdclose,
	cmdread,
	devbread,
	cmdwrite,
	devbwrite,
	devremove,
	cmdwstat
};
