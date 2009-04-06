#pragma src "/usr/inferno/libdraw"

#pragma	varargck	argpos	_drawprint	2

typedef struct	Cachefont Cachefont;
typedef struct	Cacheinfo Cacheinfo;
typedef struct	Cachesubf Cachesubf;
typedef struct	Display Display;
typedef struct	Font Font;
typedef struct	Fontchar Fontchar;
typedef struct	Image Image;
typedef struct	Mouse Mouse;
typedef struct	Point Point;
typedef struct	Rectangle Rectangle;
typedef struct	RGB RGB;
typedef struct	Refreshq Refreshq;
typedef struct	Screen Screen;
typedef struct	Subfont Subfont;

#pragma varargck	type	"R"	Rectangle
#pragma varargck	type	"P"	Point
extern	int	Rfmt(Fmt*);
extern	int	Pfmt(Fmt*);

enum
{
	DOpaque		= 0xFFFFFFFF,
	DTransparent	= 0x00000000,		/* only useful for allocimage, memfillcolor */
	DBlack		= 0x000000FF,
	DWhite		= 0xFFFFFFFF,
	DRed		= 0xFF0000FF,
	DGreen		= 0x00FF00FF,
	DBlue		= 0x0000FFFF,
	DCyan		= 0x00FFFFFF,
	DMagenta	= 0xFF00FFFF,
	DYellow		= 0xFFFF00FF,
	DPaleyellow	= 0xFFFFAAFF,
	DDarkyellow	= 0xEEEE9EFF,
	DDarkgreen	= 0x448844FF,
	DPalegreen	= 0xAAFFAAFF,
	DMedgreen	= 0x88CC88FF,
	DDarkblue	= 0x000055FF,
	DPalebluegreen	= 0xAAFFFFFF,
	DPaleblue	= 0x0000BBFF,
	DBluegreen	= 0x008888FF,
	DGreygreen	= 0x55AAAAFF,
	DPalegreygreen	= 0x9EEEEEFF,
	DYellowgreen	= 0x99994CFF,
	DMedblue	= 0x000099FF,
	DGreyblue	= 0x005DBBFF,
	DPalegreyblue	= 0x4993DDFF,
	DPurpleblue	= 0x8888CCFF,

	DNotacolor	= 0xFFFFFF00,
	DNofill		= DNotacolor,

};

enum
{
	Displaybufsize	= 8000,
	ICOSSCALE	= 1024,
	Borderwidth	= 4,
};

enum
{
	/* refresh methods */
	Refbackup	= 0,
	Refnone		= 1,
	Refmesg		= 2
};
#define	NOREFRESH	((void*)-1)

enum
{
	/* line ends */
	Endsquare	= 0,
	Enddisc		= 1,
	Endarrow	= 2,
	Endmask		= 0x1F
};

#define	ARROW(a, b, c)	(Endarrow|((a)<<5)|((b)<<14)|((c)<<23))

/*
 * image channel descriptors
 */
enum {
	CRed = 0,  /* r */
	CGreen,    /* g */
	CBlue,     /* b */
	CGrey,     /* k */
	CAlpha,    /* a */
	CMap,      /* m */
	CIgnore,   /* x */
	NChan,
};

#define __DC(type, nbits)	((((type)&15)<<4)|((nbits)&15))
#define CHAN1(a,b)	__DC(a,b)
#define CHAN2(a,b,c,d)	(CHAN1((a),(b))<<8|__DC((c),(d)))
#define CHAN3(a,b,c,d,e,f)	(CHAN2((a),(b),(c),(d))<<8|__DC((e),(f)))
#define CHAN4(a,b,c,d,e,f,g,h)	(CHAN3((a),(b),(c),(d),(e),(f))<<8|__DC((g),(h)))

#define NBITS(c) ((c)&15)
#define TYPE(c) (((c)>>4)&15)

