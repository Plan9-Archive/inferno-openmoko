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
echo 'hwgc vmwarehwgc' > '#v/vgactl'
*/

enum {
	PCI_DEVICE_ID_VMWARE_SVGA2	= 0x0405,
	PCI_DEVICE_ID_VMWARE_SVGA	= 0x0710,
	PCI_VENDOR_ID_VMWARE		= 0x15AD,

	SVGA_LEGACY_BASE_PORT = 0x4560,

	SVGA_INDEX_PORT	= 0x0,
	SVGA_VALUE_PORT	= 0x1,
	SVGA_BIOS_POR	= 0x2,
	SVGA_NUM_PORTS	= 0x3,
};

/* constants from XWindows-vmware and L4 */
enum {
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

	SVGA_FIFO_MIN	      		= 0,
	SVGA_FIFO_MAX	      		= 1,
	SVGA_FIFO_NEXT_CMD   		= 2,
	SVGA_FIFO_STOP	      		= 3,
	SVGA_FIFO_USER	      		= 4,

	/*
	 *  Capabilities
	 */
	SVGA_CAP_NONE              = 0x0000,
	SVGA_CAP_RECT_FILL	   = 0x0001,
	SVGA_CAP_RECT_COPY	   = 0x0002,
	SVGA_CAP_RECT_PAT_FILL     = 0x0004,
	SVGA_CAP_LEGACY_OFFSCREEN  = 0x0008,
	SVGA_CAP_RASTER_OP	   = 0x0010,
	SVGA_CAP_CURSOR		   = 0x0020,
	SVGA_CAP_CURSOR_BYPASS	   = 0x0040,
	SVGA_CAP_CURSOR_BYPASS_2   = 0x0080,
	SVGA_CAP_8BIT_EMULATION    = 0x0100,
	SVGA_CAP_ALPHA_CURSOR      = 0x0200,
	SVGA_CAP_GLYPH             = 0x0400,
	SVGA_CAP_GLYPH_CLIPPING    = 0x0800,
	SVGA_CAP_OFFSCREEN_1       = 0x1000,
	SVGA_CAP_ALPHA_BLEND       = 0x2000,

	/*
	 *  Commands in the command FIFO
	 */
	SVGA_CMD_UPDATE = 1,
	 /* FIFO layout:
	    X, Y, Width, Height */

	SVGA_CMD_RECT_FILL = 2,
	 /* FIFO layout:
	    Color, X, Y, Width, Height */

	SVGA_CMD_RECT_COPY = 3,
	 /* FIFO layout:
	    Source X, Source Y, Dest X, Dest Y, Width, Height */

	SVGA_CMD_DEFINE_BITMAP = 4,
	 /* FIFO layout:
	    Pixmap ID, Width, Height, <scanlines> */

	SVGA_CMD_DEFINE_BITMAP_SCANLINE = 5,
	 /* FIFO layout:
	    Pixmap ID, Width, Height, Line #, scanline */

	SVGA_CMD_DEFINE_PIXMAP = 6,
	 /* FIFO layout:
	    Pixmap ID, Width, Height, Depth, <scanlines> */

	SVGA_CMD_DEFINE_PIXMAP_SCANLINE = 7,
	 /* FIFO layout:
	    Pixmap ID, Width, Height, Depth, Line #, scanline */

	SVGA_CMD_RECT_BITMAP_FILL = 8,
	 /* FIFO layout:
	    Bitmap ID, X, Y, Width, Height, Foreground, Background */

	SVGA_CMD_RECT_PIXMAP_FILL = 9,
	 /* FIFO layout:
	    Pixmap ID, X, Y, Width, Height */

	SVGA_CMD_RECT_BITMAP_COPY = 10,
	 /* FIFO layout:
	    Bitmap ID, Source X, Source Y, Dest X, Dest Y,
	    Width, Height, Foreground, Background */

	SVGA_CMD_RECT_PIXMAP_COPY = 11,
	 /* FIFO layout:
	    Pixmap ID, Source X, Source Y, Dest X, Dest Y, Width, Height */

