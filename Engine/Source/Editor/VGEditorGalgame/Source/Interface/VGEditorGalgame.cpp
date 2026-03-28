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

#include "VGEditorGalgame.h"
#include "GalComDrawer.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGEditorComponent/Interface/ComponentDrawerRegistry.h"
#include "VGEditorCore/Include/EditorCore/AssetWatcher.h"
#include "VGGalgame/Include/GalGameEngine.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"

namespace VisionGal::Editor
{
	void VGEditorGalGame::Initialize()
	{
		// 注册剧情脚本监视更新回调
		AssetWatcher::GetInstance().AddWatchUpdateCallback([]()
			{
				auto* galEngine = dynamic_cast<GalGame::GalGameEngine*>(GalGame::GameEngineCore::GetCurrentEngine());

				if (galEngine == nullptr)
					return;

				//auto* storyScript = galEngine->GetCurrentStoryScript();
				//
				//if (storyScript == nullptr)
				//	return;
				auto storyScriptPath = galEngine->GetStoryScriptSystem()->GetCurrentStoryScriptPath();

				//auto absPath = VFS::GetInstance()->AbsolutePath(storyScript->GetResourcePath());
				auto absPath = VFS::GetInstance()->AbsolutePath(storyScriptPath);
				if (Horizon::HFileSystem::ExistsFile(absPath) == false)
					return;

				if (galEngine->GetStoryScriptSystem()->GetScriptLastWriteTime() != std::filesystem::last_write_time(absPath))
					//if (storyScript->GetScriptLastWriteTime() != std::filesystem::last_write_time(absPath))
				{
					galEngine->ReloadStoryScript();
				}
			});

		// 注册 GalGameEngineComponent 的组件绘制器
		ComponentDrawerRegistry::GetInstance().RegisterDrawer(MakeRef<GalGameEngineComponentDrawer>());
	}
}
