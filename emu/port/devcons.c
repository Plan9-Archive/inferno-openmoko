#include <dat.h>
#include <fns.h>
#include <error.h>
#include <version.h>
#include <mp.h>
#include <libsec.h>
#include <keyboard.h>

extern int cflag;
int exdebug;
extern int keepbroken;

enum
{
    Qcons_dir,
    Qcons_cons,
    Qcons_consctl,
    Qcons_drivers,
    Qcons_hostowner,
    Qcons_hoststdin,
    Qcons_hoststdout,
    Qcons_hoststderr,
    Qcons_jit,
    Qcons_keyboard,
    Qcons_kprint,
    Qcons_memory,
    Qcons_msec,
    Qcons_notquiterandom,
    Qcons_null,
    Qcons_pin,
    Qcons_random,
    Qcons_scancode,
    Qcons_sysctl,
    Qcons_sysname,
    Qcons_time,
    Qcons_user
};

Dirtab contab[] =
{
    ".",            {Qcons_dir, 0, QTDIR},  0,  DMDIR|0555,
    "cons",         {Qcons_cons},       0,  0666,
    "consctl",      {Qcons_consctl},    0,  0222,
    "drivers",      {Qcons_drivers},    0,  0444,
    "hostowner",        {Qcons_hostowner},  0,  0644,
    "hoststdin",        {Qcons_hoststdin},  0,  0444,
    "hoststdout",       {Qcons_hoststdout}, 0,  0222,
    "hoststderr",       {Qcons_hoststderr}, 0,  0222,
    "jit",          {Qcons_jit},        0,  0666,
    "keyboard",     {Qcons_keyboard},   0,  0666,
    "kprint",       {Qcons_kprint},     0,  0444,
    "memory",       {Qcons_memory},     0,  0444,
    "msec",         {Qcons_msec},       NUMSIZE,0444,
    "notquiterandom",   {Qcons_notquiterandom}, 0,  0444,
    "null",         {Qcons_null},       0,  0666,
    "pin",          {Qcons_pin},        0,  0666,
    "random",       {Qcons_random},     0,  0444,
    "scancode",     {Qcons_scancode},   0,  0444,
    "sysctl",       {Qcons_sysctl},     0,  0644,
    "sysname",      {Qcons_sysname},    0,  0644,
    "time",         {Qcons_time},       0,  0644,
    "user",         {Qcons_user},       0,  0644,
};

Queue*  gkscanq;        /* Graphics keyboard raw scancodes */
char*   gkscanid;       /* name of raw scan format (if defined) */
Queue*  gkbdq;          /* Graphics keyboard unprocessed input */
Queue*  kbdq;           /* Console window unprocessed keyboard input */
Queue*  lineq;          /* processed console input */

char    *ossysname;

static struct
{
    RWlock l;
    Queue*  q;
} kprintq;

vlong   timeoffset;

extern int  dflag;

static int  sysconwrite(const char*, ulong);
extern const char** rebootargv;

static struct
{
    QLock   q;
    QLock   gq;     /* separate lock for the graphical input */

    int raw;        /* true if we shouldn't process input */
    Ref ctl;        /* number of opens to the control file */
    Ref ptr;        /* number of opens to the ptr file */
    int scan;       /* true if reading raw scancodes */
    int x;      /* index into line */
    char    line[1024]; /* current input line */

    Rune    c;
    int count;
} kbd;

void
kbdslave(const void *a)
{
    char b;

    USED(a);
    for(;;) {
        b = readkbd();
        if(kbd.raw == 0){
            switch(b){
            case 0x15:
                write(1, "^U\n", 3);
                break;
            default:
                write(1, &b, 1);
                break;
            }
        }
        qproduce(kbdq, &b, 1);
    }
    /* pexit("kbdslave", 0); */ /* not reached */
}

void
gkbdputc(Queue *q, Rune ch)
{
    int n;
    Rune r;
    static char kc[5*UTFmax];
    static int nk, collecting = 0;
    char buf[UTFmax];

    r = ch;
    if(r == Latin) {
        collecting = 1;
        nk = 0;
        return;
    }
    if(collecting) {
        int c;
        nk += runetochar(&kc[nk], r);
        c = latin1(kc, nk);
        if(c < -1)  /* need more keystrokes */
            return;
        collecting = 0;
        if(c == -1) {   /* invalid sequence */
            qproduce(q, kc, nk);
            return;
        }
        r = (Rune)c;
    }
    n = runetochar(buf, r);
    if(n == 0)
        return;
    /* if(!isdbgkey(r)) */
        qproduce(q, buf, n);
}