	SVGA_CMD_FREE_OBJECT = 12,
	 /* FIFO layout:
	    Object (pixmap, bitmap, ...) ID */

	SVGA_CMD_RECT_ROP_FILL = 13,
         /* FIFO layout:
            Color, X, Y, Width, Height, ROP */

	SVGA_CMD_RECT_ROP_COPY = 14,
         /* FIFO layout:
            Source X, Source Y, Dest X, Dest Y, Width, Height, ROP */

	SVGA_CMD_RECT_ROP_BITMAP_FILL = 15,
         /* FIFO layout:
            ID, X, Y, Width, Height, Foreground, Background, ROP */

	SVGA_CMD_RECT_ROP_PIXMAP_FILL = 16,
         /* FIFO layout:
            ID, X, Y, Width, Height, ROP */

	SVGA_CMD_RECT_ROP_BITMAP_COPY = 17,
         /* FIFO layout:
            ID, Source X, Source Y,
            Dest X, Dest Y, Width, Height, Foreground, Background, ROP */

	SVGA_CMD_RECT_ROP_PIXMAP_COPY = 18,
         /* FIFO layout:
            ID, Source X, Source Y, Dest X, Dest Y, Width, Height, ROP */

	SVGA_CMD_DEFINE_CURSOR = 19,
	/* FIFO layout:
	   ID, Hotspot X, Hotspot Y, Width, Height,
	   Depth for AND mask, Depth for XOR mask,
	   <scanlines for AND mask>, <scanlines for XOR mask> */

	SVGA_CMD_DISPLAY_CURSOR = 20,
	/* FIFO layout:
	   ID, On/Off (1 or 0) */

	SVGA_CMD_MOVE_CURSOR = 21,
	/* FIFO layout:
	   X, Y */

	SVGA_CMD_MAX = 22,

	/* Legal values for the SVGA_REG_CURSOR_ON register in cursor bypass mode */
	SVGA_CURSOR_ON_HIDE = 0,
	SVGA_CURSOR_ON_SHOW = 1,
	SVGA_CURSOR_ON_REMOVE_FROM_FB = 2,
	SVGA_CURSOR_ON_RESTORE_TO_FB = 3,
};

//static int  	vm_may_be_busy;
static ulong	vm_id;
static ulong	vm_idx;
static ulong	vm_val;
static ulong	vm_caps;
static ulong	vm_mmiobase;
static ulong	vm_mmiosize;
static ulong*	vm_fifo;


static ulong
vmwareReadReg(int i)
{
	outl(vm_idx, i);
	return inl(vm_val);
}

static void
vmwareWriteReg(int i, ulong v)
{
	outl(vm_idx, i);
	outl(vm_val, v);
}

#define SVGA_MAGIC         0x900000
#define SVGA_MAKE_ID(ver)  (SVGA_MAGIC << 8 | (ver))

/* Version 2 let the address of the frame buffer be unsigned on Win32 */
#define SVGA_VERSION_2     2
#define SVGA_ID_2          SVGA_MAKE_ID(SVGA_VERSION_2)

/* Version 1 has new registers starting with SVGA_REG_CAPABILITIES so
   PALETTE_BASE has moved */
#define SVGA_VERSION_1     1
#define SVGA_ID_1          SVGA_MAKE_ID(SVGA_VERSION_1)

/* Version 0 is the initial version */
#define SVGA_VERSION_0     0
#define SVGA_ID_0          SVGA_MAKE_ID(SVGA_VERSION_0)

/* Invalid SVGA_ID_ */
#define SVGA_ID_INVALID    0xFFFFFFFF

