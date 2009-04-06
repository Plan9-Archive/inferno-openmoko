typedef unsigned char       DISBYTE;    /* 8  bits */
typedef int         DISINT;     /* 32 bits */
typedef unsigned int        DISUINT;    /* 32 bits, only for >> */
typedef long long       DISBIG;     /* 64 bits */
typedef unsigned long long  DISUBIG;    /* 64 bits, only for >> */
typedef double          DISREAL;    /* 64 double IEEE754 */
typedef short           DISINT16;   /* 16 bits */
typedef float           DISREAL32;  /* 32 float IEEE754 */
/*typedef void*         DISPOINTER; /* for sizeof */

#define DISAPI(name) void name(F_##name *f)

//#define ASSIGN(place, value) {destroyxx((void**)&(place), (value));}
#define ASSIGN(place, value) {void* old=(place); (place)=(value); destroy(old);}
/* memory saver: destroy old value before calc new one */
//#define ASSIGN(place, value) {void* old=(place); (place)=H; destroy(old); (place)=(value); }

enum ProgState
{
    Palt,               /* blocked in alt instruction */
    Psend,              /* waiting to send */
    Precv,              /* waiting to recv */
    Pdebug,             /* debugged */
    Pready,             /* ready to be scheduled */
    Prelease,           /* interpreter released */
    Pexiting,           /* exit because of kill or error */
    Pbroken,            /* thread crashed */
    Pdeadbeef,          /* = 0xdeadbeef */
};

enum
{
    propagator  = 3,        /* gc marking color */

    PRNSIZE = 1024,
    BIHASH  = 23,
    PQUANTA = 2048, /* prog time slice */

    /* STRUCTALIGN is the unit to which the compiler aligns structs. */
    /* It really should be defined somewhere else */
    STRUCTALIGN = sizeof(int)   /* must be >=2 because of Strings */
};

typedef int ProgFlags;
enum
{
    Ppropagate = 1<<0,
    Pnotifyleader = 1<<1,
    Prestrict = 1<<2,
    Prestricted = 1<<3,
    Pkilled = 1<<4
};

typedef struct Alt  Alt;
typedef struct Channel  Channel;
typedef struct Progq    Progq;
typedef struct Import   Import;
typedef struct Inst Inst;
typedef struct Module   Module;
typedef struct Modlink  Modlink;
typedef struct Modl Modl;
typedef struct Type Type;
typedef struct Progs    Progs;
typedef struct Heap Heap;
typedef struct Link Link;
typedef struct List List;
typedef struct Array    Array;
typedef struct String   String;
typedef union  Linkpc   Linkpc;
typedef struct REG  REG;
typedef struct Frame    Frame;
typedef union  Stkext   Stkext;
typedef struct Atidle   Atidle;
typedef struct Altc Altc;
typedef struct Except   Except;
typedef struct Handler  Handler;
typedef union Disdata Disdata;


struct Altc
{
    Channel*    c;
    DISINT*     ptr;
};


struct Alt
{
    int     nsend;
    int     nrecv;
    Altc        ac[1];
};


union Disdata
{
    DISBYTE     disbyte;
    DISINT      disint;
    DISINT      disints[1];     /* for igoto */
    DISBIG      disbig;
    DISREAL     disreal;
    DISINT16    disint16;
    DISREAL32   disreal32;
    List*       plist;
    Channel*    pchannel;
    Array*      parray;
    String*     pstring;
    Frame*      pframe;
    Inst*       pinst;
    Type*       ptype;
    Disdata*    pdisdata;
    Modlink*    pmodlink;
    void*       pvoid;
    Alt     alt;        /* not a pointer */
};

struct Frame
{
    Inst*       lr;     /* REGLINK isa.h */
    Frame*      parent;     /* REGFP */ /* parent frame */
    Modlink*    ml;     /* REGMOD */
    Type*       _t_;        /* REGTYPE FEXME: remove it */
    DISINT      _regret_;   /* REGRET  */
    DISINT      stmp;       /* for fixed-point */
    DISINT      dtmp;       /* for fixed-point */
};

/**
 * Limbo's array (TODO: vlist, O(log n) appends)
 */
