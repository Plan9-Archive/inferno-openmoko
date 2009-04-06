#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
//#include <math.h>
#include <fcntl.h>
#include <setjmp.h>
#include <float.h>
#include <time.h>
#include <process.h>
//#include <io.h>
#include <direct.h>
/*#include <stddef.h>*/

#define strtod infstrtod

#ifdef __cplusplus
#define this _this
#define new _new
#define delete _delete
#define public _public
#define export _export
#endif

#ifndef _MSC_VER
#define __notnull
#define __in
#define __in_z
#define __in_z_opt
#define __out_z
#define __format_string
#define __out_bcount_z(x)

#define __in_ecount(a)
#define __in_ecount_z(a)
#define __in_ecount_opt(a)

#define __out_ecount_part(a,b)
#define __out_ecount_full(a)
#define __out_ecount_full_z(a)
#define __out_ecount_z(a)
#define __out_ecount(a)

#define __inout_ecount_opt(a)
#define __inout_ecount_full(a)
#define __inout_ecount(a)
#define __inout
#define __checkReturn 
#endif

#define getwd   infgetwd

typedef struct Proc Proc;
typedef struct Prog Prog;
typedef struct Chan Chan;
typedef struct Osenv    Osenv;
typedef struct SrvFile  SrvFile;

/*
 * math module dtoa
 */
#define __LITTLE_ENDIAN

   /* there must be a Win32 header macro for endian-ness!!! */

/*
 * disable various silly warnings
 * 4018 signed/unsigned comparison
 * 4245 signed/unsigned conversion
 * 4244 long to char conversion
 * 4068 unknown pragma
 * 4090 different volatile quals
 * 4554 operator precedence
 * 4146 unary - on unsigned type
 * 4996 `deprecated' functions: they often suggest non-portable replacements
 */
#pragma warning( disable : 4305 4244 4102 4761 4018 4245 4244 4068 4090 4554 4146 4996 4237 6244)

#define NORETURN __declspec(noreturn) void

#define nil     NULL

typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef signed char schar;
typedef unsigned short  ushort;
typedef wchar_t     Rune;
typedef long long   vlong;
typedef unsigned long long  uvlong;
typedef unsigned int    mpdigit;    /* for /sys/include/mp.h */
typedef unsigned long long  u64int;
typedef unsigned int        u32int;
typedef unsigned short      u16int;
typedef unsigned char       u8int;
typedef unsigned long uintptr;

#define USED(x)     if(x){}else{}
#define SET(x)

/*#undef nelem*/
#define nelem(x)    (sizeof(x)/sizeof((x)[0]))
/* freetype macroses are not happy with stddef.h  although they are completely equal */
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
extern NORETURN panic(__in_z __format_string const char *fmt, ...);

#define assert(x)   ((x)||(panic("assert failed: %s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, #x),0))

/*
 * most mem and string routines are declared by ANSI/POSIX files above
 */
extern  char*   strecpy(__out_bcount_z(e-to) char *to, __in char *e, __in_z const char *from);
extern  int cistrncmp(char*, char*, int);
extern  int cistrcmp(char*, char*);
extern  char*   cistrstr(char*, char*);
extern  int tokenize(char*, char**, int);
extern  vlong   strtoll(const char*, char**, int);
extern  uvlong  strtoull(const char*, char**, int);

enum
{
    UTFmax      = 3,        /* maximum bytes per rune */
    Runesync    = 0x80,     /* cannot represent part of a UTF sequence (<) */
    Runeself    = 0x80,     /* rune and UTF sequences are the same (<) */
    Runeerror   = 0x80      /* decoding error in UTF */
};

/*
 * rune routines
 */
extern  int         runetochar(__out_ecount_part(3, return) char*, __in_ecount(1) const Rune*);
extern  int         chartorune(__out_ecount_full(1) Rune*, __in_ecount(3) const char*);
extern  int         runelen(long);
extern  int         runenlen(const Rune*, int);
extern  int         fullrune(const char*, int);
extern  int         utflen(const char*);
extern  int         utfnlen(const char*, long);
extern  /*const*/ char* utfrune(/*const*/ char*, long);
extern  /*const*/ char* utfrrune(/*const*/ char*, long);
extern  const char* utfutf(const char*, const char*);
extern  char*       utfecpy(char*, char*, const char*);