/* it is really needed, otherwise SVGA_REG_CAPABILITIES and SVGA_REG_MEM_START will be 0 */
static unsigned
VMXGetVMwareSvgaId(void)
{
	ulong vmware_svga_id;

	vmwareWriteReg(SVGA_REG_ID, SVGA_ID_2);
	vmware_svga_id = vmwareReadReg(SVGA_REG_ID);
	print("vmware_svga_id=%lux\n", vmware_svga_id);
	if (vmware_svga_id == SVGA_ID_2)
		return SVGA_ID_2;

	vmwareWriteReg(SVGA_REG_ID, SVGA_ID_1);
	vmware_svga_id = vmwareReadReg(SVGA_REG_ID);
	print("vmware_svga_id=%lux\n", vmware_svga_id);
	if (vmware_svga_id == SVGA_ID_1)
		return SVGA_ID_1;

	if (vmware_svga_id == SVGA_ID_0)
		return SVGA_ID_0;

	/* No supported VMware SVGA devices found */
	return SVGA_ID_INVALID;
}


static void
vmwareWriteWordToFIFO(unsigned value)
{
  	/* Need to sync? */
	if ((vm_fifo[SVGA_FIFO_NEXT_CMD]+sizeof(unsigned)==vm_fifo[SVGA_FIFO_STOP])
	|| (vm_fifo[SVGA_FIFO_NEXT_CMD]==vm_fifo[SVGA_FIFO_MAX] -sizeof(unsigned) &&
		vm_fifo[SVGA_FIFO_STOP]==vm_fifo[SVGA_FIFO_MIN]))
	{
		vmwareWriteReg(SVGA_REG_SYNC, 1);
		while (vmwareReadReg(SVGA_REG_BUSY))
			;
	}
	vm_fifo[vm_fifo[SVGA_FIFO_NEXT_CMD] / sizeof(unsigned)] = value;

	value = vm_fifo[SVGA_FIFO_NEXT_CMD] + sizeof(unsigned);
	if (value == vm_fifo[SVGA_FIFO_MAX])
	{
		value = vm_fifo[SVGA_FIFO_MIN];
	}
	vm_fifo[SVGA_FIFO_NEXT_CMD] = value;
}

static void
vmwareWaitForFB(void)
{
	vmwareWriteReg(SVGA_REG_SYNC, 1);     /* Write to force synchronization */
	while (vmwareReadReg(SVGA_REG_BUSY))  /* Read to check if sync is done */
		;
}

static void
vmwareflush(VGAscr*, Rectangle r)
{
	if(vm_fifo == nil)
		return;

	vmwareWriteWordToFIFO(SVGA_CMD_UPDATE);
	vmwareWriteWordToFIFO(r.min.x);
	vmwareWriteWordToFIFO(r.min.y);
	vmwareWriteWordToFIFO(Dx(r));
	vmwareWriteWordToFIFO(Dy(r));

	vmwareWaitForFB();
}

static int
vmwarefill(VGAscr*, Rectangle r, ulong color)
{
	vmwareWriteWordToFIFO(SVGA_CMD_RECT_FILL);
	vmwareWriteWordToFIFO(color);
	vmwareWriteWordToFIFO(r.min.x);
	vmwareWriteWordToFIFO(r.min.y);
	vmwareWriteWordToFIFO(Dx(r));
	vmwareWriteWordToFIFO(Dy(r));

	vmwareWaitForFB();
	return 1;
}

static int
vmwarescroll(VGAscr*, Rectangle r, Rectangle sr)
{
	vmwareWriteWordToFIFO(SVGA_CMD_RECT_COPY);
	vmwareWriteWordToFIFO(sr.min.x);
	vmwareWriteWordToFIFO(sr.min.y);
	vmwareWriteWordToFIFO(r.min.x);
	vmwareWriteWordToFIFO(r.min.y);
	vmwareWriteWordToFIFO(Dx(r));
	vmwareWriteWordToFIFO(Dy(r));

	vmwareWaitForFB();
	return 1;
}


/* since fifo has been set up, we have to use SVGA_CMD_UPDATE
   if we want to see something on the screen */
