#pragma once
#include <string>
#include "HFileSystem.h"
#include <filesystem>

namespace Horizon
{
	struct H_CORE_PLATFORM_API HSequenceGenerator
	{
		static fsPath GenerateDirectory(const fsPath& path);
		static fsPath GenerateAsset(const fsPath& path, const std::wstring& name);
		static std::wstring GenerateAssetName(HPath path, std::wstring name);
	};
}
