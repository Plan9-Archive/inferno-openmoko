/*
 * This implementation of the screen functions for X11 uses the
 * portable implementation of the Inferno drawing operations (libmemdraw)
 * to do the work, then has flushmemscreen copy the result to the X11 display.
 * Thus it potentially supports all colour depths but with a possible
 * performance penalty (although it tries to use the X11 shared memory extension
 * to copy the result to the screen, which might reduce the latter).
 *
 *       CraigN 
 */

#define _GNU_SOURCE 1
#include "dat.h"
#include "fns.h"
#include <draw.h>

#include <cursor.h>
#include <keyboard.h>

#include "framebuffer.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>


const int theDisplayDepth = 16;
static int theScreenIsInited = 0;
static unsigned char* theScreenData = 0;
static unsigned char* theFrameBuffer = 0;
extern ulong displaychan;

static Point thePointerPosition = { 0, 0 };
static Lock thePointerLock;
const  uchar thePointer[] 	= 
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const  int thePointerHeight = 16;
const  int thePointerWidth  = 10;
static int isPointerVisible = 1;
//extern Pointer mouse;
static void drawPointer ( int, int );
static void touchscreen_init(void);

uchar*
attachscreen(Rectangle *r, ulong *chan, int *d, int *width, int *softscreen)
{
    //fprint ( 2,"attachscreen %d %d\n", Xsize, Ysize);
    Xsize &= ~0x3;  /* ensure multiple of 4 */
    r->min.x = 0;
    r->min.y = 0;
    r->max.x = Xsize;
    r->max.y = Ysize;
    *d = theDisplayDepth;
    *width = (Xsize/4)*(*d/8);
    *softscreen = 1;
    *chan = displaychan = RGB16;

    if ( !theScreenIsInited ) {
        //initScreen ( Xsize, Ysize, &theDisplayChannel, &theDisplayDepth );
        if ( !framebuffer_init () )
        {
			touchscreen_init();
            theFrameBuffer = framebuffer_get_buffer();

            theScreenData = malloc ( Xsize * Ysize * ( theDisplayDepth / 8  ) );
            if ( !theScreenData )
                fprint ( 2, "cannot allocate screen buffer" );
        }
        theScreenIsInited = 1;
    }
    //fprint ( 2,"attachscreen ok theScreenData=%x *chan=%x *d=%x *width=%d *softscreen=%d\n", theScreenData, *chan, *d, *width, *softscreen);
    return theScreenData;
}

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

void
flushmemscreen(Rectangle r)
{
    //fprint ( 2,"flushmemscreen %d*%d...%d*%d\n", r.min.x, r.min.y, r.max.x, r.max.y);

	if ( !theFrameBuffer || !theScreenData )
    	return;
	int aDepth = theDisplayDepth / 8;
	int aBytesPerLine = Xsize * aDepth;
	int i;
	uchar* aFrameBuffer = theFrameBuffer;
	uchar* aScreenData = theScreenData;
	int aWidth;

	if ( r.min.x < 0 )
		r.min.x = 0;
	if ( r.min.y < 0 )
		r.min.y = 0;
	if ( r.max.x > Xsize )
		r.max.x = Xsize;
	if ( r.max.y > Ysize )
		r.max.y = Ysize;
  
	if ( ( r.max.x < r.min.x ) || ( r.max.y < r.min.y ) )
		return;

	aFrameBuffer += r.min.y * aBytesPerLine + r.min.x * aDepth;
	aScreenData += r.min.y * aBytesPerLine + r.min.x * aDepth;
	aWidth = ( r.max.x - r.min.x ) * aDepth;
	for ( i = r.min.y; i < r.max.y; i++ ) {
		memcpy ( aFrameBuffer, aScreenData, aWidth );
		aFrameBuffer += aBytesPerLine;
		aScreenData += aBytesPerLine;
	}

  	if ( isPointerVisible )
	{
  		if (!( max ( r.min.x, thePointerPosition.x ) > min (r.max.x, thePointerPosition.x + thePointerWidth ) ) ||
  			 ( max ( r.min.y, thePointerPosition.y ) > min (r.max.y, thePointerPosition.y + thePointerHeight ) ) ) 
//		if(	r.min.x < -thePointerWidth || 
//			r.min.y < thePointerHeight ||
		{
			if ( canlock ( &thePointerLock ) ) 
			{
				drawPointer ( thePointerPosition.x , thePointerPosition.y );
				unlock ( &thePointerLock );
	  		}
		}
	}
}


void
drawcursor(Drawcursor* c)
{
    //fprint ( 2,"drawcursor %x\n", c);
}

