#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"

#define	Image	IMAGE
#include <draw.h>
#include <memdraw.h>
#include <cursor.h>
#include "screen.h"

/*
HOWTO switch to graphic mode:
echo 'type vmware' > '#v/vgactl'
echo 'size 1024x768x32 x8r8g8b8' > '#v/vgactl'
echo 'drawinit' > '#v/vgactl'
*/

enum {
	PCIVMWARE	= 0x15AD,	/* PCI VID */

	VMWARE1		= 0x0710,	/* PCI DID */
	VMWARE2		= 0x0405,
};


enum {
	/* ID 0, 1 and 2 registers */
	SVGA_REG_ID			= 0,
	SVGA_REG_ENABLE			= 1,
	SVGA_REG_WIDTH			= 2,
	SVGA_REG_HEIGHT			= 3,
	SVGA_REG_MAX_WIDTH		= 4,
	SVGA_REG_MAX_HEIGHT		= 5,
	SVGA_REG_DEPTH			= 6,
	SVGA_REG_BITS_PER_PIXEL		= 7,	/* Current bpp in the guest */
	SVGA_REG_PSEUDOCOLOR		= 8,
	SVGA_REG_RED_MASK		= 9,
	SVGA_REG_GREEN_MASK		= 10,
	SVGA_REG_BLUE_MASK		= 11,
	SVGA_REG_BYTES_PER_LINE		= 12,
	SVGA_REG_FB_START		= 13,
	SVGA_REG_FB_OFFSET		= 14,
	SVGA_REG_VRAM_SIZE		= 15,
	SVGA_REG_FB_SIZE		= 16,
	/* ID 1 and 2 registers */
	SVGA_REG_CAPABILITIES		= 17,
	SVGA_REG_MEM_START		= 18,	/* Memory for command FIFO */
	SVGA_REG_MEM_SIZE		= 19,
	SVGA_REG_CONFIG_DONE		= 20,	/* Set when memory area configured */
	SVGA_REG_SYNC			= 21,	/* Write to force synchronization */
	SVGA_REG_BUSY			= 22,	/* Read to check if sync is done */
	SVGA_REG_GUEST_ID		= 23,	/* Set guest OS identifier */
	SVGA_REG_CURSOR_ID		= 24,	/* ID of cursor */
	SVGA_REG_CURSOR_X		= 25,	/* Set cursor X position */
	SVGA_REG_CURSOR_Y		= 26,	/* Set cursor Y position */
	SVGA_REG_CURSOR_ON		= 27,	/* Turn cursor on/off */

	SVGA_REG_HOST_BITS_PER_PIXEL	= 28,	/* Current bpp in the host */
	SVGA_REG_SCRATCH_SIZE		= 29,	/* Number of scratch registers */
	SVGA_REG_MEM_REGS		= 30,	/* Number of FIFO registers */
	SVGA_REG_NUM_DISPLAYS		= 31,	/* Number of guest displays */
	SVGA_REG_PITCHLOCK		= 32,	/* Fixed pitch for all modes */

	SVGA_PALETTE_BASE		= 1024,		/* Base of SVGA color map */
	SVGA_PALETTE_END		= SVGA_PALETTE_BASE + 767,
	SVGA_SCRATCH_BASE		= SVGA_PALETTE_BASE + 768,

	Crectfill = 1<<0,
	Crectcopy = 1<<1,
	Crectpatfill = 1<<2,
	Coffscreen = 1<<3,
	Crasterop = 1<<4,
	Ccursor = 1<<5,
	Ccursorbypass = 1<<6,
	Ccursorbypass2 = 1<<7,
	C8bitemulation = 1<<8,
	Calphacursor = 1<<9,

	FifoMin = 0,
	FifoMax = 1,
	FifoNextCmd = 2,
	FifoStop = 3,
	FifoUser = 4,

	Xupdate = 1,
	Xrectfill = 2,
	Xrectcopy = 3,
	Xdefinebitmap = 4,
	Xdefinebitmapscanline = 5,
	Xdefinepixmap = 6,
	Xdefinepixmapscanline = 7,
	Xrectbitmapfill = 8,
	Xrectpixmapfill = 9,
	Xrectbitmapcopy = 10,
	Xrectpixmapcopy = 11,
	Xfreeobject = 12,
	Xrectropfill = 13,
	Xrectropcopy = 14,
	Xrectropbitmapfill = 15,
	Xrectroppixmapfill = 16,
	Xrectropbitmapcopy = 17,
	Xrectroppixmapcopy = 18,
	Xdefinecursor = 19,
	Xdisplaycursor = 20,
	Xmovecursor = 21,
	Xdefinealphacursor = 22,
	Xcmdmax = 23,

