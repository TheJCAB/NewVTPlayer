
#include "stdafx.h"

#include "Channel.h"
#include "SoundMixer.h"

#include <cassert>
#include <cmath>

namespace VTPlayerLib
{

ModChannel::ModChannel(std::shared_ptr<ModSong const> song, uint32_t index)
    : m_song{std::move(song)}
    , m_volumeSlide{
        tickAction,
        [this](){ SetNoEffect(); },
        [this](int32_t volumeIncrement){ return IncrementVolume(volumeIncrement); },
        m_song->DoFirstTickOnVolumeSlide,
    }
{
    panning = ((index & 1) ^ ((index & 2) >> 1)) == 0? 3: 0xC;

    period = m_song->periodTargetLo;
    if (m_song->DoLinearToneSlides)
    {
        toneSlideEngine = std::make_unique<LinearToneSlideEngine>();
    }
    else
    {
        toneSlideEngine = std::make_unique<AmigaToneSlideEngine>();
    }
}

void ModChannel::Tick(uint32_t div, uint32_t tick)
{
    if (tick == 0)
    {
        SetNoEffect();
        while (!commands.empty() && commands[0].Division == div)
        {
            InterpretCommand(commands[0]);
            commands = commands.subspan(1);
        }
    }
    else
    {
        if (!finished)
        {
            EngineTick(periodEngine, period);
            EngineTick(volumeEngine, m_volume);
            EngineTick(periodAddEngine, periodAdd);
            EngineTick(periodMultEngine, periodMult);
            EngineTick(volumeAddEngine, volumeAdd);
            EngineTick(volumeMultEngine, volumeMult);

            assert(period >= m_song->periodTargetLo);
            assert(period <= m_song->periodTargetHi);
        }

        if (tickAction != nullptr)
        {
            if (!tickAction(tick))
            {
                tickAction = nullptr;
            }
        }
    }

    if (!finished)
    {
        EngineTick(volumeEnvelopeEngine, volumeEnvelopeMult);

        if (noteCut && instrument != nullptr)
        {
            fadeOutVolume += instrument->FadeOut;
            if (fadeOutVolume <= 0)
            {
                finished = true;
            }
        }
    }
}

template < typename E, typename T >
void ModChannel::EngineTick(E*& engine, T& value)
// where E : class, IValueEngine<T>
{
    if (engine != nullptr)
    {
        if (!engine->Tick())
        {
            value = engine->GetValue();
            engine = nullptr;
        }
        else
        {
            value = engine->GetValue();
        }
    }
}

void ModChannel::InterpretCommand(ModSong::ChannelCommand command)
{
    bool noteTriggered = false;

    if (command.Effect != ModSong::ChannelEffect::NoteDelay)
    {
        noteTriggered = TriggerNote(command);

        if (command.SetVolume)
        {
            SetVolume(command.Volume);
        }
    }

    switch (command.Effect)
    {
    case ModSong::ChannelEffect::VolumeSlide:
        m_volumeSlide.SetupVolumeSlide(command.SParam);
        break;

    case ModSong::ChannelEffect::ContinueVolumeSlide:
		if (m_volumeSlide.IsFine())
		{
			SetNoEffect();
			m_volumeSlide.SetupFineVolumeSlide(m_volumeSlide.GetIncrement());
		}
		else
		{
			m_volumeSlide.SetupVolumeSlide(m_volumeSlide.GetIncrement());
		}
		break;

    case ModSong::ChannelEffect::Tremolo:
        SetupTremolo(command.UParam, static_cast<uint8_t>(command.SParam));
        break;

    case ModSong::ChannelEffect::Arpeggio:
        SetupArpeggio(command.SParam, command.UParam);
        break;

    case ModSong::ChannelEffect::ToneSlide:
        SetupToneSlide(command.SParam);
        break;

    case ModSong::ChannelEffect::ContinueToneSlide:
        SetupContinueToneSlide(command.SParam);
        break;

    case ModSong::ChannelEffect::TonePortamento:
        SetupPortamento(static_cast<uint8_t>(command.UParam), command.SParam);
        break;

    case ModSong::ChannelEffect::Vibrato:
        SetupVibrato(command.UParam, command.SParam);
        break;

    case ModSong::ChannelEffect::VolumeSlideWithPortamento:
        SetupVolumeSlideWithPortamento(static_cast<uint8_t>(command.UParam), command.SParam);
        break;

    case ModSong::ChannelEffect::VolumeSlideWithVibrato:
        SetupVolumeSlideWithVibrato(command.SParam);
        break;

    case ModSong::ChannelEffect::DelayCut:
        SetupDelayTickAction(command.UParam,
            [this]()
            {
                SetVolume(0);
            }
        );
        break;

    case ModSong::ChannelEffect::NoteDelay:
        SetupDelayTickAction(command.UParam,
            [this, command]()
            {
                TriggerNote(command);

                if (command.SetVolume)
                {
                    SetVolume(command.Volume);
                }
            }
        );
        break;

    case ModSong::ChannelEffect::Retrigger:
        SetupRetrigger(command.UParam);
        if (noteTriggered)
        {
            retriggerCurrent = retriggerValue;
        }
        else if (tickAction != nullptr)
        {
            tickAction(0);
        }
        break;

    case ModSong::ChannelEffect::RetriggerWithVolumeAdd:
        SetupRetriggerWithVolumeAdd(command.UParam, command.SParam);
        if (noteTriggered)
        {
            retriggerCurrent = retriggerValue;
        }
        else if (tickAction != nullptr)
        {
            tickAction(0);
        }
        break;

    case ModSong::ChannelEffect::RetriggerWithVolumeMult:
        SetupRetriggerWithVolumeMult(command.UParam, command.SParam);
        if (noteTriggered)
        {
            retriggerCurrent = retriggerValue;
        }
        else if (tickAction != nullptr)
        {
            tickAction(0);
        }
        break;


    case ModSong::ChannelEffect::FineVolumeSlide:
		SetNoEffect();
		m_volumeSlide.SetupFineVolumeSlide(command.SParam);
        break;

    case ModSong::ChannelEffect::FineToneSlide:
        SetupFineToneSlide(command.SParam);
        break;

    case ModSong::ChannelEffect::SetFinetune:
        SetFinetune(command.SParam);
        break;

    case ModSong::ChannelEffect::SampleOffset:
        SetNoEffect();
        if (command.UParam > 0)
        {
            sampleOffset = uint64_t{command.UParam} << 32;
        }
        m_position = sampleOffset;
        if (sample == nullptr || (m_position >> 32) >= sample->PCM->size())
        {
            finished = true;
        }
        break;


    default:
        SetNoEffect();
        break;
    }
}

bool ModChannel::TriggerNote(ModSong::ChannelCommand command)
{
    if (command.Note == UINT8_MAX)
    {
        if (volumeEnvelopeEngine != nullptr)
        {
            volumeEnvelopeEngine->Release();
        }
        noteCut = true;
        retriggerAction = nullptr;
        //nextInstrument = nullptr;
        //nextSample = nullptr;

        if (instrument == nullptr)
        {
            finished = true;
        }
        return true;
    }

    if (command.Instrument > 0)
    {
        if (!m_song->Instruments.empty())
        {
            nextInstrument = command.Instrument > m_song->Instruments.size() ? nullptr : &m_song->Instruments[command.Instrument - 1];
            if (nextInstrument != nullptr && nextInstrument->Samples.size() == 1)
            {
                nextSample = &m_song->Samples[nextInstrument->Samples[0].Sample];
            }
            else
            {
                nextSample = nullptr;
            }
        }
        else
        {
            nextSample = command.Instrument > m_song->Samples.size() ? nullptr : &m_song->Samples[command.Instrument - 1];
            m_volume = nextSample != nullptr ? nextSample->Volume : 0;
        }
    }

    if (command.Note != 0)
    {
        instrument = nextInstrument;
        sample = nextSample;

        if (instrument != nullptr)
        {
            if (sample == nullptr && !instrument->Samples.empty())
            {
                ModSong::Instrument::PeriodSamplePair const key{ command.Note, 0 };
                auto sampleIt = std::lower_bound(instrument->Samples.begin(), instrument->Samples.end(), key);
                if (sampleIt == instrument->Samples.end() || key < *sampleIt)
                {
                    if (sampleIt != instrument->Samples.begin())
                    {
                        --sampleIt;
                    }
                }
                auto const sampleIndex = sampleIt->Sample;
                sample = sampleIndex >= m_song->Samples.size() ? nullptr : &m_song->Samples[sampleIndex];
            }
        }

        if (sample != nullptr && sample->PCM && !sample->PCM->empty())
        {
            auto const sampleTune = sample->Tune;
            auto const sampleFineTune = sample->FineTune;

            noteCut = false;

            if (!m_song->Instruments.empty())
            {
                m_volume = sample != nullptr ? sample->Volume : 0;
                fadeOutVolume = 1;
            }

            retriggerAction =
                [this, command, sampleFineTune, sampleTune]()
                {
                    finetune = sampleFineTune;

                    period = (uint32_t)(ModSong::NoteToPeriod(command.Note) * sampleTune);
                    vibrato.Reset();

                    assert(period >= m_song->periodTargetLo);
                    assert(period <= m_song->periodTargetHi);

                    if (instrument != nullptr)
                    {
                        volumeEnvelopeEngine = &volumeEnvelope;
                        volumeEnvelopeEngine->Set(instrument->VolumeEnvelope, 1.0);
                    }

                    m_position = 0;
                    finished = false;
                };

            retriggerAction();
        }
        else
        {
            retriggerAction = nullptr;
            finished = true;
        }

        return true;
    }
    else
    {
        return false;
    }
}

#pragma region Delay note

void ModChannel::SetupDelayTickAction(uint32_t delay, std::function<void()> action)
{
    tickAction =
        [delay, action](uint32_t tick)
        {
            if (tick >= delay)
            {
                action();

                return false;
            }

            return true;
        };
}

#pragma endregion
#pragma region Retrigger

bool ModChannel::RetriggerTick(uint32_t)
{
    --retriggerCurrent;
    if (retriggerCurrent == 0)
    {
        if (retriggerAction != nullptr)
        {
            retriggerAction();
        }
        retriggerCurrent = retriggerValue;
        return true;
    }
    return false;
}

void ModChannel::SetupRetrigger(uint32_t delay)
{
    if (delay != 0)
    {
        retriggerValue = delay;
        lastRetriggerTick = [this](uint32_t tick){ return RetriggerTick(tick); };
    }
    tickAction = lastRetriggerTick;
}

void ModChannel::SetupRetriggerWithVolumeAdd(uint32_t delay, int increment)
{
    if (delay > 0)
    {
        retriggerValue = delay;
    }

    retrigVolumeMult = 0;
    retrigVolumeInc = increment;
    tickAction = lastRetriggerTick =
        [this](uint32_t tick)
        {
            auto const savedVolume = m_volume;

            if (RetriggerTick(tick))
            {
                m_volume = savedVolume;
                m_volume = (uint32_t)(savedVolume + retrigVolumeInc);
                if (m_volume > 64)
                {
                    m_volume = retrigVolumeInc > 0 ? (uint32_t)64 : 0;
                    tickAction = [this](uint32_t tick){ return RetriggerTick(tick); };
                }
            }

            return true;
        };
}

void ModChannel::SetupRetriggerWithVolumeMult(uint32_t delay, int increment)
{
    if (delay > 0)
    {
        retriggerValue = delay;
    }

    retrigVolumeMult = increment / 65536.0;
    retrigVolumeInc = 0;
    tickAction = lastRetriggerTick =
        [this](uint32_t tick)
        {
            auto const savedVolume = m_volume;

            if (RetriggerTick(tick))
            {
                m_volume = static_cast<uint32_t>(savedVolume * retrigVolumeMult);
                if (m_volume > 64)
                {
                    m_volume = retrigVolumeMult > 1 ? 64u : 0u;
                    tickAction = [this](uint32_t tick){ return RetriggerTick(tick); };
                }
            }

            return true;
        };
}

#pragma endregion
#pragma region One-shot effects

void ModChannel::SetNoEffect()
{
    periodEngine = nullptr;
    volumeEngine = nullptr;
    periodAddEngine = nullptr;
    periodMultEngine = nullptr;
    volumeAddEngine = nullptr;
    volumeMultEngine = nullptr;
    tickAction = nullptr;
    periodAdd = 0;
    periodMult = 1;
    volumeAdd = 0;
    volumeMult = 1;
}

void ModChannel::SetVolume(uint32_t volume)
{
    SetNoEffect();

    m_volume = volume;
}

void ModChannel::SetFinetune(int finetuneNum)
{
    SetNoEffect();

    finetune = pow(2, (-1 / 96.0) * finetuneNum);
}

#pragma endregion
#pragma region Tone slide

void ModChannel::SetupToneSlide(int increment)
{
    assert(increment != 0);

    periodSlideIsFine = false;
    periodEngine = toneSlideEngine.get();

    if (increment > 0)
    {
        toneSlideEngine->Set(period, increment, m_song->periodTargetHi);
    }
    else if (increment < 0)
    {
        toneSlideEngine->Set(period, increment, m_song->periodTargetLo);
    }
}

void ModChannel::SetupFineToneSlide(int increment)
{
    periodSlideIsFine = true;

    if (increment == 0)
    {
        toneSlideEngine->Set(period, increment, 0);
        return;
    }

    if (increment > 0)
    {
        toneSlideEngine->Set(period, increment, m_song->periodTargetHi);
    }
    else
    {
        toneSlideEngine->Set(period, increment, m_song->periodTargetLo);
    }

    toneSlideEngine->Tick();
    period = toneSlideEngine->GetValue();
    periodEngine = nullptr;
}

void ModChannel::SetupContinueToneSlide(int direction)
{
    int increment = toneSlideEngine->GetIncrement();

    if (direction * increment < 0)
    {
        increment = -increment;
    }

    if (periodSlideIsFine)
    {
        SetupFineToneSlide(increment);
    }
    else if (increment != 0)
    {
        SetupToneSlide(increment);
    }
}

#pragma endregion
#pragma region Tone portamento

void ModChannel::SetupPortamentoTarget(uint8_t target)
{
    if (target > 0)
    {
        portamentoTarget = (uint32_t)(ModSong::NoteToPeriod(target) * (sample != nullptr ? sample->Tune : 1.0));
    }
    else if (portamentoTarget == 0)
    {
        portamentoTarget = period;
    }
    if ((portamentoTarget > period && portamentoIncrement < 0)
        || (portamentoTarget < period && portamentoIncrement > 0))
    {
        portamentoIncrement = -portamentoIncrement;
    }
}

void ModChannel::SetupPortamento(uint8_t target, int increment)
{
    if (increment != 0)
    {
        portamentoIncrement = increment;
    }
    SetupPortamentoTarget(target);
    tickAction = [this](uint32_t){ return PortamentoTick(); };
}

void ModChannel::SetupVolumeSlideWithPortamento(uint8_t target, int increment)
{
    SetupPortamentoTarget(target);
    m_volumeSlide.SetupVolumeSlideValues(increment);
    if (m_song->DoFirstTickOnVolumeSlide)
    {
        m_volumeSlide.VolumeSlideTick();
    }
    tickAction = //m_volumeSlide.SetupVolumeSlideTick(
        [this](uint32_t){ return VolumeSlideWithPortamentoTick(); }
        //,
        //nullptr
    ;
}

bool ModChannel::PortamentoTick()
{
    if (portamentoIncrement > 0 && (uint32_t)portamentoIncrement > portamentoTarget - period)
    {
        period = portamentoTarget;
        return false;
    }
    else if (portamentoIncrement < 0 && (uint32_t)-portamentoIncrement > period - portamentoTarget)
    {
        period = portamentoTarget;
        return false;
    }
    else
    {
        period = (uint32_t)(period + portamentoIncrement);
        return true;
    }
}

bool ModChannel::VolumeSlideWithPortamentoTick()
{
    PortamentoTick();

    if (!m_volumeSlide.VolumeSlideTick())
    {
        tickAction = [this](uint32_t){ return PortamentoTick(); };
    }

    return true;
}

#pragma endregion
#pragma region Vibrato

void ModChannel::SetupVibrato(uint32_t speed, int width)
{
    vibrato.Set(speed, width);
    periodAddEngine = &vibrato;

    tickAction = nullptr;
    volumeAdd = 0;
}

void ModChannel::SetupVolumeSlideWithVibrato(int increment)
{
    periodAddEngine = &vibrato;
    m_volumeSlide.SetupVolumeSlideValues(increment);
    tickAction = m_volumeSlide.SetupVolumeSlideTick(
        [this](uint32_t){ return m_volumeSlide.VolumeSlideTick(); },
        nullptr
    );
}

#pragma endregion
#pragma region Arpeggio

void ModChannel::SetupArpeggio(int n1, uint32_t n2)
{
    arpeggioPosition = 0;

    auto const adj1 = pow(2, -(1 / 12.0) * n1);
    auto const adj2 = pow(2, -(1 / 12.0) * n2);

    arpeggioPeriodAdd1 = (int)(period * adj1 - period);
    arpeggioPeriodAdd2 = (int)(period * adj2 - period);

    tickAction = [this](uint32_t){ return ArpeggioTick(); };
}

bool ModChannel::ArpeggioTick()
{
    if (arpeggioPosition == 0)
    {
        arpeggioPosition = 1;
        periodAdd = 0;
    }
    else if (arpeggioPosition == 1)
    {
        arpeggioPosition = 2;
        periodAdd = arpeggioPeriodAdd1;
    }
    else
    {
        arpeggioPosition = 0;
        periodAdd = arpeggioPeriodAdd2;
    }

    volumeAdd = 0;

    return true;
}

#pragma endregion
#pragma region Tremolo

void ModChannel::SetupTremolo(uint32_t speed, uint8_t width)
{
    if (speed > 0)
    {
        tremoloSpeed = speed;
    }
    if (width > 0)
    {
        tremoloWidth = width;
    }

    tickAction = [this](uint32_t){ return TremoloTick(); };
}

bool ModChannel::TremoloTick()
{
    tremoloPosition += tremoloSpeed;

    volumeAdd = static_cast<int32_t>(sin(tremoloPosition * (M_PI / 0x8000'0000u)) * tremoloWidth);

    return true;
}

#pragma endregion
#pragma region Volume slide

void VolumeSlide::SetupVolumeSlideValues(int increment)
{
    m_volumeSlideIsFine = false;
    if (increment != 0)
    {
        m_volumeIncrement = increment;
    }
}

TickAction VolumeSlide::SetupVolumeSlideTick(TickAction normal, TickAction fine)
{
    if (m_volumeSlideIsFine)
    {
        VolumeSlideTick();
        return fine;
    }
    else
    {
        if (doFirstTickOnVolumeSlide)
        {
            normal(0);
        }
        return normal;
    }
}

void VolumeSlide::SetupVolumeSlide(int increment)
{
    if (increment != 0)
    {
        m_volumeSlideIsFine = false;
        m_volumeIncrement = increment;
    }

    if (m_volumeSlideIsFine)
    {
        VolumeSlideTick();
    }
    else
    {
        if (doFirstTickOnVolumeSlide)
        {
            VolumeSlideTick();
        }
        tickAction = [this](uint32_t){ return VolumeSlideTick(); };
    }
}

void VolumeSlide::SetupFineVolumeSlide(int increment)
{
    m_volumeSlideIsFine = true;
    m_volumeIncrement = increment;

    VolumeSlideTick();
}

bool VolumeSlide::VolumeSlideTick()
{
    return IncrementVolume(m_volumeIncrement);
}

bool ModChannel::IncrementVolume(int32_t volumeIncrement)
{
    m_volume = static_cast<uint32_t>(m_volume + volumeIncrement);
    if (m_volume > 64)
    {
        if (volumeIncrement > 0)
        {
            m_volume = 64;
        }
        else
        {
            m_volume = 0;
        }
        return false;
    }
    return true;
}

#pragma endregion

Fragment ModChannel::CopyForMixing(int sampleCount, double volumeMultiplier, double periodSpeed)
{
    if (finished || sample == nullptr)
    {
        return nullptr;
    }

    auto const speed = static_cast<uint64_t>(periodSpeed / ((period + periodAdd * sample->Tune) * periodMult * finetune));
    auto const oldPosition = m_position;
    auto const volumeValue = (m_volume + volumeAdd) * volumeMult * volumeMultiplier * fadeOutVolume * std::max(volumeEnvelopeMult, 0.0);
    auto const volumeLeft = volumeValue * panningLeft[panning];
    auto const volumeRight = volumeValue * panningRight[panning];

    m_position += speed * sampleCount;

    if (volumeLeft == 0 && volumeRight == 0)
    {
        if (!sample->PCM)
        {
            finished = true;
            return nullptr;
        }
        // Just keep up with the current position.
        while ((m_position >> 32) >= sample->PCM->size() - 1)
        {
            if (sample->LoopLength > 0)
            {
                m_position -= uint64_t{sample->LoopLength} << 32;
            }
            else
            {
                finished = true;
                break;
            }
        }
        return nullptr;
    }

    assert(m_position >= 0);

    if ((m_position >> 32) < sample->PCM->size() - 1)
    {
        return MakeStraightFragment(sample->PCM, sampleCount, oldPosition, speed, volumeLeft, volumeRight);
    }

    auto const fragmentSampleCount = (int)(((uint64_t{sample->PCM->size() - 1} << 32) - oldPosition - 1) / speed) + 1;

    assert(fragmentSampleCount > 0);

    auto fragment = MakeStraightFragment(sample->PCM, fragmentSampleCount, oldPosition, speed, volumeLeft, volumeRight);

    if (sample->LoopLength == 0)
    {
        finished = true;
        return fragment;
    }

    m_position = oldPosition + speed * fragmentSampleCount - (uint64_t{sample->LoopLength} << 32);
    if (sampleCount == fragmentSampleCount)
    {
        return fragment;
    }

    auto const reminder = CopyForMixing(sampleCount - fragmentSampleCount, volumeMultiplier, periodSpeed);
    return MakeFragmentChain(std::move(fragment), reminder);
}

}
// namespace VTPlayerLib
