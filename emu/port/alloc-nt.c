#include <windows.h>

#include <dat.h>
#include <fns.h>

#ifndef HEAP_CREATE_ENABLE_EXECUTE
#define HEAP_CREATE_ENABLE_EXECUTE 0x00040000
#endif

#ifndef __FUNCTION__
#define __FUNCTION__ ""
#endif

typedef struct Bhdr Bhdr;
typedef struct Btail Btail;

#define MYMARK 0xECA01F37
struct Bhdr
{
	int		mymark;
	size_t		size;
	int		tag;
	const char*	file;
	int		line;
	const char*	function;
	const char*	comment;
	int		guard;
};

struct Btail
{
	int		guard;
	int		_align_;
};
#define GUARD(b) ( ((int)(b)) ^ (((Bhdr*)(b))->size & ~3) ^ 0xABABCDEF )
#define BTAIL(b) ((Btail*)((char*)B2D(b)+((Bhdr*)(b))->size))

#define B2D(bp)		(void*)((Bhdr*)(bp)+1)
#define D2B(dp)		((Bhdr*)(dp)-1)

//panic(__FUNCTION__ "(invalid ptr b=%p) %s:%d %s\n", b, file, line, function); \

#define VALIDATE_PTR(p) { \
	Bhdr* z=(Bhdr*)(p); \
	if(IsBadReadPtr(z, sizeof(Bhdr))) \
		panic(__FUNCTION__ "(invalid ptr %p)", z); \
	if(IsBadStringPtrA(z->file, 256)) \
		panic(__FUNCTION__ "(invalid file name b=%p b->file=%p)", z, z->file); \
	if(IsBadStringPtrA(z->function, 256)) \
		panic(__FUNCTION__ "(invalid function name b=%p b->file=%p)", z, z->function); \
	if(z->comment!=0 && IsBadStringPtrA(z->comment, 256)) \
		panic(__FUNCTION__ "(invalid comment b=%p b->file=%p)", z, z->comment); \
	if(z->guard != GUARD(z)) \
		panic(__FUNCTION__ "(invalid head guard %lux!=%lux b=%lux size=%d) %s:%d %s", \
			z->guard, GUARD(z), z, z->size, z->file, z->line, z->function); \
	if(BTAIL(z)->guard != GUARD(z)) \
		panic(__FUNCTION__ "(invalid tail guard %lux!=%lux b=%lux size=%d) %s:%d %s", \
			BTAIL(z)->guard, GUARD(z), z, z->size, z->file, z->line, z->function); \
	}

/* pool aware */
#define VALIDATE_PTR2(pool, p) { \
	VALIDATE_PTR(p) \
{Bhdr* z=(Bhdr*)(p); \
	if(!HeapValidate(pool->handle, 0, b)) \
		panic(__FUNCTION__ "(invalid block b=%p size=%d) %s:%d %s", \
			z, z->size, z->file, z->line, z->function); \
}}




struct Pool
{
	char*	name;
	size_t	maxsize;
	int	chunk;
	HANDLE	handle;

	uvlong	nalloc;
	uvlong	nfree;
	uvlong	nrealloc;
	uvlong	nrefree;
	uvlong	allocbytes;
	uvlong	freebytes;
	uvlong	reallocbytes;	/* allocated in realloc */
	uvlong	refreebytes;	/* freed in realloc */

};

enum
{
	MAXPOOL		= 4
};

static struct
{
	int	n;
	Pool	pool[MAXPOOL];
	/* Lock l; */
} table = {
	3,
	{
		{ "main",  32*1024*1024,	  512*1024 },
		{ "heap",  32*1024*1024,	  512*1024 },
#if defined(LINUX_ARM) || defined(_WIN32_WCE)
		{ "image", 32*1024*1024+256, 	512*1024 },
#else
		{ "image", 32*1024*1024+256, 	4*1024*1024 },
#endif
	}
};

Pool*	mainmem = &table.pool[0];
Pool*	heapmem = &table.pool[1];
Pool*	imagmem = &table.pool[2];

