// helper file for defining wchar for other than Windows based environments
#pragma once

#include <cstdint>

#ifndef WCHAR_T
#ifndef _NATIVE_WCHAR_T_DEFINED
typedef uint16_t WCHAR_T;
#else
typedef wchar_t WCHAR_T;
#endif
#endif