static void
_setpointer(int x, int y)
{
    //fprint ( 2,"_setpointer %d,%d\n", x,y);

	if ( !theFrameBuffer || !theScreenData )
    	return;

	if ( isPointerVisible )
	{
		int aDepth = theDisplayDepth / 8;
		int aBytesPerLine = Xsize * aDepth;
		int i;
		lock ( &thePointerLock );
		uchar* aFrameBuffer = theFrameBuffer + thePointerPosition.y * aBytesPerLine + thePointerPosition.x * aDepth;
		uchar* aScreenData = theScreenData + thePointerPosition.y * aBytesPerLine + thePointerPosition.x * aDepth;
		int aWidth = thePointerWidth * aDepth;
		int aHeight = ( ( thePointerPosition.y + thePointerHeight ) < Ysize ) ? thePointerHeight : Ysize - thePointerPosition.y;
		thePointerPosition.x = x;
		thePointerPosition.y = y;
		unlock ( &thePointerLock );
    
		for ( i = 0; i < aHeight; i++ ) 
		{
			memcpy ( aFrameBuffer, aScreenData, aWidth );
        	aFrameBuffer += aBytesPerLine;
        	aScreenData += aBytesPerLine;
		}
    
  	  	drawPointer ( thePointerPosition.x , thePointerPosition.y );
	}
}

void
setpointer(int x, int y)
{
//	if ( ( x != mouse.x ) || ( y != mouse.y ) ) 
//	{
//		Pointer aPointer;
//		aPointer.x = x;
//		aPointer.y = y;
//		aPointer.b = 0;
//		mouseproduce ( aPointer );
		mousetrack(0, x, y, 0);
//	}
	_setpointer(x, y);
}



static void drawPointer ( int x, int y )
{
  	uchar i,j;
  	uchar aDepth = theDisplayDepth / 8;
  	uchar* aStart = theFrameBuffer + y * Xsize * aDepth + x * aDepth;
  	int aWidth = ( ( x + thePointerWidth ) < Xsize ) ? thePointerWidth : Xsize - x;
  	int aHeight = ( ( y + thePointerHeight ) < Ysize ) ? thePointerHeight : Ysize - y;
  	for ( i = 0; i < aHeight; i++, aStart += Xsize * aDepth )
    	for ( j = 0; j < aWidth * aDepth; j++ ) 
      		aStart [ j ] &= thePointer [ i * aDepth * thePointerWidth + j ];
}

//KEY_RESERVED..KEY_COMPOSE
const Rune kbtab[] = 
{
[0x00]	No,		0x1b,	'1',	'2',	'3',	'4',	'5',	'6',
[0x08]	'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
[0x10]	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
[0x18]	'o',	'p',	'[',	']',	'\n',	LCtrl,	'a',	's',
[0x20]	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
[0x28]	'\'',	'`',	Shift,	'\\',	'z',	'x',	'c',	'v',
[0x30]	'b',	'n',	'm',	',',	'.',	'/',	Shift,	'*',
[0x38]	Latin,	' ',	Caps,	KF|1,	KF|2,	KF|3,	KF|4,	KF|5,
[0x40]	KF|6,	KF|7,	KF|8,	KF|9,	KF|10,	Num,	Scroll,	'7',
[0x48]	'8',	'9',	'-',	'4',	'5',	'6',	'+',	'1',
[0x50]	'2',	'3',	'0',	'.',	No,		No,		No,		KF|11,
[0x58]	KF|12,	No,		No,		No,		No,		No,		No,		No,
[0x60]	'\n',	RCtrl,	'/',	Print,	Latin,	No,		Home,	Up,
[0x68]	Pgup,	Left,	Right,	End,	Down,	Pgdown,	Ins,	Del,
[0x70]	No,		No,		No,		No,		No,		No,		No,		Break,
[0x78]	No,		No,		No,		No,		No,		No/*LWin*/,	No/*RWin*/,	No/*Menu*/,
};

const Rune kbtabshift[] =
{
[0x00]	No,		0x1b,	'!',	'@',	'#',	'$',	'%',	'^',
[0x08]	'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
[0x10]	'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
[0x18]	'O',	'P',	'{',	'}',	'\n',	LCtrl,	'A',	'S',
[0x20]	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
[0x28]	'"',	'~',	Shift,	'|',	'Z',	'X',	'C',	'V',
[0x30]	'B',	'N',	'M',	'<',	'>',	'?',	Shift,	'*',
[0x38]	Latin,	' ',	Caps,	KF|1,	KF|2,	KF|3,	KF|4,	KF|5,
[0x40]	KF|6,	KF|7,	KF|8,	KF|9,	KF|10,	Num,	Scroll,	'7',
[0x48]	'8',	'9',	'-',	'4',	'5',	'6',	'+',	'1',
[0x50]	'2',	'3',	'0',	'.',	No,		No,		No,		KF|11,
[0x58]	KF|12,	No,		No,		No,		No,		No,		No,		No,
[0x60]	'\n',	RCtrl,	'/',	Print,	Latin,	No,		Home,	Up,
[0x68]	Pgup,	Left,	Right,	End,	Down,	Pgdown,	Ins,	Del,
[0x70]	No,		No,		No,		No,		No,		No,		No,		Break,
[0x78]	No,		No,		No,		No,		No,		No/*LWin*/,	No/*RWin*/,	No/*Menu*/,
};


