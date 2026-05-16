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
#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNRuntimeCore/Include/Core/Core.h>
//#include <NNRuntimeAsset/Include/HAsset.h>
#include <NNRuntimeAsset/Interface/Package.h>

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API AssetEditor
	{
		void OpenAsset(const VGPath& path, const VGAssetMetaData& metaData);
		void OpenAsset(const VGPath& path);

		void RegisterHandler(std::string type, std::function<void(const VGPath&)> handle);

		static AssetEditor& Get();

		void OpenTextFile(const VGPath& path);
	private:
		std::unordered_map<std::string, std::function<void(const VGPath&)>> m_Handlers;
	};

}
