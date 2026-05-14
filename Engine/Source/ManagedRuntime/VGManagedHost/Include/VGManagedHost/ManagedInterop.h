#pragma once

#include <string>

#if defined(_WIN32)
#include <filesystem>
#endif

namespace VGManagedInterop
{
#if defined(_WIN32)
/** UTF-8 narrow string to UTF-16 for hostfxr / CoreCLR char_t on Windows. */
std::wstring Utf8ToWide(const char* utf8, std::size_t len = static_cast<std::size_t>(-1));

std::wstring PathToWide(const std::filesystem::path& path);
#endif
}
