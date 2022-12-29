#include "plat.h"

#include "Stream.h"
#include "Loaders.h"
#include "Player.h"

#include <cstdio>
#include <new>

std::shared_ptr<VTPlayerLib::ModSong> song;

Generator<VTPlayerLib::Fragment> player;
Generator<VTPlayerLib::Fragment>::iterator playerIt;
VTPlayerLib::Fragment playerFragment;
uint64_t playerFragmentLeft = 0;

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
    if (!song)
    {
        song = LoadUnknown(VTPlayerLib::Stream{ fopen("CHECKNOB.MOD", "rb") });
        if (!song)
        {
            return 0;
        }
        player = MixBufferEngine(song, 48000u);
        playerIt = player.begin();
    }

    if (playerFragmentLeft == 0)
    {
        if (playerIt == player.end())
        {
            return 0;
        }

        playerFragment = *playerIt;
        playerFragmentLeft = playerFragment->GetCount();
        playerIt++;
    }

    auto const fragmentPos = playerFragment->GetCount() - playerFragmentLeft;
    auto const count = std::min<uint64_t>(playerFragmentLeft, sizeInFloats);

    playerFragment->MixMono(std::span{ buffer, static_cast<size_t>(count) }, fragmentPos, true);

    playerFragmentLeft -= count;

    return static_cast<int>(count);
}
