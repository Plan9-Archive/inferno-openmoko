#include "lib9.h"
#include "draw.h"

#include "isa.h"
#include "interp.h"
#include "../libinterp/runt.h"
#include "tk.h"

TkStab tkorient[] =
{
	"vertical",	Tkvertical,
	"horizontal",	Tkhorizontal,
	nil
};

#define RGB(r,g,b) ((r<<24)|(g<<16)|(b<<8)|0xff)

TkStab tkcolortab[] =
{
	"black",	RGB(0,0,0),
	"blue",		RGB(0,0,204),
	"darkblue",	RGB(93,0,187),
	"red",		RGB(255,0,0),
	"yellow",	RGB(255,255,0),
	"green",	RGB(0,128,0),
	"white",	RGB(255,255,255),
	"orange",	RGB(255,170,0),
	"aqua",		RGB(0,255,255),
	"fuchsia",	RGB(255,0,255),
	"gray",		RGB(128,128,128),
	"grey",		RGB(128,128,128),
	"lime",		RGB(0,255,0),
	"maroon",	RGB(128,0,0),
	"navy",		RGB(0,0,128),
	"olive",	RGB(128,128,0),
	"purple",	RGB(128,0,128),
	"silver",	RGB(192,192,192),
	"teal",		RGB(0,128,128),
	"transparent",	DTransparent,
	nil
};

TkStab tkrelief[] =
{
	"raised",	TKraised,
	"sunken",	TKsunken,
	"flat",		TKflat,
	"groove",	TKgroove,
	"ridge",	TKridge,
	nil
};

TkStab tkbool[] =
{
	"0",		BoolF,
	"no",		BoolF,
	"off",		BoolF,
	"false",	BoolF,
	"1",		BoolT,
	"yes",		BoolT,
	"on",		BoolT,
	"true",		BoolT,
	nil
};

TkStab tkanchor[] =
{
	"center",	Tkcenter,
	"c",		Tkcenter,
	"n",		Tknorth,
	"ne",		Tknorth|Tkeast,
	"e",		Tkeast,
	"se",		Tksouth|Tkeast,
	"s",		Tksouth,
	"sw",		Tksouth|Tkwest,
	"w",		Tkwest,
	"nw",		Tknorth|Tkwest,
	nil
};

static
TkStab tkstate[] =
{
	"normal",	0,
	"active",	Tkactive,
	"disabled",	Tkdisabled,
	nil
};

static
TkStab tktakefocus[] =
{
	"0",	0,
	"1",	Tktakefocus,
	nil
};

TkStab tktabjust[] =
{
	"left",		Tkleft,
	"right",	Tkright,
	"center",	Tkcenter,
	"numeric",	Tknumeric,
	nil
};

TkStab tkwrap[] =
{
	"none",		Tkwrapnone,
	"word",		Tkwrapword,
	"char",		Tkwrapchar,
	nil
};

TkStab tkjustify[] =
{
	"left",		Tkleft,
	"right",	Tkright,
	"center",	Tkcenter,
	nil
};

TkOption tkgeneric[] =
{
 "actx",		OPTact,		0,				(TkStab*)0,
 "acty",		OPTact,		0,				(TkStab*)1,
 "actwidth",		OPTdist,	offsetof(Tk, act.width),	(TkStab*)offsetof(Tk, env),
 "actheight",		OPTdist,	offsetof(Tk, act.height),	(TkStab*)offsetof(Tk, env),
 "bd",			OPTnndist,	offsetof(Tk, borderwidth),	nil,
 "borderwidth",		OPTnndist,	offsetof(Tk, borderwidth),	nil,
 "highlightthickness",	OPTnndist,	offsetof(Tk, highlightwidth), 	nil,
 "height",		OPTsize,	0,				(TkStab*)offsetof(Tk, env),
 "width",		OPTsize,	0,				(TkStab*)offsetof(Tk, env),
 "relief",		OPTstab, 	offsetof(Tk, relief),		tkrelief,
 "state",		OPTflag, 	offsetof(Tk, flag),		tkstate,
 "font",		OPTfont, 	offsetof(Tk, env),		nil,
 "foreground",		OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCforegnd,
 "background",		OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCbackgnd,
 "fg",			OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCforegnd,
 "bg",			OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCbackgnd,
 "selectcolor",		OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCselect,
 "selectforeground",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCselectfgnd,
 "selectbackground",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCselectbgnd,
 "activeforeground",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCactivefgnd,
 "activebackground",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCactivebgnd,
 "highlightcolor",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkChighlightfgnd,
 "disabledcolor",	OPTcolr, 	offsetof(Tk, env),		(TkStab*)TkCdisablefgnd,
 "padx",		OPTnndist, 	offsetof(Tk, pad.x),		nil,
 "pady",		OPTnndist, 	offsetof(Tk, pad.y),		nil,
 "takefocus",		OPTflag, 	offsetof(Tk, flag),		tktakefocus,
 nil
};

TkOption tktop[] =
{
	"x",		OPTdist,	offsetof(TkWin, req.x),		nil,
	"y",		OPTdist,	offsetof(TkWin, req.y),		nil,
	nil
};

TkOption tktopdbg[] =
{
	"debug",	OPTbool,	offsetof(TkTop, debug),	nil,
	nil
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

char TkNomem[]	= "!out of memory";
char TkBadop[]	= "!bad option";
char TkOparg[]	= "!arg requires option";
char TkBadvl[]	= "!bad value";
char TkBadwp[]	= "!bad window path";
char TkWpack[]	= "!window is already packed";
char TkNotop[]	= "!no toplevel";
char TkDupli[]  = "!window path already exists";
char TkNotpk[]	= "!window not packed";
char TkBadcm[]	= "!bad command";
char TkIstop[]	= "!can't pack top level";
char TkBadbm[]	= "!failed to load bitmap";
char TkBadft[]	= "!failed to open font";
char TkBadit[]	= "!bad item type";
char TkBadtg[]	= "!bad/no matching tag";
char TkFewpt[]	= "!wrong number of points";
char TkBadsq[]	= "!bad event sequence";
char TkBadix[]	= "!bad index";
char TkNotwm[]	= "!not a window";
char TkBadvr[]	= "!variable does not exist";
char TkNotvt[]	= "!variable is wrong type";
char TkMovfw[]	= "!too many events buffered";
char TkBadsl[]	= "!selection already exists";
char TkSyntx[]	= "!bad [] or {} syntax";
char TkRecur[] = "!cannot pack recursively";
char TkDepth[] = "!execution stack too big";
char TkNomaster[] = "!no master given";
char TkNotgrid[] = "!not a grid";
char TkIsgrid[] = "!cannot use pack inside a grid";
char TkBadgridcell[] = "!grid cell in use";
char TkBadspan[] = "!bad grid span";
char TkBadcursor[] = "!bad cursor image";
