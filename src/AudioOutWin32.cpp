#include "AudioOut.h"

#include <MmDeviceApi.h>
#include <AudioClient.h>

#include <optional>
#include <memory>

namespace Cpp
{

struct ComDeleter { void operator()(IUnknown* p) const noexcept { p->Release(); } };

template < typename Interface > using UniqueComPtr = std::unique_ptr<Interface, ComDeleter>;

template < typename Object, typename Interface = Object >
UniqueComPtr<Interface> CoCreateInstance(IUnknown* outer = nullptr)
{
    Interface* result = nullptr;
    auto const hr = ::CoCreateInstance(__uuidof(Object), outer, CLSCTX_ALL, __uuidof(Interface), (void**)&result);
    if (hr != S_OK)
    {
        throw hr;
    }
    return UniqueComPtr<Interface>{ result };
}

}
// namespace Cpp

namespace MmDevice
{

struct Enumerator
{
    std::shared_ptr<IMMDeviceEnumerator> This;

    Cpp::UniqueComPtr<IMMDevice> GetDefaultAudioEndpoint(EDataFlow dataFlow, ERole role) const
    {
        IMMDevice* result = nullptr;
        auto const hr = This->GetDefaultAudioEndpoint(dataFlow, role, &result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return Cpp::UniqueComPtr<IMMDevice>{ result };
    }
};

struct Device
{
    std::shared_ptr<IMMDevice> This;

    template < typename Interface >
    Cpp::UniqueComPtr<Interface> Activate(PROPVARIANT* activationParams = nullptr) const
    {
        Interface* result = nullptr;
        auto const hr = This->Activate(__uuidof(Interface), CLSCTX_ALL, activationParams, (void**)&result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return Cpp::UniqueComPtr<Interface>{ result };
    }
};

}
// namespace MmDevice

namespace Audio
{

struct Client
{
    std::shared_ptr<IAudioClient> This;

    WAVEFORMATEX const& GetMixFormat() const
    {
        WAVEFORMATEX* result = nullptr;
        auto const hr = This->GetMixFormat(&result);
        if (hr != S_OK)
        {
            throw hr;
        }
        if (result == nullptr)
        {
            throw E_UNEXPECTED;
        }
        return *result;
    }

    void Initialize(
        AUDCLNT_SHAREMODE          sharemode,
        DWORD                      streamFlags,
        REFERENCE_TIME             bufferDuration,
        REFERENCE_TIME             periodicity,
        WAVEFORMATEX const&        format,
        std::optional<GUID> const& audioSession = std::nullopt
    ) const
    {
        auto const hr = This->Initialize(
            sharemode,
            streamFlags,
            bufferDuration,
            periodicity,
            &format,
            audioSession ? &audioSession.value() : nullptr
        );
        if (hr != S_OK)
        {
            throw hr;
        }
    }