enum {
	GREY1	= CHAN1(CGrey, 1),
	GREY2	= CHAN1(CGrey, 2),
	GREY4	= CHAN1(CGrey, 4),
	GREY8	= CHAN1(CGrey, 8),
	CMAP8	= CHAN1(CMap, 8),
	RGB15	= CHAN4(CIgnore, 1, CRed, 5, CGreen, 5, CBlue, 5),
	RGB16	= CHAN3(CRed, 5, CGreen, 6, CBlue, 5),
	RGB24	= CHAN3(CRed, 8, CGreen, 8, CBlue, 8),
	RGBA32	= CHAN4(CRed, 8, CGreen, 8, CBlue, 8, CAlpha, 8),
	ARGB32	= CHAN4(CAlpha, 8, CRed, 8, CGreen, 8, CBlue, 8),	/* stupid VGAs */
	XRGB32  = CHAN4(CIgnore, 8, CRed, 8, CGreen, 8, CBlue, 8),
	BGR24	= CHAN3(CBlue, 8, CGreen, 8, CRed, 8),
	ABGR32	= CHAN4(CAlpha, 8, CBlue, 8, CGreen, 8, CRed, 8),
	XBGR32	= CHAN4(CIgnore, 8, CBlue, 8, CGreen, 8, CRed, 8),

	/* compact image formats (for PDA) */
	BWA8	= CHAN2(CGrey, 1, CAlpha, 7),
	GRAY4A	= CHAN2(CGrey, 4, CAlpha, 4),
	RGBA8 	= CHAN4(CRed, 2, CGreen, 2, CBlue, 2, CAlpha, 2),
	RGBA16	= CHAN4(CRed, 4, CGreen, 4, CBlue, 4, CAlpha, 4),
	MRGB16	= CHAN4(CAlpha, 1, CRed, 5, CGreen, 5, CBlue, 5),	/* RGB15 + mask bit */
};

/* compositing operators */

typedef enum
{
	SinD	= 1<<3,
	DinS	= 1<<2,
	SoutD	= 1<<1,
	DoutS	= 1 <<0,

	S	= SinD|SoutD,
	SoverD	= SinD|SoutD|DoutS,
	SatopD	= SinD|DoutS,
	SxorD	= SoutD|DoutS,

	D	= DinS|DoutS,
	DoverS	= DinS|DoutS|SoutD,
	DatopS	= DinS|SoutD,
	DxorS	= DoutS|SoutD,

	Clear	= 0,

	Ncomp	= 12,
} Drawop;

extern	char*	chantostr(char*, ulong);
extern	ulong	strtochan(char*);
extern	int		chantodepth(ulong);

struct	Point
{
	int	x;
	int	y;
};

struct Rectangle
{
	Point	min;
	Point	max;
};

typedef void	(*Reffn)(Image*, Rectangle, void*);

struct Screen
{
	Display	*display;	/* display holding data */
	int	id;		/* id of system-held Screen */
	Image	*image;		/* unused; for reference only */
	Image	*fill;		/* color to paint behind windows */
};

struct Refreshq
{
	Reffn		reffn;
	void		*refptr;
	Rectangle	r;
	Refreshq	*next;
};

typedef struct DRef DRef;
struct Display
{
	QLock		*qlock;
	int		locking;	/*program is using lockdisplay */
	int		dirno;
	Chan		*datachan;
	Chan		*refchan;
	Chan		*ctlchan;
	int		imageid;
	int		local;
	int		depth;
	ulong		chan;
	void		(*error)(Display*, char*);
	char		*devdir;
	char		*windir;
	char		oldlabel[64];
	ulong		dataqid;
	Image		*white;
	Image		*black;
	Image		*image;
	Image		*opaque;
	Image		*transparent;
	char		buf[Displaybufsize+1];	/* +1 for flush message */
	int		bufsize;
	char		*bufp;
	Font		*defaultfont;
	Subfont		*defaultsubfont;
	Image		*windows;
	DRef		*limbo;
	Refreshq	*refhead;
	Refreshq	*reftail;
};

struct Image
{
	Display		*display;	/* display holding data */
	int		id;		/* id of system-held Image */
	Rectangle	r;		/* rectangle in data area, local coords */
	Rectangle 	clipr;		/* clipping region */
	int		depth;		/* number of bits per pixel */
	ulong		chan;
	int		repl;		/* flag: data replicates to tile clipr */
	Screen		*screen;	/* 0 if not a window */
	Image		*next;	/* next in list of windows */
	Reffn		reffn;
	void		*refptr;
};