void
consinit(void)
{
    kbdq = qopen(512, 0, nil, nil);
    if(kbdq == 0)
        panic("no memory");
    lineq = qopen(2*1024, 0, nil, nil);
    if(lineq == 0)
        panic("no memory");
    gkbdq = qopen(512, 0, nil, nil);
    if(gkbdq == 0)
        panic("no memory");
    randominit();
}

/*
 *  return true if current user is eve
 */
int
iseve(void)
{
    return strcmp(eve, up->env->user) == 0;
}

static Chan*
consattach(const char *spec)
{
    static int kp;

    if(kp == 0 && !dflag) {
        kp = 1;
        kproc("kbd", kbdslave, 0, 0);
    }
    return devattach('c', spec);
}

static Walkqid*
conswalk(Chan *c, Chan *nc, const char **name, int nname)
{
    return devwalk(c, nc, name, nname, contab, nelem(contab), devgen);
}

static int
consstat(Chan *c, char *db, int n)
{
    return devstat(c, db, n, contab, nelem(contab), devgen);
}

static Chan*
consopen(Chan *c, int omode)
{
    c = devopen(c, omode, contab, nelem(contab), devgen);
    switch((ulong)c->qid.path) {
    case Qcons_consctl:
        incref(&kbd.ctl);
        break;

    case Qcons_scancode:
        qlock(&kbd.gq);
        if(gkscanq != nil || gkscanid == nil) {
            qunlock(&kbd.q);
            c->flag &= ~COPEN;
            if(gkscanq)
                error(Einuse);
            else
                error("not supported");
        }
        gkscanq = qopen(256, 0, nil, nil);
        qunlock(&kbd.gq);
        break;

    case Qcons_kprint:
        wlock(&kprintq.l);
        if(waserror()){
            wunlock(&kprintq.l);
            c->flag &= ~COPEN;
            nexterror();
        }
        if(kprintq.q != nil)
            error(Einuse);
        kprintq.q = qopen(32*1024, Qcoalesce, nil, nil);
        if(kprintq.q == nil)
            error(Enomem);
        qnoblock(kprintq.q, 1);
        poperror();
        wunlock(&kprintq.l);
        c->iounit = qiomaxatomic;
        break;
    }
    return c;
}

static void
consclose(Chan *c)
{
    if((c->flag & COPEN) == 0)
        return;

    switch((ulong)c->qid.path) {
    case Qcons_consctl:
        /* last close of control file turns off raw */
        if(decref(&kbd.ctl) == 0)
            kbd.raw = 0;
        break;

    case Qcons_scancode:
        qlock(&kbd.gq);
        if(gkscanq) {
            qfree(gkscanq);
            gkscanq = nil;
        }
        qunlock(&kbd.gq);
        break;

    case Qcons_kprint:
        wlock(&kprintq.l);
        qfree(kprintq.q);
        kprintq.q = nil;
        wunlock(&kprintq.l);
        break;
    }
}

