#pragma once

#include <cstdint>
#include <string>

namespace VTPlayerLib
{

enum class SeekOrigin : uint8_t
{
    Begin = SEEK_SET,
    Current = SEEK_CUR,
    End = SEEK_END,
};

constexpr wchar_t c_oemTable[256] =
{
    (wchar_t)0x0000,
    (wchar_t)0x0001,
    (wchar_t)0x0002,
    (wchar_t)0x0003,
    (wchar_t)0x0004,
    (wchar_t)0x0005,
    (wchar_t)0x0006,
    (wchar_t)0x0007,
    (wchar_t)0x0008,
    (wchar_t)0x0009,
    (wchar_t)0x000A,
    (wchar_t)0x000B,
    (wchar_t)0x000C,
    (wchar_t)0x000D,
    (wchar_t)0x000E,
    (wchar_t)0x000F,
    (wchar_t)0x0010,
    (wchar_t)0x0011,
    (wchar_t)0x0012,
    (wchar_t)0x0013,
    (wchar_t)0x0014,
    (wchar_t)0x0015,
    (wchar_t)0x0016,
    (wchar_t)0x0017,
    (wchar_t)0x0018,
    (wchar_t)0x0019,
    (wchar_t)0x001A,
    (wchar_t)0x001B,
    (wchar_t)0x001C,
    (wchar_t)0x001D,
    (wchar_t)0x001E,
    (wchar_t)0x001F,
    (wchar_t)0x0020,
    (wchar_t)0x0021,
    (wchar_t)0x0022,
    (wchar_t)0x0023,
    (wchar_t)0x0024,
    (wchar_t)0x0025,
    (wchar_t)0x0026,
    (wchar_t)0x0027,
    (wchar_t)0x0028,
    (wchar_t)0x0029,
    (wchar_t)0x002A,
    (wchar_t)0x002B,
    (wchar_t)0x002C,
    (wchar_t)0x002D,
    (wchar_t)0x002E,
    (wchar_t)0x002F,
    (wchar_t)0x0030,
    (wchar_t)0x0031,
    (wchar_t)0x0032,
    (wchar_t)0x0033,
    (wchar_t)0x0034,
    (wchar_t)0x0035,
    (wchar_t)0x0036,
    (wchar_t)0x0037,
    (wchar_t)0x0038,
    (wchar_t)0x0039,
    (wchar_t)0x003A,
    (wchar_t)0x003B,
    (wchar_t)0x003C,
    (wchar_t)0x003D,
    (wchar_t)0x003E,
    (wchar_t)0x003F,
    (wchar_t)0x0040,
    (wchar_t)0x0041,
    (wchar_t)0x0042,
    (wchar_t)0x0043,
    (wchar_t)0x0044,
    (wchar_t)0x0045,
    (wchar_t)0x0046,
    (wchar_t)0x0047,
    (wchar_t)0x0048,
    (wchar_t)0x0049,
    (wchar_t)0x004A,
    (wchar_t)0x004B,
    (wchar_t)0x004C,
    (wchar_t)0x004D,
    (wchar_t)0x004E,
    (wchar_t)0x004F,
    (wchar_t)0x0050,
    (wchar_t)0x0051,
    (wchar_t)0x0052,
    (wchar_t)0x0053,
    (wchar_t)0x0054,
    (wchar_t)0x0055,
    (wchar_t)0x0056,
    (wchar_t)0x0057,
    (wchar_t)0x0058,
    (wchar_t)0x0059,
    (wchar_t)0x005A,
    (wchar_t)0x005B,
    (wchar_t)0x005C,
    (wchar_t)0x005D,
    (wchar_t)0x005E,
    (wchar_t)0x005F,
    (wchar_t)0x0060,
    (wchar_t)0x0061,
    (wchar_t)0x0062,
    (wchar_t)0x0063,
    (wchar_t)0x0064,
    (wchar_t)0x0065,
    (wchar_t)0x0066,
    (wchar_t)0x0067,
    (wchar_t)0x0068,
    (wchar_t)0x0069,
    (wchar_t)0x006A,
    (wchar_t)0x006B,
    (wchar_t)0x006C,
    (wchar_t)0x006D,
    (wchar_t)0x006E,
    (wchar_t)0x006F,
    (wchar_t)0x0070,
    (wchar_t)0x0071,
    (wchar_t)0x0072,
    (wchar_t)0x0073,
    (wchar_t)0x0074,
    (wchar_t)0x0075,
    (wchar_t)0x0076,
    (wchar_t)0x0077,
    (wchar_t)0x0078,
    (wchar_t)0x0079,
    (wchar_t)0x007A,
    (wchar_t)0x007B,
    (wchar_t)0x007C,
    (wchar_t)0x007D,
    (wchar_t)0x007E,
    (wchar_t)0x007F,
    (wchar_t)0x00C7,
    (wchar_t)0x00FC,
    (wchar_t)0x00E9,
    (wchar_t)0x00E2,
    (wchar_t)0x00E4,
    (wchar_t)0x00E0,
    (wchar_t)0x00E5,
    (wchar_t)0x00E7,
    (wchar_t)0x00EA,
    (wchar_t)0x00EB,
    (wchar_t)0x00E8,
    (wchar_t)0x00EF,
    (wchar_t)0x00EE,
    (wchar_t)0x00EC,
    (wchar_t)0x00C4,
    (wchar_t)0x00C5,
    (wchar_t)0x00C9,
    (wchar_t)0x00E6,
    (wchar_t)0x00C6,
    (wchar_t)0x00F4,
    (wchar_t)0x00F6,
    (wchar_t)0x00F2,
    (wchar_t)0x00FB,
    (wchar_t)0x00F9,
    (wchar_t)0x00FF,
    (wchar_t)0x00D6,
    (wchar_t)0x00DC,
    (wchar_t)0x00A2,
    (wchar_t)0x00A3,
    (wchar_t)0x00A5,
    (wchar_t)0x20A7,
    (wchar_t)0x0192,
    (wchar_t)0x00E1,
    (wchar_t)0x00ED,
    (wchar_t)0x00F3,
    (wchar_t)0x00FA,
    (wchar_t)0x00F1,
    (wchar_t)0x00D1,
    (wchar_t)0x00AA,
    (wchar_t)0x00BA,
    (wchar_t)0x00BF,
    (wchar_t)0x2310,
    (wchar_t)0x00AC,
    (wchar_t)0x00BD,
    (wchar_t)0x00BC,
    (wchar_t)0x00A1,
    (wchar_t)0x00AB,
    (wchar_t)0x00BB,
    (wchar_t)0x2591,
    (wchar_t)0x2592,
    (wchar_t)0x2593,
    (wchar_t)0x2502,
    (wchar_t)0x2524,
    (wchar_t)0x2561,
    (wchar_t)0x2562,
    (wchar_t)0x2556,
    (wchar_t)0x2555,
    (wchar_t)0x2563,
    (wchar_t)0x2551,
    (wchar_t)0x2557,
    (wchar_t)0x255D,
    (wchar_t)0x255C,
    (wchar_t)0x255B,
    (wchar_t)0x2510,
    (wchar_t)0x2514,
    (wchar_t)0x2534,
    (wchar_t)0x252C,
    (wchar_t)0x251C,
    (wchar_t)0x2500,
    (wchar_t)0x253C,
    (wchar_t)0x255E,
    (wchar_t)0x255F,
    (wchar_t)0x255A,
    (wchar_t)0x2554,
    (wchar_t)0x2569,
    (wchar_t)0x2566,
    (wchar_t)0x2560,
    (wchar_t)0x2550,
    (wchar_t)0x256C,
    (wchar_t)0x2567,
    (wchar_t)0x2568,
    (wchar_t)0x2564,
    (wchar_t)0x2565,
    (wchar_t)0x2559,
    (wchar_t)0x2558,
    (wchar_t)0x2552,
    (wchar_t)0x2553,
    (wchar_t)0x256B,
    (wchar_t)0x256A,
    (wchar_t)0x2518,
    (wchar_t)0x250C,
    (wchar_t)0x2588,
    (wchar_t)0x2584,
    (wchar_t)0x258C,
    (wchar_t)0x2590,
    (wchar_t)0x2580,
    (wchar_t)0x03B1,
    (wchar_t)0x00DF,
    (wchar_t)0x0393,
    (wchar_t)0x03C0,
    (wchar_t)0x03A3,
    (wchar_t)0x03C3,
    (wchar_t)0x00B5,
    (wchar_t)0x03C4,
    (wchar_t)0x03A6,
    (wchar_t)0x0398,
    (wchar_t)0x03A9,
    (wchar_t)0x03B4,
    (wchar_t)0x221E,
    (wchar_t)0x03C6,
    (wchar_t)0x03B5,
    (wchar_t)0x2229,
    (wchar_t)0x2261,
    (wchar_t)0x00B1,
    (wchar_t)0x2265,
    (wchar_t)0x2264,
    (wchar_t)0x2320,
    (wchar_t)0x2321,
    (wchar_t)0x00F7,
    (wchar_t)0x2248,
    (wchar_t)0x00B0,
    (wchar_t)0x2219,
    (wchar_t)0x00B7,
    (wchar_t)0x221A,
    (wchar_t)0x207F,
    (wchar_t)0x00B2,
    (wchar_t)0x25A0,
    (wchar_t)0x00A0,
};

struct uint24_t
{
    uint8_t bytes[3];