struct RGB
{
	ulong	red;
	ulong	green;
	ulong	blue;
};

/*
 * Subfonts
 *
 * given char c, Subfont *f, Fontchar *i, and Point p, one says
 *	i = f->info+c;
 *	draw(b, Rect(p.x+i->left, p.y+i->top,
 *		p.x+i->left+((i+1)->x-i->x), p.y+i->bottom),
 *		color, f->bits, Pt(i->x, i->top));
 *	p.x += i->width;
 * to draw characters in the specified color (itself an Image) in Image b.
 */

struct	Fontchar
{
	int		x;		/* left edge of bits */
	uchar		top;		/* first non-zero scan-line */
	uchar		bottom;		/* last non-zero scan-line + 1 */
	char		left;		/* offset of baseline */
	uchar		width;		/* width of baseline */
};

struct	Subfont
{
	char		*name;
	short		n;		/* number of chars in font */
	uchar		height;		/* height of image */
	char		ascent;		/* top of image to baseline */
	Fontchar 	*info;		/* n+1 character descriptors */
	Image		*bits;		/* of font */
	int		ref;
};

enum
{
	/* starting values */
	LOG2NFCACHE =	6,
	NFCACHE =	(1<<LOG2NFCACHE),	/* #chars cached */
	NFLOOK =	5,			/* #chars to scan in cache */
	NFSUBF =	2,			/* #subfonts to cache */
	/* max value */
	MAXFCACHE =	1024+NFLOOK,		/* upper limit */
	MAXSUBF =	50,			/* generous upper limit */
	/* deltas */
	DSUBF = 	4,
	/* expiry ages */
	SUBFAGE	=	10000,
	CACHEAGE =	10000
};

struct Cachefont
{
	Rune		min;	/* lowest rune value to be taken from subfont */
	Rune		max;	/* highest rune value+1 to be taken from subfont */
	int		offset;	/* position in subfont of character at min */
	char		*name;			/* stored in font */
	char		*subfontname;		/* to access subfont */
};

struct Cacheinfo
{
	ushort		x;		/* left edge of bits */
	uchar		width;		/* width of baseline */
	schar		left;		/* offset of baseline */
	Rune		value;	/* value of character at this slot in cache */
	ushort		age;
};

struct Cachesubf
{
	ulong		age;	/* for replacement */
	Cachefont	*cf;	/* font info that owns us */
	Subfont		*f;	/* attached subfont */
};

struct Font
{
	char		*name;
	Display		*display;
	short		height;	/* max height of image, interline spacing */
	short		ascent;	/* top of image to baseline */
	short		width;	/* widest so far; used in caching only */
	short		nsub;	/* number of subfonts */
	ulong		age;	/* increasing counter; used for LRU */
	int		maxdepth;	/* maximum depth of all loaded subfonts */
	int		ncache;	/* size of cache */
	int		nsubf;	/* size of subfont list */
	Cacheinfo	*cache;
	Cachesubf	*subf;
	Cachefont	**sub;	/* as read from file */
	Image		*cacheimage;
};

#define	Dx(r)	((r).max.x-(r).min.x)
#define	Dy(r)	((r).max.y-(r).min.y)

/*
 * Image management
 */
