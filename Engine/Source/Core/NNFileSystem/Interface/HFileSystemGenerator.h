#pragma once
#include <string>
#include "HFileSystem.h"
#include <filesystem>

namespace NN::Core
{
	struct H_FILE_SYSTEM_API HSequenceGenerator
	{
		static fsPath GenerateDirectory(const fsPath& path);
		static fsPath GenerateAsset(const fsPath& path, const std::wstring& name);
		static std::wstring GenerateAssetName(HPath path, std::wstring name);
	};
}