	CursorOnHide = 0,
	CursorOnShow = 1,
	CursorOnRemoveFromFb = 2,
	CursorOnRestoreToFb = 3,
};
/*
static char*
rname[] = {
	[SVGA_REG_ID			] = "ID",
	[SVGA_REG_ENABLE		] = "Enable",
	[SVGA_REG_WIDTH			] = "Width",
	[SVGA_REG_HEIGHT		] = "Height",
	[SVGA_REG_MAX_WIDTH		] = "MaxWidth",
	[SVGA_REG_MAX_HEIGHT		] = "MaxHeight",
	[SVGA_REG_DEPTH			] = "Depth",
	[SVGA_REG_BITS_PER_PIXEL	] = "Bpp",
	[SVGA_REG_PSEUDOCOLOR		] = "PseudoColor",
	[SVGA_REG_RED_MASK		] = "RedMask",
	[SVGA_REG_GREEN_MASK		] = "GreenMask",
	[SVGA_REG_BLUE_MASK		] = "BlueMask",
	[SVGA_REG_BYTES_PER_LINE	] = "Bpl",
	[SVGA_REG_FB_START		] = "FbStart",
	[SVGA_REG_FB_OFFSET		] = "FbOffset",
	[SVGA_REG_VRAM_SIZE		] = "FbMaxSize",
	[SVGA_REG_FB_SIZE		] = "FbSize",
	[SVGA_REG_CAPABILITIES		] = "Cap",
	[SVGA_REG_MEM_START		] = "MemStart",
	[SVGA_REG_MEM_SIZE		] = "MemSize",
	[SVGA_REG_CONFIG_DONE		] = "ConfigDone",
	[SVGA_REG_SYNC			] = "Sync",
	[SVGA_REG_BUSY			] = "Busy",
	[SVGA_REG_GUEST_ID		] = "GuestID",
	[SVGA_REG_CURSOR_ID		] = "CursorID",
	[SVGA_REG_CURSOR_X		] = "CursorX",
	[SVGA_REG_CURSOR_Y		] = "CursorY",
	[SVGA_REG_CURSOR_ON		] = "CursorOn",
        [SVGA_REG_HOST_BITS_PER_PIXEL	] = "HostBpp",
        [SVGA_REG_SCRATCH_SIZE		] = "ScratchSize",
        [SVGA_REG_MEM_REGS		] = "MemRegs",
        [SVGA_REG_NUM_DISPLAYS		] = "NumDisplays",
        [SVGA_REG_PITCHLOCK		] = "PitchLock",
};*/

//typedef struct Vmware	Vmware;
//struct Vmware {
//	ulong	fb;

//	ulong	ra;
//	ulong	rd;

//	ulong	r[SVGA_NREG];
//	ulong	*mmio;
//	ulong	mmiosize;

//	char	chan[32];
//	int	depth;
//};

//Vmware xvm;
//Vmware *vm=&xvm;

static ulong	vmra;
static ulong	vmrd;

static ulong
vmread(int i)
{
	outl(vmra, i);
	return inl(vmrd);
}

static void
vmwrite(int i, ulong v)
{
	outl(vmra, i);
	outl(vmrd, v);
}
/*
static void
vmwait()
{
	vmwrite(SVGA_REG_SYNC, 1);  	/* Write to force synchronization *
	while(vmread(SVGA_REG_BUSY))	/* Read to check if sync is done *
		;
}
*/

