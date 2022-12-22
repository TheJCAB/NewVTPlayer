#include "plat.h"

#include <coroutine>

extern "C" EMSCRIPTEN_KEEPALIVE int add(int a, int b)
{
    return a + b;
}

extern "C" EMSCRIPTEN_KEEPALIVE int sub(int a, int b)
{
    return a - b;
}
