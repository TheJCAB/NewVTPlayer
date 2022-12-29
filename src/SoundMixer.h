#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <algorithm>

namespace VTPlayerLib
{

using SampleMono   = float;
using SampleStereo = std::tuple<float, float>;

using MonoSpan   = std::span<SampleMono  >;
using StereoSpan = std::span<SampleStereo>;

template < typename T >
std::span<T> MakeSpan(std::vector<T>& data, size_t start = 0, size_t count = SIZE_MAX)
{
    auto const dataSize = static_cast<size_t>(data.size());
    if (start >= dataSize)
    {
        return {};
    }
    else if (start + count > dataSize)
    {
        return std::span<T>{ data.data() + start, data.data() + dataSize };
    }
    else
    {
        return std::span<T>{ data.data() + start, data.data() + start + count };
    }
}

template < typename T >
std::span<T const> MakeSpan(std::vector<T> const& data, size_t start = 0, size_t count = SIZE_MAX)
{
    auto const dataSize = static_cast<size_t>(data.size());
    if (start >= dataSize)
    {
        return {};
    }
    else if (start + count > dataSize)
    {
        return std::span<T const>{ data.data() + start, data.data() + dataSize };
    }
    else
    {
        return std::span<T const>{ data.data() + start, data.data() + start + count };
    }
}

template < typename T >
std::span<T> MakeSpan(std::span<T>& data, size_t start, size_t count = SIZE_MAX)
{
    auto const dataSize = static_cast<size_t>(data.size());
    if (start >= dataSize)
    {
        return {};
    }
    else if (start + count > dataSize)
    {
        return std::span<T>{ data.data() + start, data.data() + dataSize };
    }
    else
    {
        return std::span<T>{ data.data() + start, data.data() + start + count };
    }
}

class IFragment
{
public:
    virtual ~IFragment() = default;

    virtual uint64_t GetCount() const noexcept = 0;

    virtual void MixMono  (MonoSpan   dest, uint64_t offset, bool copy) const noexcept = 0;
    virtual void MixStereo(StereoSpan dest, uint64_t offset, bool copy) const noexcept = 0;
};

using Fragment = std::shared_ptr<IFragment>;

void SetSilence(MonoSpan   dest) noexcept;
void SetSilence(StereoSpan dest) noexcept;

Fragment MakeFragmentChain   (Fragment first, Fragment second);
Fragment MakeSilenceFragment (size_t count);
Fragment MakeStraightFragment(std::shared_ptr<std::vector<SampleMono>> sampleData, size_t count, uint64_t position, uint64_t speed, double volumeLeft, double volumeRight);
Fragment MakeMixFragment     (std::vector<Fragment> channels, size_t count);


template < typename S, S scale, S bias >
constexpr S FloatToIntSample(double sample)
{
    return
        sample >  1.0 ? static_cast<S>(bias + scale) :
        sample < -1.0 ? static_cast<S>(bias - scale) :
        static_cast<S>(bias + static_cast<S>(sample * scale));
}

template < typename S, S scale, S bias = 0 >
size_t RenderMono(std::span<S> dest, MonoSpan samples)
{
    size_t const count = static_cast<size_t>(std::min(dest.size(), samples.size()));

    for (size_t i = 0; i < count; ++i)
    {
        dest[i] = FloatToIntSample<S, scale, bias>(samples[i]);
    }

    return count;
}

template < typename S, S scale, S bias = 0 >
size_t RenderStereo(std::span<S> dest, StereoSpan samples, double crosstalk)
{
    crosstalk = std::min(std::max(crosstalk, 0.0), 1.0);

    size_t const count = static_cast<size_t>(std::min(dest.size() / 2, samples.size()));

    for (size_t i = 0; i < count; ++i)
    {
        auto [sLeftRaw, sRightRaw] = samples[i];

        auto const sLeft  = sLeftRaw + sRightRaw * crosstalk;
        auto const sRight = sRightRaw + sLeftRaw * crosstalk;

        dest[i * 2 + 0] = FloatToIntSample<S, scale, bias>(sLeft);
        dest[i * 2 + 1] = FloatToIntSample<S, scale, bias>(sRight);
    }

    return count;
}

}
// namespace VTPlayerLib
