#pragma once

#include <cstdint>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

namespace VTPlayerLib
{

class ModSong final
{
public:
    enum class ChannelEffect : uint8_t
    {
        None = 0,

        // Channel tick commands

        VolumeSlide,
        ContinueVolumeSlide,
        Tremolo,
        Arpeggio,
        ToneSlide,
        ContinueToneSlide,
        TonePortamento,
        Vibrato,
        VolumeSlideWithPortamento,
        VolumeSlideWithVibrato,
        NoteDelay,
        Retrigger,
        RetriggerWithVolumeAdd,
        RetriggerWithVolumeMult,
        DelayCut,

        // Channel instant commands

        FineVolumeSlide = 64,
        FineToneSlide,
        SetFinetune,
        SampleOffset,
    };

    static constexpr wchar_t const* GetChannelEffectName(ChannelEffect effect) noexcept
    {
        switch (effect)
        {
        case ChannelEffect::None                     : return L"None";
        case ChannelEffect::VolumeSlide              : return L"VolumeSlide";
        case ChannelEffect::ContinueVolumeSlide      : return L"ContinueVolumeSlide";
        case ChannelEffect::Tremolo                  : return L"Tremolo";
        case ChannelEffect::Arpeggio                 : return L"Arpeggio";
        case ChannelEffect::ToneSlide                : return L"ToneSlide";
        case ChannelEffect::ContinueToneSlide        : return L"ContinueToneSlide";
        case ChannelEffect::TonePortamento           : return L"TonePortamento";
        case ChannelEffect::Vibrato                  : return L"Vibrato";
        case ChannelEffect::VolumeSlideWithPortamento: return L"VolumeSlideWithPortamento";
        case ChannelEffect::VolumeSlideWithVibrato   : return L"VolumeSlideWithVibrato";
        case ChannelEffect::NoteDelay                : return L"NoteDelay";
        case ChannelEffect::Retrigger                : return L"Retrigger";
        case ChannelEffect::RetriggerWithVolumeAdd   : return L"RetriggerWithVolumeAdd";
        case ChannelEffect::RetriggerWithVolumeMult  : return L"RetriggerWithVolumeMult";
        case ChannelEffect::DelayCut                 : return L"DelayCut";
        case ChannelEffect::FineVolumeSlide          : return L"FineVolumeSlide";
        case ChannelEffect::FineToneSlide            : return L"FineToneSlide";
        case ChannelEffect::SetFinetune              : return L"SetFinetune";
        case ChannelEffect::SampleOffset             : return L"SampleOffset";
        default                                      : return L"<unknown>";
        }
    }

    enum class GlobalEffect : uint8_t
    {
        None,

        SetTicksPerMinute,
        SetTicksPerDivision,
        PatternDelay,
        PatternBreak,
        Jump,
    };

    static constexpr wchar_t const* GetGlobalEffectName(GlobalEffect effect) noexcept
    {
        switch (effect)
        {
        case GlobalEffect::None               : return L"None";
        case GlobalEffect::SetTicksPerMinute  : return L"SetTicksPerMinute";
        case GlobalEffect::SetTicksPerDivision: return L"SetTicksPerDivision";
        case GlobalEffect::PatternDelay       : return L"PatternDelay";
        case GlobalEffect::PatternBreak       : return L"PatternBreak";
        case GlobalEffect::Jump               : return L"Jump";
        default                               : return L"<unknown>";
        }
    }

    struct Envelope
    {
        struct Point
        {
            uint32_t tick;
            double   value;
        };

        uint32_t SustainPoint;
        uint32_t LoopStartPoint;
        uint32_t LoopEndPoint;

        std::vector<Point> Points;
    };

    struct Instrument
    {
        struct PeriodSamplePair
        {
            uint8_t  MinNote;
            uint32_t Sample;

            friend bool operator==(PeriodSamplePair const a, PeriodSamplePair const b) { return a.MinNote == b.MinNote; }
            friend bool operator!=(PeriodSamplePair const a, PeriodSamplePair const b) { return a.MinNote != b.MinNote; }
            friend bool operator< (PeriodSamplePair const a, PeriodSamplePair const b) { return a.MinNote <  b.MinNote; }
        };

        std::wstring				  Name;
        std::vector<PeriodSamplePair> Samples;

        double FadeOut;

        Envelope VolumeEnvelope{};
    };

    struct Sample
    {
        std::wstring                        Name;
        std::shared_ptr<std::vector<float>> PCM;
        double                              FineTune;
        double                              Tune;
        uint32_t                            Volume;
        uint32_t                            LoopLength;
    };

    struct ChannelCommand
    {
        uint32_t Division;

        uint32_t Instrument;
        uint8_t  Note;

        bool     SetVolume;
        uint32_t Volume;

        ChannelEffect Effect;
        uint32_t      UParam;
        int32_t       SParam;
    };

    struct GlobalCommand
    {
        uint32_t Division;

        GlobalEffect Effect;
        uint32_t     UParam;
    };

    struct Pattern
    {
        uint32_t                                 Length;
        std::vector<GlobalCommand>               GlobalCommands;
        std::vector<std::vector<ChannelCommand>> ChannelCommands;
        std::vector<std::wstring>                Lines;
    };

    static int32_t PeriodToNote(uint32_t period)
    {
        assert(period > 0);
        auto d = (1712.0 * 1024) / period;
        return (int32_t)(log2(d) * 12.0 + 1.5);
    }

    static uint32_t NoteToPeriod(int32_t note)
    {
        auto d = pow(2, (note - 1) / 12.0);
        return (uint32_t)((1712.0 * 1024) / d + 0.5);
    }

    std::wstring Title;
    std::wstring Marker;

    std::wstring Info;

    uint32_t NumChannels = 0;

    uint32_t StartTicksPerDivision    = 6;
    uint32_t StartTicksPerMinute      = 3000;
    bool     DoFirstTickOnVolumeSlide = false;
    bool     DoLinearToneSlides       = false;

    uint32_t periodTargetLo = NoteToPeriod(96); //14 * 256; //27 * 256; //54 * 256;
    uint32_t periodTargetHi = NoteToPeriod(0); //3628 * 256; //1814 * 256;

    std::vector<Instrument> Instruments{};
    std::vector<Sample>     Samples    {};
    std::vector<uint32_t>   Positions  {};
    std::vector<Pattern>    Patterns   {};
};

}
// namespace VTPlayerLib
