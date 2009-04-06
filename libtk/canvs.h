typedef struct TkCimeth TkCimeth;
typedef struct TkCitem TkCitem;
typedef struct TkCanvas TkCanvas;
typedef struct TkCline TkCline;
typedef struct TkCtag TkCtag;
typedef struct TkCpoints TkCpoints;
typedef struct TkCwind TkCwind;

struct TkCline
{
    int     arrow;
    int     shape[3];
    int     width;
    Image*      stipple;
    Image*      pen;
    int     arrowf;
    int     arrowl;
    int     capstyle;
    int     smooth;
    int     steps;
};

struct TkCwind
{
    Tk*     sub;        /* Subwindow of canvas */
    Tk*     focus;      /* Current Mouse focus */
    int     width;      /* Requested width */
    int     height;     /* Requested height */
    int     flags;      /* possible: Tkanchor|Tksetwidth|Tksetheight */
};

struct TkCpoints
{
    int     npoint;     /* Number of points */
    Point*      parampt;    /* Parameters in fixed point */
    Point*      drawpt;     /* Draw coord in pixels */
    Rectangle   bb;     /* Bounding box in pixels */
};

struct TkCitem
{
    int     id;     /* Unique id */
    int     type;       /* Object type */
    TkCpoints   p;      /* Points plus bounding box */
    TkEnv*      env;        /* Colors & fonts */
    TkCitem*    next;       /* Z order */
    TkName*     tags;       /* Temporary tag spot */
    TkCtag*     stag;       /* Real tag structure */
//  char        obj[TKSTRUCTALIGN];
};

struct TkCtag
{
    TkCitem*    item;       /* Link to item */
    TkName*     name;       /* Text name or id */
    TkCtag*     taglist;    /* link items with this tag */
    TkCtag*     itemlist;   /* link tags for this item */
};

enum
{
    /* Item types */
    TkCVline,
    TkCVtext,
    TkCVrect,
    TkCVoval,
    TkCVbitmap,
    TkCVpoly,
    TkCVwindow,
    TkCVimage,
    TkCVarc,

    TkCselto    = 0,
    TkCselfrom,
    TkCseladjust,

    TkCbufauto  = 0,
    TkCbufnone,
    TkCbufvisible,
    TkCbufall,

    TkCadd      = 0,
    TkCfind,

    TkChash     = 32,

    TkCarrowf   = (1<<0),
    TkCarrowl   = (1<<1),
    Tknarrow    = 6     /* Number of points in arrow */
};

struct TkCanvas
{
    int     close;
    int     confine;
    int     cleanup;
    int     scrollr[4];
    Rectangle   region;
    Rectangle   update;     /* Area to paint next draw */
    Point       view;
    TkCitem*    selection;
    int     width;
    int     height;
    int     sborderwidth;
    int     xscrolli;   /* Scroll increment */
    int     yscrolli;
    char*       xscroll;    /* Scroll commands */
    char*       yscroll;
    int     id;     /* Unique id */
    TkCitem*    head;       /* Items in Z order */
    TkCitem*    tail;       /* Head is lowest, tail is highest */
    TkCitem*    focus;      /* Keyboard focus */
    TkCitem*    mouse;      /* Mouse focus */
    TkCitem* grab;
    TkName*     current;    /* Fake for current tag */
    TkCtag      curtag;
    Image*      image;      /* Drawing space */
    int         ialloc;     /* image was allocated by us? */
    Image*      mask;       /* mask space (for stippling) */
    TkName*     thash[TkChash]; /* Tag hash */
    int     actions;
    int     actlim;
    int     buffer;
};