extern  Rune*       runestrcat(Rune*, const Rune*);
extern  const Rune* runestrchr(const Rune*, Rune);
extern  int         runestrcmp(const Rune*, const Rune*);
extern  Rune*       runestrcpy(Rune*, const Rune*);
extern  Rune*       runestrncpy(Rune*, const Rune*, long);
extern  Rune*       runestrecpy(Rune*, const Rune*, const Rune*);
extern  Rune*       runestrdup(const Rune*);
extern  Rune*       runestrncat(Rune*, const Rune*, long);
extern  int         runestrncmp(Rune*, Rune*, long);
extern  const Rune* runestrrchr(const Rune*, Rune);
extern  long        runestrlen(const Rune*);
extern  const Rune* runestrstr(const Rune*, const Rune*);

extern  Rune        tolowerrune(Rune);
extern  Rune        totitlerune(Rune);
extern  Rune        toupperrune(Rune);
extern  int         isalpharune(Rune);
extern  int         islowerrune(Rune);
extern  int         isspacerune(Rune);
extern  int         istitlerune(Rune);
extern  int         isupperrune(Rune);

/*
 * malloc
 */
#ifndef EMU
extern  void*   kmalloc(size_t);
extern  void*   smalloc(size_t);
extern  void*   malloc(size_t);
extern  void*   mallocz(size_t, int);
extern  void    free(void*);
extern  void*   realloc(void*, size_t);
extern  void*   calloc(size_t, size_t);
extern  size_t  msize(void *v);
extern  char*   strdup(const char*);

//extern    void    setmalloctag(void*, ulong);
//extern    void    setrealloctag(void*, ulong);
//extern    ulong   getmalloctag(void*);
//extern    ulong   getrealloctag(void*);
//extern    void*   malloctopoolblock(void*);
#else
extern  void*   v_kmalloc(size_t size, const char*, int, const char*);
extern  void*   v_smalloc(size_t size, const char*, int, const char*);
extern  void*   v_malloc(size_t size, const char*, int, const char*);
extern  void*   v_mallocz(size_t size, int clr, const char*, int, const char*);
extern  void    v_free(void *v, const char*, int, const char*);
extern  void*   v_realloc(void *v, size_t size, const char*, int, const char*);
extern  void*   v_calloc(size_t num, size_t size, const char*, int, const char*);
extern  size_t  v_msize(void *v, const char*, int, const char*);
extern  char*   v_strdup(const char*, const char*, int, const char*);


#define kmalloc(size)       v_kmalloc(size, __FILE__, __LINE__, __FUNCTION__)
#define smalloc(size)       v_smalloc(size, __FILE__, __LINE__, __FUNCTION__)
#define malloc(size)        v_malloc(size, __FILE__, __LINE__, __FUNCTION__)
#define mallocz(size, clr)  v_mallocz(size, clr, __FILE__, __LINE__, __FUNCTION__)
#define free(v)             {v_free((void*)(v), __FILE__, __LINE__, __FUNCTION__); *(void**)&(v)=(void*)0xDEAFBEEFL; }
#define realloc(v, size)    v_realloc(v, size, __FILE__, __LINE__, __FUNCTION__)
#define calloc(num, size)   v_calloc(num, size, __FILE__, __LINE__, __FUNCTION__)
#define msize(v)            v_msize(v, __FILE__, __LINE__, __FUNCTION__)
#define strdup(v)           v_strdup(v, __FILE__, __LINE__, __FUNCTION__)

#endif

#define GBIT8I(p,i) (((const u8int *)(p))[i])
#define GBIT8(p)    GBIT8I(p,0)

#define GBIT16I(p,i)    ((GBIT8I(p,(i)+0)<<0) |\
             (GBIT8I(p,(i)+1)<<8))
#define GBIT32I(p,i)    ((GBIT8I(p,(i)+0)<<0) |\
             (GBIT8I(p,(i)+1)<<8) |\
             (GBIT8I(p,(i)+2)<<16) |\
             (GBIT8I(p,(i)+3)<<24))
