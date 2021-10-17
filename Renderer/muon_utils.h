/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2021/03
Description : General utilities
----------------------------------------------*/
#ifndef MUON_UTILS_H
#define MUON_UTILS_H

#include <windows.h>
#include <stdexcept>

// Exception throwing utilities for debug builds
#if defined(MN_DEBUG)
inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), mHR(hr) {}
    HRESULT Error() const { return mHR; }
private:
    const HRESULT mHR;
};

#define HR(hr) if (FAILED(hr)) throw HrException(hr)

#elif defined(MN_RELEASE)

#define HR(hr)

#endif

#endif