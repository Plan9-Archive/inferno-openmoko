#include <lib9.h>

int
_tas(int *la)
{
	int v;
#ifdef _MSC_VER
	_asm {
		mov eax, la
		mov ebx, 1
		xchg	ebx, [eax]
		mov	v, ebx
	}
#else
	asm("xchgl %0,%1" : "=r" (v), "=m" (*la) : "0"(1));
#endif
	return v;
}