#define GBIT64I(p,i)    ((GBIT32I(p,(i)+0)<<0) |\
             ((u64int)GBIT32I(p,(i)+4)<<32))

#define GBIT16IBE(p,i)  ((GBIT8I(p,(i)+0)<<8) |\
             (GBIT8I(p,(i)+1)<<0))
#define GBIT32IBE(p,i)  ((GBIT8I(p,(i)+0)<<24) |\
             (GBIT8I(p,(i)+1)<<16) |\
             (GBIT8I(p,(i)+2)<<8) |\
             (GBIT8I(p,(i)+3)<<0))
#define GBIT64IBE(p,i)  ((((u64int)GBIT32I(p,(i)+0))<<32) |\
             (GBIT32I(p,(i)+4)<<0))

#define GBIT16(p)   GBIT16I(p,0)
#define GBIT32(p)   GBIT32I(p,0)
#define GBIT64(p)   GBIT64I(p,0)
#define GBIT16BE(p) GBIT16IBE(p,0)
#define GBIT32BE(p) GBIT32IBE(p,0)
#define GBIT64BE(p) GBIT64IBE(p,0)


#define PBIT8I(p,i,v)   (((u8int *)(p))[i] = (v))
#define PBIT8(p,v)  PBIT8I(p,0,v)

#define PBIT16I(p,i,v) (PBIT8I(p,(i)+0,(v)>>0), \
            PBIT8I(p,(i)+1,(v)>>8), \
            (v))
#define PBIT32I(p,i,v) (PBIT8I(p,(i)+0,(v)>>0), \
            PBIT8I(p,(i)+1,(v)>>8), \
            PBIT8I(p,(i)+2,(v)>>16), \
            PBIT8I(p,(i)+3,(v)>>24), \
            (v))
#define PBIT64I(p,i,v) (PBIT8I(p,(i)+0,(v)>>0), \
            PBIT8I(p,(i)+1,(v)>>8), \
            PBIT8I(p,(i)+2,(v)>>16), \
            PBIT8I(p,(i)+3,(v)>>24), \
            PBIT8I(p,(i)+4,(v)>>32), \
            PBIT8I(p,(i)+5,(v)>>40), \
            PBIT8I(p,(i)+6,(v)>>48), \
            PBIT8I(p,(i)+7,(v)>>56), \
            (v))
#define PBIT16IBE(p,i,v) (PBIT8I(p,(i)+0,(v)>>8), \
              PBIT8I(p,(i)+1,(v)>>0), \
              (v))
#define PBIT32IBE(p,i,v) (PBIT8I(p,(i)+0,(v)>>24), \
              PBIT8I(p,(i)+1,(v)>>16), \
              PBIT8I(p,(i)+2,(v)>>8), \
              PBIT8I(p,(i)+3,(v)>>0), \
              (v))
#define PBIT64IBE(p,i,v) (PBIT8I(p,(i)+0,(v)>>56), \
              PBIT8I(p,(i)+1,(v)>>48), \
              PBIT8I(p,(i)+2,(v)>>40), \
              PBIT8I(p,(i)+3,(v)>>32), \
              PBIT8I(p,(i)+4,(v)>>24), \
              PBIT8I(p,(i)+5,(v)>>16), \
              PBIT8I(p,(i)+6,(v)>>8), \
              PBIT8I(p,(i)+7,(v)>>0), \
              (v))

#define PBIT16(p,v) PBIT16I(p,0,v)
#define PBIT32(p,v) PBIT32I(p,0,v)
#define PBIT64(p,v) PBIT64I(p,0,v)
#define PBIT16BE(p,v)   PBIT16IBE(p,0,v)
#define PBIT32BE(p,v)   PBIT32IBE(p,0,v)
#define PBIT64BE(p,v)   PBIT64IBE(p,0,v)

#if OBJTYPE==386
/*little endian and unaligned read/write*/
/*may not work on ARM*/
#undef  GBIT16
#undef  GBIT32
#undef  GBIT64
#undef  PBIT16
#undef  PBIT32
#undef  PBIT64