struct Array
{
    DISINT  len;            /* num elements in data */
    Type*   t;
    Array*  root;
    char*   data;           /* usually =this+1 */
};

struct List
{
    List*   tail;
    Type*   t;
    Disdata data;   /* may be incomplete, less in size then sizeof(Disdata), refer t for actual sizeof */
            /* custom type is in-place here, while Disdata in REG may be as pointer */
};

struct Channel
{
    Array*      buf;        /* For buffered channels - must be first */
    Progq*      send;       /* Queue of progs ready to send */
    Progq*      recv;       /* Queue of progs ready to receive */
    /*void*     aux;        /* Rock for devsrv */
    union {
        SrvFile*srv;        /* devsrv */
    } aux;
    void        (*mover)(void*d, void*s, Channel*c);    /* Data mover */
    union {
        DISINT  w;
        Type*   t;
    } mid;
    int     front;      /* Front of buffered queue */
    int     size;       /* Number of data items in buffered queue */
};

/**
 * Queue of Progs (lisp-like cons)
 */
struct Progq
{
    Prog*   prog;
    Progq*  next;
};

/**
 * Limbo's string (TODO: cord, O(log n) appends)
 */
struct String
{
    int     len;        /* string length */
    int     max;        /* maximum length in representation */
    char*       tmp;
    union {
    #define Sascii  data.ascii
    #define Srune   data.runes
        char    ascii[STRUCTALIGN]; /* string.c relies on having extra space (eg, in string2c) */
        Rune    runes[1];
    }data;
};

/**
 * Pointer to function of Dis or native code
 */
union Linkpc
{
    void    (*runt)(void*);
    Inst*   pc;
};

struct Link
{
    int     sig;
    Type*       frame;
    Linkpc      u;
    char*       name;
};

typedef union   Adr Adr;
union Adr
{
    DISINT      imm;
    DISINT      ind;
    Inst*       ins;
    struct {
        ushort  f;      /* First indirection */
        ushort  s;      /* Second indirection */
    } i;
};

struct Inst
{
    uchar       op;
    uchar       add;
    ushort      reg;
    Adr     s;
    Adr     d;
};

typedef void (*TypeDestructor)(Heap*, int);
typedef void (*TypeMark)(Type*, void*);
struct Type
{
    int     ref;
    const char* comment;    /* debugging */
    TypeDestructor  destructor; /* usually freeheap with exception for files and some special types */
    TypeMark    fnmark;     /* markheap, markarray, marklist, tkmarktop */
    int     size;
    int     np;     /* map size in bytes, 0 if there is no pointers */
    void*       destroy;    /* JITted code */
    void*       initialize; /* JITted code */
    char        map[STRUCTALIGN]; /* TODO: int map[] */
};
#define PRINT_TYPE(fd,t) {int i; \
    fprint(fd,"<%s %d %02X:", t->comment, t->size, t->np); \
    for(i=0; i<t->np; i++) fprint(fd,"%02X", (uchar)t->map[i]); \
    fprint(fd,">"); \
    }


struct REG
{
    Inst*       PC;     /* Program counter */
    Frame*      FP;     /* Frame pointer */
    Modlink*    ML;     /* Module */
    int     IC;     /* Instruction count for this quanta */
#if OBJTYPE!=386
    Inst*       xpc;        /* Saved program counter */
    DISINT      st;     /* Source temporary */
    DISINT      dt;     /* Destination temporary */
#endif
};

/**
 * Tree?
 */
struct Progs
{
    int     id;
    int     flags;
    Progs*      parent;
    Progs*      child;
    Progs*      sib;
    Prog*       head;   /* live group leader is at head */
    Prog*       tail;
};

/**
 * Limbo process
 */
