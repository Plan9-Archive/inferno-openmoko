#include "lib9.h"


long
runestrlen(const Rune *s)
{
	int i;

	i = 0;
	while(*s++)
		i++;
	return i;
}
