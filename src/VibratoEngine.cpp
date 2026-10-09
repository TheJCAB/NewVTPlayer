#include "Engine.h"

#include <cassert>
#include <cmath>
#include <numbers>

namespace VTPlayerLib
{

void VibratoEngine::Set(uint64_t speed, double width) noexcept
{
    if (speed > 0)
    {
        m_speed = speed;
    }
    if (width > 0)
    {
        m_width = width;
    }
}

bool VibratoEngine::Tick() noexcept
{
    m_position += m_speed;

    m_value = (int32_t)(sin((m_position & 0xFFFFFFFF) * (std::numbers::pi / 0x80000000)) * m_width);

    return true;
}

}
// namespace VTPlayerLib
