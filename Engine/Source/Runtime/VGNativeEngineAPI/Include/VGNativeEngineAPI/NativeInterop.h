#pragma once

/**
 * @file NativeInterop.h
 * @brief 跨 Native/Managed 邊界的呼叫約定與可互操作型別（不依賴 VGManagedCore）。
 *
 * 說明：
 * - 與託管端 `UnmanagedCallersOnly` / `delegate* unmanaged` 對齊時，Windows 上顯式使用 stdcall。
 * - 本模組僅承載 **Engine Service ABI**，不包含 Gameplay / 對白 / 存檔等語義。
 */

#include <cstdint>

#if defined(_WIN32)
#define VG_ENGINE_ABI_STDCALL __stdcall
#else
#define VG_ENGINE_ABI_STDCALL
#endif