struct TkCimeth
{
    char*   name;
    const char* (*create)(Tk*, __in_z const char *arg, char **val);
    void        (*draw)(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
    void        (*fnfree)(TkCitem*);
    const char* (*coord)(TkCitem*, __in_z const char*, int, int);
    const char* (*cget)(TkCitem*, __in_z const char*, char**);
    const char* (*conf)(Tk*, TkCitem*, __in_z const char*);
    int         (*hit)(TkCitem*, Point);
};

extern  TkCimeth    tkcimethod[];
extern  int cvslshape[];
extern  Rectangle   bbnil;
extern  Rectangle   huger;

/* General */
extern  const char* tkcaddtag(Tk*, TkCitem*, int);
extern  TkCtag*     tkcfirsttag(TkCitem*, TkCtag*);
extern  TkCtag*     tkclasttag(TkCitem*, TkCtag*);
extern  void        tkcvsappend(TkCanvas*, TkCitem*);
extern  TkCitem*    tkcnewitem(Tk*, int, int);
extern  void        tkcvsfreeitem(TkCitem*);
extern  Point       tkcvsrelpos(Tk*);
extern  Tk*         tkcvsinwindow(Tk*, Point*);
extern  const char* tkcvstextdchar(Tk*, TkCitem*, __in_z const char*);
extern  const char* tkcvstextindex(Tk*, TkCitem*, __in_z const char*, char **val);
extern  const char* tkcvstextinsert(Tk*, TkCitem*, __in_z const char*);
extern  const char* tkcvstexticursor(Tk*, TkCitem*, __in_z const char*);
extern  void        tkmkpen(Image**, TkEnv*, Image*);
extern  void        tkcvstextfocus(Tk*, TkCitem*, int);
extern  const char* tkcvstextselect(Tk*, TkCitem*, __in_z const char*, int);
extern  void        tkcvstextclr(Tk*);
extern  Tk*         tkcvsevent(Tk*, int, void*);
extern  Point       tkcvsanchor(Point, int, int, int);
extern  void        tkcvsdirty(Tk*);
extern  void        tkfreectag(TkCtag*);
extern  const char* tkparsepts(TkTop*, TkCpoints*, __inout const char**, int);
extern  void        tkfreepoint(TkCpoints*);
extern  void        tkxlatepts(Point*, int, int, int);
extern  void        tkpolybound(Point*, int, Rectangle*);
extern  TkName*     tkctaglook(Tk*, TkName*, __in_z const char*);
extern  void        tkbbmax(Rectangle*, Rectangle*);
extern  void        tkcvssetdirty(Tk*);

/* Canvas Item methods - required to populate tkcimethod in canvs.c */
extern              TH(tkcvslinecreat);
extern  void        tkcvslinedraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvslinefree(TkCitem*);
extern  const char* tkcvslinecoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvslinecget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvslineconf(Tk*, TkCitem*, __in_z const char*);
extern  int         tkcvslinehit(TkCitem*, Point);

extern              TH(tkcvstextcreat);
extern  void        tkcvstextdraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvstextfree(TkCitem*);
extern  const char* tkcvstextcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvstextcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvstextconf(Tk*, TkCitem*, __in_z const char*);

extern  const char* tkcvsrectcreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvsrectdraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvsrectfree(TkCitem*);
extern  const char* tkcvsrectcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvsrectcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvsrectconf(Tk*, TkCitem*, __in_z const char*);

extern  const char* tkcvsovalcreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvsovaldraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvsovalfree(TkCitem*);
extern  const char* tkcvsovalcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvsovalcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvsovalconf(Tk*, TkCitem*, __in_z const char*);
extern  int         tkcvsovalhit(TkCitem*, Point);

extern  const char* tkcvsarccreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvsarcdraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvsarcfree(TkCitem*);
extern  const char* tkcvsarccoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvsarccget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvsarcconf(Tk*, TkCitem*, __in_z const char*);

extern  const char* tkcvsbitcreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvsbitdraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvsbitfree(TkCitem*);
extern  const char* tkcvsbitcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvsbitcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvsbitconf(Tk*, TkCitem*, __in_z const char*);

extern  const char* tkcvswindcreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvswinddraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvswindfree(TkCitem*);
extern  const char* tkcvswindcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvswindcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvswindconf(Tk*, TkCitem*, __in_z const char*);

extern  const char* tkcvspolycreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvspolydraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvspolyfree(TkCitem*);
extern  const char* tkcvspolycoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvspolycget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvspolyconf(Tk*, TkCitem*, __in_z const char*);
extern  int         tkcvspolyhit(TkCitem*, Point);

extern  const char* tkcvsimgcreat(Tk*, __in_z const char *arg, char **val);
extern  void        tkcvsimgdraw(__in_ecount(1) const Image*, TkCitem*, TkEnv*);
extern  void        tkcvsimgfree(TkCitem*);
extern  const char* tkcvsimgcoord(TkCitem*, __in_z const char*, int, int);
extern  const char* tkcvsimgcget(TkCitem*, __in_z const char*, char**);
extern  const char* tkcvsimgconf(Tk*, TkCitem*, __in_z const char*);