static size_t
consread(Chan *c, __out_ecount(n) char *va, size_t n, vlong offset)
{
    ulong l;
    int i, send;
    char *p, buf[64], ch;

    if(c->qid.type & QTDIR)
        return devdirread(c, va, n, contab, nelem(contab), devgen);

    switch((ulong)c->qid.path) {
    default:
        error(Egreg);

    case Qcons_sysctl:
        return readstr(offset, va, n, VERSION);

    case Qcons_sysname:
        if(ossysname == nil)
            return 0;
        return readstr(offset, va, n, ossysname);

    case Qcons_random:
        return randomread(va, n);

    case Qcons_notquiterandom:
        genrandom(va, n);
        return n;

    case Qcons_pin:
        p = "pin set";
        if(up->env->pgrp->pin == Nopin)
            p = "no pin";
        return readstr(offset, va, n, p);

    case Qcons_hostowner:
        return readstr(offset, va, n, eve);

    case Qcons_hoststdin:
        return read(0, va, n);  /* should be pread */

    case Qcons_user:
        return readstr(offset, va, n, up->env->user);

    case Qcons_jit:
        snprint(buf, sizeof(buf), "%d", cflag);
        return readstr(offset, va, n, buf);

    case Qcons_time:
        snprint(buf, sizeof(buf), "%.lld", timeoffset + osusectime());
        return readstr(offset, va, n, buf);

    case Qcons_drivers:
        p = (char*)malloc(READSTR);
        if(p == nil)
            error(Enomem);
        l = 0;
        for(i = 0; devtab[i] != nil; i++)
            l += snprint(p+l, READSTR-l, "#%C %s\n", devtab[i]->dc,  devtab[i]->name);
        if(waserror()){
            free(p);
            nexterror();
        }
        n = readstr(offset, va, n, p);
        poperror();
        free(p);
        return n;

    case Qcons_memory:
        return poolread(va, n, offset);

    case Qcons_null:
        return 0;

    case Qcons_msec:
        return readnum(offset, va, n, osmillisec(), NUMSIZE);

    case Qcons_cons:
        qlock(&kbd.q);
        if(waserror()){
            qunlock(&kbd.q);
            nexterror();
        }

        if(dflag)
            error(Enonexist);

        while(!qcanread(lineq)) {
            if(qread(kbdq, &ch, 1) == 0)
                continue;
            send = 0;
            if(ch == 0){
                /* flush output on rawoff -> rawon */
                if(kbd.x > 0)
                    send = !qcanread(kbdq);
            }else if(kbd.raw){
                kbd.line[kbd.x++] = ch;
                send = !qcanread(kbdq);
            }else{
                switch(ch){
                case '\b':
                    if(kbd.x)
                        kbd.x--;
                    break;
                case 0x15:
                    kbd.x = 0;
                    break;
                case 0x04:
                    send = 1;
                    break;
                case '\n':
                    send = 1;
                default:
                    kbd.line[kbd.x++] = ch;
                    break;
                }
            }
            if(send || kbd.x == sizeof kbd.line){
                qwrite(lineq, kbd.line, kbd.x);
                kbd.x = 0;
            }
        }
        n = qread(lineq, va, n);
        qunlock(&kbd.q);
        poperror();
        return n;

    case Qcons_scancode:
        if(offset == 0)
            return readstr(0, va, n, gkscanid);
        return qread(gkscanq, va, n);

    case Qcons_keyboard:
        return qread(gkbdq, va, n);

    case Qcons_kprint:
        rlock(&kprintq.l);
        if(waserror()){
            runlock(&kprintq.l);
            nexterror();
        }
        n = qread(kprintq.q, va, n);
        poperror();
        runlock(&kprintq.l);
        return n;
    }
}

