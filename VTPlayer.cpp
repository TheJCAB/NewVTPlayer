#include "plat.h"

#include <cstdio>
#include <new>


extern "C" EMSCRIPTEN_KEEPALIVE float* AllocateAudioBuffer(int sizeInFloats)
{
    return new float[sizeInFloats];
}

extern "C" EMSCRIPTEN_KEEPALIVE void FreeAudioBuffer(float* buffer)
{
    delete[] buffer;
}

extern "C" EMSCRIPTEN_KEEPALIVE int VTPlayerGetAudio(float* buffer, int sizeInFloats)
{
    for (int i = 0; i < sizeInFloats; ++i)
    {
        buffer[i] = (i & 15) / 7.5f - 1.0f;
    }
    return sizeInFloats;
}
