#include	"os.h"
#include	<libsec.h>

/*
 *  use the X917 random number generator to create random
 *  numbers (faster than truerand() but not as random).
 */
ulong
fastrand(void)
{
	ulong x;

	genrandom((char*)&x, sizeof x);
	return x;
}
