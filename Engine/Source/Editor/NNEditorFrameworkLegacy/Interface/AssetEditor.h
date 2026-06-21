/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "../Config.h"
#include "../Include/PanelInterface.h"
#include <NNPlatformCore/Interface/FileSystem/HFileSystem.h>
#include <NNRuntimeCore/Include/Core/RuntimeCore.h>
//#include <NNRuntimeAssetLegacy/Include/HAsset.h>
#include <NNRuntimeAssetLegacy/Interface/Package.h>

namespace NN::Editor
{
	struct VG_EDITOR_FRAMEWORK_API AssetEditor
	{
		void OpenAsset(const Runtime::VGPath& path, const Runtime::VGAssetMetaData& metaData);
		void OpenAsset(const Runtime::VGPath& path);

		void RegisterHandler(std::string type, std::function<void(const Runtime::VGPath&)> handle);

		static AssetEditor& Get();

		void OpenTextFile(const Runtime::VGPath& path);
	private:
		std::unordered_map<std::string, std::function<void(const Runtime::VGPath&)>> m_Handlers;
	};

}
