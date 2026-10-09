#include "Loaders.h"

#include <array>

namespace VTPlayerLib
{

std::shared_ptr<ModSong> LoadUnknown(IStream& s)
{
    s.Seek(44, SeekOrigin::Begin);
    auto marker = s.ReadString(4);
    s.Seek(0, SeekOrigin::Begin);

    if (marker == L"SCRM")
    {
        return LoadS3M(s);
    }

    marker = s.ReadString(17);
    s.Seek(0, SeekOrigin::Begin);

    if (marker == L"Extended Module: ")
    {
        return LoadXM(s);
    }

    return LoadMod(s);
}

}
// namespace VTPlayerLib
