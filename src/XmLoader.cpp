
#include "stdafx.h"
#include "Loaders.h"
#include "Generator.h"

#include <cmath>
#include <array>
#include <span>
#include <map>

namespace VTPlayerLib
{

static Generator<ModSong::Instrument::PeriodSamplePair> ParseKeyMap(uint32_t firstSample, std::span<uint8_t> keyMap)
{
    uint8_t previousSample = UINT8_MAX;
    for (uint8_t i = 0; i < 96; ++i)
    {
        if (keyMap[i] != previousSample)
        {
            co_yield ModSong::Instrument::PeriodSamplePair
            {
                static_cast<uint8_t>(i + 1), // MinNote
                firstSample + keyMap[i],     // Sample
            };
            previousSample = keyMap[i];
        }
    }
}

std::shared_ptr<ModSong> LoadXM(Stream s)
{
    // assert(s.CanSeek);
    // assert(s.CanRead);

    auto song = std::make_shared<ModSong>();

    song->Marker = s.ReadString(17);

    if (song->Marker != L"Extended Module: ")
    {
        return {};
    }

    song->Title = s.ReadOEMString(20);
    s.ReadType<uint8_t>(); // 0x1A
    auto const trackerName = s.ReadOEMString(20);
    auto const versionNumber = s.ReadType<uint16_t>();

    auto const hdrPosition = s.Position();

    auto const hdrSize = s.ReadType<uint32_t>();
    auto const numPositions = s.ReadType<uint16_t>();
    auto const restartPosition = s.ReadType<uint16_t>();
    song->NumChannels = s.ReadType<uint16_t>();
    auto const numPatterns = s.ReadType<uint16_t>();
    auto const numInstruments = s.ReadType<uint16_t>();
    auto const flags = s.ReadType<uint16_t>();
    song->StartTicksPerDivision = s.ReadType<uint16_t>();
    song->StartTicksPerMinute = s.ReadType<uint16_t>() * 24;
    song->DoLinearToneSlides = (flags & 1) == 1;

    song->periodTargetLo = ModSong::NoteToPeriod(118);
    song->periodTargetHi = ModSong::NoteToPeriod(0);

    wprintf(L"Tracker:         %s\n", trackerName.c_str());
    wprintf(L"Version:         %X\n", versionNumber);
    wprintf(L"Title:           %s\n", song->Title.c_str());
    wprintf(L"NumPositions:    %u\n", numPositions);
    wprintf(L"RestartPosition: %u\n", restartPosition);
    wprintf(L"NumChannels:     %u\n", song->NumChannels);
    wprintf(L"NumPatterns:     %u\n", numPatterns);
    wprintf(L"NumInstruments:  %u\n", numInstruments);
    wprintf(L"Flags:           %X\n", flags);
    wprintf(L"Speed:           %u\n", song->StartTicksPerDivision);
    wprintf(L"BPM:             %u\n", song->StartTicksPerMinute);

    song->Positions.reserve(numPositions);
    for (uint32_t i = 0; i < numPositions; ++i)
    {
        auto const position = s.ReadType<uint8_t>();
        if (position == 255)
        {
            s.Seek(256 - i - 1, SeekOrigin::Current);
            break;
        }
        else if (position != 254)
        {
            song->Positions.push_back(position);
        }
    }

    s.Seek(hdrSize + hdrPosition, SeekOrigin::Begin);

    song->Patterns.reserve(numPatterns);
    for (int i = 0; i < numPatterns; ++i)
    {
        auto const patternPosition = s.Position();
        auto const patHdrSize = s.ReadType<uint32_t>();
        s.ReadType<uint8_t>();
        auto const numRows = s.ReadType<uint16_t>();
        auto const dataSize = s.ReadType<uint16_t>();

        ModSong::Pattern pat{};
        std::vector<ModSong::GlobalCommand> globalCommands{};
        std::vector<std::vector<ModSong::ChannelCommand>> channelCommands{};
        channelCommands.resize(song->NumChannels);

        pat.Length = numRows;
        pat.Lines.resize(pat.Length);

        s.Seek(patternPosition + patHdrSize, SeekOrigin::Begin);

        if (dataSize > 0)
        {
            for (uint32_t row = 0; row < numRows; ++row)
            {
                for (uint32_t channel = 0; channel < song->NumChannels; ++channel)
                {
                    uint8_t mask = s.ReadType<uint8_t>();
                    uint8_t note;
                    uint8_t effect;
                    uint8_t XY;

                    ModSong::GlobalCommand globalCommand{};
                    ModSong::ChannelCommand command{};

                    if ((mask & 0x80) == 0)
                    {
                        note = mask;
                        command.Instrument = s.ReadType<uint8_t>();
                        command.Volume = s.ReadType<uint8_t>();
                        effect = s.ReadType<uint8_t>();
                        XY = s.ReadType<uint8_t>();
                    }
                    else
                    {
                        if ((mask & 1) != 0)
                        {
                            note = s.ReadType<uint8_t>();
                        }
                        else
                        {
                            note = 0;
                        }
                        if ((mask & 2) != 0)
                        {
                            command.Instrument = s.ReadType<uint8_t>();
                        }
                        else
                        {
                            command.Instrument = 0;
                        }
                        if ((mask & 4) != 0)
                        {
                            command.Volume = s.ReadType<uint8_t>();
                        }
                        else
                        {
                            command.Volume = 0;
                        }
                        if ((mask & 8) != 0)
                        {
                            effect = s.ReadType<uint8_t>();
                        }
                        else
                        {
                            effect = 0;
                        }
                        if ((mask & 16) != 0)
                        {
                            XY = s.ReadType<uint8_t>();
                        }
                        else
                        {
                            XY = 0;
                        }
                    }

                    if (note > 96)
                    {
                        // Cut.
                        command.Note = UINT8_MAX;
                    }
                    else if (note != 0)
                    {
                        command.Note = note;
                    }

                    if (command.Volume >= 0x10 && command.Volume <= 0x50)
                    {
                        command.SetVolume = true;
                        command.Volume -= 0x10;
                    }
                    else
                    {
                        command.Volume = 0;
                    }

                    if (effect == 0xA && XY == 0)
                    {
                        command.Effect = ModSong::ChannelEffect::VolumeSlide;
                    }
                    else if (effect < 0x10)
                    {
                        if (!ParseModEffect(effect, XY, globalCommand, command))
                        {
                            static std::map<std::tuple<uint8_t, uint8_t>, uint32_t> s_unsupportedCommandsSeen;
                            auto& count = s_unsupportedCommandsSeen[std::make_tuple(effect, XY)];
                            ++count;
                            if (count == 1)
                            {
                                if (effect != 14)
                                {
                                    wprintf(L"Unsupported MOD command (%3d, %2d, %2d): %1X (%2X)\n", i, row, channel, effect, XY);
                                }
                                else
                                {
                                    wprintf(L"Unsupported MOD command (%3d, %2d, %2d): %1X%1X (%1X)\n", i, row, channel, effect, XY >> 4, XY & 15);
                                }
                            }
                        }
                    }

                    if (command.Effect != ModSong::ChannelEffect::None
                        || command.Instrument > 0
                        || command.Note > 0
                        || command.SetVolume)
                    {
                        command.Division = row;
                        channelCommands[channel].push_back(command);
                    }
                    if (globalCommand.Effect != ModSong::GlobalEffect::None)
                    {
                        globalCommand.Division = row;
                        globalCommands.push_back(globalCommand);
                    }
                }
            }
        }

        pat.GlobalCommands = std::move(globalCommands);
        pat.ChannelCommands = std::move(channelCommands);

        song->Patterns.push_back(pat);

        assert(static_cast<uint32_t>(s.Position()) == patternPosition + patHdrSize + dataSize);

        s.Seek(patternPosition + patHdrSize + dataSize, SeekOrigin::Begin);
    }

    std::wstring infoString;
    std::wstring sampleInfoString;

    std::array<uint8_t, 96> keyMap{};

    for (int instrumentIndex = 0; instrumentIndex < numInstruments; ++instrumentIndex)
    {
        auto const instrumentPosition = s.Position();

        if (instrumentPosition >= s.Length())
        {
            // No more instruments. MOD2XM does this (sigh!)
            break;
        }

        ModSong::Instrument instrument{};

        auto const instrumentHdrSize = s.ReadType<uint32_t>();
        instrument.Name = s.ReadOEMString(22);
        s.ReadType<uint8_t>();
        auto const numSamples = s.ReadType<uint16_t>();

        infoString += instrument.Name + L"\n";

        if (numSamples == 0)
        {
            wprintf(L"Instrument %3u: %s\n", instrumentIndex + 1, instrument.Name.c_str());

            song->Instruments.emplace_back();

            s.Seek(instrumentPosition + instrumentHdrSize, SeekOrigin::Begin);
        }
        else
        {
            wprintf(L"Instrument %3u: %-22s  Samples:%3u\n", instrumentIndex + 1, instrument.Name.c_str(), numSamples);

            auto const sampleHdrSize = s.ReadType<uint32_t>();

            auto firstSample = static_cast<uint32_t>(song->Samples.size());

            auto const instrument2Position = s.Position();

            keyMap = s.ReadType<std::array<uint8_t, 96>>();
            auto parsedKeyMap = ParseKeyMap(firstSample, keyMap);
            instrument.Samples.assign(parsedKeyMap.begin(), parsedKeyMap.end());

            s.Seek(instrument2Position + 192, SeekOrigin::Begin);

            auto const numVolumePoints = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const numPanningPoints = s.ReadType<uint8_t>();

            if (numVolumePoints > 0)
            {
                s.Seek(instrument2Position + 96, SeekOrigin::Begin);

                instrument.VolumeEnvelope.Points.resize(numVolumePoints);

                for (uint8_t i = 0; i < numVolumePoints; ++i)
                {
                    instrument.VolumeEnvelope.Points[i].tick = s.ReadType<uint16_t>();
                    instrument.VolumeEnvelope.Points[i].value = s.ReadType<uint16_t>() / 64.0;
                }
            }

            s.Seek(instrument2Position + 194, SeekOrigin::Begin);

            instrument.VolumeEnvelope.SustainPoint = s.ReadType<uint8_t>();
            instrument.VolumeEnvelope.LoopStartPoint = s.ReadType<uint8_t>();
            instrument.VolumeEnvelope.LoopEndPoint = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const panningSustainPoint = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const panningLoopStartPoint = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const panningLoopEndPoint = s.ReadType<uint8_t>();
            auto const volumeType = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const panningType = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const vibratoType = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const vibratoSweep = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const vibratoDepth = s.ReadType<uint8_t>();
            [[maybe_unused]] auto const vibratoRate = s.ReadType<uint8_t>();
            instrument.FadeOut = s.ReadType<uint16_t>() / -65536.0;

            if ((volumeType & 1) == 0)
            {
                //instrument.FadeOut = 0;
                instrument.VolumeEnvelope.Points.clear();
            }
            else if ((volumeType & 2) == 0)
            {
                instrument.VolumeEnvelope.SustainPoint = UINT32_MAX;
            }
            else if ((volumeType & 4) == 0)
            {
                instrument.VolumeEnvelope.LoopStartPoint = UINT32_MAX;
                instrument.VolumeEnvelope.LoopEndPoint = 0;
            }

            auto sampleDataPosition = instrumentPosition + instrumentHdrSize + numSamples * sampleHdrSize;

            for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
            {
                ModSong::Sample sample{};

                auto const samplePosition = instrumentPosition + instrumentHdrSize + sampleIndex * sampleHdrSize;

                s.Seek(samplePosition, SeekOrigin::Begin);

                auto sampleLength = s.ReadType<uint32_t>();
                auto sampleLoopStart = s.ReadType<uint32_t>();
                sample.LoopLength = s.ReadType<uint32_t>();
                sample.Volume = s.ReadType<uint8_t>();
                sample.FineTune = pow(2.0, -s.ReadType<int8_t>() / (128.0 * 12));
                auto const sampleType = s.ReadType<uint8_t>();
                [[maybe_unused]] auto const samplePanning = s.ReadType<uint8_t>();
                sample.Tune = pow(2.0, -s.ReadType<int8_t>() / 12.0);
                auto const sampleCompression = s.ReadType<uint8_t>();
                sample.Name = s.ReadOEMString(22);

                sampleInfoString += sample.Name + L"\n";

                if ((sampleType & 3) == 0)
                {
                    sample.LoopLength = 0;
                }

                if ((sampleType & 16) != 0)
                {
                    assert((sampleLength & 1) == 0);
                    sampleLength /= 2;
                    sampleLoopStart /= 2;
                    sample.LoopLength /= 2;
                }

                //wprintf(L"Sample %u: %s\n", firstSample + sampleIndex + 1, sample.Name.c_str());
                //wprintf(L"Sample Length     : %u\n", sampleLength);
                //wprintf(L"Sample LoopStart  : %u\n", sampleLoopStart);
                //wprintf(L"Sample LoopLength : %u\n", sample.LoopLength);
                //wprintf(L"Sample Volume     : %u\n", sample.Volume);
                //wprintf(L"Sample Finetune   : %f\n", sample.FineTune);
                //wprintf(L"Sample Type       : %X\n", sampleType);
                //wprintf(L"Sample Panning    : %u\n", samplePanning);
                //wprintf(L"Sample Relnote    : %f\n", sample.Tune);
                //wprintf(L"Sample Compression: %X\n", sampleCompression);

                if (sampleLength > 0)
                {
                    s.Seek(sampleDataPosition, SeekOrigin::Begin);

                    sampleDataPosition += sampleLength * ((sampleType & 16) == 0? 1: 2);

                    if (sample.LoopLength > 0)
                    {
                        if (sampleLoopStart + sample.LoopLength < sampleLength)
                        {
                            sampleLength = sampleLoopStart + sample.LoopLength;
                        }

                        sample.LoopLength = sampleLength - sampleLoopStart;
                    }

                    auto const sampleLengthWithPad = sampleLength + 1;


                    if (sampleCompression == 0xAD)
                    {
                        //throw new NotImplementedException("XM ADPCM samples not yet supported");
                        wprintf(L"XM ADPCM samples not yet supported\n");
                        return {};
                    }

                    sample.PCM = std::make_shared<std::vector<float>>();
                    sample.PCM->resize(sampleLengthWithPad);

                    if ((sampleType & 16) == 0)
                    {
                        // 8 bits per sample

                        int8_t acc = 0;
                        for (uint32_t i = 0; i < sampleLength; ++i)
                        {
                            acc += s.ReadType<int8_t>();
                            (*sample.PCM)[i] = acc / 128.0f;
                        }
                    }
                    else
                    {
                        // 16 bits per sample

                        int16_t acc = 0;
                        for (uint32_t i = 0; i < sampleLength; ++i)
                        {
                            acc += s.ReadType<int16_t>();
                            (*sample.PCM)[i] = acc / 32768.0f;
                        }
                    }

                    if (sample.LoopLength > 0)
                    {
                        (*sample.PCM)[sampleLength] = (*sample.PCM)[sampleLoopStart];
                    }
                    else
                    {
                        (*sample.PCM)[sampleLength] = (*sample.PCM)[sampleLength - 1];
                    }

                    song->Samples.push_back(std::move(sample));
                }
                else
                {
                    song->Samples.emplace_back();
                }
            }

            song->Instruments.push_back(instrument);

            s.Seek(sampleDataPosition, SeekOrigin::Begin);
        }
    }

    infoString += sampleInfoString;

    song->Info = infoString;

    return song;
}

}
// namespace VTPlayerLib
