#include "stdafx.h"

#include "AudioOut.h"

#include <alsa/asoundlib.h>

#include <cerrno>
#include <memory>
#include <vector>

namespace
{

struct PcmDeleter
{
    void operator()(snd_pcm_t* pcm) const noexcept
    {
        if (pcm != nullptr)
        {
            snd_pcm_close(pcm);
        }
    }
};

using UniquePcm = std::unique_ptr<snd_pcm_t, PcmDeleter>;

void ReportAlsaError(char const* operation, int error)
{
    std::fprintf(stderr, "%s: %s\n", operation, snd_strerror(error));
}

}

namespace VTPlayerLib
{

void PlayAudioSound(Generator<Fragment>&& sound)
{
    snd_pcm_t* rawPcm = nullptr;
    auto error = snd_pcm_open(&rawPcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (error < 0)
    {
        ReportAlsaError("Unable to open the default audio device", error);
        return;
    }

    UniquePcm pcm{ rawPcm };

    constexpr unsigned int sampleRate = 48000;
    constexpr unsigned int channels = 2;
    error = snd_pcm_set_params(
        pcm.get(),
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels,
        sampleRate,
        1,
        500'000
    );
    if (error < 0)
    {
        ReportAlsaError("Unable to configure the audio device", error);
        return;
    }

    constexpr snd_pcm_uframes_t chunkSize = 128;
    std::vector<SampleStereo> samples(chunkSize);
    std::vector<int16_t> rendered(chunkSize * channels);

    for (auto&& fragment : sound)
    {
        auto fragmentRemaining = fragment->GetCount();
        uint64_t fragmentOffset = 0;

        while (fragmentRemaining > 0)
        {
            auto const frameCount = static_cast<snd_pcm_uframes_t>(
                std::min<uint64_t>(fragmentRemaining, chunkSize)
            );
            auto sampleSpan = MakeSpan(samples, 0, frameCount);
            fragment->MixStereo(sampleSpan, fragmentOffset, true);
            RenderStereo<int16_t, INT16_MAX>(
                std::span<int16_t>{ rendered.data(), static_cast<size_t>(frameCount) * channels },
                sampleSpan,
                0
            );

            snd_pcm_uframes_t framesWritten = 0;
            while (framesWritten < frameCount)
            {
                auto const result = snd_pcm_writei(
                    pcm.get(),
                    rendered.data() + static_cast<size_t>(framesWritten) * channels,
                    frameCount - framesWritten
                );
                if (result < 0)
                {
                    ReportAlsaError("Error, maybe recoverable, attempting recovery", error);
                    error = snd_pcm_recover(pcm.get(), static_cast<int>(result), 1);
                    if (error < 0)
                    {
                        ReportAlsaError("Unable to write audio samples", error);
                        return;
                    }
                    continue;
                }

                if (result == 0)
                {
                    ReportAlsaError("Audio device made no write progress", -EIO);
                    return;
                }

                framesWritten += static_cast<snd_pcm_uframes_t>(result);
            }

            fragmentOffset += frameCount;
            fragmentRemaining -= frameCount;
        }
    }

    error = snd_pcm_drain(pcm.get());
    if (error < 0)
    {
        ReportAlsaError("Unable to finish audio playback", error);
    }
}

}