/* made after nvidiaenable() */
static void
vmwareenable(VGAscr* scr)
{
	ulong aperture, size, offset;
	int ;
	Pcidev *p;

	scr->aperture = 0;
	scr->apsize = 0;

	p = pcimatch(nil, PCIVMWARE, 0);
	if(p == nil)
		error("no vmware card found");

	switch(p->did){
	default:
		errorf("unknown vmware id %.4ux", p->did);

	case VMWARE1:
		vmra = 0x4560;
		vmrd = 0x4560+4;
		break;

	case VMWARE2:
		vmra = p->mem[0].bar&~3;
		vmrd = vmra + 1;
	}

	aperture = (ulong)(vmread(SVGA_REG_FB_START));
	size = vmread(SVGA_REG_FB_SIZE);
	offset = vmread(SVGA_REG_FB_OFFSET);

	if(aperture == 0 || size==0 || offset!=0)
		error("vmware get address error");

	aperture = upamalloc(aperture, size, 0 /*align*/);

	addvgaseg("vmwarescreen", aperture, size);

	scr->isupamem = 0;
	scr->aperture = aperture;
	scr->apsize = size;
}
/*
static void
vmfifowr(Vmware *vm, ulong v)
{
	ulong *mm;

	mm = vm->mmio;
	if(mm == nil){
		iprint("!");
		return;
	}

	if(mm[FifoNextCmd]+sizeof(ulong) == mm[FifoStop]
	|| (mm[FifoNextCmd]+sizeof(ulong) == mm[FifoMax]
	    && mm[FifoStop] == mm[FifoMin]))
		vmwait(vm);

	mm[mm[FifoNextCmd]/sizeof(ulong)] = v;

	/* must do this way so mm[FifoNextCmd] is never mm[FifoMax] *
	v = mm[FifoNextCmd] + sizeof(ulong);
	if(v == mm[FifoMax])
		v = mm[FifoMin];
	mm[FifoNextCmd] = v;
}
*/
static void
vmwareflush(VGAscr*, Rectangle /*r*/)
{
//	if(vm->mmio == nil)
//		return;
//
//	vmfifowr(vm, Xupdate);
//	vmfifowr(vm, r.min.x);
//	vmfifowr(vm, r.min.y);
//	vmfifowr(vm, r.max.x-r.min.x);
//	vmfifowr(vm, r.max.y-r.min.y);
//	vmwait(vm);
}

static void
vmwarecurload(VGAscr*, Cursor* /*c*/)
{
//	int i;
//	ulong clr, set;
//	ulong and[16];
//	ulong xor[16];
//
//	if(vm->mmio == nil)
//		return;
//	vmfifowr(vm, Xdefinecursor);
//	vmfifowr(vm, 1);	/* cursor id */
//	vmfifowr(vm, -c->offset.x);
//	vmfifowr(vm, -c->offset.y);
//
//	vmfifowr(vm, 16);	/* width */
//	vmfifowr(vm, 16);	/* height */
//	vmfifowr(vm, 1);	/* depth for and mask */
//	vmfifowr(vm, 1);	/* depth for xor mask */
//
//	for(i=0; i<16; i++){
//		clr = (c->clr[i*2+1]<<8) | c->clr[i*2];
//		set = (c->set[i*2+1]<<8) | c->set[i*2];
//		and[i] = ~(clr|set);	/* clr and set pixels => black */
//		xor[i] = clr&~set;		/* clr pixels => white */
//	}
//	for(i=0; i<16; i++)
//		vmfifowr(vm, and[i]);
//	for(i=0; i<16; i++)
//		vmfifowr(vm, xor[i]);
//
//	vmwait(vm);
}

static int
vmwarecurmove(VGAscr*, Point p)
{
	vmwrite(SVGA_REG_CURSOR_ID, 1);
	vmwrite(SVGA_REG_CURSOR_X, p.x);
	vmwrite(SVGA_REG_CURSOR_Y, p.y);
	vmwrite(SVGA_REG_CURSOR_ON, CursorOnShow);
	return 0;
}

static void
vmwarecurdisable(VGAscr*)
{
	vmwrite(SVGA_REG_CURSOR_ID, 1);
	vmwrite(SVGA_REG_CURSOR_ON, CursorOnHide);
}

static void
vmwarecurenable(VGAscr*)
{
	vmwrite(SVGA_REG_CURSOR_ID, 1);
	vmwrite(SVGA_REG_CURSOR_ON, CursorOnShow);
}

static void
vmwareblank(int)
{
}

/*
static int
vmwarescroll(VGAscr*, Rectangle r, Rectangle sr)
{
	if(vm->mmio == nil)
		return 0;
	vmfifowr(Xrectcopy);
	vmfifowr(sr.min.x);
	vmfifowr(sr.min.y);
	vmfifowr(r.min.x);
	vmfifowr(r.min.y);
	vmfifowr(Dx(r));
	vmfifowr(Dy(r));
	vmwait(vm);
	return 1;
}

static int
vmwarefill(VGAscr*, Rectangle r, ulong sval)
{
	if(vm->mmio == nil)
		return 0;
	vmfifowr(Xrectfill);
	vmfifowr(sval);
	vmfifowr(r.min.x);
	vmfifowr(r.min.y);
	vmfifowr(r.max.x-r.min.x);
	vmfifowr(r.max.y-r.min.y);
	vmwait(vm);
	return 1;
}
*/
static void
vmwaredrawinit(VGAscr * /*scr*/)
{
/*
	ulong mmiobase, mmiosize;

	if(scr->mmio==nil){
		mmiobase = vmread(SVGA_REG_MEM_START);  /* Memory for command FIFO *
		if(mmiobase == 0)
			return;
		mmiosize = vmread(SVGA_REG_MEM_SIZE);
		scr->mmio = KADDR(upamalloc(mmiobase, mmiosize, 0));
		vm->mmio = scr->mmio;
		vm->mmiosize = mmiosize;
		if(scr->mmio == nil)
			return;
		addvgaseg("vmwaremmio", mmiobase, mmiosize);
	}

	scr->mmio[FifoMin] = 4*sizeof(ulong);
	scr->mmio[FifoMax] = vm->mmiosize;
	scr->mmio[FifoNextCmd] = 4*sizeof(ulong);
	scr->mmio[FifoStop] = 4*sizeof(ulong);
	vmwrite(SVGA_REG_CONFIG_DONE, 1);  	/* Set when memory area configured *

	scr->scroll = vmwarescroll;
	scr->fill = vmwarefill;
*/
}


