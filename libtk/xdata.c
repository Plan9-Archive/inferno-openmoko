#include <lib9.h>
#include <draw.h>

#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <tk.h>

TkStab tkorient[] =
{
	{"vertical",	Tkvertical},
	{"horizontal",	Tkhorizontal},
	{nil}
};

#define TKRGB(r,g,b) ((r<<24)|(g<<16)|(b<<8)|0xff)

TkStab tkcolortab[] =
{
	{"black",	TKRGB(0,0,0)	},
	{"blue",	TKRGB(0,0,204)	},
	{"darkblue",	TKRGB(93,0,187)	},
	{"red",		TKRGB(255,0,0)	},
	{"yellow",	TKRGB(255,255,0)	},
	{"green",	TKRGB(0,128,0)	},
	{"white",	TKRGB(255,255,255)},
	{"orange",	TKRGB(255,170,0)	},
	{"aqua",	TKRGB(0,255,255)	},
	{"fuchsia",	TKRGB(255,0,255)	},
	{"gray",	TKRGB(128,128,128)},
	{"grey",	TKRGB(128,128,128)},
	{"lime",	TKRGB(0,255,0)	},
	{"maroon",	TKRGB(128,0,0)	},
	{"navy",	TKRGB(0,0,128)	},
	{"olive",	TKRGB(128,128,0)	},
	{"purple",	TKRGB(128,0,128)	},
	{"silver",	TKRGB(192,192,192)},
	{"teal",	TKRGB(0,128,128)	},
	{"transparent",	DTransparent	},
	{nil}
};
#undef TKRGB

TkStab tkrelief[] =
{
	{"raised",	TKraised},
	{"sunken",	TKsunken},
	{"flat",	TKflat},
	{"groove",	TKgroove},
	{"ridge",	TKridge},
	{nil}
};

TkStab tkbool[] =
{
	{"0",		BoolF},
	{"no",		BoolF},
	{"off",		BoolF},
	{"false",	BoolF},
	{"1",		BoolT},
	{"yes",		BoolT},
	{"on",		BoolT},
	{"true",	BoolT},
	{nil}
};

TkStab tkanchor[] =
{
	{"center",	Tkcenter},
	{"c",		Tkcenter},
	{"n",		Tknorth},
	{"ne",		Tknorth|Tkeast},
	{"e",		Tkeast},
	{"se",		Tksouth|Tkeast},
	{"s",		Tksouth},
	{"sw",		Tksouth|Tkwest},
	{"w",		Tkwest},
	{"nw",		Tknorth|Tkwest},
	{nil}
};

static
TkStab tkstate[] =
{
	{"normal",	0},
	{"active",	Tkactive},
	{"disabled",	Tkdisabled},
	{nil}
};

static
TkStab tktakefocus[] =
{
	{"0",		0},
	{"1",		Tktakefocus},
	{nil}
};

TkStab tktabjust[] =
{
	{"left",	Tkleft},
	{"right",	Tkright},
	{"center",	Tkcenter},
	{"numeric",	Tknumeric},
	{nil}
};

TkStab tkwrap[] =
{
	{"none",	Tkwrapnone},
	{"word",	Tkwrapword},
	{"char",	Tkwrapchar},
	{nil}
};

TkStab tkjustify[] =
{
	{"left",	Tkleft},
	{"right",	Tkright},
	{"center",	Tkcenter},
	{nil}
};

