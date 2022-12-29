
#include "stdafx.h"
#include "Loaders.h"
#include "StringUtils.h"
#include "Player.h"

#include <map>

namespace VTPlayerLib
{

bool ParseExtendedModEffect(uint8_t X, uint8_t Y, ModSong::GlobalCommand& globalCommand, ModSong::ChannelCommand& command)
{
    switch (X)
    {
    case 0x0:
        return false;

    case 0x1:
        command.Effect = ModSong::ChannelEffect::FineToneSlide;
        command.SParam = -static_cast<int32_t>(Y) * 256;
        return false;

    case 0x2:
        command.Effect = ModSong::ChannelEffect::FineToneSlide;
        command.SParam = Y * 256;
        return false;

    case 0x3:
        return false;

    case 0x4:
        return false;

    case 0x5:
        command.Effect = ModSong::ChannelEffect::SetFinetune;
        if ((Y & 15) <= 7)
        {
            command.SParam = Y;
        }
        else
        {
            command.SParam = static_cast<int32_t>(Y) - 16;
        }
        return true;

    case 0x6:
        return false;

    case 0x7:
        return false;

    case 0x8:
        return false;

    case 0x9:
        return false;

    case 0xA:
        command.Effect = ModSong::ChannelEffect::FineVolumeSlide;
        command.SParam = Y;
        return true;

    case 0xB:
        command.Effect = ModSong::ChannelEffect::FineVolumeSlide;
        command.SParam = -static_cast<int32_t>(Y);
        return true;

    case 0xC:
        if (Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::DelayCut;
            command.UParam = Y;
        }
        return true;

    case 0xD:
        if (Y > 0)
        {
            command.Effect = ModSong::ChannelEffect::NoteDelay;
            command.UParam = Y;
        }
        return true;

    case 0xE:
        globalCommand.Effect = ModSong::GlobalEffect::PatternDelay;
        globalCommand.UParam = Y;
        return true;

    case 0xF:
        return false;
    }
    return true;
}

bool ParseModEffect(uint8_t effect, uint8_t XY, ModSong::GlobalCommand& globalCommand, ModSong::ChannelCommand& command)
{
    uint8_t X = (XY >> 4) & 15;
    uint8_t Y = XY & 15;

    switch (effect)
    {
    case 0x0:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::None;
        }
        else
        {
            // Arpeggio
            command.Effect = ModSong::ChannelEffect::Arpeggio;
            command.SParam = X;
            command.UParam = Y;
        }
        return true;

    case 0x1:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::None;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::ToneSlide;
            command.SParam = -static_cast<int32_t>(XY) * 256;
        }
        return true;

