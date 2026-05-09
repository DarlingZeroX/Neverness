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

#include "EditorCore/AssetWatcher.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGEngine/Include/Engine/Manager.h"
//#include "VGGalgame/Include/GalGameEngine.h"
//#include "VGGalgame/Include/GameEngineCore.h"
#include <VGUI/Interface/UISystem.h>
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal::Editor
{
	AssetWatcher::AssetWatcher()
	{
		EngineEventBus::Get().OnUISystemEvent.Subscribe([this](const UISystemEvent& evt)
			{
				switch (evt.EventType)
				{
				case UISystemEventType::UIFileOpen:
					OnUIFileOpen(evt.UIFilePath);
					break;
				case UISystemEventType::UIFileClose:
					OnUIFileClose(evt.UIFilePath);
					break;
				}
				return;
			});
	}

	AssetWatcher& AssetWatcher::GetInstance()
	{
		static AssetWatcher instance;
		return instance;
	}

	void AssetWatcher::OnUpdate()
	{
		if (GetSceneManager()->IsPlayMode())
		{
			// 剧情脚本监视更新
			//OnStoryScriptWatchUpdate();

			// UI文件监视更新
			OnUIFileWatchUpdate();
		}
	}

	void AssetWatcher::AddWatchUpdateCallback(std::function<void()> callback)
	{
		m_WatchUpdateCallbacks.push_back(std::move(callback));
	}

	void AssetWatcher::OnUIFileWatchUpdate()
	{
		for (auto& [path, fileState]: m_UIFilesState)
		{
			auto absPath = VFS::GetInstance()->AbsolutePath(path);
			if (Horizon::HFileSystem::ExistsFile(absPath) == false)
				return;

			auto currentWriteTime = std::filesystem::last_write_time(absPath);
			if (fileState.LastWriteTime != currentWriteTime)
			{
				UISystem::Get()->ReloadAllUIDocument();

				fileState.LastWriteTime = currentWriteTime;
				break;
			}
		}
	}

	void AssetWatcher::OnUIFileOpen(const std::string& path)
	{
		if (path.empty())
			return;

		auto it = m_UIFilesState.find(path);
		if (it != m_UIFilesState.end())
			return;

		std::filesystem::path fsPath = path;
		std::string ext = fsPath.extension().string();

		if (ext != ".html" && ext != ".css")
			return;

		auto absPath = VFS::GetInstance()->AbsolutePath(path);
		if (Horizon::HFileSystem::ExistsFile(absPath) == false)
			return;

		UIFileState fileState;
		fileState.LastWriteTime = std::filesystem::last_write_time(absPath);

		m_UIFilesState[path] = fileState;
	}

	void AssetWatcher::OnUIFileClose(const std::string& path)
	{
		if (path.empty())
			return;

		//m_UIFilesState.erase(path);
	}
}
