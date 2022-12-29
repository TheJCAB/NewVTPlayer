#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

inline
std::wstring StdWStringPrintf(wchar_t const* const fmt, ...)
{
    // TODO: std::format
    return {};

//    va_list args;
//    va_start(args, fmt);
//    
//    constexpr size_t MinBufLen = 256;
//    {
//        wchar_t buf[MinBufLen];
//        if (int const len = _vsnwprintf_s(buf, _TRUNCATE, fmt, args); len != -1)
//        {
//            va_end(args);
//            return buf;
//        }
//    }
//
//    size_t bufLen = MinBufLen;
//    for (;;)
//    {
//        bufLen *= 2;
//        std::vector<wchar_t> buf;
//        buf.resize(bufLen);
//        if (int const len = _vsnwprintf_s(buf.data(), bufLen, _TRUNCATE, fmt, args); len != -1)
//        {
//            va_end(args);
//            return buf.data();
//        }
//    }
}
