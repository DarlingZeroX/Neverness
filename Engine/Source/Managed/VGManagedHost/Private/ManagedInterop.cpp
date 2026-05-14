#include "VGManagedHost/ManagedInterop.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include <string>

#if defined(_WIN32)

std::wstring VGManagedInterop::Utf8ToWide(const char* utf8, std::size_t len)
{
	if (!utf8)
		return {};
	const int input_len =
		(len == static_cast<std::size_t>(-1)) ? -1 : static_cast<int>(len);
	const int required = ::MultiByteToWideChar(CP_UTF8, 0, utf8, input_len, nullptr, 0);
	if (required <= 0)
		return {};
	std::wstring out(static_cast<std::size_t>(required), L'\0');
	if (::MultiByteToWideChar(CP_UTF8, 0, utf8, input_len, out.data(), required) <= 0)
		return {};
	return out;
}

std::wstring VGManagedInterop::PathToWide(const std::filesystem::path& path)
{
	return path.native();
}

#endif