struct Prog
{
    REG     R;      /* Register set */
    DISINT*     iii;        /* Alt */
    Alt*        aaa;        /* Alt */
    Prog*       link;       /* Run queue */
    Channel*    chan;       /* Channel pointer */
    void*       ptr;        /* Channel data pointer */
    /*union {
        WORD*   pword;
    } ptr;*/
    enum ProgState  state;      /* Scheduler state */
    char*       kill;       /* Set if prog should error */
    char*       killstr;    /* kill string buffer when needed */
    int         pid;        /* unique Prog id */
    int         quanta;     /* time slice */
    ulong       ticks;      /* time used */ /* TODO: emu never writes the variable */
    ProgFlags   flags;      /* error recovery flags */
    Prog*       prev;
    Prog*       next;
    Prog*       pidlink;    /* next in pid hash chain */
    Progs*      group;      /* process group */
    Prog*       grpprev;    /* previous group member */
    Prog*       grpnext;    /* next group member */
    String*     exval;      /* current exception, FIXME: String* or String** */
    char*       exstr;      /* last exception */
    void        (*addrun)(Prog*);
    void        (*xec)(Prog*);
    Osenv*      osenv;
};

struct Module
{
    int         ref;        /* Use count */
    int         compiled;   /* Compiled into native assembler */
    ulong       _ss_deprecated_;/* Stack size */
    enum ModRtFlags rt;     /* Runtime flags */
    ulong       mtime;      /* Modtime of dis file */
    Qid         qid;        /* Qid of dis file */
    ushort      dtype;      /* type of dis file's server*/
    uint        dev;        /* subtype of dis file's server */
    int         nprog;      /* number of instructions */
    Inst*       prog;       /* text segment */
    char*       origmp;     /* unpolluted Module data */
    int         ntype;      /* Number of type descriptors */
    Type**      type;       /* Type descriptors */
    Inst*       entry;      /* Entry PC */
    Type*       entryt;     /* Entry frame */
    const char* name;       /* Implements type */
    const char* path;       /* File module loaded from */
    Module*     link;       /* Links */
    Link*       ext;        /* External dynamic links */
    Import**    ldt;        /* Internal linkage descriptor tables */
                            /* все сигнатуры модулей (возможно неполные), которые могут понадобиться этому модулю */
    Handler*    htab;       /* Exception handler table */
    ulong*      pctab;      /* dis pc to code pc when compiled */
    void*       dlm;        /* dynamic C module */
};

struct Modl
{
    Linkpc      u;      /* PC of Dynamic link */
    Type*       frame;      /* Frame type for this entry */
};

/**
 * Imported module instance
 */
struct Modlink
{
    char*       MP;     /* Module data for this instance */
    Module*     m;      /* The real module */
    int     compiled;   /* Compiled into native assembler (overwrites m->compiled) */
    Inst*       prog;       /* text segment (overwrites m->prog) */
    Type**      type;       /* Type descriptors (overwrites m->type)*/
    uchar*      data;       /* for dynamic C modules */
    int     nlinks;     /* ?apparently required by Java */
    Modl        links[1];
};

/**
 * Header of heap allocated memory blocks
 * must be a multiple of 8 bytes
 * D2H() casts from data to Heap pointer
 * H2D() casts from Heap to data  pointer
 */
struct Heap
{
    int     color /*: 2*/;      /* Allocation color */
    int     ref;
    Type*       t;
    ulong       hprof;      /* heap profiling */
};

struct  Atidle
{
    int     (*fn)(void*);
    void*       arg;
    Atidle*     link;
};

struct Import
{
    int     sig;
    char*       name;
};

struct Except
{
    char*       s;
    ulong       pc;
};

struct Handler
{
    ulong       pc1;
    ulong       pc2;
    ulong       eoff;
    ulong       ne;
    Type*       t;
    Except*     etab;
};

#define H2D(t, x)   ((t)((Heap*)(x)+1))
#define D2H(x)      ((Heap*)(x)-1)
#define ADDREF(x)   (++((Heap*)(x)-1)->ref)
#define DELREF(x)   (--((Heap*)(x)-1)->ref)
//#define H       ((void*)(-1))
#define H NULL
#define Setmark(h)  if((h)->color!=mutator) { (h)->color = propagator; nprop=1; }
#define gclock()    (gchalt++)
#define gcunlock()  (gchalt--)
#define gcruns()    (gchalt == 0)