void*	v_poolalloc(Pool* pool, size_t size, const char*file, int line, const char*function)
{
	Bhdr *b;
	void* v;

//	size = (size + 3) & ~3; /* ARM */
	if(size >= 1024*1024*1024)	/* for sanity and to avoid overflow */
	{
		print("v_poolalloc(%s,%d)=0\n", pool->name, size);
		return nil;
	}

	if(NULL==pool->handle)
	{
		pool->handle = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, pool->chunk, 0 /* BUG: can grow big */ );
		if(pool->handle==NULL)
			panic("HeapCreate");
		poolwalk(pool, 0);
	}
	b = (Bhdr*)HeapAlloc(pool->handle, 0, sizeof(Bhdr)+size+sizeof(Btail));
	if(b==nil)
		return nil;
	pool->nalloc++;
	pool->allocbytes += size;

	/*b->magic = MAGIC_A;*/
	b->mymark = MYMARK;
	b->tag = 0;
	b->size = size;
	b->file = file;
	b->line = line;
	b->function = function;
	b->comment = 0;
	b->guard = GUARD(b);
	BTAIL(b)->guard = GUARD(b);

	v = B2D(b);
	/*memset(v, 0xDA, size); /* ~random */
	memset(v, 0, size);  /* some count on it */
	assert( (((int)v)&7)==0 );
	return v;
}
void*	v_poolrealloc(Pool* pool, void* v, size_t size, const char*file, int line, const char*function)
{
	Bhdr *b;
	void* nv;
	size_t oldsize;
	int oldtag;

//	size = (size + 3) & ~3; /* ARM */
	if(size >= 1024*1024*1024)	/* for sanity and to avoid overflow */
	{
		print("v_poolrealloc(%s,%d)=0\n", pool->name, size);
		return nil;
	}

	if(v==nil)
		return v_poolalloc(pool, size, file, line, function);
	assert(NULL!=pool->handle);
	b = D2B(v);
	VALIDATE_PTR2(pool, b);

	oldtag = b->tag;
	oldsize = b->size;
	b = (Bhdr*)HeapReAlloc(pool->handle, 0, b, sizeof(Bhdr)+size+sizeof(Btail));
	if(b==nil)
	{
		pool->nfree++;
		pool->freebytes += oldsize;
		return nil;
	}
	b->mymark = MYMARK;
	b->tag = oldtag;
	b->size = size;
	b->file = file;
	b->line = line;
	b->function = function;
	b->comment = 0;
	b->guard = GUARD(b);
	BTAIL(b)->guard = GUARD(b);

	nv = B2D(b);
	if(size > oldsize)
	{
		pool->nrealloc++;
		pool->reallocbytes += size-oldsize;
		memset((char*)nv+oldsize, 0, size-oldsize);  /* some count on it */
	} else
	{
		pool->nrefree++;
		pool->refreebytes += oldsize-size;
	}
	return nv;
}
void	v_poolfree(Pool* pool, void* v, const char*file, int line, const char*function)
{
	Bhdr*b;
	if(v==nil)
		return;

	b = D2B(v);
	VALIDATE_PTR2(pool, b);
	pool->nfree++;
	pool->freebytes += b->size;
	memset(v, 0xCD, b->size);
	memset(BTAIL(b), 0xF1, sizeof(Btail));
	memset(b, 0xFD, sizeof(Bhdr));
	HeapFree(pool->handle, 0, b);
}
size_t	v_poolmsize(Pool *pool, void *v, const char*file, int line, const char*function)
{
	Bhdr*b;
	if(v==nil)
		return -1;

	b = D2B(v);
	VALIDATE_PTR2(pool, b);
	return b->size;
}
void	setmemcomment(void *v, const char* comment)
{
	Bhdr*b;
	b = D2B(v);
	VALIDATE_PTR(b);
	b->comment = comment;
}

