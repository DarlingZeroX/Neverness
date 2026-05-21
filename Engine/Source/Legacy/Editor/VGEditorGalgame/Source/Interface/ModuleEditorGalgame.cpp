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

#include "ModuleEditorGalgame.h"
#include "GalComDrawer.h"
#include "SequenceEditor.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeVFS/Include/VFSService.h"
#include "NNEditorFrameworkLegacy/Interface/ComponentDrawerRegistry.h"
#include "NNEditorFrameworkLegacy/Include/EditorCore/AssetWatcher.h"
#include "NNEditorFrameworkLegacy/Interface/AssetEditor.h"
#include "VGGalgame/Include/GalGameEngine.h"
#include "VGGalgameCore/Include/GalGameEngineAccess.h"
#include "VGGalgameSequenceRuntime/Include/Asset/Asset.h"

namespace VisionGal::Editor
{
	void ModuleEditorGalGame::MountToEditor()
	{
		// 注册剧情脚本监视更新回调
		AssetWatcher::GetInstance().AddWatchUpdateCallback([]()
			{
				auto* galEngine = dynamic_cast<GalGame::GalGameEngine*>(GalGame::GalGameEngineAccess::Current());

				if (galEngine == nullptr)
					return;

				//auto* storyScript = galEngine->GetCurrentStoryScript();
				//
				//if (storyScript == nullptr)
				//	return;
				auto storyScriptPath = galEngine->GetSubsystemBus()->Script()->GetStoryScriptSystem()->GetCurrentStoryScriptPath();

				//auto absPath = VFSService::GetInstance()->AbsolutePath(storyScript->GetResourcePath());
				auto absPath = VFSService::GetInstance()->AbsolutePath(storyScriptPath);
				if (Horizon::HFileSystem::ExistsFile(absPath) == false)
					return;

				if (galEngine->GetSubsystemBus()->Script()->GetStoryScriptSystem()->GetScriptLastWriteTime() != std::filesystem::last_write_time(absPath))
					//if (storyScript->GetScriptLastWriteTime() != std::filesystem::last_write_time(absPath))
				{
					galEngine->GetStoryScriptSystemPtr()->ReloadStoryScript();
				}
			});

		// 注册 GalGameEngineComponent 的组件绘制器
		ComponentDrawerRegistry::GetInstance().RegisterDrawer(MakeRef<GalGameEngineComponentDrawer>());

		// 注册 GalGameStoryScript 的资产编辑器
		AssetEditor::Get().RegisterHandler(GalGame::SequenceScriptAssetType{}.GetNameID(), [](const VGPath& path) {
			auto& manager = ImGuiEx::ImTaskManager::Get();
			manager.NewTask(new VGScriptSequenceEditor(path), "GalGame Visual Script Editor");
			});
	}
}
