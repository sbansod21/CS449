/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

// Rating: 1
int test_bitAnd(int x, int y)
{
  return x&y;
}
int test_bitXor(int x, int y)
{
  return x^y;
}
// Rating: 2
int test_getByte(int x, int n)
{
    unsigned char byte;
    switch(n) {
    case 0:
      byte = x;
      break;
    case 1:
      byte = x >> 8;
      break;
    case 2:
      byte = x >> 16;
      break;
    default:
      byte = x >> 24;
      break;
    }
    return (int) (unsigned) byte;
}
int test_sign(int x) {
    if ( !x ) return 0;
    return (x < 0) ? -1 : 1;
}
unsigned test_floatNegate(unsigned uf) {
    float f = u2f(uf);
    float nf = -f;
    if (isnan(f))
 return uf;
    else
 return f2u(nf);
}
int test_floatIsEqual(unsigned uf, unsigned ug) {
    float f = u2f(uf);
    float g = u2f(ug);
    return f == g;
}
// Rating: 3
int test_invert(int x, int p, int n)
{
    int mask = 0x1;
    int temp = x >> p;
    int toadd = 0;
    int i;
    for (i = 0; i < n; i++) {
        if (!(mask & temp)) {
            toadd += (1 << i);
        }
        temp = temp >> 1;
    }
    mask = (1 << n) - 1;
    mask = mask << p;
    toadd = toadd << p;
    return (x & ~mask) | toadd;
}
int test_addOK(int x, int y)
{
    long long lsum = (long long) x + y;
    return lsum == (int) lsum;
}
// Extra Credit Rating: 4
unsigned test_floatInt2Float(int x) {
  float f = (float) x;
  return f2u(f);
}
