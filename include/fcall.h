#pragma src "/usr/inferno/lib9"
#pragma	lib	"libc.a"

#define	VERSION9P	"9P2000"

#define	MAXWELEM	16

typedef
struct	Fcall
{
	uchar	type;
	u32int	fid;
	ushort	tag;
	/* union { */
		/* struct { */
			u32int	msize;		/* Tversion, Rversion */
			char	*version;	/* Tversion, Rversion */
		/* }; */
		/* struct { */
			u16int	oldtag;		/* Tflush */
		/* }; */
		/* struct { */
			char	*ename;		/* Rerror */
		/* }; */
		/* struct { */
			Qid	qid;		/* Rattach, Ropen, Rcreate */
			u32int	iounit;		/* Ropen, Rcreate */
		/* }; */
		/* struct { */
			Qid	aqid;		/* Rauth */
		/* }; */
		/* struct { */
			u32int	afid;		/* Tauth, Tattach */
			char	*uname;		/* Tauth, Tattach */
			char	*aname;		/* Tauth, Tattach */
		/* }; */
		/* struct { */
			u32int	perm;		/* Tcreate */
			const char	*name;		/* Tcreate */
			uchar	mode;		/* Tcreate, Topen */
		/* }; */
		/* struct { */
			u32int	newfid;		/* Twalk */
			u16int	nwname;		/* Twalk */
			char	*wname[MAXWELEM];	/* Twalk */
		/* }; */
		/* struct { */
			u16int	nwqid;		/* Rwalk */
			Qid	wqid[MAXWELEM];		/* Rwalk */
		/* }; */
		/* struct { */
			vlong	offset;		/* Tread, Twrite */
			u32int	count;		/* Tread, Twrite, Rread */
			char	*data;		/* Twrite, Rread */
		/* }; */
		/* struct { */
			u16int	nstat;		/* Twstat, Rstat */
			const char	*stat;		/* Twstat, Rstat */
		/* }; */
	/* }; */
} Fcall;


#define	BIT8SZ		1
#define	BIT16SZ		2
#define	BIT32SZ		4
#define	BIT64SZ		8
#define	QIDSZ	(BIT8SZ+BIT32SZ+BIT64SZ)

/* STATFIXLEN includes leading 16-bit count */
/* The count, however, excludes itself; total size is BIT16SZ+count */
#define STATFIXLEN	(BIT16SZ+QIDSZ+5*BIT16SZ+4*BIT32SZ+1*BIT64SZ)	/* amount of fixed length data in a stat buffer */

#define	NOTAG		(ushort)~0U	/* Dummy tag */
#define	NOFID		(u32int)~0U	/* Dummy fid */
#define	IOHDRSZ		24	/* ample room for Twrite/Rread header (iounit) */

enum
{
	Tversion =	100,
	Rversion,
	Tauth =	102,
	Rauth,
	Tattach =	104,
	Rattach,
	Terror =	106,	/* illegal */
	Rerror,
	Tflush =	108,
	Rflush,
	Twalk =		110,
	Rwalk,
	Topen =		112,
	Ropen,
	Tcreate =	114,
	Rcreate,
	Tread =		116,
	Rread,
	Twrite =	118,
	Rwrite,
	Tclunk =	120,
	Rclunk,
	Tremove =	122,
	Rremove,
	Tstat =		124,
	Rstat,
	Twstat =	126,
	Rwstat,
	Tmax,
};

size_t	convM2S(const char*, size_t, Fcall*);
size_t	convS2M(const Fcall*, char*, size_t);
size_t	sizeS2M(const Fcall*);

int	statcheck(const char *abuf, size_t nbuf);
size_t	convM2D(const char*, size_t, Dir*, char*);
size_t	convD2M(const Dir*, char*, size_t);
size_t	sizeD2M(const Dir*);

int	fcallfmt(Fmt*);
int	dirfmt(Fmt*);
int	dirmodefmt(Fmt*);

int	read9pmsg(int, void*, uint);

#pragma	varargck	type	"F"	Fcall*
#pragma	varargck	type	"M"	ulong
#pragma	varargck	type	"D"	Dir*
