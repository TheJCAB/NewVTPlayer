
#include "stdafx.h"

#include "SoundMixer.h"

#include <cstdint>
#include <span>

namespace VTPlayerLib
{

class FragmentChain : public IFragment
{
    Fragment first;
    Fragment second;

    uint64_t firstCount  = 0;
    uint64_t secondCount = 0;

public:
    friend Fragment MakeFragmentChain(Fragment first, Fragment second);

    #pragma region IFragment Members

    uint64_t GetCount() const noexcept override
    {
        return firstCount + secondCount;
    }

    void MixMono(std::span<SampleMono> dest, uint64_t offset, bool copy) const noexcept override
    {
        size_t const count = dest.size();

        assert(!dest.empty());
        assert(offset + count <= firstCount + secondCount);

        if (firstCount >= offset + count)
        {
            // It's all within first.
            first->MixMono(dest, offset, copy);
        }
        else if (firstCount > offset)
        {
            // Straddles first and second.
            auto const n = static_cast<size_t>(firstCount - offset);
            first->MixMono(MakeSpan(dest, 0, n), offset, copy);
            second->MixMono(MakeSpan(dest, n, count - n), 0, copy);
        }
        else
        {
            // It's all within second.
            second->MixMono(dest, offset - firstCount, copy);
        }
    }

    void MixStereo(std::span<SampleStereo> dest, uint64_t offset, bool copy) const noexcept override
    {
        size_t const count = dest.size();

        assert(!dest.empty());
        assert(offset + count <= firstCount + secondCount);

        if (firstCount >= offset + count)
        {
            // It's all within first.
            first->MixStereo(dest, offset, copy);
        }
        else if (firstCount > offset)
        {
            // Straddles first and second.
            auto const n = static_cast<size_t>(firstCount - offset);
            first->MixStereo(MakeSpan(dest, 0, n), offset, copy);
            offset  = 0;
            second->MixStereo(MakeSpan(dest, n, count - n), offset, copy);
        }
        else
        {
            // It's all within second.
            second->MixStereo(dest, offset - firstCount, copy);
        }
    }

    #pragma endregion
};

Fragment MakeFragmentChain(Fragment first, Fragment second)
{
    assert(first);
    assert(second);
    assert(first->GetCount() > 0);
    assert(second->GetCount() > 0);

    auto result = std::make_shared<FragmentChain>();

    result->firstCount  = first->GetCount();
    result->secondCount = second->GetCount();
    result->first       = std::move(first);
    result->second      = std::move(second);

    return result;
}

void SetSilence(std::span<SampleMono> dest) noexcept
{
    for (auto& sample : dest)
    {
        sample = 0;
    }
}

void SetSilence(std::span<SampleStereo> dest) noexcept
{
    for (auto& [left, right] : dest)
    {
        left  = 0;
        right = 0;
    }
}

class SilenceFragment : public IFragment
{
    size_t m_count = 0;

public:
    friend Fragment MakeSilenceFragment(size_t count);

    #pragma region IFragment Members

    uint64_t GetCount() const noexcept override
    {
        return m_count;
    }

    void MixMono(std::span<SampleMono> dest, uint64_t, bool copy) const noexcept override
    {
        if (copy)
        {
            SetSilence(dest);
        }
    }

    void MixStereo(std::span<SampleStereo> dest, uint64_t, bool copy) const noexcept override
    {
        if (copy)
        {
            SetSilence(dest);
        }
    }

    #pragma endregion
};

Fragment MakeSilenceFragment(size_t count)
{
    auto result = std::make_shared<SilenceFragment>();

    result->m_count = count;

    return result;
}

class StraightFragment : public IFragment
{
    std::shared_ptr<std::vector<float>> sampleData;
    size_t m_count = 0;

    uint64_t position = 0;
    uint64_t speed = 0;
    float volumeLeft = 0;
    float volumeRight = 0;

public:
    friend Fragment MakeStraightFragment(std::shared_ptr<std::vector<float>> sampleData, size_t count, uint64_t position, uint64_t speed, double volumeLeft, double volumeRight);

    uint64_t GetCount() const noexcept override { return m_count; }

    void MixMono(std::span<SampleMono> dest, uint64_t offset, bool copy) const noexcept override
    {
        size_t const count = dest.size();

        assert(!dest.empty());

        auto const n = m_count <= offset ? size_t{0} : static_cast<size_t>(std::min<uint64_t>(m_count - offset, count));
        if (copy)
        {
            MixMonoOp<true>(MakeSpan(dest, 0, n), offset);
            if (n < count)
            {
                SetSilence(MakeSpan(dest, n, count - n));
            }
        }
        else
        {
            MixMonoOp<false>(MakeSpan(dest, 0, n), offset);
        }
    }

