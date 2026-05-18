#pragma once

#include <cstdint>

/** Phase-1 helper: void(void) with CoreCLR unmanaged export calling convention (stdcall on Windows). */
#if defined(_WIN32)
using VGManagedVoidThunk = void(__stdcall*)();
#else
using VGManagedVoidThunk = void(*)();
#endif