static size_t
conswrite(Chan *c, __in_ecount(n) const char *va, size_t n, vlong offset)
{
    char buf[128], ch;
    const char *a;
    int x;

    if(c->qid.type & QTDIR)
        error(Eperm);

    switch((ulong)c->qid.path) {
    default:
        error(Egreg);

    case Qcons_cons:
        if(canrlock(&kprintq.l)){
            if(kprintq.q != nil){
                if(waserror()){
                    runlock(&kprintq.l);
                    nexterror();
                }
                qwrite(kprintq.q, va, n);
                poperror();
                runlock(&kprintq.l);
                return n;
            }
            runlock(&kprintq.l);
        }
        return write(1, va, n);

    case Qcons_sysctl:
        return sysconwrite(va, n);

    case Qcons_consctl:
        if(n >= sizeof(buf))
            n = sizeof(buf)-1;
        strncpy(buf, va, n);
        buf[n] = 0;
        for(a = buf; a;){
            if(strncmp(a, "rawon", 5) == 0){
                kbd.raw = 1;
                /* clumsy hack - wake up reader */
                ch = 0;
                qwrite(kbdq, &ch, 1);
            } else if(strncmp(buf, "rawoff", 6) == 0){
                kbd.raw = 0;
            }
            if((a = strchr(a, ' ')) != nil)
                a++;
        }
        break;

    case Qcons_keyboard:
        for(x=0; x<n; ) { // BUG, buffer overflow
            Rune r;
            x += chartorune(&r, va+x);
            gkbdputc(gkbdq, r);
        }
        break;

    case Qcons_null:
        break;

    case Qcons_pin:
        if(up->env->pgrp->pin != Nopin)
            error("pin already set");
        if(n >= sizeof(buf))
            n = sizeof(buf)-1;
        strncpy(buf, va, n);
        buf[n] = '\0';
        up->env->pgrp->pin = atoi(buf);
        break;

    case Qcons_time:
        if(n >= sizeof(buf))
            n = sizeof(buf)-1;
        strncpy(buf, va, n);
        buf[n] = '\0';
        timeoffset = strtoll(buf, 0, 0)-osusectime();
        break;

    case Qcons_hostowner:
        if(!iseve())
            error(Eperm);
        if(offset != 0 || n >= sizeof(buf))
            error(Ebadarg);
        memmove(buf, va, n);
        buf[n] = '\0';
        if(n > 0 && buf[n-1] == '\n')
            buf[--n] = '\0';
        if(n == 0)
            error(Ebadarg);
        /* renameuser(eve, buf); */
        /* renameproguser(eve, buf); */
        kstrdup(&eve, buf);
        kstrdup(&up->env->user, buf);
        break;

    case Qcons_user:
        if(!iseve())
            error(Eperm);
        if(offset != 0)
            error(Ebadarg);
        if(n <= 0 || n >= sizeof(buf))
            error(Ebadarg);
        strncpy(buf, va, n);
        buf[n] = '\0';
        if(n > 0 && buf[n-1] == '\n')
            buf[--n] = '\0';
        if(n == 0)
            error(Ebadarg);
        setid(buf, 0);
        break;

    case Qcons_hoststdout:
        return write(1, va, n);

    case Qcons_hoststderr:
        return write(2, va, n);

    case Qcons_jit:
        if(n >= sizeof(buf))
            n = sizeof(buf)-1;
        strncpy(buf, va, n);
        buf[n] = '\0';
        x = atoi(buf);
        if(x < 0 || x > 9)
            error(Ebadarg);
        cflag = x;
        break;

    case Qcons_sysname:
        if(offset != 0)
            error(Ebadarg);
        if(n < 0 || n >= sizeof(buf))
            error(Ebadarg);
        strncpy(buf, va, n);
        buf[n] = '\0';
        if(n>0 && buf[n-1] == '\n')
            buf[n-1] = 0;
        kstrdup(&ossysname, buf);
        break;
    }
    return n;
}

static int
sysconwrite(const char *va, ulong count)
{
    Cmdbuf *cb;
    int e;
    cb = parsecmd(va, count);
    if(waserror()){
        free(cb);
        nexterror();
    }
    if(cb->nf == 0)
        error(Enoctl);
    if(strcmp(cb->f[0], "reboot") == 0){
        osreboot(rebootargv[0], rebootargv);
        error("reboot not supported");
    }else if(strcmp(cb->f[0], "halt") == 0){
        if(cb->nf > 1)
            e = atoi(cb->f[1]);
        else
            e = 0;
        cleanexit(e);       /* XXX ignored for the time being (and should be a string anyway) */
    }else if(strcmp(cb->f[0], "broken") == 0)
        keepbroken = 1;
    else if(strcmp(cb->f[0], "nobroken") == 0)
        keepbroken = 0;
    else if(strcmp(cb->f[0], "exdebug") == 0)
        exdebug = !exdebug;
    else
        error(Enoctl);
    poperror();
    free(cb);
    return count;
}

Dev consdevtab = {
    'c',
    "cons",

    consinit,
    consattach,
    conswalk,
    consstat,
    consopen,
    devcreate,
    consclose,
    consread,
    devbread,
    conswrite,
    devbwrite,
    devremove,
    devwstat
};

static  ulong   randn;

static void
seedrand(void)
{
    randomread((char*)&randn, sizeof(randn));
}

int
nrand(int n)
{
    if(randn == 0)
        seedrand();
    randn = randn*1103515245 + 12345 + osusectime();
    return (randn>>16) % n;
}

int
rand(void)
{
    nrand(1);
    return randn;
}

ulong
truerand(void)
{
    ulong x;

    randomread((char*)&x, sizeof(x));
    return x;
}

QLock grandomlk;

void
_genrandomqlock(void)
{
    qlock(&grandomlk);
}


void
_genrandomqunlock(void)
{
    qunlock(&grandomlk);
}