extern  int bflag;
extern  int cflag;
extern  int nproc;
extern  Type    Tarray;
extern  Type    Tstring;
extern  Type    Tchannel;
extern  Type    Tlist;
extern  Type    Tmodlink;
extern  Type*   TImage;
extern  Type    Tptr;
extern  Type    Tbyte;
extern  Type    Tword;
extern  Type    Tlong;
extern  Type    Treal;
/*extern    REG R;*/
extern  String  snil;
extern  void    (*optab[256])(Disdata*s, Disdata*m, Disdata*d, REG*rr /*FP,MP,PC,IC*/);
extern  void    (*comvec)(void);
typedef struct Dec {
    Disdata     *s,*m,*d;
    DISINT      tmp;        /* */
    } Dec;
extern  void dec(
    Dec*        dec,        /* return */
    const Inst* PC,     /* Program counter */
    void*       MP,     /* Module data */
    Frame*      FP      /* Frame pointer */
    );
extern  Module* modules;
extern  int mutator;
extern  int nprop;
extern  int gchalt;
extern  int gccolor;
extern  int minvalid;

typedef struct{const char *name; long sig; void (*fn)(void*); int size; int np; char map[16];} Runtab;

extern  int     Dconv(Fmt*);
extern  void        acquire(void);
extern  void        addrun(Prog*);
extern  void        altdone(Alt*, Prog*, Channel*, int);
extern  void        altgone(Prog*);
extern  Array*      allocimgarray(Heap*, Heap*);
extern  Module*     builtinmod(const char*, const Runtab*, int);
extern  void        cblock(Prog*);
extern  void        cmovw(void*, void*);
extern  Channel*    cnewc(Type*, void (*mover)(void*d, void*s, Channel*c), int);
extern  int     compile(Module*, int, Modlink*);
extern  void        cqadd(Progq**, Prog*);
extern  void        cqdel(Progq**);
extern  void        cqdelp(Progq**, Prog*);
extern  void        crecv(Channel*, void*);
extern  void        csend(Channel*, void*);
extern  int     csendalt(Channel*, void*, Type *, int);
extern  Prog*       currun(void);
extern  void        dbgexit(Prog*, int, char*);
extern  void        dbgxec(Prog*);
extern  void        delprog(Prog*, char*);
extern  Prog*       delrun(enum ProgState);
extern  void        delrunq(Prog*);
extern  Prog*       delruntail(enum ProgState);
extern  void        destroy(void*v);
extern  void        destroyxx(void**pp, void*new);
extern  void        destroyimage(ulong);
extern  void        destroylinks(Module*);
extern  void        destroystack(REG*);
extern  void        drawmodinit(void);
extern  int     dynldable(int);
extern  void        loadermodinit(void);
extern  Type*       dtype(void (*destructor)(Heap*, int), int, const char*map, int mapsize, const char*comment);
extern  Module*     dupmod(Module*);
//extern  NORETURN    error(const char*);
NORETURN    v_error(__in_z const char*msg, const char*, int, const char*);
#define error(msg)  v_error(msg, __FILE__, __LINE__, __FUNCTION__)

extern  NORETURN    errorf(const char*, ...);
extern  void        extend(void);
extern  void        freedyncode(Module*);
extern  void        freedyndata(Modlink*);
extern  void        freemod(Module*);
extern  void        freeheap(Heap*, int);
extern  void        freeptrs(void*, Type*);
extern  void        freestring(Heap*, int);
extern  void        freetype(Type*);
extern  void        freetypemodinit(void);
extern  long        getdbreg();
extern  int     gfltconv(Fmt*);
extern  void        go(Module*);
extern  int     handler(char*);
extern  void        (*heapmonitor)(int, void*, ulong);
extern  int     heapref(void*);

extern  Heap*       v_heap(Type*, const char*, int, const char*);
extern  Heap*       v_heaparray(Type*, int, const char*, int, const char*);
extern  Heap*       v_heapz(Type*, const char*, int, const char*);
extern  Heap*       v_nheap(int, const char*, int, const char*);
#define heap(t)         v_heap(t, __FILE__, __LINE__, __FUNCTION__)
#define heaparray(t, sz)    v_heaparray(t, sz, __FILE__, __LINE__, __FUNCTION__)
#define heapz(t)        v_heapz(t, __FILE__, __LINE__, __FUNCTION__)
#define nheap(n)        v_nheap(n, __FILE__, __LINE__, __FUNCTION__)

