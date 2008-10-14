#include <windows.h>
#include "dat.h"
#include "fns.h"
#include "pool.h"

typedef struct Bhdr Bhdr;
typedef struct Btail Btail;

struct Bhdr
{
	ulong	tag;
	ulong	size;
	const char* file;
	int line;
	const char* function;
	const char* comment;
	int _align_;
	int guard;
};

struct Btail
{
	int guard;
	int _align_;
};
#define GUARD(b) ( ((int)(b)) ^ ((b)->size & ~3) ^ 0xABABCDEF )
#define BTAIL(b) ((Btail*)((char*)B2D(b)+(b)->size))

#define B2D(bp)		(void*)((Bhdr*)(bp)+1)
#define D2B(dp)		((Bhdr*)(dp)-1)

#define VALIDATE_PTR(b) \
	if(IsBadReadPtr(b, sizeof(Bhdr))) \
	{ \
		panic(__FUNCTION__ "(invalid ptr b=%p) %s:%d %s\n", b, file, line, function); \
	} \
	if((b)->guard != GUARD(b)) \
	{ \
		panic(__FUNCTION__ "(invalid head guard %lux!=%lux b=%lux size=%d) %s:%d %s\n", \
			b->guard, GUARD(b), b, b->size, b->file, b->line, b->function); \
	} \
	if(BTAIL(b)->guard != GUARD(b)) \
	{ \
		panic(__FUNCTION__ "(invalid tail guard %lux!=%lux b=%lux size=%d) %s:%d %s\n", \
			BTAIL(b)->guard, GUARD(b), b, b->size, b->file, b->line, b->function); \
	}


#define VALIDATE_PTR2(pool, b) \
	VALIDATE_PTR(b) \
	if(!HeapValidate(pool->handle, 0, b)) \
	{ \
		panic(__FUNCTION__ "(invalid block b=%p size=%d) %s:%d %s\n", b, b->size, b->file, b->line, b->function); \
	}




struct Pool
{
	char*	name;
	size_t	maxsize;
	int	chunk;
	HANDLE	handle;
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
	}
	b = HeapAlloc(pool->handle, 0, sizeof(Bhdr)+size+sizeof(Btail));
	if(b==nil)
		return nil;

	/*b->magic = MAGIC_A;*/
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
	b = HeapReAlloc(pool->handle, 0, b, sizeof(Bhdr)+size+sizeof(Btail));
	if(b==nil)
		return nil;
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
		memset((char*)nv+oldsize, 0, size-oldsize);  /* some count on it */
	return nv;
}
void	v_poolfree(Pool* pool, void* v, const char*file, int line, const char*function)
{
	Bhdr*b;
	if(v==nil)
		return;

	b = D2B(v);
	VALIDATE_PTR2(pool, b);
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
void	v_setmemcomment(void *v, const char* comment, const char*file, int line, const char*function)
{
	Bhdr*b;
	b = D2B(v);
	VALIDATE_PTR(b);
	b->comment = comment;
}

void	poolwalk(Pool* pool, void(*callback)(void *))
{
	PROCESS_HEAP_ENTRY entry;
	while (HeapWalk( pool->handle, &entry ))
	{
		if (entry.wFlags & PROCESS_HEAP_REGION)
		{
			/*
	            pLocal32Info->dwMemReserved += entry.u.Region.dwCommittedSize
	                                           + entry.u.Region.dwUnCommittedSize;
	            pLocal32Info->dwMemCommitted = entry.u.Region.dwCommittedSize;
			 */
		}
		else if (!(entry.wFlags & PROCESS_HEAP_ENTRY_BUSY))
		{
			/*
	            DWORD size = entry.cbData + entry.cbOverhead;
	            pLocal32Info->dwTotalFree += size;
	            if (size > pLocal32Info->dwLargestFreeBlock) pLocal32Info->dwLargestFreeBlock = size;
			 */
		}
	}
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

size_t
v_msize(void *v, const char*file, int line, const char*function)
{
	return v_poolmsize(mainmem, v, file, line, function);
}
