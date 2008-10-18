#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include <kernel.h>

/*
 * these stubs are used when devsign isn't configured
 */

int
verifysigner(const char *sign, int len, const char *data, ulong ndata)
{
	USED(sign);
	USED(len);
	USED(data);
	USED(ndata);

	return 1;
}

int
mustbesigned(const char *path, const char *code, ulong length, const Dir *dir)
{
	USED(path);
	USED(code);
	USED(length);
	USED(dir);
	return 0;
}
