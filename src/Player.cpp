
#include "stdafx.h"

#include "Player.h"
#include "Channel.h"
#include "StringUtils.h"

#include <set>

namespace VTPlayerLib
{

Generator<ModPositionData> ModPositionEnumerator(ModSong const& song, uint32_t sampleRate)
{
    uint64_t samplesPerMinute = (uint64_t{sampleRate} << 32) * 60;

    uint64_t samplesPerTick = samplesPerMinute / song.StartTicksPerMinute;
    uint32_t ticksPerDivision = song.StartTicksPerDivision;

    uint32_t position = 0;
    uint64_t tickPosition = 0;
    uint32_t nextDivision = 0;
    uint32_t totalTicks = 0;
    uint64_t totalSamples = 0;

    std::set<ModPositionData> visited;

    while (position < song.Positions.size())
    {
        auto const  pattern     = song.Positions[position];
        auto const& patternData = song.Patterns [pattern];

        auto globalCommandsIt = std::lower_bound(
            patternData.GlobalCommands.begin(),
            patternData.GlobalCommands.end(),
            nextDivision,
            [](auto& a, uint32_t div){ return a.Division < div; }
        );

        uint32_t division = nextDivision;
        nextDivision = 0;
        uint32_t nextPosition = position + 1;

        bool endPattern = false;

        for (; division < patternData.Length && !endPattern; ++division)
        {
            uint32_t delayDivisions = 0;

            while (globalCommandsIt != patternData.GlobalCommands.end() && globalCommandsIt->Division == division)
            {
                auto const& command = *globalCommandsIt;
                switch (command.Effect)
                {
                case ModSong::GlobalEffect::SetTicksPerMinute:
                    samplesPerTick = samplesPerMinute / command.UParam;
                    break;

                case ModSong::GlobalEffect::SetTicksPerDivision:
                    ticksPerDivision = command.UParam;
                    break;

                case ModSong::GlobalEffect::PatternDelay:
                    delayDivisions = command.UParam;
                    break;

                case ModSong::GlobalEffect::PatternBreak:
                    endPattern = true; // Terminate pattern
                    nextDivision = command.UParam;
                    break;

                case ModSong::GlobalEffect::Jump:
                    endPattern = true; // Terminate pattern
                    nextPosition = command.UParam;
                    break;

                case ModSong::GlobalEffect::None:
                    break;
                }
                ++globalCommandsIt;
            }

            auto const ticks = ticksPerDivision * (delayDivisions + 1);

            auto const oldTickPosition = tickPosition;
            tickPosition += samplesPerTick * ticks;

            auto const sampleCount = static_cast<uint32_t>((tickPosition - oldTickPosition) >> 32);
            tickPosition -= uint64_t{sampleCount} << 32;

            ModPositionData const modPosition
            {
                .position {
                    .position    = position,
                    .line        = division,
                },
                .pattern     = pattern,
                .startTick   = totalTicks,
                .startSample = totalSamples,
                .numTicks    = ticks,
                .numSamples  = sampleCount,
            };

            if (visited.find(modPosition) != visited.end())
            {
                // Looping!
                co_return;
            }

            visited.insert(modPosition);

            co_yield modPosition;

            totalTicks   += ticks;
            totalSamples += sampleCount;
        }

        position = nextPosition;
    }
}

Generator<ModFragment> MixBufferEngine(std::shared_ptr<ModSong const> song, uint32_t sampleRate)
{
    double periodSpeed = (4.0 * 1024.0 * 1024.0 * 1024.0) * 7093789.2 * 256.0 / (sampleRate * 2);

    std::vector<ModChannel> channels;
    channels.reserve(song->NumChannels);

    for (uint32_t i = 0; i < song->NumChannels; ++i)
    {
        channels.emplace_back(song, i);
    }

    uint64_t tickPosition = 0;
    uint32_t currentPosition = UINT32_MAX;
    uint32_t currentLine = UINT32_MAX;

    for (auto&& modPosition : ModPositionEnumerator(*song, sampleRate))
    {
        if (modPosition.position.position != currentPosition || modPosition.position.line != currentLine + 1)
        {
            auto const& patternData = song->Patterns[modPosition.pattern];

            currentPosition = modPosition.position.position;
            currentLine = modPosition.position.line;

            for (uint32_t channel = 0; channel < song->NumChannels; ++channel)
            {
                auto const& commands = patternData.ChannelCommands[channel];
                auto const commandsBegin = std::lower_bound(commands.begin(), commands.end(), modPosition.position.line,
                    [](auto const& a, uint32_t line){ return a.Division < line; }
                );

                if (commands.end() != commandsBegin)
                {
                    channels[channel].commands = std::span<ModSong::ChannelCommand const>{ &*commandsBegin, static_cast<size_t>(commands.end() - commandsBegin) };
                }
                else
                {
                    channels[channel].commands = {};
                }
            }
        }

        auto const volumeMultiplier = 1.0 / (64.0 * std::min(song->NumChannels, 4u));

        auto const samplesPerTick = (uint64_t{modPosition.numSamples} << 32) / modPosition.numTicks;

        for (uint32_t tick = 0; tick < modPosition.numTicks; ++tick)
        {
            for (auto& channel : channels)
            {
                channel.Tick((uint32_t)modPosition.position.line, tick);
            }

            auto const oldTickPosition = tickPosition;
            tickPosition += samplesPerTick;

            auto const sampleCount = static_cast<uint32_t>((tickPosition - oldTickPosition) >> 32);
            tickPosition -= uint64_t{sampleCount} << 32;

            assert(sampleCount > 0);

            std::vector<Fragment> mixChannels;

            for (auto& channel : channels)
            {
                if (!channel.finished)
                {
                    if (auto fragment = channel.CopyForMixing(sampleCount, volumeMultiplier, periodSpeed))
                    {
                        mixChannels.push_back(std::move(fragment));
                    }
                }
            }

            if (mixChannels.empty())
            {
                co_yield { modPosition, MakeSilenceFragment(sampleCount) };
            }
            else if (mixChannels.size() == 1)
            {
                co_yield { modPosition, std::move(mixChannels[0]) };
            }
            else
            {
                co_yield { modPosition, MakeMixFragment(std::move(mixChannels), sampleCount) };
            }
        }

        // Adjust any remainder for perfect precision.
        tickPosition += (uint64_t{modPosition.numSamples} << 32) - samplesPerTick * modPosition.numTicks;
    }
}

//ITimeline<IFragment> RenderMod(ModSong song, int sampleRate)
//{
//    TimelineList<IFragment> timeline = new TimelineList<IFragment>(null, 16);
//
//    uint64_t sample = 0;
//
//    for (auto const fragment : MixBufferEngine(song, sampleRate))
//    {
//        timeline.Add(sample, fragment);
//
//        sample += fragment.Count;
//    }
//
//    timeline.Length = sample;
//
//    return timeline;
//}

Generator<std::pair<uint64_t, ModPositionData>> RenderModPosition(ModSong song, int sampleRate)
{
    uint64_t sample = 0;

    for (auto const modPosition : ModPositionEnumerator(song, sampleRate))
    {
        co_yield std::make_pair(sample, modPosition);

        sample += modPosition.numSamples;
    }
}

static constexpr wchar_t const* NoteList[12]
{
    L"C-", L"C#", L"D-", L"D#", L"E-", L"F-", L"F#", L"G-", L"G#", L"A-", L"A#", L"B-"
};

std::wstring GetNoteName(uint32_t const note)
{
    return StdWStringPrintf(L"%ls%1u", NoteList[note % 12], note / 12);
}

//static std::wstring RenderCommand(ModSong::ChannelCommand const& command)
//{
//    wchar_t buffer[32];
//
//    std::wstring result;
//
//    if (command.Instrument != 0)
//    {
//        _snwprintf_s(buffer, _TRUNCATE, L"%2u ", command.Instrument);
//        result += buffer;
//    }
//    else
//    {
//        result += L"   ";
//    }
//
//    if (command.Note != 0)
//    {
//        _snwprintf_s(buffer, _TRUNCATE, L"%ls%1u ", NoteList[command.Note % 12], command.Note / 12);
//        result += buffer;
//    }
//    else
//    {
//        result += L"     ";
//    }
//
//    if (command.SetVolume)
//    {
//        _snwprintf_s(buffer, _TRUNCATE, L"%2u ", command.Volume);
//        result += buffer;
//    }
//    else
//    {
//        result += L"   ";
//    }
//
//    _snwprintf_s(buffer, _TRUNCATE, L"%22s %3u %3d", ModSong::GetChannelEffectName(command.Effect), command.UParam, command.SParam);
//    result += buffer;
//}

std::wstring RenderPosition([[maybe_unused]] ModSong const& song, [[maybe_unused]] ModPositionData const& modPosition)
{
    wchar_t buffer[1024]{};

// TODO: std::format.
//    _snwprintf_s(buffer, _TRUNCATE, L"%03u %03u %02u %02u - %ls", modPosition.position, modPosition.pattern, modPosition.line, modPosition.numTicks, song.Patterns[modPosition.pattern].Lines[modPosition.line].c_str());

    return std::wstring{ buffer };
}

//static std::wstring RenderDivision(uint32_t position, uint32_t pattern, uint32_t division, std::span<ModSong::ChannelCommand> commands)
//{
//    wchar_t buffer[1024];
//
//    _snwprintf_s(buffer, _TRUNCATE, L"%03u %03u %02u - ", position, pattern, division);
//
//    std::wstring result{ buffer };
//
//    for (auto& command : commands)
//    {
//        result += RenderCommand(command) + L" | ";
//    }
//
//    return result;
//}

}
// namespace VTPlayerLib
