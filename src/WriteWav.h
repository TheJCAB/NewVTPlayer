#pragma once

#include <cstdint>
#include <array>
#include <span>

#include "SoundMixer.h"

namespace VTPlayerLib
{

constexpr std::array<uint8_t, 44> c_wavHeader
{
    0x52, // 'RIFF'
    0x49,
    0x46,
    0x46,

    0x00, // File size
    0x00,
    0x00,
    0x00,

    0x57, // 'WAVE'
    0x41,
    0x56,
    0x45,

    0x66, // 'fmt '
    0x6D,
    0x74,
    0x20,

    0x10, // Chunk size
    0x00,
    0x00,
    0x00,

    0x01, // Format code
    0x00,
    0x01, // Channels
    0x00,

    0x44, // Samples per second
    0xAC,
    0x00,
    0x00,
    0x44, // Bytes per second
    0xAC,
    0x00,
    0x00,

    0x02, // Sample size
    0x00,
    0x10, // Bits per sample
    0x00,

    0x64, // 'data'
    0x61,
    0x74,
    0x61,
    0x00, // Data size
    0x00,
    0x00,
    0x00,
};

inline
void WriteWavMono(wchar_t const* fileName, std::span<SampleMono> buffer)
{
    std::vector<int16_t> rendered;
    rendered.resize(buffer.size());
    RenderMono<int16_t, INT16_MAX>(MakeSpan(rendered), buffer);

    auto const size = rendered.size() * 2;
    auto header = c_wavHeader;

    header[04] = (uint8_t)((size + 36) & 255);
    header[05] = (uint8_t)(((size + 36) >> 8) & 255);
    header[06] = (uint8_t)(((size + 36) >> 16) & 255);
    header[07] = (uint8_t)(((size + 36) >> 24) & 255);

    header[40] = (uint8_t)(size & 255);
    header[41] = (uint8_t)((size >> 8) & 255);
    header[42] = (uint8_t)((size >> 16) & 255);
    header[43] = (uint8_t)((size >> 24) & 255);

    if (FILE* const f = _wfopen(fileName, L"wb"))
    {
        fwrite(header.data(), 1, header.size(), f);
        fwrite(rendered.data(), 2, rendered.size(), f);
        fclose(f);
    }
}

}
// namespace VTPlayerLib