#define GBIT16(p)   (*((const u16int*)(p)))
#define GBIT32(p)       (*((const u32int*)(p)))
#define GBIT64(p)   (*((const u64int*)(p)))
#define PBIT16(p,v) (*((u16int*)(p)) = (v))
#define PBIT32(p,v) (*((u32int*)(p)) = (v))
#define PBIT64(p,v) (*((u64int*)(p)) = (v))
#endif


/*#define BITS(v,o,l)   ( ((v)>>(o)) & ((1<<l)-1) )*/
/*
 * print routines
 */
typedef struct Fmt  Fmt;
struct Fmt{
    uchar   runes;          /* output buffer is runes or chars? */
    void    *start;         /* of buffer */
    void    *to;            /* current place in the buffer */
    void    *stop;          /* end of the buffer; overwritten if flush fails */
    int (*flush)(Fmt *);    /* called when to == stop */
    void    *farg;          /* to make flush a closure */
    int nfmt;           /* num chars formatted so far */
    va_list args;           /* args passed to dofmt */
    int r;          /* % format Rune */
    int width;
    int prec;
    ulong   flags;
};

enum{
    FmtWidth    = 1,
    FmtLeft     = FmtWidth << 1,
    FmtPrec     = FmtLeft << 1,
    FmtSharp    = FmtPrec << 1,
    FmtSpace    = FmtSharp << 1,
    FmtSign     = FmtSpace << 1,
    FmtZero     = FmtSign << 1,
    FmtUnsigned = FmtZero << 1,
    FmtShort    = FmtUnsigned << 1,
    FmtLong     = FmtShort << 1,
    FmtVLong    = FmtLong << 1,
    FmtComma    = FmtVLong << 1,
    FmtByte     = FmtComma << 1,

    FmtFlag     = FmtByte << 1
};

extern  int print(const char*, ...);
extern  char*   seprint(char*, char*, const char*, ...);
extern  char*   vseprint(char*, char*, const char*, va_list);
extern  int snprint(char*, int, const char* fmt, ...);
extern  int vsnprint(char*, int, const char*, va_list);
extern  char*   smprint(const char*, ...);
extern  char*   vsmprint(const char*, va_list);
extern  int sprint(char*, const char*, ...);
extern  int fprint(int, const char*, ...);
extern  int vfprint(int, const char*, va_list);

extern  int runesprint(Rune*, const char*, ...);
extern  int runesnprint(Rune*, int, const char*, ...);
extern  int runevsnprint(Rune*, int, const char*, va_list);
extern  Rune*   runeseprint(Rune*, Rune*, const char*, ...);
extern  Rune*   runevseprint(Rune*, Rune*, const char*, va_list);
extern  Rune*   runesmprint(const char*, ...);
extern  Rune*   runevsmprint(const char*, va_list);

extern  int fmtfdinit(Fmt*, int, char*, int);
extern  int fmtfdflush(Fmt*);
extern  int fmtstrinit(Fmt*);
extern  char*   fmtstrflush(Fmt*);
extern  int runefmtstrinit(Fmt*);
extern  Rune*   runefmtstrflush(Fmt*);

extern  int fmtinstall(int, int (*)(Fmt*));
extern  int dofmt(Fmt*, const char*);
extern  int dorfmt(Fmt*, const Rune*);
extern  int fmtprint(Fmt*, const char*, ...);
extern  int fmtvprint(Fmt*, const char*, va_list);
extern  int fmtrune(Fmt*, int);
extern  int fmtstrcpy(Fmt*, char*);
extern  int fmtrunestrcpy(Fmt*, Rune*);
/*
 * error string for %r
 * supplied on per os basis, not part of fmt library
 */
extern  int errfmt(Fmt *f);

/*
 * quoted strings
 */
extern  char    *unquotestrdup(char*);
extern  Rune    *unquoterunestrdup(Rune*);
extern  char    *quotestrdup(char*);
extern  Rune    *quoterunestrdup(Rune*);
extern  int quotestrfmt(Fmt*);
extern  int quoterunestrfmt(Fmt*);
extern  void    quotefmtinstall(void);
extern  int (*doquote)(int);

