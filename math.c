#include "math.h"

unsigned int s1 = 0x1234, s2 = 0x4567, s3 = 0x89AB;

int FIXED_sqrt(int x)
{
    unsigned int t, q, b, r;
    r = x;
    b = 0x400000;
    q = 0;
    while( b > 0x0 )
    {
        t = q + b;
        if( r >= t )
        {
            r -= t;
            q = t + b; // equivalent to q += 2*b
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 8;
    return (int)q;
}
unsigned int RNG() {
	s1 = (50000 * s1) % 715827817;
	s2 = (500001 * s2) % 715827821;
	s3 = (6100003 * s3) % 715827829;
	return s1 + s2 + s3;
}
void SetRNGSeed(int _s1, int _s2, int _s3) {
	s1 = _s1;
	s2 = _s2;
	s3 = _s3;
}