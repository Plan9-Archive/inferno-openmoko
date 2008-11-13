#include <lib9.h>
#include <draw.h>

#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <tk.h>

#define TKRGB(R,G,B) ((R<<24)|(G<<16)|(B<<8)|(0xff))

enum
{
	tkBackR		= 0xdd,		/* Background base color */
	tkBackG 	= 0xdd,
	tkBackB 	= 0xdd,

	tkSelectR	= 0xb0,		/* Check box selected color */
	tkSelectG	= 0x30,
	tkSelectB	= 0x60,

	tkSelectbgndR	= 0x40,		/* Selected item background */
	tkSelectbgndG	= 0x40,
	tkSelectbgndB	= 0x40
};

typedef struct Coltab Coltab;
struct Coltab {
	int	c;
	ulong rgba;
	int shade;
};

static Coltab coltab[] =
{
	{TkCbackgnd,
		TKRGB(tkBackR, tkBackG, tkBackB),
		TkSameshade},
	{TkCbackgndlght,
		TKRGB(tkBackR, tkBackG, tkBackB),
		TkLightshade},
	{TkCbackgnddark,
		TKRGB(tkBackR, tkBackG, tkBackB),
		TkDarkshade},
	{TkCactivebgnd,
		TKRGB(tkBackR+0x10, tkBackG+0x10, tkBackB+0x10),
		TkSameshade},
	{TkCactivebgndlght,
		TKRGB(tkBackR+0x10, tkBackG+0x10, tkBackB+0x10),
		TkLightshade},
	{TkCactivebgnddark,
		TKRGB(tkBackR+0x10, tkBackG+0x10, tkBackB+0x10),
		TkDarkshade},
	{TkCactivefgnd,
		TKRGB(0, 0, 0),
		TkSameshade},
	{TkCforegnd,
		TKRGB(0, 0, 0),
		TkSameshade},
	{TkCselect,
		TKRGB(tkSelectR, tkSelectG, tkSelectB),
		TkSameshade},
	{TkCselectbgnd,
		TKRGB(tkSelectbgndR, tkSelectbgndG, tkSelectbgndB),
		TkSameshade},
	{TkCselectbgndlght,
		TKRGB(tkSelectbgndR, tkSelectbgndG, tkSelectbgndB),
		TkLightshade},
	{TkCselectbgnddark,
		TKRGB(tkSelectbgndR, tkSelectbgndG, tkSelectbgndB),
		TkDarkshade},
	{TkCselectfgnd,
		TKRGB(0xff, 0xff, 0xff),
		TkSameshade},
	{TkCdisablefgnd,
		TKRGB(0x88, 0x88, 0x88),
		TkSameshade},
	{TkChighlightfgnd,
		TKRGB(0, 0, 0),
		TkSameshade},
	{-1}
};
#undef TKRGB
void
tksetenvcolours(TkEnv *env)
{
	Coltab *c;

	c = &coltab[0];
	while(c->c != -1) {
		env->colors[c->c] = tkrgbashade(c->rgba, c->shade);
		env->set |= (1<<c->c);
		c++;
	}
}
