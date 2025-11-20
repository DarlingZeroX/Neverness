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
#include <HCore/Include/Asset/HAsset.h>
#include <HCore/Include/System/HFileSystem.h>
#include <VGEngine/Include/Core/Core.h>
#include <VGEngine/Include/Core/Window.h>
#include <VGEngine/Include/Asset/Package.h>

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API AssetImporterManager
	{
		//void OpenAsset(const VGPath& path, const VGAssetMetaData& metaData);

		//void RegisterHandler(std::string type, std::function<void(const VGPath&)> handle);

		void Initialize(Ref<VGWindow>& window); 

		static AssetImporterManager& GetInstance();

		//void OpenTextFile(const VGPath& path);
	private:
		void OnFileDropEvent(const Horizon::Events::HWindowEvent& evt);

		std::unordered_map<std::string, std::function<void(const VGPath&)>> m_Handlers;

		Ref<VGWindow> m_EditorWindow;
	};

}