static int fds[10] = {0};
static int alt, caps, ctl, num, shift;

static void pointerProc( int* phandle )
{
	int dx=0, dy=0, ax=0, ay=0, button=0, wasabs=0;
	for (;;)
	{
		struct input_event ev;
    	int aCount = read ( *phandle, &ev, sizeof(struct input_event) );
		if ( aCount > 0 )
		{
			//fprint(2, "event(5, %d, %d, %d)\n", ev.type, ev.code, ev.value);
			switch(ev.type)
			{
			case EV_ABS:
				wasabs = 1;
				switch(ev.code)
				{
				case ABS_X: ax = ev.value; break;
				case ABS_Y: ay = ev.value; break;
				}
				break;

			case EV_REL:
				switch(ev.code)
				{
				case REL_X: dx = ev.value; break;
				case REL_Y: dy = ev.value; break;
				}
				break;

			case EV_KEY:
				{
					int value = ev.value ? 1 : 0;  /* 1==touch, 0==release */
					switch(ev.code)
					{
					case BTN_TOUCH: button = shift ? value<<1 : value; break;
					case BTN_LEFT:  button = (button & ~(1<<0)) | (value<<0); break;
					case BTN_RIGHT: button = (button & ~(1<<1)) | (value<<1); break;
					case BTN_MIDDLE:button = (button & ~(1<<2)) | (value<<2); break;

					case KEY_PHONE: /* AUX key, threat it as shift for a while */ 
						ev.code = KEY_LEFTSHIFT;
					default:
						if(0<=ev.code && ev.code<(sizeof(kbtab)/sizeof(*kbtab)))
						{
							Rune c = (shift?kbtabshift:kbtab)[ev.code];
						    if(caps && c<='z' && c>='a')
        						c += 'A' - 'a';
      						if(ev.value==0 /*keyup*/)
							{
        						switch(c)
								{
								case Latin: alt=0;	break;
        						case Shift: shift=0;break;
        						case LCtrl:
        						case RCtrl: ctl=0;	break;
        						}
      						}
							else if(ev.value==1)
							{
	                          	/*
	                           	*  normal character
	                           	*/
								if(c==No)
								{
	      						}
								else if(!(c & Spec))
								{
	        						if(ctl)
	          							c &= 0x1f;
							       	gkbdputc(gkbdq, c);
								} else switch(c)
								{
        						case Caps: caps ^= 1; break;
        						case Num:  num ^= 1;  break;
        						case Shift:shift = 1; break;
						    	case Latin:alt = 1;   break;
        						case LCtrl:
        						case RCtrl:ctl = 1;	  break;
        						case Del:  if(ctl&&alt) cleanexit(0); /*pass*/
								default:
									gkbdputc(gkbdq, c);
								}
							}
						}
						break;
					}
				}
				break;

			case EV_SYN:
				{
					int screenx, screeny;
					if(wasabs)
					{
						screenx = (ay-100)*480/800;
						screeny = (900-ax)*640/800;
						wasabs = 0;
					} else
					{
						screenx = thePointerPosition.x + dx;
						screeny = thePointerPosition.y + dy;
					}
					if ( screenx < 0 ) screenx = 0;
					dx = 0;
					dy = 0;
					if ( screeny < 0 ) screeny = 0;
					if ( screenx > Xsize ) screenx = Xsize;
					if ( screeny > Ysize ) screeny = Ysize;
					//fprint(2, "mousetrack(%d, %d, %d, 0 )\n", button, screenx, screeny);
					mousetrack(button, screenx, screeny, 0 );
					_setpointer(screenx, screeny);
				}
				break;
			}
		}
	}
}

static void touchscreen_init(void)
{
	int i;

	char sz[18] = "/dev/input/event?";
	for(i=0; i<sizeof(fds)/sizeof(*fds); i++)
	{
		if(i==0 /* Neo1973 Buttons 		*/
		|| i==1 /* s3c2410 TouchScreen 	*/
		/* i==2 /* lis302-1 (top) 		*/
		/* i==3 /* lis302-2 (bottom) 	*/
		/* i==4 /* GTA02 PMU events	 	*/
		|| i==5 /* usb keyboard or mouse*/
		|| i==6 /* usb keyboard or mouse*/
		)
		{
			sz[16]='0'+i;
			if ( ( fds[i] = open ( sz, O_RDONLY ) ) < 0 )
			{
				fprint ( 2, "can't open %s: %s\n", sz, strerror ( errno ) );
				//    	return;
		  	}
		}
	}
    
	for(i=0; i<sizeof(fds)/sizeof(*fds); i++)
	{
		if(fds[i]>0)
		{
			if ( kproc ( "eventProc", (void(*)(void*))pointerProc, fds+i, 0 ) < 0 ) 
			{
				fprint ( 2, "emu: can't start event procedure" );
				for(i=0; i<sizeof(fds); i++)
					if(fds[i]>0)
						close(fds[i]);
				return;
			}
		}
	}
}

