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
#include "../../Config.h"
//#include <NNRuntimeAssetLegacy/Include/HAsset.h>
#include <NNPlatformCore/Interface/FileSystem/HFileSystem.h>
#include <NNRuntimeCore/Include/Core/RuntimeCore.h>
#include <NNRuntimeApplication/Include/Core/Window.h>
#include <NNRuntimeAssetLegacy/Interface/Package.h>

namespace NN::Editor
{
	struct VG_EDITOR_FRAMEWORK_API AssetImporterManager
	{
		//void OpenAsset(const VGPath& path, const VGAssetMetaData& metaData);

		//void RegisterHandler(std::string type, std::function<void(const VGPath&)> handle);

		void Initialize(Ref<Runtime::VGWindow>& window); 

		static AssetImporterManager& GetInstance();

		//void OpenTextFile(const VGPath& path);
	private:
		void OnFileDropEvent(const NN::Core::Events::HWindowEvent& evt);

		std::unordered_map<std::string, std::function<void(const Runtime::VGPath&)>> m_Handlers;

		Ref<Runtime::VGWindow> m_EditorWindow;
	};

}
