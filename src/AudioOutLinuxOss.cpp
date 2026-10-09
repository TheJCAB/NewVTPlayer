#include "AudioOut.h"

#include <sys/soundcard.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <vector>

namespace
{

struct FileDescriptor
{
    explicit FileDescriptor(int value) : value(value) {}

    ~FileDescriptor()
    {
        if (value >= 0)
        {
            close(value);
        }
    }

    FileDescriptor(FileDescriptor const&) = delete;
    FileDescriptor& operator=(FileDescriptor const&) = delete;

    int value;
};

bool ReportSystemError(char const* operation)
{
    std::fprintf(stderr, "%s: %s\n", operation, std::strerror(errno));
    return false;
}

bool SetDeviceParameter(int fd, unsigned long request, int expectedValue, char const* operation)
{
    int value = expectedValue;
    if (ioctl(fd, request, &value) == -1)
    {
        return ReportSystemError(operation);
    }
    if (value != expectedValue)
    {
        std::fprintf(stderr, "%s: device selected unsupported value %d\n", operation, value);
        return false;
    }
    return true;
}

}

namespace VTPlayerLib
{

void PlayAudioSound(Generator<Fragment>&& sound)
{
    int const rawFd = open("/dev/dsp", O_WRONLY);
    if (rawFd == -1)
    {
        ReportSystemError("Unable to open /dev/dsp");
        return;
    }
    FileDescriptor const fd{ rawFd };

    constexpr int sampleRate = 48000;
    constexpr int channels = 2;
    if (!SetDeviceParameter(fd.value, SNDCTL_DSP_SETFMT, AFMT_S16_NE, "Unable to set PCM format")
        || !SetDeviceParameter(fd.value, SNDCTL_DSP_CHANNELS, channels, "Unable to set channel count")
        || !SetDeviceParameter(fd.value, SNDCTL_DSP_SPEED, sampleRate, "Unable to set sample rate"))
    {
        return;
    }

    constexpr size_t chunkSize = 128;
    std::vector<SampleStereo> samples(chunkSize);
    std::vector<int16_t> rendered(chunkSize * channels);

    for (auto&& fragment : sound)
    {
        auto fragmentRemaining = fragment->GetCount();
        uint64_t fragmentOffset = 0;

        while (fragmentRemaining > 0)
        {
            auto const frameCount = static_cast<size_t>(
                std::min<uint64_t>(fragmentRemaining, chunkSize)
            );
            auto sampleSpan = MakeSpan(samples, 0, frameCount);
            fragment->MixStereo(sampleSpan, fragmentOffset, true);
            RenderStereo<int16_t, INT16_MAX>(
                std::span<int16_t>{ rendered.data(), frameCount * channels },
                sampleSpan,
                0
            );

            auto const byteCount = frameCount * channels * sizeof(int16_t);
            auto const* data = reinterpret_cast<char const*>(rendered.data());
            size_t bytesWritten = 0;
            while (bytesWritten < byteCount)
            {
                auto const result = write(fd.value, data + bytesWritten, byteCount - bytesWritten);
                if (result == -1)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    ReportSystemError("Unable to write audio samples");
                    return;
                }
                if (result == 0)
                {
                    std::fprintf(stderr, "Unable to write audio samples: device made no progress\n");
                    return;
                }
                bytesWritten += static_cast<size_t>(result);
            }

            fragmentOffset += frameCount;
            fragmentRemaining -= frameCount;
        }
    }

    if (ioctl(fd.value, SNDCTL_DSP_SYNC, nullptr) == -1)
    {
        ReportSystemError("Unable to finish audio playback");
    }
}

}
