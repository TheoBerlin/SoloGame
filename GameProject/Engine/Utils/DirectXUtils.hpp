#pragma once

#include <system_error>
#include <winerror.h>

inline std::string hresultToString(HRESULT hr)
{
    return std::system_category().message(hr);
}
