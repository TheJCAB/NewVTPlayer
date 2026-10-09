#include "Engine.h"

#include <cassert>
#include <cmath>

namespace VTPlayerLib
{

void ToneSlideEngine::Set(uint32_t period, int32_t increment, uint32_t limit) noexcept
{
    assert((increment > 0 && limit >= period) || (increment < 0 && limit <= period));

    m_period    = period;
    m_increment = increment;
    m_limit     = limit;
}

void LinearToneSlideEngine::Set(uint32_t period, int32_t increment, uint32_t limit) noexcept
{
    ToneSlideEngine::Set(period, increment, limit);

    multiplier = pow(2, increment / 32768.0);
}

bool LinearToneSlideEngine::Tick() noexcept
{
    double newPeriod = m_period * multiplier;
    if (m_increment > 0 && newPeriod > m_limit)
    {
        m_period = m_limit;
        return false;
    }
    if (m_increment < 0 && newPeriod < m_limit)
    {
        m_period = m_limit;
        return false;
    }
    else
    {
        m_period = (uint32_t)newPeriod;
        return true;
    }
}

bool AmigaToneSlideEngine::Tick() noexcept
{
    if (m_increment > 0 && (uint32_t)m_increment > m_limit - m_period)
    {
        m_period = m_limit;
        return false;
    }
    else if (m_increment < 0 && (uint32_t)-m_increment > m_period - m_limit)
    {
        m_period = m_limit;
        return false;
    }
    else
    {
        m_period = (uint32_t)(m_period + m_increment);
        return true;
    }
}

}
// namespace VTPlayerLib