int
vmwaresetmode(VGAscr* scr, int x, int y, int z, ulong chan)
{
	ulong maxwidth, maxheight;

	if(scr->aperture==0) /* not initialized or was initialization error */
	{
		return 1;
	}

	if(!((z==16&&chan==RGB16) || (z==32&&chan==XRGB32)))
	{
		return 1;
	}

	maxwidth = vmread(SVGA_REG_MAX_WIDTH);
	maxheight = vmread(SVGA_REG_MAX_HEIGHT);
	/*bpp = vmread(vm, SVGA_REG_BITS_PER_PIXEL);*/

	if(x < 64) x = 64;
	if(y < 64) y = 64;
	if(x > maxwidth) x = maxwidth;
	if(y > maxheight) y = maxheight;

	vmwrite(SVGA_REG_ENABLE, 0);
	vmwrite(SVGA_REG_WIDTH, x);
	vmwrite(SVGA_REG_HEIGHT, y);
	switch(chan) {
	case RGB16:
		vmwrite(SVGA_REG_DEPTH, 16);
		vmwrite(SVGA_REG_RED_MASK, 0x0000F800);
		vmwrite(SVGA_REG_GREEN_MASK, 0x000007E0);
		vmwrite(SVGA_REG_BLUE_MASK, 0x0000001F);
		vmwrite(SVGA_REG_BITS_PER_PIXEL, 16);
		break;
	case XRGB32:
		vmwrite(SVGA_REG_DEPTH, 24);
		vmwrite(SVGA_REG_RED_MASK, 0x00FF0000);
		vmwrite(SVGA_REG_GREEN_MASK, 0x0000FF00);
		vmwrite(SVGA_REG_BLUE_MASK, 0x000000FF);
		vmwrite(SVGA_REG_BITS_PER_PIXEL, 24);
		break;
	}
	vmwrite(SVGA_REG_GUEST_ID, 0x5010);	/* OS type is "Other" */
	vmwrite(SVGA_REG_ENABLE, 1);

	return 0;
}

/*
static void
load(Vga* vga, Ctlr *ctlr)
{
	char buf[64];
	int x;
	Vmware *vm;

	vm = vga->private;


	vmwrite(vm, SVGA_REG_WIDTH, vmr[Rwidth]);
	vmwrite(vm, SVGA_REG_HEIGHT, vmr[Rheight]);
	vmwrite(vm, SVGA_REG_ENABLE, 1);
	vmwrite(vm, SVGA_REG_GUEST_ID, 0x5010);	/* OS type is "Other" *

	x = vmread(SVGA_REG_BYTES_PER_LINE)/(vm->depth/8);
	if(x != vga->mode->x){
		vga->virtx = x;
		sprint(buf, "%ludx%ludx%d %s", vga->virtx, vga->virty,
			vga->mode->z, vga->mode->chan);
		vgactlw("size", buf);
	}
	ctlr->flag |= Fload;
}

static void
dump(/*Vga* vga, Ctlr* ctlr*)
{
	int i;
	Vmware *vm;

	vm = vga->private;

	for(i=0; i<Nreg; i++){
		printitem(ctlr->name, rname[i]);
		Bprint(&stdout, " %.8lux\n", vmr[i]);
	}

	printitem(ctlr->name, "chan");
	Bprint(&stdout, " %s\n", vm->chan);
	printitem(ctlr->name, "depth");
	Bprint(&stdout, " %d\n", vm->depth);
	printitem(ctlr->name, "linear");
}
*/
VGAdev vgavmwaredev = {
	"vmware",

	vmwareenable,
	0,
	0,
	0,//vmwarelinear,
	0,//vmwaredrawinit,
	0,
	0,
	0,
	vmwareflush,
	vmwaresetmode,
};

VGAcur vgavmwarecur = {
	"vmwarehwgc",

	vmwarecurenable,
	vmwarecurdisable,
	vmwarecurload,
	vmwarecurmove,
};
