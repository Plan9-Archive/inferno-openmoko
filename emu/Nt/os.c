#include <windows.h>
#include <winsock.h>
#include <Lmcons.h>

#include <dat.h>
#include <fns.h>
#include <error.h>

#include <isa.h>
#include <interp.h> /* debug */


#define MAXSLEEPERS 1500 /* WTF */

extern  int cflag;

DWORD   PlatformId;
DWORD   consolestate;
static  char*   path = 0;
static  HANDLE  kbdh = INVALID_HANDLE_VALUE;
static  HANDLE  conh = INVALID_HANDLE_VALUE;
static  HANDLE  errh = INVALID_HANDLE_VALUE;
static  int donetermset = 0;
static  int sleepers = 0;

    Rune    *widen(const char *s);
    char        *narrowen(const Rune *ws);
    int     widebytes(const Rune *ws);
//  int     runeslen(const Rune*);
    Rune*   runesdup(const Rune*);
    Rune*   utftorunes(Rune*, const char*, int);
    char*   runestoutf(char*, const Rune*, int);
    int     runescmp(const Rune*, const Rune*);

#ifdef _MSC_VER_TLS
__declspec(thread)       Proc    *up;
#define setup(p) up=(p);
#else
static DWORD tlsi_up;
Proc* getup()
{
    return (Proc*)TlsGetValue(tlsi_up);
}
static void setup(Proc*v)
{
    TlsSetValue(tlsi_up, v);
}
#endif

HANDLE  ntfd2h(int);
int nth2fd(HANDLE);
void    termrestore(void);
char*   hosttype = "Nt";
char*   cputype = "386";

static void
pfree(Proc *p)
{
    Osenv *e;

    lock(&procs.l);
    if(p->prev)
        p->prev->next = p->next;
    else
        procs.head = p->next;

    if(p->next)
        p->next->prev = p->prev;
    else
        procs.tail = p->prev;
    unlock(&procs.l);

    e = p->env;
    if(e != nil) {
        closefgrp(e->fgrp);
        closepgrp(e->pgrp);
        closeegrp(e->egrp);
        closesigs(e->sigs);
    }
    free(e->user);
    free(p->prog);
    CloseHandle((HANDLE)p->os);
    free(p);
}

void
osblock(void)
{
    if(WaitForSingleObject((HANDLE)up->os, INFINITE) != WAIT_OBJECT_0)
        panic("osblock failed");
}

void
osready(Proc *p)
{
    if(SetEvent((HANDLE)p->os) == FALSE)
        panic("osready failed");
}

void
pexit(char *msg, int t)
{
    pfree(up);
    ExitThread(0);
}

LONG WINAPI TrapHandler(LPEXCEPTION_POINTERS ureg);

DWORD WINAPI
tramp(LPVOID p)
{
#if(_WIN32_WINNT >= 0x0400)
    if(sflag == 0)
        SetUnhandledExceptionFilter(&TrapHandler); /* Win2000+, does not work on NT4 and Win95 */
#endif
    setup((Proc*)p);
    up->func(up->arg);
    pexit("", 0);
    /* not reached */
    for(;;)
        panic("tramp");
    return 0;
}

