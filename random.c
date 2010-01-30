#include "dm.h"

static struct {
    unsigned long a, b, c, d;
} CTX;

#define ROT(x, k)  ( ( (x)<<(k)) | ((x)>>(32-(k)) ) )

unsigned long
random_val()
{
    unsigned long e = CTX.a - ROT(CTX.b, 27);
    CTX.a = CTX.b ^ ROT(CTX.c, 17);
    CTX.b = CTX.c + CTX.d;
    CTX.c = CTX.d + e;
    CTX.d = e + CTX.a;
    return CTX.d;
}

static void
init(unsigned long seed)
{
    unsigned long i;
    CTX.a = 0xf1ea5eed;
    CTX.b = CTX.c = CTX.d = seed;
    for (i=0; i<20; ++i)
        random_val();
}

void
random_init()
{
    init(time(NULL));
}

/*
float Random::floatVal() {
  u4 v = val();
  return static_cast<float>(v)/ULONG_MAX;
}

int Random::intValMinMax(int min, int max) {
  int span = max - min + 1;
  u4 v = val() % span;
  return v + min;
}
*/