    uint32_t GetBufferSize() const
    {
        UINT32 result = 0;
        auto const hr = This->GetBufferSize(&result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return result;
    }

    uint32_t GetCurrentPadding() const
    {
        UINT32 result = 0;
        auto const hr = This->GetCurrentPadding(&result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return result;
    }

    template < typename Interface >
    Cpp::UniqueComPtr<Interface> GetService() const
    {
        Interface* result = nullptr;
        auto const hr = This->GetService(__uuidof(Interface), (void**)&result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return Cpp::UniqueComPtr<Interface>{ result };
    }

    void Start() const
    {
        auto const hr = This->Start();
        if (hr != S_OK)
        {
            throw hr;
        }
    }

    void Stop() const
    {
        auto const hr = This->Stop();
        if (hr != S_OK)
        {
            throw hr;
        }
    }
};

struct RenderClient
{
    std::shared_ptr<IAudioRenderClient> This;

    void* GetBuffer(uint32_t numFramesRequested) const
    {
        BYTE* result = nullptr;
        auto const hr = This->GetBuffer(numFramesRequested, &result);
        if (hr != S_OK)
        {
            throw hr;
        }
        return result;
    }

    void ReleaseBuffer(uint32_t numFramesWritten, uint32_t flags) const
    {
        auto const hr = This->ReleaseBuffer(numFramesWritten, flags);
        if (hr != S_OK)
        {
            throw hr;
        }
    }

};

}
// namespace Audio

namespace VTPlayerLib
{

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC   10'000'000
#define REFTIMES_PER_MILLISEC  10'000

void PlayAudioSound(Generator<Fragment>&& sound)
{
    CoInitialize(nullptr);

    try
    {

    MmDevice::Enumerator const deviceEnumerator{ Cpp::CoCreateInstance<MMDeviceEnumerator, IMMDeviceEnumerator>() };
    MmDevice::Device const mmDevice{ deviceEnumerator.GetDefaultAudioEndpoint(eRender, eConsole) };
    Audio::Client const client{ mmDevice.Activate<IAudioClient>() };

    auto const& format = client.GetMixFormat();
    WAVEFORMATEXTENSIBLE const& formatExtensible = *reinterpret_cast<WAVEFORMATEXTENSIBLE const*>(&format);
    client.Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10'000'000, 0, format);
    auto const bufferFrameCount = client.GetBufferSize();
    Audio::RenderClient const renderClient{ client.GetService<IAudioRenderClient>() };

    // Calculate the actual duration of the allocated buffer.
    auto const hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / format.nSamplesPerSec;

    bool started = false;

    // Each loop fills about half of the shared buffer.
    for (auto&& fragment : sound)
    {
        auto fragmentRemaining = fragment->GetCount();
        uint64_t fragmentOffset = 0;

        while (fragmentRemaining > 0)
        {
            // See how much buffer space is available.
            auto const numFramesPadding = client.GetCurrentPadding();

            auto const numFramesAvailable = bufferFrameCount - numFramesPadding;

            if (numFramesAvailable <= bufferFrameCount / 16)
            {
                if (!started)
                {
                    client.Start();  // Start playing.

                    started = true;
                }

                // Sleep for half the buffer duration.
                Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

                // Try again.
                continue;
            }

            auto const frameCount = static_cast<uint32_t>(std::min<uint64_t>(std::min<uint64_t>(fragmentRemaining, numFramesAvailable), 128));

            // Grab all the available space in the shared buffer.
            auto const pData = renderClient.GetBuffer(frameCount);

            // Get next chunk of data from the audio source.
            if (format.nChannels == 1)
            {
                std::vector<SampleMono> samples;
                samples.resize(frameCount);
                fragment->MixMono(MakeSpan(samples), fragmentOffset, true);

                if (format.wBitsPerSample == 8)
                {
                    RenderMono<uint8_t, UINT8_MAX / 2, UINT8_MAX / 2>(std::span<uint8_t>{ static_cast<uint8_t*>(pData), frameCount }, MakeSpan(samples));
                }
                else if (format.wBitsPerSample == 16)
                {
                    RenderMono<int16_t, INT16_MAX>(std::span<int16_t>{ static_cast<int16_t*>(pData), frameCount }, MakeSpan(samples));
                }
                else if (format.wBitsPerSample == 32)
                {
                    if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && formatExtensible.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
                    {
                        memcpy(pData, samples.data(), frameCount * 4);
                    }
                    else
                    {
                        RenderMono<int32_t, INT32_MAX>(std::span<int32_t>{ static_cast<int32_t*>(pData), frameCount }, MakeSpan(samples));
                    }
                }
            }
            else
            {
                std::vector<SampleStereo> samples;
                samples.resize(frameCount);
                fragment->MixStereo(MakeSpan(samples), fragmentOffset, true);

                if (format.wBitsPerSample == 8)
                {
                    RenderStereo<uint8_t, UINT8_MAX / 2, UINT8_MAX / 2>(std::span<uint8_t>{ static_cast<uint8_t*>(pData), frameCount * 2 }, MakeSpan(samples), 0);
                }
                else if (format.wBitsPerSample == 16)
                {
                    RenderStereo<int16_t, INT16_MAX>(std::span<int16_t>{ static_cast<int16_t*>(pData), frameCount * 2 }, MakeSpan(samples), 0);
                }
                else if (format.wBitsPerSample == 32)
                {
                    if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && formatExtensible.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
                    {
                        memcpy(pData, samples.data(), frameCount * 8);
                    }
                    else
                    {
                        RenderStereo<int32_t, INT32_MAX>(std::span<int32_t>{ static_cast<int32_t*>(pData), frameCount * 2 }, MakeSpan(samples), 0);
                    }
                }
            }

            renderClient.ReleaseBuffer(frameCount, 0);

            fragmentOffset += frameCount;
            fragmentRemaining -= frameCount;
        }
    }

    if (started)
    {
        // Wait for last data in buffer to play before stopping.
        Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

        client.Stop();  // Stop playing.
    }

    }
    catch (HRESULT hr)
    {
        printf("HRESULT = 0x%08X\n", static_cast<uint32_t>(hr));
    }
}

}
// namespace VTPlayerLib