void	poolwalk(Pool* pool, poolwalk_callback callback)
{
	PROCESS_HEAP_ENTRY phe;
	int nblock=0;
	int err;
	Bhdr* b;


	print(	"name\t%s\n"
		"alloc\t%lld\t%lld\n"
		"free\t%lld\t%lld\n"
		"realloc\t%lld\t%lld\n"
		"refree\t%lld\t%lld\n"
		"active\t%lld\t%lld\n",
		pool->name,
		pool->nalloc,	pool->allocbytes,
		pool->nfree,	pool->freebytes,
		pool->nrealloc,	pool->reallocbytes,
		pool->nrefree,	pool->refreebytes,
		pool->nalloc-pool->nfree,
		(pool->allocbytes-pool->freebytes) + (pool->reallocbytes-pool->refreebytes)
		);
	if(0==pool->handle)
		return;

	if (!HeapLock(pool->handle))
		panic("poolwalk: cannot lock the heap %d", GetLastError());

	phe.lpData = 0;
	while (HeapWalk( pool->handle, &phe ))
	{
		b = (Bhdr*)phe.lpData;
		if (phe.wFlags & PROCESS_HEAP_ENTRY_BUSY && b->mymark==MYMARK )
		{
			/*print("<%d w=%x %p/%x bsize=%x>",
				nblock,
				phe.wFlags,
				phe.lpData, phe.cbData,
				((Bhdr*)phe.lpData)->size + sizeof(Bhdr) + sizeof(Btail));*/
			/* do not go to TrapHandler from callback function */
#ifdef _MSC_VER
			__try
			{
#endif
				VALIDATE_PTR2(pool, phe.lpData);
				if(phe.cbData != b->size + sizeof(Bhdr) + sizeof(Btail)) \
					panic(__FUNCTION__ "(invalid size %p)", b);
				//assert(phe.cbData == b->size + sizeof(Bhdr) + sizeof(Btail));
				callback(B2D(b), b->size, b->tag, b->file, b->line, b->function, b->comment);
#ifdef _MSC_VER
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				panic("poolwalk: EXCEPTION %p:%x flags=%x",
					phe.lpData, phe.cbData, phe.wFlags);
				/*continue, break*/
			}
#endif
			++nblock;

		}
	}
	err = GetLastError();
	if (err != ERROR_NO_MORE_ITEMS) {
		panic("poolwalk: HeapWalk aborted with error %d", err);
	}
	print("\n");
	print("walked\t%d\n", nblock);
	print("\n");
	assert(nblock == pool->nalloc-pool->nfree);
	if(!HeapUnlock(pool->handle))
		panic("poolwalk: cannot unlock the heap %d", GetLastError());
}


void
v_free(void *v, const char*file, int line, const char*function)
{
	v_poolfree(mainmem, v, file, line, function);
}

void*
v_realloc(void *v, size_t size, const char*file, int line, const char*function)
{
	void *nv;
	nv = v_poolrealloc(mainmem, v, size, file, line, function);
	return nv;
}

void*
v_kmalloc(size_t size, const char*file, int line, const char*function)
{
	void *v;

	v = v_poolalloc(mainmem, size, file, line, function);
	if(v==nil)
		return nil;
	/*memset(v, 0, size); /* some count on it */
	return v;
}

void*
v_calloc(size_t n, size_t szelem, const char*file, int line, const char*function)
{
	return v_mallocz(n*szelem, 1, file, line, function);
}

void*
v_malloc(size_t size, const char*file, int line, const char*function)
{
	void *v;

	v = v_poolalloc(mainmem, size, file, line, function);
	if(v==nil)
		return nil;
	/*memset(v, 0, size); /* some count on it */
	return v;
}

void*
v_smalloc(size_t size, const char*file, int line, const char*function)
{
	void *v;

	v = v_poolalloc(mainmem, size, file, line, function);
	if(v==nil)
		return nil;
	/*memset(v, 0, size); /* some count on it */
	return v;
}

void*
v_mallocz(size_t size, int clr, const char*file, int line, const char*function)
{
	void *v;

	v = v_poolalloc(mainmem, size, file, line, function);
	/*if(clr)
		memset(v, 0, size);
	else
		memset(v, 0xA4, size);*/
	return v;
}

char*
v_strdup(const char *s, const char*file, int line, const char*function)
{
	char *os;

	os = (char*)v_malloc(strlen(s) + 1, file, line, function);
	if(os == 0)
		return 0;
	return strcpy(os, s);
}

size_t
v_msize(void *v, const char*file, int line, const char*function)
{
	return v_poolmsize(mainmem, v, file, line, function);
}