int
kproc(const char *name, void (*func)(void*), void *arg, KProcFlags flags)
{
    DWORD h;
    Proc *p;
    Pgrp *pg;
    Fgrp *fg;
    Egrp *eg;

    p = newproc();
    if(p == nil){
        print("out of kernel processes\n");
        return -1;
    }
    p->os = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(p->os == NULL){
        pfree(p);
        print("can't allocate os event\n");
        return -1;
    }

    if(flags & KPDUPPG) {
        pg = up->env->pgrp;
        incref(&pg->r);
        p->env->pgrp = pg;
    }
    if(flags & KPDUPFDG) {
        fg = up->env->fgrp;
        incref(&fg->r);
        p->env->fgrp = fg;
    }
    if(flags & KPDUPENVG) {
        eg = up->env->egrp;
        incref(&eg->r);
        p->env->egrp = eg;
    }

    p->env->ui = up->env->ui;
    kstrdup(&p->env->user, up->env->user);
    strcpy(p->text, name);

    p->func = func;
    p->arg = arg;

    lock(&procs.l);
    if(procs.tail != nil) {
        p->prev = procs.tail;
        procs.tail->next = p;
    }
    else {
        procs.head = p;
        p->prev = nil;
    }
    procs.tail = p;
    unlock(&procs.l);

    p->pid = (int)CreateThread(0, 16384, tramp, p, 0, &h);
    print("kproc(%s) p->pid=%x\n", name, p->pid);
    if(p->pid == 0){
        pfree(p);
        print("ran out of  kernel processes\n");
        return -1;
    }
    return p->pid;
}
/*
#if(_WIN32_WINNT >= 0x0400)
void APIENTRY sleepintr(DWORD param)
{
}
#endif
/**/
void
oshostintr(Proc *p)
{
    if (p->syscall == SYSCALL_SOCK_SELECT)
        return;
    p->intwait = 0;
#if(_WIN32_WINNT >= 0x0400)
    /*
    if(p->syscall == SYS_SLEEP) {
        QueueUserAPC(sleepintr, (HANDLE) p->pid, (DWORD) p->pid);
    }*/
#endif
}
/**/
NORETURN
oslongjmp(void *regs, osjmpbuf env, int val)
{
    USED(regs);
    longjmp(env, val);
}

int
readkbd(void)
{
    DWORD r;
    char buf[1];

    if(ReadFile(kbdh, buf, sizeof(buf), &r, 0) == FALSE)
        panic("keyboard fail");
    if (r == 0)
        panic("keyboard EOF");

    if (buf[0] == 0x03) {
        // INTR (CTRL+C)
        cleanexit(0);
    }
    if(buf[0] == '\r')
        buf[0] = '\n';
    return buf[0];
}

unsigned lg10(unsigned n)
{
    if(n<10) return 1;
    if(n<100) return 2;
    if(n<1000) return 3;
    if(n<10000) return 4;
    if(n<100000) return 5;
    if(n<1000000) return 6;
    if(n<10000000) return 7;
    if(n<100000000) return 8;
    if(n<1000000000) return 9;
    return 10;
}

int fdheap = 2;
void heapview_callback( void* v, size_t size, int tag,
    const char* file, int line, const char* function, const char* comment)
{
    if(file==nil) file = "";
    if(function==nil) function = "";
    if(comment==nil) comment = "";
//  print("%08p %8d %8x %s:%d %s [%s]", v, size, tag, file, line, function, comment);
    fprint(fdheap,"o:%s:%d %*s %-16s %08p %8d %8x [%s]", file, line, 32-(strlen(file)+lg10(line)), "", function, v, size, tag, comment);
    //PRINT_TYPE(fdheap, ((Heap*)v)->t);
    fprint(fdheap,"\n");
    /*fprint(fdheap, "%08p %8d %8x %s:%d %s %s\n", v, size, tag, file, line, function, comment);*/
}

void mainview_callback( void* v, size_t size, int tag,
    const char* file, int line, const char* function, const char* comment)
{
    if(file==nil) file = "";
    if(function==nil) function = "";
    if(comment==nil) comment = "";
    fprint(fdheap, "o:%s:%d %*s %-16s %08p %8d %8x [%s]", file, line, 32-(strlen(file)+lg10(line)), "", function, v, size, tag, comment);
    fprint(fdheap, "\n");
    /*fprint(fdheap, "%08p %8d %8x %s:%d %s %s\n", v, size, tag, file, line, function, comment);*/
}


NORETURN
cleanexit(int x)
{
    if (x == 0) {
#if 0
        //fdheap = create("heap.log", O_WRONLY|O_TRUNC|O_CREAT, 0666);
        /*print("fdheap=%d\n", fdheap);*/
        //poolwalk(mainmem, mainview_callback);
        poolwalk(heapmem, heapview_callback);
        //poolwalk(imagmem, heapview_callback);
        /*print("close=%d\n", fdheap);*/
        //close(fdheap);
#endif
    }
    termrestore();
    //for(;;)ExitProcess(x);
    exit(x); /* it flushes files */
}

