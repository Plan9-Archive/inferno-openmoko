#include "os.h"
#include <mp.h>
#include <libsec.h>

//
//  just use the libc prng to fill a buffer
//
void
prng(char *p, int n)
{
	char *e;

	for(e = p+n; p < e; p++)
		*p = rand();
}