    operator uint32_t() const noexcept { return (uint32_t{bytes[2]} << 16) | (uint32_t{bytes[1]} << 8) | bytes[0]; }
};

static_assert(sizeof(uint24_t) == 3);

struct Stream
{
    FILE* f;

    int32_t Position() const
    {
        return ftell(f);
    }

    void Seek(int32_t pos, SeekOrigin origin)
    {
        fseek(f, pos, static_cast<uint8_t>(origin));
    }

    int32_t Length()
    {
        auto const pos = Position();
        Seek(0, SeekOrigin::End);
        auto const result = Position();
        Seek(pos, SeekOrigin::Begin);
        return result;
    }

    template < typename T >
    T ReadType()
    {
        alignas(T) uint8_t buf[sizeof(T)];
        fread(buf, 1, sizeof(T), f);
        return *reinterpret_cast<T const*>(buf);
    }

    template < typename T >
    T ReadTypeBE()
    {
        alignas(T) uint8_t buf[sizeof(T)];
        fread(buf, 1, sizeof(T), f);
        std::reverse(std::begin(buf), std::end(buf));
        return *reinterpret_cast<T const*>(buf);
    }

    std::wstring ReadString(size_t length)
    {
        std::string s(length, ' ');
        fread(s.data(), 1, length, f);
        return std::wstring{ s.begin(), s.end() };
    }

    std::wstring ReadOEMString(size_t length)
    {
        std::string s(length, ' ');
        std::wstring w(length, L' ');
        fread(s.data(), 1, length, f);
        for (size_t i = 0; i < length; ++i)
        {
            w[i] = c_oemTable[static_cast<uint8_t>(s[i])];
        }
        return w;
    }
};

}
// namespace VTPlayerLib
