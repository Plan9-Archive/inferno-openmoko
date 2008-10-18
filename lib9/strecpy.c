#include "lib9.h"

char*
strecpy(char *to, char *e, const char *from)
{
	if(to >= e)
		return to;
	to = _memccpy(to, from, '\0', e - to); /*strncpy?*/
	if(to == nil){
		to = e - 1;
		*to = '\0';
	}else{
		to--;
	}
	return to;
}