static void
vmwareInitFIFO(void)
{
	vm_mmiobase = vmwareReadReg(SVGA_REG_MEM_START);
	vm_mmiosize = vmwareReadReg(SVGA_REG_MEM_SIZE) & ~3;
	/*print("vm_mmiobase: %lux\n", vm_mmiobase);
	print("vm_mmiosize: %lux\n", vm_mmiosize);*/
	//vm_fifo = (ulong*)upamalloc(vm_mmiobase, vm_mmiosize, 0);
	vm_fifo = KADDR(upamalloc(vm_mmiobase, vm_mmiosize, 0));
	/*print("vm_fifo: %lux\n", vm_fifo);*/

	vm_fifo[SVGA_FIFO_MIN]      = 4 * sizeof(unsigned);
	vm_fifo[SVGA_FIFO_MAX]      = vm_mmiosize;
	vm_fifo[SVGA_FIFO_NEXT_CMD] = 4 * sizeof(unsigned);
	vm_fifo[SVGA_FIFO_STOP]     = 4 * sizeof(unsigned);

  	vmwareWriteReg(SVGA_REG_CONFIG_DONE, 1);
}

/* made after nvidiaenable() */
static int
vmwareenable(VGAscr* scr)
{
	ulong aperture, size, offset;
	Pcidev *p;

	if(scr->aperture)
		upafree(scr->aperture, scr->apsize);

	scr->aperture = 0;
	scr->apsize = 0;
	scr->isupamem = 0;

	p = pcimatch(nil, PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA2);
	if(p == nil) {
		p = pcimatch(nil, PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA);
		if(p == nil) {
			print("no vmware card found\n");
			return 1;
		}
		vm_idx = SVGA_LEGACY_BASE_PORT + SVGA_INDEX_PORT*sizeof(unsigned);
		vm_val = SVGA_LEGACY_BASE_PORT + SVGA_VALUE_PORT*sizeof(unsigned);
	} else {
		vm_idx = p->mem[0].bar&~3;
		vm_val = vm_idx + SVGA_VALUE_PORT;
	}
	/*print("vm_idx: %lux\n", vm_idx);*/
	/*print("vm_val: %lux\n", vm_val);*/
	vm_id = VMXGetVMwareSvgaId();
	if(vm_id == SVGA_ID_INVALID || vm_id==SVGA_ID_0 ) {
		print("no supported vmware card found\n");
		return 1;
	}


	/* accel */
	vm_caps = vmwareReadReg(SVGA_REG_CAPABILITIES);
	print("vm_caps: %lux\n", vm_caps);
	if(vm_caps & SVGA_CAP_RECT_COPY)
		scr->scroll = vmwarescroll;
	if(vm_caps & SVGA_CAP_RECT_FILL)
		scr->fill = vmwarefill;

	/* fifo */
	vmwareInitFIFO();
	scr->mmio = vm_fifo; /* ??? */


	/* linear buffer */
	aperture = vmwareReadReg(SVGA_REG_FB_START);
	size = vmwareReadReg(SVGA_REG_FB_SIZE);
	offset = vmwareReadReg(SVGA_REG_FB_OFFSET);
	//print("aperture: %lux\n", aperture);
	//print("size: %lux\n", size);
	//print("offset: %lux\n", offset);
	if(aperture == 0 || size==0 || offset!=0) {
		print("vmware: get address error\n");
		return 1;
	}
	aperture = upamalloc(aperture, size, 0 /*align*/);
	if(!aperture) {
		print("vmware: upamalloc error\n");
		return 1;
	}
	addvgaseg("vmwarescreen", aperture, size);
	scr->isupamem = 1;
	scr->aperture = aperture;
	scr->apsize = size;


	/* default cursor */
	/*vmwarecurload(0, &arrow1);*/

	return 0;
}


static void
vmwaredisable(VGAscr* scr)
{
	vmwareWriteReg(SVGA_REG_ENABLE, 0);
	if(scr->aperture)
		upafree(scr->aperture, scr->apsize);
	if(vm_mmiobase)
		upafree(vm_mmiobase, vm_mmiosize);
}

