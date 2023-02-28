#include <rand.h>
static unsigned long next1 = 1;
int myrand(void)
{
    next1 = next1 * 1103515245 + 12345;
    return ((unsigned)(next1 / 65536) % 32768);
}

void mysrand(unsigned seed)
{
    next1 = seed;
}