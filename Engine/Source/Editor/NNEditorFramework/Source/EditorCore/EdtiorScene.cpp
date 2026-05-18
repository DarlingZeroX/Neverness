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

#include "EditorCore/EdtiorScene.h"
#include "UITask/MessageUITask.h"
#include "EditorCore/Localization.h"
#include <NNEngineLegacy/Include/Engine/Manager.h>
#include <NNRuntimeImGui/Include/ImGuiEx/ImNotify.h>
#include <NNPlatformCore/Include/NativeFileDialog/portable-file-dialogs.h>
#include <NNFileSystem/Interface/HFileSystem.h>
#include "NNRuntimeVFS/Include/VFSService.h"

namespace NN::Editor
{
	void EditorScene::OpenSaveCurrentSceneDialog(const std::function<void(int)>& callback)
	{
		auto* message = new MessageUITask("Please Confirm...", "Save current scene");
		message->SetChoices({ "Save","Don't Save" });

		auto task = ImGuiEx::NewUITask(message, "New Directory");

		message->SetCallback([callback](int choice)
			{
				if (choice == 0)
				{
					SaveCurrentScene();
				}

				callback(choice);
			});
	}

	void EditorScene::NewScene()
	{
		OpenSaveCurrentSceneDialog([](int choice)
			{
				Runtime::GetSceneManager()->LoadNewScene();
			});
	}

	bool EditorScene::OpenNewScene(const Runtime::VGPath& path)
	{
		// 当打开场景就是本场景直接返回
		if (Runtime::GetSceneManager()->GetCurrentRunningScene() && Runtime::GetSceneManager()->GetCurrentRunningScene()->GetResourcePath() == path)
		{
			return true;
		} 

		OpenSaveCurrentSceneDialog([path](int choice)
			{
				Runtime::GetSceneManager()->LoadScene(path);
			});

		return true;
	}

	void EditorScene::OpenSceneByFileDialog()
	{
		Runtime::String contentPath = Runtime::VFS::VFSService::GetInstance()->AbsolutePath(Runtime::RuntimeCore::GetAssetsPathVFS());
		auto root = contentPath;
		root = NN::Core::HFileSystem::ToWindowsPath(root);

		auto selection = pfd::open_file(EditorText{ "Open Project" }.c_str(), root, { "Scene (.vgasset)", "*.vgasset" }, pfd::opt::multiselect | pfd::opt::force_path).result();

		if (selection.empty())
			return;

		OpenSaveCurrentSceneDialog([selection](int choice)
			{
			auto scenePath = Runtime::RuntimeCore::GetResourcePathVFS(selection[0]);
			auto scene = Runtime::GetSceneManager()->LoadScene(scenePath);

			if (scene)
			{
				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Open scene successfully" });
			}
		});
	}

	bool EditorScene::SaveCurrentScene()
	{
		auto* scene = Runtime::GetSceneManager()->GetCurrentEditorScene();
		if (scene == nullptr)
			return false;

		if (scene->GetResourcePath().empty())
		{
			return SaveCurrentSceneAs();
		}
		
		if (Runtime::GetSceneManager()->SaveScene(dynamic_cast<Runtime::Scene*>(scene), scene->GetResourcePath()))
		{
			ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Save current scene successfully" });
			//pfd::notify(
			//	EditorText{ "Info" }.c_str(), 
			//	EditorText{ "Save current scene successfully" }.c_str(), 
			//	pfd::icon::info
			//);
			return true;
		}
		else
		{
			ImGuiEx::PushNotification({ ImGuiExToastType::Warning, "Save current scene failed" });
			return false;
		}
	}

	bool EditorScene::SaveCurrentSceneAs()
	{
		Runtime::String contentPath = Runtime::VFS::VFSService::GetInstance()->AbsolutePath(Runtime::RuntimeCore::GetAssetsPathVFS());
		auto root = contentPath;
		root = NN::Core::HFileSystem::ToWindowsPath(root);

		auto destination = pfd::save_file(EditorText{ "Save Scene As..." }.c_str(), root, { "Scene (.vgasset)", "*.vgasset" }).result();
		if (destination.empty())
			return false;

		auto* scene = Runtime::GetSceneManager()->GetCurrentEditorScene();
		auto scenePath = Runtime::RuntimeCore::GetResourcePathVFS(destination);

		if (Runtime::GetSceneManager()->SaveScene(dynamic_cast<Runtime::Scene*>(scene), scenePath))
		{
			ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Save scene successfully" });
		}
		else
		{
			ImGuiEx::PushNotification({ ImGuiExToastType::Warning, "Save scene failed" });
			return false;
		}

		return true;
	}
}
