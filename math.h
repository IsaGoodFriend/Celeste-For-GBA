#ifndef __ISA_MATH__
#define __ISA_MATH__

#define FIXED2BLOCK(n) ((n >> (ACC+BLOCK_SHIFT)))
#define BLOCK2FIXED(n) (n << (ACC+BLOCK_SHIFT))
#define INT2BLOCK(n) ((n >> BLOCK_SHIFT))
#define BLOCK2INT(n) (n << BLOCK_SHIFT)
#define FIXED2INT(n) ((n) >> ACC)
#define INT2FIXED(n) ((n) << ACC)

#define FIXED_DIV(a, b) (INT2FIXED(a) / b)
#define FIXED_MULT(a, b) (FIXED2INT(INT_ABS(a) * INT_ABS(b) * INT_SIGN(a * b)))

#define INT_ABS(n) (n * INT_SIGN(n))
#define INT_SIGN(n) (n == 0 ? 0 : (n>>31) | 1)
#define FIXED_LERP(a, b, t) (FIXED_MULT((b), t) + FIXED_MULT((a), 0x100 - t))
#define FIXED_APPROACH(a, b, m) (a > b ? SIGNED_MAX(a - m, b) : SIGNED_MIN(a + m, b))

#define SIGNED_MAX(a, b) (a > b ? a : b)
#define SIGNED_MIN(a, b) (a < b ? a : b)

#define COLOR_LERP(a, b, t) ((FIXED_LERP(a, b, t) & 0x7C00) | (FIXED_LERP(a & 0x03E0, b & 0x03E0, t) & 0x03E0) | (FIXED_LERP(a & 0x001F, b & 0x001F, t)) )

#define ACC 8
#define BLOCK_SHIFT 3

int FIXED_sqrt(int x);
unsigned int RNG();
#endif
