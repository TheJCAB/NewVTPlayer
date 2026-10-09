#include "Engine.h"
#include "Generator.h"

#include <cassert>

namespace VTPlayerLib
{

Generator<double> EnvelopeEngine::TickMachine(uint32_t findTick)
{
    if (m_envelope->Points.empty())
    {
        co_return;
    }

    uint32_t point = 0;

    while (true)
    {
        if (point == m_envelope->SustainPoint && findTick == 0)
        {
            auto const value = m_envelope->Points[point].value;

            while (!m_release)
            {
                co_yield value;
            }
        }

        if (m_envelope->LoopEndPoint > m_envelope->LoopStartPoint && point == m_envelope->LoopEndPoint)
        {
            point = m_envelope->LoopStartPoint;

            auto const loopTicks = m_envelope->Points[m_envelope->LoopEndPoint].tick - m_envelope->Points[m_envelope->LoopStartPoint].tick;

            assert(loopTicks > 0);

            findTick %= loopTicks;
        }

        uint32_t tickStart = m_envelope->Points[point].tick;

        if (point + 1 == m_envelope->Points.size())
        {
            auto const value = m_envelope->Points[point].value;

            if (value == 0)
            {
                co_return;
            }

            while (true)
            {
                co_yield value;
            }
        }

        auto const tickEnd = m_envelope->Points[point + 1].tick;

        if (tickEnd > tickStart)
        {
            auto const ticks = tickEnd - tickStart;

            if (findTick >= ticks)
            {
                findTick -= ticks;
            }
            else
            {
                auto const valueStart = m_envelope->Points[point].value;
                auto const valueEnd = m_envelope->Points[point + 1].value;

                auto const speed = (valueEnd - valueStart) / ticks;

                for (uint32_t tick = findTick; tick < ticks; ++tick)
                {
                    co_yield valueStart + tick*speed;
                }

                findTick = 0;
            }
        }

        ++point;
    }
}

}
// namespace VTPlayerLib