extern  int     hmsize(void*);
extern  void        incmem(const void*, Type*);
extern  void        initarray(Type*, Array*);
extern  void        initmem(const Type*, void*);
extern  void        irestore(Prog*);
extern  Prog*       isave(void);
extern  void        keyringmodinit(void);
extern  void        killcomm(Progq **p);
extern  int     killprog(Prog*, char*);
extern  int     killgrp(Prog*, char*);
extern  Modlink*    linkmod(Module*, Import*, int);
extern  Modlink*    mklinkmod(Module*, int);
extern  Module*     load(char*);
extern  Module*     lookmod(char*);
extern  long        magic(void);
extern  void        markarray(Type*, Array*);
extern  void        markchan(Type*, void*);
extern  void        markheap(Type*, void*);
extern  void        marklist(Type*, List*);
extern  void        markmodl(Type*, void*);
extern  void        mathmodinit(void);
extern  Array*      mem2array(const void*, int);
extern  void        mlink(Module*, Link*, const char*, int, int, Type*);
extern  void        modinit(void);
extern  DISINT      modstatus(REG*, char*, int);
//extern    void        movp(void);
extern  void        moverp(void*d, void*s, Channel*c);
extern  void        movertmp(void*d, void*s, Channel*c);
extern  void        movtmpsafe(void);
extern  int     mustbesigned(const char *, const char*, ulong, const Dir*);
extern  Module*     newmod(const char*);
extern  Module*     newdyncode(int, const char*, Dir*);
extern  void        newdyndata(Modlink*);
extern  void        newgrp(Prog*);
extern  void        newmp(void*, void*, Type*);
extern  Prog*       newprog(Prog*, Modlink*);
extern  void        newstack(Prog*);
extern  void        noptrs(Type*, void*);
extern  int     nprog(void);
extern  void        opinit(void);
extern  Module*     parsemod(const char*, const char*, ulong, const Dir*);
extern  Module*     parsedmod(char*, int, ulong, Qid);
extern  void        prefabmodinit(void);
extern  Prog*       progn(int);
extern  Prog*       progpid(int);
extern  void        ptradd(Heap*);
extern  void        ptrdel(Heap*);
extern  void        pushrun(Prog*);
extern  Module*     readmod(const char*, Module*, int sync);
extern  int         _irecv(Channel *c, void* v);
extern  void        release(void);
extern  void        releasex(void);
extern  void        retnstr(const char*, int, String**);
extern  void        retstr(const char*, String**);
extern  void        rungc(Prog*);
extern  void        runtime(Module*, Link*, const char*, int, void(*)(void*), Type*);
extern  void        safemem(void*, Type*, void (*)(void*));
extern  int     segflush(void *, ulong);
extern  int     _isend(Channel *c, void *v);
extern  void        setdbreg(uchar*);
extern  uchar*      setdbloc(uchar*);
extern  void        seterror(char*, ...);
extern  void        sethints(String*, int);
extern  String*     splitc(String**, int);
extern  uchar*      stack(Frame*);
extern  int     stringblen(String*);
extern  int     stringcmp(const String*, const String*);
extern  String*     stringdup(const String*);
extern  String*     stringheap(int, int, int, int);
extern  char*       syserr(char*, char*, Prog*);
extern  void        sysinit(void);
extern  void        sysmodinit(void);
extern  void        tellsomeone(Prog*, char*);
extern  void        tkmodinit(void);
extern  void        unload(Module*);
extern  int     verifysigner(const char*, int, const char*, ulong);
extern  void        xec(Prog*);
extern  int     xecalt(int, Alt *a, DISINT*ret);
extern  int     xprint(Prog*, const void*, const void*, String*, char*, int);
extern  int     bigxprint(Prog*, const void*, const void*, String*, char**, int);
extern  void        iyield(void);
extern  String*     newrunes(int);
extern  String*     newstring(int);
extern  int     runeslen(const Rune*);
extern  String*     c2string(const char*, int);
extern  char*       string2c(String*);
extern  List*       cons(ulong, List**);
extern  String*     slicer(ulong, ulong, const String*);
extern  String*     addstring(String*, String*, int);
extern  int     brpatch(Inst*, Module*);
extern  void        readimagemodinit(void);


#pragma varargck    type    "D" Inst*
#pragma varargck argpos errorf 1