struct ecodes {
    DWORD   code;
    char*   name;
} ecodes[] = {
    EXCEPTION_ACCESS_VIOLATION,     "Segmentation violation",
    EXCEPTION_DATATYPE_MISALIGNMENT,    "Data Alignment",
    EXCEPTION_BREAKPOINT,                   "Breakpoint",
    EXCEPTION_SINGLE_STEP,                  "SingleStep",
    EXCEPTION_ARRAY_BOUNDS_EXCEEDED,    "Array Bounds Check",
    EXCEPTION_FLT_DENORMAL_OPERAND,     "Denormalized Float",
    EXCEPTION_FLT_DIVIDE_BY_ZERO,       "Floating Point Divide by Zero",
    EXCEPTION_FLT_INEXACT_RESULT,       "Inexact Floating Point",
    EXCEPTION_FLT_INVALID_OPERATION,    "Invalid Floating Operation",
    EXCEPTION_FLT_OVERFLOW,         "Floating Point Result Overflow",
    EXCEPTION_FLT_STACK_CHECK,      "Floating Point Stack Check",
    EXCEPTION_FLT_UNDERFLOW,        "Floating Point Result Underflow",
    EXCEPTION_INT_DIVIDE_BY_ZERO,       "Divide by Zero",
    EXCEPTION_INT_OVERFLOW,         "Integer Overflow",
    EXCEPTION_PRIV_INSTRUCTION,     "Privileged Instruction",
    EXCEPTION_IN_PAGE_ERROR,        "Page-in Error",
    EXCEPTION_ILLEGAL_INSTRUCTION,      "Illegal Instruction",
    EXCEPTION_NONCONTINUABLE_EXCEPTION, "Non-Continuable Exception",
    EXCEPTION_STACK_OVERFLOW,       "Stack Overflow",
    EXCEPTION_INVALID_DISPOSITION,      "Invalid Disposition",
    EXCEPTION_GUARD_PAGE,           "Guard Page Violation",
    0,                  nil
};

void
dodisfault(void)
{
    disfault(nil, up->env->errstr);
}