    void MixStereo(std::span<SampleStereo> dest, uint64_t offset, bool copy) const noexcept override
    {
        size_t const count = dest.size();

        assert(!dest.empty());

        auto const n = m_count <= offset ? size_t{0} : static_cast<size_t>(std::min<uint64_t>(m_count - offset, count));
        if (copy)
        {
            MixStereoOp<true>(MakeSpan(dest, 0, n), offset);
            if (n < count)
            {
                SetSilence(MakeSpan(dest, n, count - n));
            }
        }
        else
        {
            MixStereoOp<false>(MakeSpan(dest, 0, n), offset);
        }
    }

private:
    template < bool copy >
    void MixMonoOp(std::span<SampleMono> dest, uint64_t offset) const noexcept
    {
        size_t const count = dest.size();

        auto const volume = (volumeLeft + volumeRight) / 2;
        uint64_t p = position + speed * offset;
        for (size_t i = 0; i < count; ++i, p += speed)
        {
            auto const pos = static_cast<uint32_t>(p >> 32);
            float a = (p & 0x0000'0000'FFFF'FFFF) / (65536.0f * 65536.0f);

            auto const sample = ((*sampleData)[pos] * (1 - a) + (*sampleData)[pos + 1] * a) * volume;
            if constexpr (copy)
            {
                dest[i] = sample;
            }
            else
            {
                dest[i] += sample;
            }
        }
    }

    template < bool copy >
    void MixStereoOp(std::span<SampleStereo> dest, uint64_t offset) const noexcept
    {
        size_t const count = dest.size();

        uint64_t p = position + speed * offset;
        for (size_t i = 0; i < count; ++i, p += speed)
        {
            int pos = (int)(p >> 32);
            float a = (p & 0x00000000FFFFFFFF) / (65536.0f * 65536.0f);

            auto const sampleLeft  = ((*sampleData)[pos] * (1 - a) + (*sampleData)[pos + 1] * a) * volumeLeft;
            auto const sampleRight = ((*sampleData)[pos] * (1 - a) + (*sampleData)[pos + 1] * a) * volumeRight;
            auto& [left, right] = dest[i];
            if constexpr (copy)
            {
                left  = sampleLeft;
                right = sampleRight;
            }
            else
            {
                left  += sampleLeft;
                right += sampleRight;
            }
        }
    }
};

Fragment MakeStraightFragment(std::shared_ptr<std::vector<float>> sampleData, size_t count, uint64_t position, uint64_t speed, double volumeLeft, double volumeRight)
{
    assert(sampleData);
    assert(!sampleData->empty());
    assert(count > 0);
    assert(volumeLeft > 0 || volumeRight > 0);
    assert((size_t)((position + speed * (count - 1)) >> 32) >= 0);
    assert((size_t)((position + speed * (count - 1)) >> 32) < sampleData->size() - 1);

    auto result = std::make_shared<StraightFragment>();

    result->sampleData  = std::move(sampleData);
    result->m_count     = count;
    result->position    = position;
    result->speed       = speed;
    result->volumeLeft  = static_cast<float>(volumeLeft );
    result->volumeRight = static_cast<float>(volumeRight);

    return result;
}

class MixFragment : public IFragment
{
    std::vector<Fragment> m_channels;
    size_t                                  m_count;

    friend Fragment MakeMixFragment(std::vector<Fragment> channels, size_t count);

public:
#pragma region IFragment Members

    uint64_t GetCount() const noexcept override { return m_count; }

    void MixMono(std::span<SampleMono> dest, uint64_t offset, bool copy) const noexcept override
    {
        for (auto&& channel : m_channels)
        {
            channel->MixMono(dest, offset, copy);
            copy = false;
        }

        if (copy)
        {
            SetSilence(dest);
        }
    }

    void MixStereo(std::span<SampleStereo> dest, uint64_t offset, bool copy) const noexcept override
    {
        for (auto&& channel : m_channels)
        {
            channel->MixStereo(dest, offset, copy);
            copy = false;
        }

        if (copy)
        {
            SetSilence(dest);
        }
    }

#pragma endregion
};

Fragment MakeMixFragment(std::vector<Fragment> channels, size_t count)
{
    assert(!channels.empty());
    assert(count > 0);

    auto result = std::make_shared<MixFragment>();
    result->m_channels = std::move(channels);
    result->m_count    = count;
    return result;
}

}
// namespace VTPlayerLib
