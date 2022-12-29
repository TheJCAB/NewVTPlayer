#pragma once

#include "Song.h"

#include "Generator.h"

namespace VTPlayerLib
{

struct ITickEngine
{
    virtual ~ITickEngine() noexcept = default;
    virtual bool Tick() noexcept = 0;
};

template < typename T >
struct IValueEngine : public ITickEngine
{
    virtual T GetValue() const noexcept = 0;
};

class EnvelopeEngine final : IValueEngine<double>
{
    ModSong::Envelope const* m_envelope = nullptr;

    double m_value = 1.0;

    bool m_release = false;

public:
    double GetValue() const noexcept override { return m_value; }

    void Set(ModSong::Envelope const& envelope, double defaultValue)
    {
        m_envelope = &envelope;
        m_value = defaultValue;
        m_release = false;

        machine = TickMachine(0);
        it = machine.begin();
    }

    void Release()
    {
        m_release = true;
    }

    bool Tick() noexcept override
    {
        if (it == machine.end())
        {
            return false;
        }

        m_value = *it;
        ++it;

        return true;
    }

    Generator<double>::iterator it;
    Generator<double> machine;

    Generator<double> TickMachine(uint32_t findTick);
};

class ToneSlideEngine : public IValueEngine<uint32_t>
{
protected:
    uint32_t m_period = 0;

public:
    int32_t m_increment = 0;
    uint32_t m_limit = 0;

    virtual void Set(uint32_t period, int32_t increment, uint32_t limit) noexcept;

    uint32_t GetValue() const noexcept override { return m_period; }
    int32_t GetIncrement() const noexcept { return m_increment; }
};

class LinearToneSlideEngine final : public ToneSlideEngine
{
    double multiplier = 1;

public:
    void Set(uint32_t period, int32_t increment, uint32_t limit) noexcept override;

    bool Tick() noexcept override;
};

class AmigaToneSlideEngine final : public ToneSlideEngine
{
public:
    bool Tick() noexcept override;
};

class VibratoEngine final : public IValueEngine<int32_t>
{
    int32_t m_value = 0;

    uint64_t m_position = 0;
    uint64_t m_speed = 0;
    double m_width = 0;

public:
    int32_t GetValue() const noexcept override { return m_value; }

    void Reset() noexcept
    {
        m_position = 0;
        m_value = 0;
    }

    void Set(uint64_t speed, double width) noexcept;

    bool Tick() noexcept override;
};

}
// namespace VTPlayerLib
