#pragma once

#include "Song.h"
#include "SoundMixer.h"

#include "Generator.h"

namespace VTPlayerLib
{

struct ModPosition
{
    uint32_t position    = 0;
    uint32_t line        = 0;

    auto ToTuple() const noexcept { return std::tie(position, line); }
    friend bool operator<(ModPosition const& a, ModPosition const& b) noexcept { return a.ToTuple() < b.ToTuple(); }
};

struct ModPositionData
{
    ModPosition position;
    uint32_t    pattern     = 0;
    uint32_t    startTick   = 0;
    uint64_t    startSample = 0;
    uint32_t    numTicks    = 0;
    uint32_t    numSamples  = 0;

    friend bool operator<(ModPositionData const& a, ModPositionData const& b) noexcept { return a.position < b.position; }
};

struct ModFragment
{
    ModPositionData            position;
    std::shared_ptr<IFragment> fragment;
};

Generator<ModPositionData> ModPositionEnumerator(ModSong const& song, uint32_t sampleRate);
Generator<ModFragment> MixBufferEngine(std::shared_ptr<ModSong const> song, uint32_t sampleRate);

std::wstring RenderPosition(ModSong const& song, ModPositionData const& modPosition);

std::wstring GetNoteName(uint32_t const note);

}
// namespace VTPlayerLib
