
#include "stdafx.h"
#include "Loaders.h"
#include "Player.h"

#include <array>
#include <format>
#include <map>

namespace VTPlayerLib
{

static wchar_t GetEffectChar(uint8_t const effect)
{
    if (effect == 0)
    {
        return L' ';
    }
    else if (effect > L'Z' - L'A')
    {
        return L'?';
    }
    else
    {
        return L'A' + (effect - 1);
    }
}

static bool ParseExtendedS3MEffect(uint8_t X, uint8_t Y, ModSong::GlobalCommand globalCommand, ModSong::ChannelCommand command)
{
    switch (X)
    {
    case 1:
        return ParseExtendedModEffect(3, Y, globalCommand, command);

    case 2:
        return ParseExtendedModEffect(5, Y, globalCommand, command);

    case 3:
        return ParseExtendedModEffect(4, Y, globalCommand, command);

    case 4:
        return ParseExtendedModEffect(7, Y, globalCommand, command);

    case 10: // Panic panning
        return false;

    case 12:
        return ParseExtendedModEffect(12, Y, globalCommand, command);

    case 13:
        return ParseExtendedModEffect(13, Y, globalCommand, command);

    case 14:
        return ParseExtendedModEffect(14, Y, globalCommand, command);

    default:
        return false;
    }
}

static bool ParseS3MEffect(uint8_t effect, uint8_t XY, ModSong::GlobalCommand& globalCommand, ModSong::ChannelCommand& command)
{
    uint8_t X = (XY >> 4) & 15;
    uint8_t Y = XY & 15;

    switch (effect)
    {
    case 0:
        return true;

    case 1: // 'A'
        if (XY != 0)
        {
            globalCommand.Effect = ModSong::GlobalEffect::SetTicksPerDivision;
            globalCommand.UParam = XY;
        }
        return true;

    case 2: // 'B'
        return ParseModEffect(11, XY, globalCommand, command);

    case 3: // 'C'
        return ParseModEffect(13, XY, globalCommand, command);

    case 4: // 'D'
        if (X == 15 && Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineVolumeSlide;
            command.SParam = -static_cast<int32_t>(Y) - 1;
        }
        else if (Y == 15 && X > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineVolumeSlide;
            command.SParam = X + 1;
        }
        else if (X > 0)
        {
            command.Effect = ModSong::ChannelEffect::VolumeSlide;
            command.SParam = X;
        }
        else if (Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::VolumeSlide;
            command.SParam = -static_cast<int32_t>(Y);
        }
        else // X == 0
        {
            command.Effect = ModSong::ChannelEffect::ContinueVolumeSlide;
        }
        return true;

    case 5: // 'E'
        if (X == 15 && Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineToneSlide;
            command.SParam = Y * 256;
        }
        else if (X == 14 && Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineToneSlide;
            command.SParam = Y * 64;
        }
        else if (XY > 0)
        {
            command.Effect = ModSong::ChannelEffect::ToneSlide;
            command.SParam = XY * 256;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::ContinueToneSlide;
            command.SParam = 1;
        }
        return true;

    case 6: // 'F'
        if (X == 15 && Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineToneSlide;
            command.SParam = -static_cast<int32_t>(Y) * 256;
        }
        else if (X == 14 && Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::FineToneSlide;
            command.SParam = -static_cast<int32_t>(Y) * 64;
        }
        else if (XY > 0)
        {
            command.Effect = ModSong::ChannelEffect::ToneSlide;
            command.SParam = -static_cast<int32_t>(XY) * 256;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::ContinueToneSlide;
            command.SParam = -1;
        }
        return true;

    case 7: // 'G'
        return ParseModEffect(3, XY, globalCommand, command);

    case 8: // 'H'
        return ParseModEffect(4, XY, globalCommand, command);

    case 10: // 'J'
        return ParseModEffect(0, XY, globalCommand, command);

    case 11: // 'K'
        command.Effect = ModSong::ChannelEffect::VolumeSlideWithVibrato;
        if (X > 0)
        {
            command.SParam = X;
        }
        else if (Y > 0)
        {
            command.SParam = -static_cast<int32_t>(Y);
        }
        else // X == 0
        {
            command.SParam = 0; // TODO: COntinue command?
        }
        return true;

    case 12: // 'L'
        command.Effect = ModSong::ChannelEffect::VolumeSlideWithPortamento;
        if (X > 0)
        {
            command.SParam = X;
        }
        else if (Y > 0)
        {
            command.SParam = -static_cast<int32_t>(Y);
        }
        else // X == 0
        {
            command.SParam = 0; // TODO: COntinue command?
        }
        command.UParam = command.Note == UINT8_MAX ? uint8_t{0} : command.Note;
        command.Note = 0;
        return true;

    case 15: // 'O'
        return ParseModEffect(9, XY, globalCommand, command);

    case 17: // 'Q'
    {
        command.UParam = Y;
        switch (X)
        {
        case 0:
            command.Effect = ModSong::ChannelEffect::Retrigger;
            break;

        case 1:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 1;
            break;

        case 2:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 2;
            break;

        case 3:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 4;
            break;

        case 4:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 8;
            break;

        case 5:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 16;
            break;

        case 6:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeMult;
            command.SParam = 65536 * 2 / 3;
            break;

        case 7:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeMult;
            command.SParam = 65536 * 1 / 2;
            break;

        case 8:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = 0;
            break;

        case 9:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = -1;
            break;

        case 10:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = -2;
            break;

        case 11:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = -4;
            break;

        case 12:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = -8;
            break;

        case 13:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeAdd;
            command.SParam = -16;
            break;

        case 14:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeMult;
            command.SParam = 65536 * 3 / 2;
            break;

        case 15:
            command.Effect = ModSong::ChannelEffect::RetriggerWithVolumeMult;
            command.SParam = 65536 * 2 / 1;
            break;
        }
    }
    return true;

    case 19: // 'S'
        return ParseExtendedS3MEffect(X, Y, globalCommand, command);

    case 20: // 'T'
        return ParseModEffect(0xF, XY, globalCommand, command);

    case 21: // 'U' Fine vibrato
        return false;

    case 22: // 'V' Set global volume
        return false;

    default:
        return false;   
    }
}

std::shared_ptr<ModSong> LoadS3M(IStream& s)
{
    // Debug.Assert(s.CanSeek);
    // Debug.Assert(s.CanRead);

    auto song = std::make_shared<ModSong>();

    s.Seek(44, SeekOrigin::Begin);
    song->Marker = s.ReadString(4);
    s.Seek(0, SeekOrigin::Begin);

    song->Title = s.ReadOEMString(28);

    //wprintf(L"Marker: %ls\n", song->Marker.c_str());
    //wprintf(L"Name: %ls\n", song->Title.c_str());

    if (song->Marker != L"SCRM")
    {
        return {};
    }

    s.ReadType<uint8_t>(); // 0x1a
    auto const type = s.ReadType<uint8_t>();
    //wprintf(L"Type: %u\n", type);
    s.Seek(2, SeekOrigin::Current);

    uint32_t const ordNum = s.ReadType<uint16_t>();
    uint32_t const insNum = s.ReadType<uint16_t>();
    uint32_t const patNum = s.ReadType<uint16_t>();
    uint32_t const flags = s.ReadType<uint16_t>();
    uint32_t const cwtv = s.ReadType<uint16_t>();
    uint32_t const ffv = s.ReadType<uint16_t>();

    //wprintf(L"ordNum: %4u\n", ordNum);
    //wprintf(L"insNum: %4u\n", insNum);
    //wprintf(L"patNum: %4u\n", patNum);
    //wprintf(L"flags:  %4X\n", flags);
    //wprintf(L"cwtv:   %4X\n", cwtv);
    //wprintf(L"ffv:    %4X\n", ffv);

    s.Seek(4, SeekOrigin::Current); // Marker

    [[maybe_unused]] uint32_t const globalVolume = s.ReadType<uint8_t>();
    uint32_t const initialSpeed = s.ReadType<uint8_t>();
    uint32_t const initialTempo = s.ReadType<uint8_t>();
    [[maybe_unused]] uint32_t const masterVolume = s.ReadType<uint8_t>();

    song->StartTicksPerDivision = initialSpeed;
    song->StartTicksPerMinute = initialTempo * 24;
    song->DoFirstTickOnVolumeSlide = (cwtv & 0xFFF) == 0x300 || (flags & 64) != 0;

    song->periodTargetLo = ModSong::NoteToPeriod(118);
    song->periodTargetHi = ModSong::NoteToPeriod(0);

    s.Seek(12, SeekOrigin::Current); // ??

    auto const channelSettings = s.ReadType<std::array<uint8_t, 32>>();

    song->Positions.reserve(ordNum);
    for (uint32_t i = 0; i < ordNum; ++i)
    {
        auto const position = s.ReadType<uint8_t>();
        if (position == 255)
        {
            s.Seek(ordNum - i - 1, SeekOrigin::Current);
            break;
        }
        else if (position < patNum)
        {
            song->Positions.push_back(position);
        }
    }

    std::vector<uint32_t> instrumentOffsets;
    instrumentOffsets.resize(insNum);
    for (uint32_t i = 0; i < insNum; ++i)
    {
        instrumentOffsets[i] = s.ReadType<uint16_t>() * 16;
    }

    std::vector<uint32_t> patternOffsets;
    patternOffsets.resize(patNum);
    for (uint32_t i = 0; i < patNum; ++i)
    {
        patternOffsets[i] = s.ReadType<uint16_t>() * 16;
    }

    song->NumChannels = 0;

    for (uint32_t ch = 0; ch < 32; ++ch)
    {
        auto const channelSetting = channelSettings[ch];
        if (channelSetting < 16)
        {
            song->NumChannels = ch + 1;
        }
    }

    //wprintf(L"NumChannels: %u\n", song->NumChannels);

    int patIndex = 0;
    for (auto offset : patternOffsets)
    {
        s.Seek(offset, SeekOrigin::Begin);

        [[maybe_unused]] auto const length = s.ReadType<uint16_t>();

        ModSong::Pattern pat{};
        std::vector<ModSong::GlobalCommand> globalCommands{};
        std::vector<std::vector<ModSong::ChannelCommand>> channelCommands{};
        channelCommands.resize(song->NumChannels);

        pat.Length = 64;
        pat.Lines.resize(pat.Length);

        for (uint32_t division = 0; division < 64; ++division)
        {
            std::wstring channelText[32];
            while (true)
            {
                auto channel = s.ReadType<uint8_t>();
                if (channel == 0)
                {
                    break;
                }

                auto& line = channelText[channel & 31];

                ModSong::GlobalCommand globalCommand{};
                ModSong::ChannelCommand command{};
                if ((channel & 32) != 0)
                {
                    auto note = s.ReadType<uint8_t>();
                    command.Instrument = s.ReadType<uint8_t>();
                    if (note == 254)
                    {
                        command.Note = UINT8_MAX;
                        line += L"--- ";
                    }
                    else if (note != 255)
                    {
                        uint8_t const octave = (note >> 4) & 15;
                        note = note & 15;
                        command.Note = static_cast<uint8_t>(note + octave * 12 + 1);

                        assert(ModSong::PeriodToNote(ModSong::NoteToPeriod(command.Note)) == command.Note);

                        line += GetNoteName(command.Note);
                        line += L" ";
                    }
                    else
                    {
                        line += L"    ";
                    }

                    if (command.Instrument == 0)
                    {
                        line += L"   ";
                    }
                    else
                    {
                        line += std::format(L"{:2} ", command.Instrument).c_str();
                    }
                }
                else
                {
                    line += L"       ";
                }
                if ((channel & 64) != 0)
                {
                    auto const volume = s.ReadType<uint8_t>();
                    if (volume != 255)
                    {
                        command.SetVolume = true;
                        command.Volume = volume;

                        line += std::format(L"{:2X} ", volume).c_str();
                    }
                    else
                    {
                        line += L"   ";
                    }
                }
                else
                {
                    line += L"   ";
                }
                if ((channel & 128) != 0)
                {
                    uint8_t const effect = s.ReadType<uint8_t>();
                    uint8_t const XY = s.ReadType<uint8_t>();

                    if (effect == 0)
                    {
                        line += L"    ";
                    }
                    else
                    {
                        line += std::format(L"{:1} {:2X}", GetEffectChar(effect), XY).c_str();
                    }

                    if (!ParseS3MEffect(effect, XY, globalCommand, command))
                    {
                        static std::map<std::tuple<uint8_t, uint8_t>, uint32_t> s_unsupportedCommandsSeen;
                        auto& count = s_unsupportedCommandsSeen[std::make_tuple(effect, XY)];
                        ++count;
                        if (count == 1)
                        {
                            wprintf(L"%3u %2u %2u - Unknown command: %u %1lc %02X\n", patIndex, division, channel & 31, effect, GetEffectChar(effect), XY);
                        }
                    }
                }
                else
                {
                    line += L"    ";
                }

                channel &= 31;

                if (channel < song->NumChannels)
                {
                    if (command.Effect != ModSong::ChannelEffect::None
                        || command.Instrument > 0
                        || command.Note > 0
                        || command.SetVolume)
                    {
                        command.Division = division;
                        channelCommands[channel].push_back(command);
                    }
                    if (globalCommand.Effect != ModSong::GlobalEffect::None)
                    {
                        globalCommand.Division = division;
                        globalCommands.push_back(globalCommand);
                    }
                }
            }

            auto& line = pat.Lines[division];
            for (uint32_t i = 0; i < song->NumChannels; ++i)
            {
                if (channelText[i].empty())
                {
                    line += L"              ";
                }
                else
                {
                    line += channelText[i];
                }
                line += L" | ";
            }
        }

        pat.GlobalCommands = std::move(globalCommands);
        pat.ChannelCommands = std::move(channelCommands);

        song->Patterns.push_back(std::move(pat));

        ++patIndex;
    }

    std::wstring infoString{};

    uint32_t sampleIndex = 0;
    for (auto const offset : instrumentOffsets)
    {
        s.Seek(offset, SeekOrigin::Begin);

        ModSong::Sample sample{};

        auto const T = s.ReadType<uint8_t>();
        if (T != 1)
        {
            s.Seek(47, SeekOrigin::Current);
            infoString += s.ReadOEMString(28) + L"\n";

            //wprintf(L"Instrument %2u: %-28ls (none)\n",
            //    static_cast<uint32_t>(song->Samples.size() + 1),
            //    sample.Name.c_str()
            //);
        }
        else
        {
            sample.Name = s.ReadOEMString(12);

            auto const soffset = s.ReadType<uint24_t>() / 16;
            auto       sampleLength = s.ReadType<uint32_t>();
            auto const sampleLoopStart = s.ReadType<uint32_t>();
            sample.LoopLength = s.ReadType<uint32_t>() - sampleLoopStart;
            sample.Volume = s.ReadType<uint8_t>();
            s.ReadType<uint8_t>();
            [[maybe_unused]] auto const P = s.ReadType<uint8_t>();
            auto const F = s.ReadType<uint8_t>();
            auto const C2spd = s.ReadType<uint32_t>();
            s.Seek(12, SeekOrigin::Current);

            if ((F & 1) == 0)
            {
                sample.LoopLength = 0;
            }

            sample.Tune = 8363.0 / C2spd;
            sample.FineTune = 1.0;

            auto const sampleInfo = s.ReadOEMString(28);
            infoString += sampleInfo + L"\n";
            auto const marker = s.ReadString(4);

            assert(marker == L"SCRS");

            if (sampleLength > 0)
            {
                if (sample.LoopLength > 0)
                {
                    if (sampleLoopStart + sample.LoopLength < sampleLength)
                    {
                        sampleLength = sampleLoopStart + sample.LoopLength;
                    }
                    else
                    {
                        sample.LoopLength = sampleLength - sampleLoopStart;
                    }
                }
                auto const sampleLengthWithPad = sampleLength + 1;

                //wprintf(L"Instrument %2u: %-28ls, Length:%6u, Finetune:%4.2f, Volume:%2u, Loop:%6u,%6u\n",
                //    static_cast<uint32_t>(song->Samples.size() + 1),
                //    sampleInfo.c_str(),
                //    sampleLength,
                //    sample.FineTune,
                //    sample.Volume,
                //    sampleLoopStart,
                //    sample.LoopLength
                //);

                s.Seek(soffset, SeekOrigin::Begin);
                sample.PCM = std::make_shared<std::vector<float>>();
                sample.PCM->resize(sampleLengthWithPad);

                if (ffv == 1)
                {
                    for (uint32_t i = 0; i < sampleLength; ++i)
                    {
                        auto const b = s.ReadType<int8_t>();
                        (*sample.PCM)[i] = b / 128.0f;
                    }
                }
                else
                {
                    for (uint32_t i = 0; i < sampleLength; ++i)
                    {
                        auto const b = s.ReadType<uint8_t>();
                        (*sample.PCM)[i] = b / 128.0f - 1;
                    }
                }

                if (sample.LoopLength > 0)
                {
                    (*sample.PCM)[sampleLength] = (*sample.PCM)[sampleLoopStart];
                }
                else
                {
                    (*sample.PCM)[sampleLength] = (*sample.PCM)[sampleLength-1];
                }
            }
        }

        song->Samples.push_back(std::move(sample));
    }

    song->Info = infoString;

    return song;
}

}
// namespace VTPlayerLib
