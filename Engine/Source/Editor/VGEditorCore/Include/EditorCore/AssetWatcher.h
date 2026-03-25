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
#include "../../VGEditorCoreConfig.h"
#include <unordered_map>
#include <filesystem>

namespace VisionGal::Editor
{
	struct VG_EDITOR_CORE_API AssetWatcher
	{
		AssetWatcher();
		~AssetWatcher() = default;

		static AssetWatcher& GetInstance();

		void OnUpdate();

		void AddWatchUpdateCallback(std::function<void()> callback);
	private:
		//void OnStoryScriptWatchUpdate();
		void OnUIFileWatchUpdate();

		void OnUIFileOpen(const std::string& path);
		void OnUIFileClose(const std::string& path);

		struct UIFileState
		{
			std::filesystem::file_time_type LastWriteTime;
		};

		std::unordered_map<std::string, UIFileState> m_UIFilesState;
		std::vector<std::function<void()>> m_WatchUpdateCallbacks;
	};

}