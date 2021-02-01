#ifndef __ISA_MATH__
#define __ISA_MATH__

#define FIXED2BLOCK(n)			((n) >> BLOCK_FIXED)
#define BLOCK2FIXED(n)			((n) << BLOCK_FIXED)
#define INT2BLOCK(n)			((n) >> BLOCK_SHIFT)
#define BLOCK2INT(n)			((n) << BLOCK_SHIFT)
#define FIXED2INT(n)			((n) >> ACC)
#define INT2FIXED(n)			((n) << ACC)

#define FIXED_DIV(a, b)			(INT2FIXED(a) / (b))
#define FIXED_MULT(a, b)		(FIXED2INT((a) * (b)))

#define INT_ABS(n)				((n) * (((n)>>31) | 1))
#define INT_SIGN(n)				((n != 0) * (((n)>>31) | 1))
#define INT_SIGNED(n, s)		((n != 0) * (((n)>>31) | 1))
#define INT_LERP(a, b, t)		(((a * (0x100 - t)) + (b * t)) >> ACC)
#define FIXED_LERP(a, b, t)		(FIXED_MULT((b), t) + FIXED_MULT((a), 0x100 - t))
#define FIXED_APPROACH(a, b, m)	((a > b) * SIGNED_MAX(a - m, b) + (a <= b) * SIGNED_MIN(a + m, b))

#define SIGNED_MAX(a, b)		(b + (a - b) * (a > b))
#define SIGNED_MIN(a, b)		(b + (a - b) * (a < b))

#define SET_VAL_IF(og, val, b) 	((og) * !(b) | (val) * (b))
#define RESET_IF(val, b)		val &= ~(0xFFFFFFFF * (b))

#define BUFFER(val, max, b, rel) val = SET_VAL_IF(val, max, b);\
val &= ~(rel > 0);\
val -= val > 0

#define COLOR_LERP(a, b, t) ((INT_LERP(a, b, t) & 0x7C00) | (INT_LERP((a & 0x03E0), (b & 0x03E0), t) & 0x03E0) | (INT_LERP((a & 0x001F), (b & 0x001F), t)) )

#define ACC			8
#define BLOCK_SHIFT 3
#define BLOCK_FIXED	11

int FIXED_sqrt(int x);
unsigned int RNG();
#endif
