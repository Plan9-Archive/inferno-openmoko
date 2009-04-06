/* Inferno tree allocator */

typedef struct Pool Pool;
typedef struct Bhdr Bhdr;
typedef struct Btail Btail;

#pragma incomplete Pool

enum
{
    MAGIC_A     = 0xa110c,      /* Allocated block */
    MAGIC_F     = 0xbadc0c0a,       /* Free block */
    MAGIC_E     = 0xdeadbabe,       /* End of arena */
    MAGIC_I     = 0xabba        /* Block is immutable (hidden from gc) */
};

struct Bhdr
{
    ulong   magic;
    ulong   size;
    union {
        uchar data[1];
        struct {
            Bhdr*   bhl;
            Bhdr*   bhr;
            Bhdr*   bhp;
            Bhdr*   bhv;
            Bhdr*   bhf;
        } s;
#define clink   u.l.link
#define csize   u.l.size
        struct {
            Bhdr*   link;
            int size;
        } l;
    } u;
};

struct Btail
{
    /* ulong    pad; */
    Bhdr*   hdr;
};

#define B2D(bp)     ((void*)bp->u.data)
#define D2B(b, dp)  b = ((Bhdr*)(((uchar*)dp)-(((Bhdr*)0)->u.data))); \
            if(b->magic != MAGIC_A && b->magic != MAGIC_I)\
                poolfault(dp, "alloc:D2B", getcallerpc(&dp));
#define B2NB(b)     ((Bhdr*)((uchar*)b + b->size))
#define B2PT(b)     ((Btail*)((uchar*)b - sizeof(Btail)))
#define B2T(b)      ((Btail*)(((uchar*)b)+b->size-sizeof(Btail)))

#define B2LIMIT(b)  ((Bhdr*)((uchar*)b + b->csize))

#define BHDRSIZE    ((int)(((Bhdr*)0)->u.data)+sizeof(Btail))

extern  void    (*poolfault)(void *, char *, ulong);
extern  void    poolinit(void);
//extern  void*   poolalloc(Pool*, size_t);
extern  void*   v_poolalloc(Pool*, size_t, const char*, int, const char*);
#define poolalloc(pool, size)   v_poolalloc(pool, size, __FILE__, __LINE__, __FUNCTION__)

extern  void    poolfree(Pool*, __in_opt void*);
extern  Bhdr*   poolchain(Pool*);
extern  int     poolcompact(Pool*);
extern  void    poolimmutable(__in void*);
extern  size_t  poolmsize(Pool*, __in const void*);
extern  void    poolmutable(__in void*);
extern  char*   poolname(Pool*);
extern  int     poolread(char*, int, ulong);
extern  void*   poolrealloc(Pool*, void*, ulong);
extern  int     poolsetsize(char*, int);
extern  void    poolsetcompact(Pool*, void (*)(void*, void*));
extern  char*   poolaudit(char*(*)(int, Bhdr *));

extern  void    (*poolmonitor)(int, ulong, Bhdr*, ulong);