/*
 * random number
 */
extern  int nrand(int);
extern  ulong   truerand(void);
extern  ulong   ntruerand(ulong);

/*
 * math
 */
extern  double  pow(double, double);
extern  int isNaN(double);
extern  int isInf(double, int);

#define PIO2    1.570796326794896619231e0
#define PI  (PIO2+PIO2)

/*
 * Time-of-day
 */

typedef struct Tm Tm;
struct Tm {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    int year;
    int wday;
    int yday;
    char    zone[4];
    int tzoff;
};
extern  vlong   osnsec(void);
#define nsec    osnsec

/*
 * one-of-a-kind
 */
extern  NORETURN    _assert(const char*, ...);
extern  double      charstod(int(*)(void*), void*);
extern  char*       cleanname(char*);
extern  ulong       getcallerpc(void*);
extern  int     getfields(char*, char**, int, int, char*);
extern  char*       getuser(void);
extern  char*       getuser(void);
extern  char*       getwd(char*, int);
extern  char*       getwd(char*, int);
extern  double      ipow10(int);
#define pow10   infpow10
extern  double      pow10(int);
extern  NORETURN    sysfatal(const char*, ...);
extern  int     dec64(uchar*, int, const char*, int);
extern  int     enc64(char*, int, const uchar*, int);
extern  int     dec32(uchar*, int, const char*, int);
extern  int     enc32(char*, int, const uchar*, int);
extern  int     dec16(uchar*, int, const char*, int);
extern  int     enc16(char*, int, const uchar*, int);
extern  int     encodefmt(Fmt*);

/*
 *  synchronization
 */
typedef
struct Lock {
    int val;
    int pid;
} Lock;

extern int  _tas(int*);

extern  void    lock(Lock*);
extern  void    unlock(Lock*);
extern  int canlock(Lock*);

typedef
struct QLock
{
    Lock    use;            /* to access Qlock structure */
    Proc    *head;          /* next process waiting for object */
    Proc    *tail;          /* last process waiting for object */
    int locked;         /* flag */
} QLock;

extern  void    qlock(QLock*);
extern  void    qunlock(QLock*);
extern  int canqlock(QLock*);
extern  void    _qlockinit(ulong (*)(ulong, ulong));    /* called only by the thread library */

typedef
struct RWLock
{
    Lock    l;          /* Lock modify lock */
    QLock   x;          /* Mutual exclusion lock */
    QLock   k;          /* Lock for waiting writers */
    int readers;        /* Count of readers in lock */
} RWLock;

extern  int canrlock(RWLock*);
extern  int canwlock(RWLock*);
extern  void    rlock(RWLock*);
extern  void    runlock(RWLock*);
extern  void    wlock(RWLock*);
extern  void    wunlock(RWLock*);

/*
 * network dialing
 */
#define NETPATHLEN 40

extern int close(int fd);
extern int read(int fd, void *buf, size_t n);
extern int write(int fd, const void *buf, size_t n);

/*
 * system calls
 *
 */
#define STATMAX 65535U  /* max length of machine-independent stat structure */
#define DIRMAX  (sizeof(Dir)+STATMAX)   /* max length of Dir structure */
#define ERRMAX  128 /* max length of error string */

#define MORDER  0x0003  /* mask for bits defining order of mounting */
#define MREPL   0x0000  /* mount replaces object */
#define MBEFORE 0x0001  /* mount goes before others in union directory */
#define MAFTER  0x0002  /* mount goes after others in union directory */
#define MCREATE 0x0004  /* permit creation in mounted directory */
#define MCACHE  0x0010  /* cache some data */
#define MMASK   0x0017  /* all bits on */

#define OREAD   0   /* open for read */
#define OWRITE  1   /* write */
#define ORDWR   2   /* read and write */
#define OEXEC   3   /* execute, == read but check execute permission */
#define OTRUNC  16  /* or'ed in (except for exec), truncate file first */
#define OCEXEC  32  /* or'ed in, close on exec */
#define ORCLOSE 64  /* or'ed in, remove on close */
#define OEXCL   0x1000  /* or'ed in, exclusive use (create only) */