TkOption tkgeneric[] =
{
	{"actx",		OPTact,		0,				{(TkStab*)0}},
	{"acty",		OPTact,		0,				{(TkStab*)1}},
	{"actwidth",		OPTdist,	offsetof(Tk, act.width),	{(TkStab*)offsetof(Tk, env)}},
	{"actheight",		OPTdist,	offsetof(Tk, act.height),	{(TkStab*)offsetof(Tk, env)}},
	{"bd",			OPTnndist,	offsetof(Tk, borderwidth)	},
	{"borderwidth",		OPTnndist,	offsetof(Tk, borderwidth)	},
	{"highlightthickness",	OPTnndist,	offsetof(Tk, highlightwidth) 	},
	{"height",		OPTsize,	0,				{(TkStab*)offsetof(Tk, env)}},
	{"width",		OPTsize,	0,				{(TkStab*)offsetof(Tk, env)}},
	{"relief",		OPTstab, 	offsetof(Tk, relief),		{tkrelief}},
	{"state",		OPTflag, 	offsetof(Tk, flag),		{tkstate}},
	{"font",		OPTfont, 	offsetof(Tk, env)		},
	{"foreground",		OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCforegnd}},
	{"background",		OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCbackgnd}},
	{"fg",			OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCforegnd}},
	{"bg",			OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCbackgnd}},
	{"selectcolor",		OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCselect}},
	{"selectforeground",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCselectfgnd}},
	{"selectbackground",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCselectbgnd}},
	{"activeforeground",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCactivefgnd}},
	{"activebackground",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCactivebgnd}},
	{"highlightcolor",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkChighlightfgnd}},
	{"disabledcolor",	OPTcolr, 	offsetof(Tk, env),		{(TkStab*)TkCdisablefgnd}},
	{"padx",		OPTnndist, 	offsetof(Tk, pad.x)		},
	{"pady",		OPTnndist, 	offsetof(Tk, pad.y)		},
	{"takefocus",		OPTflag, 	offsetof(Tk, flag),		{tktakefocus}},
	{nil}
};

TkOption tktop[] =
{
	{"x",			OPTdist,	offsetof(TkWin, req.x)	},
	{"y",			OPTdist,	offsetof(TkWin, req.y)	},
	{nil}
};

TkOption tktopdbg[] =
{
	{"debug",	OPTbool,	offsetof(TkTop, debug)	},
	{nil},
};

TkMethod *tkmethod[] =
{
	&framemethod,	/* TKframe */
	&labelmethod,		/* TKlabel */
	&checkbuttonmethod,	/* TKcheckbutton */
	&buttonmethod,	/* TKbutton */
	&menubuttonmethod,	/* TKmenubutton */
	&menumethod,	/* TKmenu */
	&separatormethod,	/* TKseparator */
	&cascademethod,	/* TKcascade */
	&listboxmethod,	/* TKlistbox */
	&scrollbarmethod,	/* TKscrollbar */
	&textmethod,	/* TKtext */
	&canvasmethod,	/* TKcanvas */
	&entrymethod,	/* TKentry */
	&radiobuttonmethod,	/* TKradiobutton */
	&scalemethod,	/* TKscale */
	&panelmethod,	/* TKpanel */
	&choicebuttonmethod,	/*TKchoicebutton */
};

const char TkNomem[] = "!out of memory";
const char TkBadop[] = "!bad option";
const char TkOparg[] = "!arg requires option";
const char TkBadvl[] = "!bad value";
const char TkBadwp[] = "!bad window path";
const char TkWpack[] = "!window is already packed";
const char TkNotop[] = "!no toplevel";
const char TkDupli[] = "!window path already exists";
const char TkNotpk[] = "!window not packed";
const char TkBadcm[] = "!bad command";
const char TkIstop[] = "!can't pack top level";
const char TkBadbm[] = "!failed to load bitmap";
const char TkBadft[] = "!failed to open font";
const char TkBadit[] = "!bad item type";
const char TkBadtg[] = "!bad/no matching tag";
const char TkFewpt[] = "!wrong number of points";
const char TkBadsq[] = "!bad event sequence";
const char TkBadix[] = "!bad index";
const char TkNotwm[] = "!not a window";
const char TkBadvr[] = "!variable does not exist";
const char TkNotvt[] = "!variable is wrong type";
const char TkMovfw[] = "!too many events buffered";
const char TkBadsl[] = "!selection already exists";
const char TkSyntx[] = "!bad [] or {} syntax";
const char TkRecur[] = "!cannot pack recursively";
const char TkDepth[] = "!execution stack too big";
const char TkNomaster[] = "!no master given";
const char TkNotgrid[] = "!not a grid";
const char TkIsgrid[] = "!cannot use pack inside a grid";
const char TkBadgridcell[] = "!grid cell in use";
const char TkBadspan[] = "!bad grid span";
const char TkBadcursor[] = "!bad cursor image";
