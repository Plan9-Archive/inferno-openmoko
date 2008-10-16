/* Inferno tree allocator */

typedef struct Pool Pool;

#pragma incomplete Pool


//extern	void	(*poolfault)(void *, char *, ulong);
/*extern	Bhdr*	poolchain(Pool*);*/
/*extern	int	poolcompact(Pool*);*/
/*extern	void	poolimmutable(void*);*/
//extern	void	poolmutable(void*);
//extern	int	poolread(char*, int, ulong);
//extern	int	poolsetsize(char*, int);
//extern	void	poolsetcompact(Pool*, void (*)(void*, void*));

/*extern	void	(*poolmonitor)(int, ulong, Bhdr*, ulong);*/

extern	void	poolinit(void); /* BUG never called */

extern	char*	poolname(Pool*);
extern	void*	v_poolalloc(Pool*, size_t, const char*file, int line, const char*function);
extern	void	v_poolfree(Pool*, void*, const char*file, int line, const char*function);
extern	void*	v_poolrealloc(Pool*, void*, size_t, const char*file, int line, const char*function);
extern	size_t	v_poolmsize(Pool*, void*, const char*file, int line, const char*function);

extern	void	setmemcomment(void *v, const char* comment);
extern	void	setmemtag(void *v, int tag);
extern	int	getmemtag(void *v);

typedef void (*poolwalk_callback)( void* v, size_t size, int tag,
	const char* file, int line, const char* function, const char* comment);
extern	void	poolwalk(Pool*, poolwalk_callback);

/* main pool */
//extern	void free(void *v);
//extern	void* realloc(void *v, size_t size); /* some assume memory is zero-initialized */
//extern	void* calloc(size_t num, size_t size);
//extern	void* malloc(size_t size); /* some assume memory is zero-initialized */
//extern	void* mallocz(size_t size, int clr); /* clr==0 -> unitialized memory */

//extern	void* kmalloc(size_t size); /* the same as malloc */
//extern	void* smalloc(size_t size); /* never returns 0, will wait */

extern	void*	v_kmalloc(size_t size, const char*, int, const char*);
extern	void*	v_smalloc(size_t size, const char*, int, const char*);
extern	void*	v_malloc(size_t size, const char*, int, const char*);
extern	void*	v_mallocz(size_t size, int clr, const char*, int, const char*);
extern	void	v_free(void *v, const char*, int, const char*);
extern	void*	v_realloc(void *v, size_t size, const char*, int, const char*);
extern	void*	v_calloc(size_t num, size_t size, const char*, int, const char*);
extern	size_t	v_msize(void *v, const char*, int, const char*);
extern	char*	v_strdup(const char*, const char*, int, const char*);

#define kmalloc(size)		v_kmalloc(size, __FILE__, __LINE__, __FUNCTION__)
#define smalloc(size)		v_smalloc(size, __FILE__, __LINE__, __FUNCTION__)
#define malloc(size)		v_malloc(size, __FILE__, __LINE__, __FUNCTION__)
#define mallocz(size, clr)	v_mallocz(size, clr, __FILE__, __LINE__, __FUNCTION__)
#define free(v)			v_free(v, __FILE__, __LINE__, __FUNCTION__)
#define realloc(v, size)	v_realloc(v, size, __FILE__, __LINE__, __FUNCTION__)
#define calloc(num, size)	v_calloc(num, size, __FILE__, __LINE__, __FUNCTION__)
#define msize(v)		v_msize(v, __FILE__, __LINE__, __FUNCTION__)
#define strdup(v)		v_strdup(v, __FILE__, __LINE__, __FUNCTION__)

#define poolalloc(p,size)	v_poolalloc(p,size, __FILE__, __LINE__, __FUNCTION__)
#define poolfree(p,v)		v_poolfree(p,v, __FILE__, __LINE__, __FUNCTION__)
#define poolrealloc(p,v, size)	v_poolrealloc(p,v, size, __FILE__, __LINE__, __FUNCTION__)
#define poolmsize(p,v)		v_poolmsize(p,v, __FILE__, __LINE__, __FUNCTION__)

//#define setmemcomment(size, c)	v_setmemcomment(size, c, __FILE__, __LINE__, __FUNCTION__)
//#define setmemtag(size, t)	v_setmemtag(size, t, __FILE__, __LINE__, __FUNCTION__)
//#define getmemtag(size)		v_getmemtag(size, __FILE__, __LINE__, __FUNCTION__)

extern Pool*	mainmem;
extern Pool*	heapmem;
extern Pool*	imagmem;