#define AEXIST  0   /* accessible: exists */
#define AEXEC   1   /* execute access */
#define AWRITE  2   /* write access */
#define AREAD   4   /* read access */

/* bits in Qid.type */
#define QTDIR       0x80        /* type bit for directories */
#define QTAPPEND    0x40        /* type bit for append only files */
#define QTEXCL      0x20        /* type bit for exclusive use files */
#define QTMOUNT     0x10        /* type bit for mounted channel */
#define QTAUTH      0x08        /* type bit for authentication file */
#define QTFILE      0x00        /* plain file */

/* bits in Dir.mode */
#define DMDIR       0x80000000  /* mode bit for directories */
#define DMAPPEND    0x40000000  /* mode bit for append only files */
#define DMEXCL      0x20000000  /* mode bit for exclusive use files */
#define DMMOUNT     0x10000000  /* mode bit for mounted channel */
#define DMAUTH      0x08000000  /* mode bit for authentication file */
#define DMREAD      0x4     /* mode bit for read permission */
#define DMWRITE     0x2     /* mode bit for write permission */
#define DMEXEC      0x1     /* mode bit for execute permission */

typedef
struct Qid
{
    uvlong  path;
    ulong   vers;
    uchar   type;
} Qid;

typedef
struct Dir {
    /* system-modified data */
    ushort  type;   /* server type */
    uint    dev;    /* server subtype */
    /* file data */
    Qid qid;    /* unique id from server */
    ulong   mode;   /* permissions */
    ulong   atime;  /* last read time */
    ulong   mtime;  /* last write time */
    vlong   length; /* file length */
    const char  *name;  /* last element of path */
    /*const*/ char    *uid;   /* owner name */
    /*const*/ char    *gid;   /* group name */
    const char  *muid;  /* last modifier name */
} Dir;

extern  Dir*    dirstat(char*);
extern  Dir*    dirfstat(int);
extern  int dirwstat(char*, Dir*);
extern  int dirfwstat(int, Dir*);
extern  long    dirread(int, Dir**);
extern  void    nulldir(Dir*);
extern  long    dirreadall(int, Dir**);

typedef
struct Waitmsg
{
    int pid;    /* of loved one */
    ulong time[3];  /* of loved one & descendants */
    char    *msg;
} Waitmsg;

extern  void    _exits(char*);

extern  void    exits(char*);
extern  int create(char*, int, int);
extern  int errstr(char*, uint);

extern  long    readn(int, void*, long);
extern  void    rerrstr(char*, uint);
extern  vlong   seek(int, vlong, int);
extern  void    werrstr(const char*, ...);

extern char *argv0;
#define ARGBEGIN    for((argv0||(argv0=*argv)),argv++,argc--;\
                argv[0] && argv[0][0]=='-' && argv[0][1];\
                argc--, argv++) {\
                char *_args, *_argt;\
                Rune _argc;\
                _args = &argv[0][1];\
                if(_args[0]=='-' && _args[1]==0){\
                    argc--; argv++; break;\
                }\
                _argc = 0;\
                while(*_args && (_args += chartorune(&_argc, _args)))\
                switch(_argc)
#define ARGEND      (_argt=0);USED(_argt);USED(_argc); USED(_args);}USED(argv); USED(argc);
#define ARGF()      (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): 0))
#define EARGF(x)    (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))

#define ARGC()      _argc

/*
 *  Extensions for Inferno to basic libc.h
 */

extern  void    setbinmode(void);
extern  void*   sbrk(int);
extern  int segflush(void*, ulong);

/*
 *  Extensions for emu kernel emulation
 */
#ifdef  EMU

#undef environ
/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
typedef struct  FPU FPU;
struct FPU
{
    uchar   env[28];
};

#ifdef _MSC_VER
/* Set up private thread space */
extern  __declspec(thread) Proc*    up;
#else
Proc*   getup();
#define up (getup())
#endif

typedef jmp_buf osjmpbuf;
#define ossetjmp(buf)   setjmp(buf)

#endif
