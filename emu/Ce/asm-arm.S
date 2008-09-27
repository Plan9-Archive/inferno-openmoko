		AREA	.text, CODE
		EXPORT	_tas

_tas		mov	r3, #1
		swp	r0, r3, [r0]
		mov	pc, lr

_getcallerpc	mov	r0, lr

		END