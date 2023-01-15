#include "plat.h"

#include "Stream.h"
#include "Loaders.h"
#include "Player.h"

#include <cstdio>
#include <new>

uint32_t                                  sampleRate;

std::shared_ptr<VTPlayerLib::ModSong>     song;
std::vector<VTPlayerLib::ModPositionData> songPositions;

struct SongData
{
    uint32_t SongTicks;
    uint32_t SongMilliseconds;
};

SongData songData{};

Generator<VTPlayerLib::ModFragment> player;
Generator<VTPlayerLib::ModFragment>::iterator playerIt;
VTPlayerLib::Fragment playerFragment;
VTPlayerLib::ModPositionData playerFragmentPos;
uint64_t playerFragmentLeft = 0;

struct GetAudioMetadata
{
    uint32_t position    = UINT32_MAX;
    uint32_t line        = UINT32_MAX;
    uint32_t pattern     = UINT32_MAX;
    uint32_t millisecond = UINT32_MAX;

    friend bool operator==(GetAudioMetadata, GetAudioMetadata) = default;
};

GetAudioMetadata playerCurrentMetadata;
uint64_t playerCurrentSample = 0;

extern "C" EMSCRIPTEN_KEEPALIVE float* AllocateAudioBuffer(int sizeInFloats)
{
    return new float[sizeInFloats];
}

extern "C" EMSCRIPTEN_KEEPALIVE void FreeAudioBuffer(float* buffer)
{
    delete[] buffer;
}

extern "C" EMSCRIPTEN_KEEPALIVE void VTPlayerLoadSongFromMemory(void const* buffer, int sizeInBytes, int sampleRate)
{
    // To save memory, get rid of the previous song before loading the next one.
    song               = {};
    player             = {};
    playerIt           = {};
    playerFragmentLeft = 0;

    ::sampleRate = sampleRate;

    VTPlayerLib::MemStream s{ std::span{ static_cast<std::byte const*>(buffer), static_cast<size_t>(sizeInBytes) } };
    song = LoadUnknown(s);
    if (!song)
    {
        return;
    }

    for (auto&& position : ModPositionEnumerator(*song, sampleRate))
    {
        songPositions.push_back(position);
    }
    if (songPositions.empty())
    {
        song = {};
        return;
    }

    auto const& lastPosition = songPositions.back();
    songData.SongTicks = lastPosition.startTick + lastPosition.numTicks;
    auto const songSamples = lastPosition.startSample + lastPosition.numSamples;
    songData.SongMilliseconds = static_cast<uint32_t>(songSamples * 1000.0 / sampleRate);

    player             = MixBufferEngine(song, static_cast<uint32_t>(sampleRate));
    playerIt           = player.begin();

    playerCurrentMetadata = GetAudioMetadata{};
    playerCurrentSample = 0;
}

extern "C" EMSCRIPTEN_KEEPALIVE SongData* VTPlayerGetSongData()
{
    return &songData;
}

extern "C" EMSCRIPTEN_KEEPALIVE int VTPlayerGetAudio(float* buffer, int sizeInFloats, GetAudioMetadata* pMetadata)
{
    if (!song)
    {
        return 0;
    }

    while (playerFragmentLeft == 0)
    {
        if (playerIt == player.end())
        {
            return 0;
        }

        playerFragment     = playerIt->fragment;
        playerFragmentPos  = playerIt->position;
        playerFragmentLeft = playerFragment->GetCount();
        playerCurrentMetadata.position    = playerFragmentPos.position.position;
        playerCurrentMetadata.line        = playerFragmentPos.position.line;
        playerCurrentMetadata.pattern     = playerFragmentPos.pattern;
        playerCurrentSample = playerFragmentPos.startSample;
        playerIt++;
    }

    *pMetadata = playerCurrentMetadata;
    pMetadata->millisecond = static_cast<uint32_t>(playerCurrentSample * 1000.0 / sampleRate);

    auto const fragmentPos = playerFragment->GetCount() - playerFragmentLeft;
    auto const count = std::min<uint64_t>(playerFragmentLeft, sizeInFloats);

    playerFragment->MixMono(std::span{ buffer, static_cast<size_t>(count) }, fragmentPos, true);

    playerCurrentSample += count;
    playerFragmentLeft  -= count;

    return static_cast<int>(count);
}