static void
vmwarecurload(VGAscr*, Cursor* c)
{
	int i;
	ulong clr, set;
	ulong and[16];
	ulong xor[16];

	if(vm_fifo == nil)
		return;

	vmwareWriteWordToFIFO(SVGA_CMD_DEFINE_CURSOR);
	vmwareWriteWordToFIFO(1);	/* cursor id */
	vmwareWriteWordToFIFO(-c->offset.x);
	vmwareWriteWordToFIFO(-c->offset.y);


	vmwareWriteWordToFIFO(16);	/* width */
	vmwareWriteWordToFIFO(16);	/* height */
	vmwareWriteWordToFIFO(1);	/* depth for and mask */
	vmwareWriteWordToFIFO(1);	/* depth for xor mask */

	for(i=0; i<16; i++){
		clr = (c->clr[i*2+1]<<8) | c->clr[i*2];
		set = (c->set[i*2+1]<<8) | c->set[i*2];
		and[i] = ~(clr|set);	/* clr and set pixels => black */
		xor[i] = clr&~set;		/* clr pixels => white */
	}
	for(i=0; i<16; i++)
		vmwareWriteWordToFIFO(and[i]);
	for(i=0; i<16; i++)
		vmwareWriteWordToFIFO(xor[i]);

	vmwareWaitForFB();
}

static int
vmwarecurmove(VGAscr*, Point p)
{
	vmwareWriteReg(SVGA_REG_CURSOR_ID, 1);
	vmwareWriteReg(SVGA_REG_CURSOR_X, p.x);
	vmwareWriteReg(SVGA_REG_CURSOR_Y, p.y);
	vmwareWriteReg(SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_SHOW);

	return 0;
}

static void
vmwarecurdisable(VGAscr*)
{
	vmwareWriteReg(SVGA_REG_CURSOR_ID, 1);
	vmwareWriteReg(SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_HIDE);
}

static void
vmwarecurenable(VGAscr*)
{
	vmwareWriteReg(SVGA_REG_CURSOR_ID, 1);
	vmwareWriteReg(SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_SHOW);
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

	maxwidth = vmwareReadReg(SVGA_REG_MAX_WIDTH);
	maxheight = vmwareReadReg(SVGA_REG_MAX_HEIGHT);
	/*bpp = vmwareReadReg(vm, SVGA_REG_BITS_PER_PIXEL);*/

	if(x < 64) x = 64;
	if(y < 64) y = 64;
	if(x > maxwidth) x = maxwidth;
	if(y > maxheight) y = maxheight;

	vmwareWriteReg(SVGA_REG_ENABLE, 0);
	vmwareWriteReg(SVGA_REG_WIDTH, x);
	vmwareWriteReg(SVGA_REG_HEIGHT, y);
	switch(chan) {
	case RGB16:
		vmwareWriteReg(SVGA_REG_DEPTH, 16);
		vmwareWriteReg(SVGA_REG_RED_MASK, 0x0000F800);
		vmwareWriteReg(SVGA_REG_GREEN_MASK, 0x000007E0);
		vmwareWriteReg(SVGA_REG_BLUE_MASK, 0x0000001F);
		vmwareWriteReg(SVGA_REG_BITS_PER_PIXEL, 16);
		break;
	case XRGB32:
		vmwareWriteReg(SVGA_REG_DEPTH, 24);
		vmwareWriteReg(SVGA_REG_RED_MASK, 0x00FF0000);
		vmwareWriteReg(SVGA_REG_GREEN_MASK, 0x0000FF00);
		vmwareWriteReg(SVGA_REG_BLUE_MASK, 0x000000FF);
		vmwareWriteReg(SVGA_REG_BITS_PER_PIXEL, 24);
		break;
	}

	vmwareWriteReg(SVGA_REG_GUEST_ID, 0x5010);	/* OS type is "Other" */
	vmwareWriteReg(SVGA_REG_ENABLE, 1);

	return 0;
}

VGAdev vgavmwaredev = {
	"vmware",

	vmwareenable,
	vmwaredisable,
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