    case 0x2:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::None;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::ToneSlide;
            command.SParam = XY * 256;
        }
        return true;

    case 0x3:
        command.Effect = ModSong::ChannelEffect::TonePortamento;
        command.SParam = XY * 256;
        command.UParam = command.Note == UINT8_MAX ? uint8_t{0} : command.Note;
        command.Note = 0;
        return true;

    case 0x4:
        command.Effect = ModSong::ChannelEffect::Vibrato;
        command.SParam = Y * 2 * 256;
        command.UParam = X * 0x04000000;
        return true;

    case 0x5:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::TonePortamento;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::VolumeSlideWithPortamento;
            if (X > 0)
            {
                command.SParam = X;
            }
            else
            {
                command.SParam = -static_cast<int32_t>(Y);
            }
        }
        command.UParam = command.Note == UINT8_MAX ? (uint8_t)0 : command.Note;
        command.Note = 0;
        return true;

    case 0x6:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::Vibrato;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::VolumeSlideWithVibrato;
            if (X > 0)
            {
                command.SParam = X;
            }
            else
            {
                command.SParam = -static_cast<int32_t>(Y);
            }
        }
        return true;

    case 0x7:
        command.Effect = ModSong::ChannelEffect::Tremolo;
        command.SParam = Y * 2;
        command.UParam = X * 0x04000000;
        return true;

    case 0x8:
        return false;

    case 0x9:
        if (XY != 0)
        {
            command.Effect = ModSong::ChannelEffect::SampleOffset;
            command.UParam = XY * 0x00000100;
        }
        return true;

    case 0xA:
        if (XY == 0)
        {
            command.Effect = ModSong::ChannelEffect::None;
        }
        else
        {
            command.Effect = ModSong::ChannelEffect::VolumeSlide;
            if (X > 0)
            {
                command.SParam = X;
            }
            else
            {
                command.SParam = -static_cast<int32_t>(Y);
            }
        }
        return true;

    case 0xB:
        globalCommand.Effect = ModSong::GlobalEffect::Jump;
        globalCommand.UParam = XY;
        return true;

    case 0xC:
        command.SetVolume = true;
        command.Volume = XY;
        return true;

    case 0xD:
        globalCommand.Effect = ModSong::GlobalEffect::PatternBreak;
        globalCommand.UParam = X * 10 + Y;
        return true;

    case 0xE:
        return ParseExtendedModEffect(X, Y, globalCommand, command);

    case 0xF:
        if (XY > 48)
        {
            globalCommand.Effect = ModSong::GlobalEffect::SetTicksPerMinute;
            globalCommand.UParam = XY * 24;
            return true;
        }
        else if (XY != 0)
        {
            globalCommand.Effect = ModSong::GlobalEffect::SetTicksPerDivision;
            globalCommand.UParam = XY;
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

std::shared_ptr<ModSong> LoadMod(Stream s)
{
    // Debug.Assert(s.CanSeek);
    // Debug.Assert(s.CanRead);

    auto song = std::make_shared<ModSong>();

    s.Seek(20 + 30 * 31 + 130, SeekOrigin::Begin);
    song->Marker = s.ReadString(4);
    s.Seek(0, SeekOrigin::Begin);

    song->Title = s.ReadOEMString(20);

    wprintf(L"Marker: %s\n", song->Marker.c_str());
    wprintf(L"Name: %s\n", song->Title.c_str());

    uint16_t numSamples = 0;
    uint16_t numChannels = 0;
    if (song->Marker == L"M.K.")
    {
        numSamples = 31;
        numChannels = 4;
    }
    else if (song->Marker == L"M!K!")
    {
        numSamples = 31;
        numChannels = 4;
    }
    else if (song->Marker == L"FLT4")
    {
        numSamples = 31;
        numChannels = 4;
    }
    else if (song->Marker == L"FLT8")
    {
        numSamples = 31;
        numChannels = 8;
    }
    else if (song->Marker.length() > 1 && song->Marker.substr(1) == L"CHN")
    {
        if (song->Marker[0] >= L'0' && song->Marker[0] <= L'9')
        {
            numSamples = 31;
            numChannels = (int)(song->Marker[0] - L'0');
        }
    }
    else if (song->Marker.length() > 2 && song->Marker.substr(2) == L"CH")
    {
        if (song->Marker[0] >= L'0' && song->Marker[0] <= L'9' && song->Marker[1] >= L'0' && song->Marker[1] <= L'9')
        {
            numSamples = 31;
            numChannels = (int)(song->Marker[0] - L'0') * 10 + (int)(song->Marker[1] - L'0');
        }
    }

    if (numSamples == 0)
    {
        numSamples = 15;
        numChannels = 4;
    }

    song->NumChannels = (uint32_t)numChannels;

    // Skip sample headers - will do later.
    s.Seek(numSamples * 30, SeekOrigin::Current);

    uint32_t numPositions = s.ReadType<uint8_t>();
    uint32_t repeat = s.ReadType<uint8_t>();

    wprintf(L"Positions: %u\n", numPositions);
    wprintf(L"127 repeat: %u\n", repeat);

    uint32_t numPatterns = 0;

    //auto sb = new StringBuilder();
    //sb.Append("Positions: ");
    for (uint32_t i = 1; i <= 128; ++i)
    {
        auto dataPositiona = s.Position();
        uint32_t pat = s.ReadType<uint8_t>();
        auto dataPositionb = s.Position();
        // TODO: DEBUG if (dataPositionb != dataPositiona + 1) __debugbreak();
        if (numPatterns <= pat)
        {
            numPatterns = pat + 1;
        }
        if (i <= numPositions)
        {
            //sb.Append(" " + pat);

            song->Positions.push_back(pat);
        }
    }
    //Debug.WriteLine(sb);

    if (numSamples == 31)
    {
        s.Seek(4, SeekOrigin::Current); // Skip magic marker.
    }

    for (uint32_t i = 0; i < numPatterns; ++i)
    {
        ModSong::Pattern pat{};
        std::vector<ModSong::GlobalCommand> globalCommands;
        std::vector<std::vector<ModSong::ChannelCommand>> channelCommands;

        pat.Length = 64;
        pat.Lines.resize(pat.Length);

        channelCommands.resize(song->NumChannels);

        for (uint32_t division = 0; division < 64; ++division)
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                uint8_t p0 = s.ReadType<uint8_t>();
                uint8_t p1 = s.ReadType<uint8_t>();
                uint8_t p2 = s.ReadType<uint8_t>();
                uint8_t p3 = s.ReadType<uint8_t>();

                auto& line = pat.Lines[division];
                if (channel > 0)
                {
                    line += L" | ";
                }

                ModSong::ChannelCommand command{};
                ModSong::GlobalCommand globalCommand{};
                command.Instrument = (uint32_t)((p0 & 240) + ((p2 >> 4) & 15));

                uint32_t period = (p1 + ((uint32_t)(p0 & 15) << 8));

                if (period == 0)
                {
                    command.Note = 0;
                }
                else
                {
                    command.Note = (uint8_t)ModSong::PeriodToNote(period * 256);

                    //for (uint32_t n = 38; n < 66; ++n)
                    //{
                    //    Debug.WriteLine("{0} - {1:000.0}", n, ModSong.NoteToPeriod(n) / 256.0);
                    //}

                    assert(ModSong::PeriodToNote(ModSong::NoteToPeriod(command.Note)) == command.Note);
                }

                uint8_t effect = p2 & 15;
                uint8_t XY = p3;

                if (command.Note == 0)
                {
                    line += L"    ";
                }
                else
                {
                    line += GetNoteName(command.Note);
                    line += L" ";
                }
                if (command.Instrument == 0)
                {
                    line += L"   ";
                }
                else
                {
                    line += StdWStringPrintf(L"%2u ", command.Instrument).c_str();
                }
                if (effect == 0)
                {
                    line += L"    ";
                }
                else
                {
                    line += StdWStringPrintf(L"%1X %2X", effect, XY).c_str();
                }

                if (!ParseModEffect(effect, XY, globalCommand, command))
                {
                    static std::map<std::tuple<uint8_t, uint8_t>, uint32_t> s_unsupportedCommandsSeen;
                    auto& count = s_unsupportedCommandsSeen[std::make_tuple(effect, XY)];
                    ++count;
                    if (count == 1)
                    {
                        if (effect != 14)
                        {
                            wprintf(L"Unsupported MOD command (%3d, %2d, %2d): %1X (%2X)\n", i, division, channel, effect, XY);
                        }
                        else
                        {
                            wprintf(L"Unsupported MOD command (%3d, %2d, %2d): %1X%1X (%1X)\n", i, division, channel, effect, XY >> 4, XY & 15);
                        }
                    }
                }

                if (command.Effect != ModSong::ChannelEffect::None
                    || command.Instrument > 0
                    || command.Note > 0
                    || command.SetVolume)
                {
                    command.Division = division;
                    channelCommands[channel].push_back(std::move(command));
                }
                if (globalCommand.Effect != ModSong::GlobalEffect::None)
                {
                    globalCommand.Division = division;
                    globalCommands.push_back(std::move(globalCommand));
                }
            }
        }

        pat.GlobalCommands  = std::move(globalCommands);
        pat.ChannelCommands = std::move(channelCommands);

        song->Patterns.push_back(std::move(pat));
    }

    song->Instruments.clear();

    {
        std::wstring infoString;

        std::vector<uint8_t> buffer;
        buffer.resize(65536 * 2 + 44);

        for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            auto dataPosition = s.Position();
            s.Seek(30 * sampleIndex + 20, SeekOrigin::Begin);

            auto name = s.ReadOEMString(22);

            infoString.append(name);
            infoString.append(L"\n");

            uint32_t length = s.ReadTypeBE<uint16_t>() * 2;

            if (length > 4)
            {
                ModSong::Sample sample{};
                sample.Name = name;

                auto finetune = s.ReadType<uint8_t>();
                sample.Volume = s.ReadType<uint8_t>();
                auto loopStart = s.ReadTypeBE<uint16_t>() * 2;
                sample.LoopLength = s.ReadTypeBE<uint16_t>() * 2;
                sample.Tune = 1.0;

                if ((finetune & 15) <= 7)
                {
                    sample.FineTune = (float)pow(2, -(1 / 96.0) * ((int)finetune & 15));
                }
                else
                {
                    sample.FineTune = (float)pow(2, -(1 / 96.0) * (((int)finetune & 15) - 16));
                }

                wprintf(L"Instrument %2u: %-22s, Length:%6u, Finetune:%4.2f, Volume:%2u, Loop:%6u,%6u\n",
                    sampleIndex + 1,
                    sample.Name.c_str(),
                    length,
                    sample.FineTune,
                    sample.Volume,
                    loopStart,
                    sample.LoopLength
                );

                s.Seek(dataPosition, SeekOrigin::Begin);

                auto lengthUsed = length;

                if (sample.LoopLength > 2)
                {
                    if (loopStart + sample.LoopLength < length)
                    {
                        lengthUsed = loopStart + sample.LoopLength;
                    }
                    else
                    {
                        sample.LoopLength = length - loopStart;
                    }
                }
                else
                {
                    sample.LoopLength = 0;
                }

                sample.PCM = std::make_shared<std::vector<float>>();
                sample.PCM->resize(lengthUsed + 1);

                for (uint32_t i = 0; i < lengthUsed; ++i)
                {
                    auto b = s.ReadType<int8_t>();
                    (*sample.PCM)[i] = b / 128.0f;
                }

                if (sample.LoopLength > 0)
                {
                    (*sample.PCM)[lengthUsed] = (*sample.PCM)[loopStart];
                }
                else
                {
                    (*sample.PCM)[lengthUsed] = (*sample.PCM)[lengthUsed - 1];
                }

                song->Samples.push_back(std::move(sample));
            }
            else
            {
                song->Samples.push_back({});
            }

            s.Seek(dataPosition + length, SeekOrigin::Begin);
        }

        song->Info = std::move(infoString);
    }

    return song;
}

}
// namespace VTPlayerLib