extern Image*	_allocimage(Image*, Display*, Rectangle, ulong, int, ulong, int, int);
extern Image*	allocimage(Display*, Rectangle, ulong, int, ulong);
extern char*	bufimage(Display*, int);
extern int	bytesperline(Rectangle, int);
extern void	closedisplay(Display*);
extern void	drawerror(Display*, char*);
extern int	_drawprint(int, __in_z __format_string const char*, ...);
extern int	flushimage(Display*, int);
extern int	freeimage(Image*);
extern int	_freeimage1(Image*);
extern int	geninitdraw(char*, void(*)(Display*, char*), char*, char*, char*, int);
extern int	initdraw(void(*)(Display*, char*), char*, char*);
extern Display*	initdisplay(char*, char*, void(*)(Display*, char*));
extern int	loadimage(Image*, Rectangle, const char*, int);
extern int	cloadimage(Image*, Rectangle, const char*, int);
extern int	getwindow(Display*, int);
extern int	gengetwindow(Display*, char*, Image**, Screen**, int);
extern Image* readimage(Display*, int, int);
extern Image* creadimage(Display*, int, int);
extern int	unloadimage(Image*, Rectangle, char*, int);
extern int	wordsperline(Rectangle, int);
extern int	writeimage(int, Image*, int);
extern Image*	namedimage(Display*, char*);
extern int	nameimage(Image*, char*, int);
extern Image* allocimagemix(Display*, ulong, ulong);

/*
 * Colors
 */
extern	void	readcolmap(Display*, RGB*);
extern	void	writecolmap(Display*, RGB*);
extern	ulong	setalpha(ulong, uchar);

/*
 * Windows
 */
extern Screen*	allocscreen(Image*, Image*, int);
extern Image*	_allocwindow(Image*, Screen*, Rectangle, int, ulong);
extern Image*	allocwindow(Screen*, Rectangle, int, ulong);
extern void	bottomnwindows(Image**, int);
extern void	bottomwindow(Image*);
extern int	freescreen(Screen*);
extern Screen*	publicscreen(Display*, int, ulong);
extern void	topnwindows(Image**, int);
extern void	topwindow(Image*);
extern int	originwindow(Image*, Point, Point);

/*
 * Geometry
 */
extern Point		Pt(int, int);
extern Rectangle	Rect(int, int, int, int);
extern Rectangle	Rpt(Point, Point);
extern Point		addpt(Point, Point);
extern Point		subpt(Point, Point);
extern Point		divpt(Point, int);
extern Point		mulpt(Point, int);
extern int		eqpt(Point, Point);
extern int		eqrect(Rectangle, Rectangle);
extern Rectangle	insetrect(Rectangle, int);
extern Rectangle	rectaddpt(Rectangle, Point);
extern Rectangle	rectsubpt(Rectangle, Point);
extern Rectangle	canonrect(Rectangle);
extern int		rectXrect(Rectangle, Rectangle);
extern int		rectinrect(Rectangle, Rectangle);
extern void		combinerect(Rectangle*, Rectangle);
extern int		rectclip(Rectangle*, Rectangle);
extern int		ptinrect(Point, Rectangle);
extern void		replclipr(Image*, int, Rectangle);
extern int		drawreplxy(int, int, int);	/* used to be drawsetxy */
extern Point		drawrepl(Rectangle, Point);
extern int		rgb2cmap(int, int, int);
extern int		cmap2rgb(int);
extern int		cmap2rgba(int);
extern void		icossin(int, int*, int*);
extern void		icossin2(int, int, int*, int*);

/*
 * Graphics
 */
