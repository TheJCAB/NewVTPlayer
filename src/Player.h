#pragma once

#include "Song.h"
#include "SoundMixer.h"

#include "Generator.h"

namespace VTPlayerLib
{

struct ModPosition
{
    uint32_t position;
    uint32_t pattern;
    uint32_t line;
    uint32_t numTicks;
    uint32_t numSamples;

    auto ToTuple() const noexcept { return std::tie(position, pattern, line, numTicks, numSamples); }
    friend bool operator<(ModPosition const& a, ModPosition const& b) noexcept { return a.ToTuple() < b.ToTuple(); }
};

Generator<ModPosition> ModPositionEnumerator(ModSong const& song, uint32_t sampleRate);
Generator<Fragment> MixBufferEngine(std::shared_ptr<ModSong const> song, uint32_t sampleRate);

std::wstring RenderPosition(ModSong const& song, ModPosition const& modPosition);

std::wstring GetNoteName(uint32_t const note);

}
// namespace VTPlayerLib