LONG WINAPI
TrapHandler(LPEXCEPTION_POINTERS ureg)
{
    int i;
    char *name;
    DWORD code;
    DWORD pc;
    char buf[ERRMAX];

    code = ureg->ExceptionRecord->ExceptionCode;
    pc = ureg->ContextRecord->Eip;
    print("TrapHandler code=%08uX pc=%08uX\n", code, pc);

    name = nil;
    for(i = 0; i < nelem(ecodes); i++) {
        if(ecodes[i].code == code) {
            name = ecodes[i].name;
            break;
        }
    }

    if(name == nil) {
        snprint(buf, sizeof(buf), "Unrecognized Machine Trap (%.8lux)\n", code);
        name = buf;
    }

    if(pc != 0) {
        snprint(buf, sizeof(buf), "%s: pc=0x%lux", name, pc);
        name = buf;
    }

    /* YUCK! */
    strncpy(up->env->errstr, name, ERRMAX);
    switch (code) {
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
        /* clear exception flags and register stack */
#ifdef _MSC_VER
        _asm { fnclex };
#else
        __asm__ ("fnclex"); // TODO ; fwait ?
#endif
        ureg->ContextRecord->FloatSave.StatusWord = 0x0000;
        ureg->ContextRecord->FloatSave.TagWord = 0xffff;
    }
    ureg->ContextRecord->Eip = (DWORD)dodisfault;
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void
termset(void)
{
    DWORD flag;

    if(donetermset)
        return;
    donetermset = 1;
    conh = GetStdHandle(STD_OUTPUT_HANDLE);
    kbdh = GetStdHandle(STD_INPUT_HANDLE);
    errh = GetStdHandle(STD_ERROR_HANDLE);
    if(errh == INVALID_HANDLE_VALUE)
        errh = conh;

    // The following will fail if kbdh not from console (e.g. a pipe)
    // in which case we don't care
    GetConsoleMode(kbdh, &consolestate);
    flag = consolestate;
    flag = flag & ~(ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);
    SetConsoleMode(kbdh, flag);
}

void
termrestore(void)
{
    if(kbdh != INVALID_HANDLE_VALUE)
        SetConsoleMode(kbdh, consolestate);
}

static  int rebootok = 0;   /* is shutdown -r supported? */

void
osreboot(const char *file, const char **argv)
{
    if(rebootok){
        termrestore();
        execvp(file, argv);
        panic("reboot failure");
    }
}

NORETURN
libinit(const char *imod)
{
    WSADATA wasdat;
    DWORD lasterror, namelen;
    OSVERSIONINFO os;
    char sys[64], uname[64];
    Rune wuname[UNLEN + 1];
    char *uns;

    os.dwOSVersionInfoSize = sizeof(os);
    if(!GetVersionEx(&os))
        panic("can't get os version");
    PlatformId = os.dwPlatformId;
    if (PlatformId == VER_PLATFORM_WIN32_NT) {  /* true for NT and 2000 */
        rebootok = 1;
    } else {
        rebootok = 0;
    }
    termset();

    if((int)INVALID_HANDLE_VALUE != -1 || sizeof(HANDLE) != sizeof(int))
        panic("invalid handle value or size");

    if(WSAStartup(MAKEWORD(1, 1), &wasdat) != 0)
        panic("no winsock.dll");

    gethostname(sys, sizeof(sys));
    kstrdup(&ossysname, sys);
//  if(sflag == 0)
//      SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)TrapHandler);

    path = getenv("PATH");
    if(path == nil)
        path = ".";

    setup(newproc()); /* create Proc for main thread */
    if(up == nil)
        panic("cannot create kernel process");

    strcpy(uname, "inferno");
    namelen = sizeof(wuname);
    if(GetUserNameW(wuname, &namelen) != TRUE) {
        lasterror = GetLastError();
        if(PlatformId == VER_PLATFORM_WIN32_NT || lasterror != ERROR_NOT_LOGGED_ON)
            print("cannot GetUserName: %d\n", lasterror);
    }else{
        uns = narrowen(wuname);
        snprint(uname, sizeof(uname), "%s", uns);
        free(uns);
    }
    kstrdup(&eve, uname);

    emuinit(imod);
}

void
FPsave(FPU *fptr)
{
#ifdef _MSC_VER
    _asm {
        mov eax, fptr
        fstenv  [eax]
    }
#else
    __asm__ ("fstenv %0" : : "m" (*fptr));
#endif
}

void
FPrestore(FPU *fptr)
{
#ifdef _MSC_VER
    _asm {
        mov eax, fptr
        fldenv  [eax]
    }
#else
    __asm__ ("fldenv %0" : : "m" (*fptr));
#endif
}

int
close(int fd)
{
    if(fd == -1)
        return 0;
    CloseHandle(ntfd2h(fd));
    return 0;
}

int
read(int fd, void *buf, size_t n)
{
    DWORD bytes = n;
    if(!ReadFile(ntfd2h(fd), buf, bytes, &bytes, NULL))
        return -1;
    return n;
}

int
write(int fd, const void *buf, size_t n)
{
    HANDLE h;
    DWORD bytes = n;

    if(fd == 1 || fd == 2){
        if(!donetermset)
            termset();
        if(fd == 1)
            h = conh;
        else
            h = errh;
        if(h == INVALID_HANDLE_VALUE)
            return -1;
        if(!WriteFile(h, buf, bytes, &bytes, NULL))
            return -1;
        return n;
    }
    panic("write to hande=%d (and where it was open? we do not have working open/create on Windows)");
    if(!WriteFile(ntfd2h(fd), buf, bytes, &bytes, NULL))
        return -1;
    return n;
}

/*
 * map handles and fds.
 * this code assumes sizeof(HANDLE) == sizeof(int),
 * that INVALID_HANDLE_VALUE is -1, and assumes
 * that all tests of invalid fds check only for -1, not < 0
 */
int
nth2fd(HANDLE h)
{
    return (int)h;
}

HANDLE
ntfd2h(int fd)
{
    return (HANDLE)fd;
}

void
oslopri(void)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
}