extern void	    draw(__in_ecount(1) const Image*, Rectangle, __in_ecount_opt(1) const Image*, __in_ecount_opt(1) const Image*, Point);
extern void	    drawop(__in_ecount(1) const Image*, Rectangle, __in_ecount_opt(1) const Image*, __in_ecount_opt(1) const Image*, Point, Drawop);
extern void	    gendraw(__in_ecount(1) const Image*, Rectangle, __in_ecount_opt(1) const Image*, Point, __in_ecount_opt(1) const Image*, Point);
extern void	    gendrawop(__in_ecount(1) const Image*, Rectangle, __in_ecount_opt(1) const Image*, Point, __in_ecount_opt(1) const Image*, Point, Drawop);
extern void	    line(__in_ecount(1) const Image*, Point, Point, int, int, int, __in_ecount(1) const Image*, Point);
extern void	    lineop(__in_ecount(1) const Image*, Point, Point, int, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern void	    poly(__in_ecount(1) const Image*, Point*, int, int, int, int, __in_ecount(1) const Image*, Point);
extern void	    polyop(__in_ecount(1) const Image*, Point*, int, int, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern void	    fillpoly(__in_ecount(1) const Image*, Point*, int, int, __in_ecount(1) const Image*, Point);
extern void	    fillpolyop(__in_ecount(1) const Image*, Point*, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern Point	string(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_z const char*);
extern Point	stringop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_z const char*, Drawop);
extern Point	stringn(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_ecount_z(len) const char*, int len);
extern Point	stringnop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_ecount_z(len) const char*, int len, Drawop);
extern Point	runestring(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_z const Rune*);
extern Point	runestringop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_z const Rune*, Drawop);
extern Point	runestringn(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_ecount_z(len) const Rune*, int len);
extern Point	runestringnop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, __in_ecount_z(len) const Rune*, int len, Drawop);
extern Point	stringbg(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, char*, __in_ecount(1) const Image*, Point);
extern Point	stringbgop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, char*, __in_ecount(1) const Image*, Point, Drawop);
extern Point	stringnbg(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, char*, int, __in_ecount(1) const Image*, Point);
extern Point	stringnbgop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, char*, int, __in_ecount(1) const Image*, Point, Drawop);
extern Point	runestringbg(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, Rune*, __in_ecount(1) const Image*, Point);
extern Point	runestringbgop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, Rune*, __in_ecount(1) const Image*, Point, Drawop);
extern Point	runestringnbg(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, Rune*, int, __in_ecount(1) const Image*, Point);
extern Point	runestringnbgop(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, Rune*, int, __in_ecount(1) const Image*, Point, Drawop);
extern Point	_string(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Point, Font*, const char*, const Rune*, int, Rectangle, __in_ecount_opt(1) const Image*, Point, Drawop);
extern Point	stringsubfont(__in_ecount(1) const Image*, Point, __in_ecount(1) const Image*, Subfont*, char*);
extern int	    bezier(__in_ecount(1) const Image*, Point, Point, Point, Point, int, int, int, __in_ecount(1) const Image*, Point);
extern int	    bezierop(__in_ecount(1) const Image*, Point, Point, Point, Point, int, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern int	    bezspline(__in_ecount(1) const Image*, Point*, int, int, int, int, __in_ecount(1) const Image*, Point);
extern int	    bezsplineop(__in_ecount(1) const Image*, Point*, int, int, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern int	    getbezsplinepts(Point*, int, Point**);
extern int	    fillbezier(__in_ecount(1) const Image*, Point, Point, Point, Point, int, __in_ecount(1) const Image*, Point);
extern int	    fillbezierop(__in_ecount(1) const Image*, Point, Point, Point, Point, int, __in_ecount(1) const Image*, Point, Drawop);
extern int	    fillbezspline(__in_ecount(1) const Image*, Point*, int, int, __in_ecount(1) const Image*, Point);
extern int	    fillbezsplineop(__in_ecount(1) const Image*, Point*, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern void	    ellipse(__in_ecount(1) const Image*, Point, int, int, int, __in_ecount(1) const Image*, Point);
extern void	    ellipseop(__in_ecount(1) const Image*, Point, int, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern void	    fillellipse(__in_ecount(1) const Image*, Point, int, int, __in_ecount(1) const Image*, Point);
extern void	    fillellipseop(__in_ecount(1) const Image*, Point, int, int, __in_ecount(1) const Image*, Point, Drawop);
extern void	    arc(__in_ecount(1) const Image*, Point, int, int, int, __in_ecount(1) const Image*, Point, int, int);
extern void	    arcop(__in_ecount(1) const Image*, Point, int, int, int, __in_ecount(1) const Image*, Point, int, int, Drawop);
extern void	    fillarc(__in_ecount(1) const Image*, Point, int, int, __in_ecount(1) const Image*, Point, int, int);
extern void	    fillarcop(__in_ecount(1) const Image*, Point, int, int, __in_ecount(1) const Image*, Point, int, int, Drawop);
extern void	    border(__in_ecount(1) const Image*, Rectangle, int, __in_ecount(1) const Image*, Point);
extern void	    borderop(__in_ecount(1) const Image*, Rectangle, int, __in_ecount(1) const Image*, Point, Drawop);

/*
 * Font management
 */
extern Font*	openfont(Display*, char*);
extern Font*	buildfont(Display*, char*, char*);
extern void	    freefont(Font*);
extern Font*	mkfont(Subfont*, Rune);
extern int	    cachechars(Font*, const char**, const Rune**, ushort*, int, int*, const char**);
extern void	    agefont(Font*);
extern Subfont*	allocsubfont(const char*, int, int, int, Fontchar*, Image*);
extern Subfont*	lookupsubfont(Display*, const char*);
extern void	    installsubfont(const char*, Subfont*);
extern void	    uninstallsubfont(Subfont*);
extern void	    freesubfont(Subfont*);
extern Subfont*	readsubfont(Display*, const char*, int, int);
extern Subfont*	readsubfonti(Display*, const char*, int, Image*, int);
extern int	    writesubfont(int, Subfont*);
extern void	    _unpackinfo(Fontchar*, uchar*, int);
extern Point	stringsize(Font*, __in_z const char*);
extern int	    stringwidth(Font*, __in_z const char*);
extern int	    stringnwidth(Font*, __in_z const char*, int);
extern Point	runestringsize(Font*, Rune*);
extern int	    runestringwidth(Font*, Rune*);
extern int	    runestringnwidth(Font*, Rune*, int);
extern Point	strsubfontwidth(Subfont*, char*);
extern int	    loadchar(Font*, Rune, Cacheinfo*, int, int, const char**);
extern char*	subfontname(char*, char*, int);
extern Subfont*	_getsubfont(Display*, const char*);
extern Subfont*	getdefont(Display*);
extern int	    lockdisplay(Display*);
extern void	    unlockdisplay(Display*);

typedef struct Memimage Memimage;
typedef struct Refx Refx;
typedef void (*Refreshfn)(Memimage*, Rectangle, Refx*);
extern int	    drawlsetrefresh(ulong, int, Refreshfn, Refx*);

/* Compositing operator utility */
extern void	_setdrawop(Display*, Drawop);

/*
 * Predefined
 */
extern	uchar		defontdata[];
extern	int		sizeofdefont;
extern	Point		ZP;
extern	Rectangle	ZR;

/*
 * Set up by initdraw()
 */
extern	int	_cursorfd;
extern	int	_drawdebug;	/* set to 1 to see errors from flushimage */

#define	BGSHORT GBIT16
#define	BGLONG  GBIT32
#define	BPSHORT PBIT16
#define	BPLONG  PBIT32

/*
 * Compressed image file parameters
 */
#define	NMATCH	3		/* shortest match possible */
#define	NRUN	(NMATCH+31)	/* longest match possible */
#define	NMEM	1024		/* window size */
#define	NDUMP	128		/* maximum length of dump */
#define	NCBLOCK	6000		/* size of compressed blocks */
extern	void	_twiddlecompressed(uchar*, int);
extern	int	_compblocksize(Rectangle, int);

/* XXX backwards helps; should go */
extern	ulong	drawld2chan[];
extern	void	drawsetdebug(int);

/*
 * Inferno interface
 */
extern	Font*	font_open(Display*, char*);
extern	void	font_close(Font*);

/*
 * Macros to convert between C and Limbo types
 */
#define	IRECT(r)	(*(Rectangle*)&(r))
#define	DRECT(r)	(*(Draw_Rect*)&(r))
#define	IPOINT(p)	(*(Point*)&(p))
#define	DPOINT(p)	(*(Draw_Point*)&(p))

#define P2P(p1, p2)	(p1).x = (p2).x, (p1).y = (p2).y
#define R2R(r1, r2)	(r1).min.x = (r2).min.x, (r1).min.y = (r2).min.y,\
			(r1).max.x = (r2).max.x, (r1).max.y = (r2).max.y
extern Image*	display_open(Display*, const char*);

#define TR_ALLOCIMAGE(r,c) {char s[64]; o("%s:%d %s allocimage(%d %d %d %d 0x%08uX=%s)\n", __FILE__, __LINE__, __FUNCTION__, (r).min.x, (r).min.y, (r).max.x, (r).max.y, (c), chantostr(s,(c)) );}
