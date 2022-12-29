#pragma once

#include "Song.h"
#include "Engine.h"

#include <functional>
#include <span>

namespace VTPlayerLib
{

class IFragment;
using Fragment = std::shared_ptr<IFragment>;

using TickAction            = std::function<bool(uint32_t)>;
using Action                = std::function<void()>;
using IncrementVolumeAction = std::function<uint32_t(int32_t)>;

struct VolumeSlide
{
    TickAction&                 tickAction;
    Action                const SetNoEffect;
    IncrementVolumeAction const IncrementVolume;
    bool                  const doFirstTickOnVolumeSlide;

    void SetupVolumeSlideValues(int increment);
    TickAction SetupVolumeSlideTick(TickAction normal, TickAction fine);
    void SetupVolumeSlide(int increment);
    void SetupFineVolumeSlide(int increment);
    bool VolumeSlideTick();

	bool IsFine() const { return m_volumeSlideIsFine; }
	int GetIncrement() const { return m_volumeIncrement; }

// Inhibits aggregater initialization: private:
	bool m_volumeSlideIsFine = false;
	int  m_volumeIncrement = 0;
};

class ModChannel
{
public:
    std::shared_ptr<ModSong const> m_song;

    uint8_t panning = 7;
    ModSong::Instrument const* nextInstrument = nullptr;
    ModSong::Sample const*     nextSample = nullptr;
    ModSong::Instrument const* instrument = nullptr;
    ModSong::Sample const*     sample = nullptr;
    bool finished = true;
    uint64_t m_position = 0;

    double finetune = 1;

    EnvelopeEngine volumeEnvelope{};

    std::unique_ptr<ToneSlideEngine> toneSlideEngine = nullptr;

    TickAction tickAction{};

    IValueEngine<uint32_t>* periodEngine         = nullptr;
    IValueEngine<uint32_t>* volumeEngine         = nullptr;
    IValueEngine<int32_t>*  periodAddEngine      = nullptr;
    IValueEngine<double>*   periodMultEngine     = nullptr;
    IValueEngine<int32_t>*  volumeAddEngine      = nullptr;
    IValueEngine<double>*   volumeMultEngine     = nullptr;
    EnvelopeEngine*         volumeEnvelopeEngine = nullptr;

    double volumeEnvelopeMult = 1;

    uint32_t period = 0;
    uint32_t m_volume = 0;

    int32_t periodAdd  = 0;
    double  periodMult = 1;
    int32_t volumeAdd  = 0;
    double  volumeMult = 1;




    uint64_t sampleOffset = 0;

    double fadeOutVolume = 1.0;

    bool noteCut = false;

    std::span<ModSong::ChannelCommand const> commands;

    ModChannel(std::shared_ptr<ModSong const> song, uint32_t index);

    void Tick(uint32_t div, uint32_t tick);

    template < typename E, typename T >
    void EngineTick(E*& engine, T& value); // where E : class, IValueEngine<T>

    void InterpretCommand(ModSong::ChannelCommand command);

    std::function<void()> retriggerAction = nullptr;

    bool TriggerNote(ModSong::ChannelCommand command);

    #pragma region Delay note

    void SetupDelayTickAction(uint32_t delay, std::function<void()> action);

    #pragma endregion
    #pragma region Retrigger

    uint32_t retriggerValue = 0;
    uint32_t retriggerCurrent = 0;
    int retrigVolumeInc = 0;
    double retrigVolumeMult = 1;

    TickAction lastRetriggerTick = nullptr;

    bool RetriggerTick(uint32_t tick);
    void SetupRetrigger(uint32_t delay);
    void SetupRetriggerWithVolumeAdd(uint32_t delay, int increment);
    void SetupRetriggerWithVolumeMult(uint32_t delay, int increment);

    #pragma endregion
    #pragma region One-shot effects

    void SetNoEffect();
    void SetVolume(uint32_t volume);
    void SetFinetune(int finetuneNum);

    #pragma endregion
    #pragma region Tone slide

    bool periodSlideIsFine = false;

    void SetupToneSlide(int increment);
    void SetupFineToneSlide(int increment);
    void SetupContinueToneSlide(int direction);

    #pragma endregion
    #pragma region Tone portamento

    uint32_t portamentoTarget = 0;
    int portamentoIncrement = 0;

    void SetupPortamentoTarget(uint8_t target);
    void SetupPortamento(uint8_t target, int increment);
    void SetupVolumeSlideWithPortamento(uint8_t target, int increment);
    bool PortamentoTick();
    bool VolumeSlideWithPortamentoTick();

    #pragma endregion
    #pragma region Vibrato

    VibratoEngine vibrato{};

    void SetupVibrato(uint32_t speed, int width);
    void SetupVolumeSlideWithVibrato(int increment);

    #pragma endregion
    #pragma region Arpeggio

    uint8_t arpeggioPosition = 0;
    int arpeggioPeriodAdd1 = 0;
    int arpeggioPeriodAdd2 = 0;

    void SetupArpeggio(int n1, uint32_t n2);
    bool ArpeggioTick();

    #pragma endregion
    #pragma region Tremolo

    uint32_t tremoloPosition = 0;
    uint32_t tremoloSpeed = 0;
    uint8_t  tremoloWidth = 0;

    void SetupTremolo(uint32_t speed, uint8_t width);
    bool TremoloTick();

    #pragma endregion
    #pragma region Volume slide

    VolumeSlide m_volumeSlide;

    #pragma endregion

    static constexpr double panningLeft[16]
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        7.0 / 8.0, 6.0 / 8.0, 5.0 / 8.0, 4.0 / 8.0,
        3.0 / 8.0, 2.0 / 8.0, 1.0 / 8.0, 0,
    };

    static constexpr double panningRight[16]
    {
        0, 1.0 / 8.0, 2.0 / 8.0, 3.0 / 8.0,
        4.0 / 8.0, 5.0 / 8.0, 6.0 / 8.0, 7.0 / 8.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    };

    Fragment CopyForMixing(int sampleCount, double volumeMultiplier, double periodSpeed);

private:
    bool IncrementVolume(int32_t volumeIncrement);
};

}
// namespace VTPlayerLib