void* mem_reserve(ulong size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

void* mem_commit(void* p, ulong size)
{
    return VirtualAlloc(p, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

ulong
getcallerpc(void *arg)
{
/*
    ulong cpc;
    _asm {
        mov eax, dword ptr [ebp]
        mov eax, dword ptr [eax+4]
        mov dword ptr cpc, eax
    }
    return cpc;
*/
    return 0;
}

/*
 * Return an abitrary millisecond clock time
 */
long
osmillisec(void)
{
    return GetTickCount();
}

#define SEC2MIN 60L
#define SEC2HOUR (60L*SEC2MIN)
#define SEC2DAY (24L*SEC2HOUR)

/*
 *  days per month plus days/year
 */
static  int dmsize[] =
{
    365, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static  int ldmsize[] =
{
    366, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*
 *  return the days/month for the given year
 */
static int*
yrsize(int yr)
{
    /* a leap year is a multiple of 4, excluding centuries
     * that are not multiples of 400 */
    if( (yr % 4 == 0) && (yr % 100 != 0 || yr % 400 == 0) )
        return ldmsize;
    else
        return dmsize;
}

static long
tm2sec(SYSTEMTIME *tm)
{
    long secs;
    int i, *d2m;

    secs = 0;

    /*
     *  seconds per year
     */
    for(i = 1970; i < tm->wYear; i++){
        d2m = yrsize(i);
        secs += d2m[0] * SEC2DAY;
    }

    /*
     *  seconds per month
     */
    d2m = yrsize(tm->wYear);
    for(i = 1; i < tm->wMonth; i++)
        secs += d2m[i] * SEC2DAY;

    /*
     * secs in last month
     */
    secs += (tm->wDay-1) * SEC2DAY;

    /*
     * hours, minutes, seconds
     */
    secs += tm->wHour * SEC2HOUR;
    secs += tm->wMinute * SEC2MIN;
    secs += tm->wSecond;

    return secs;
}

/*
 * Return the time since the epoch in microseconds
 * The epoch is defined at 1 Jan 1970
 */
vlong
osusectime(void)
{
    SYSTEMTIME tm;
    vlong secs;

    GetSystemTime(&tm);
    secs = tm2sec(&tm);
    return secs * 1000000 + tm.wMilliseconds * 1000;
}

vlong
osnsec(void)
{
    return osusectime()*1000;   /* TO DO better */
}

int
osmillisleep(ulong milsec)
{
    Sleep(milsec);
    return 0;
}

int
limbosleep(ulong milsec)
{
    if (sleepers > MAXSLEEPERS) /* BUG WTF MAXSLEEPERS? */
        return -1;
    sleepers++;
    up->syscall = SYSCALL_SLEEP;
    /*SleepEx(milsec, TRUE); /* TODO: alertable? */
    Sleep(milsec);
    up->syscall = SYSCALL_NO;
    sleepers--;
    return 0;
}

void
osyield(void)
{
    /*Sleep(0);*/
    Sleep(1); /* Sleep(0) does not yield well? */
}

NORETURN
ospause(void)
{
      for(;;)
              Sleep(1000000);
}

int
segflush(void *a, ulong n)
{
    return 0;
}

Rune *
widen(const char *s)
{
    int n;
    Rune *ws;

    n = utflen(s) + 1;
    ws = (Rune*)smalloc(n*sizeof(wchar_t));
    utftorunes(ws, s, n);
    return ws;
}


char *
narrowen(const Rune *ws)
{
    char *s;
    int n;

    n = widebytes(ws);
    s = (char*)smalloc(n);
    runestoutf(s, ws, n);
    return s;
}


int
widebytes(const wchar_t *ws)
{
    int n = 0;

    while (*ws)
        n += runelen(*ws++);
    return n+1;
}
