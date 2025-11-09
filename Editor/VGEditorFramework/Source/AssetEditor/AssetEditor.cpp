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

#include "AssetEditor/AssetEditor.h"
#include <VGImgui/IncludeImGuiEx.h>

#include "PanelManager.h"
#include "AssetEditor/TextureViewer.h"
#include "EditorComponents/CodeStudio/CodeStudio.h"
#include "EditorCore/EdtiorScene.h"
#include "MainEditor/MainPanel.h"
#include "VGEngine/Include/Engine/Manager.h"

namespace VisionGal::Editor
{
	void AssetEditor::OpenAsset(const VGPath& path, const VGAssetMetaData& metaData)
	{
		auto& manager = ImGuiEx::ImTaskManager::GetInstance();

		if (metaData.AssetType == "Texture")
		{
			manager.NewTask(new TextureViewer(path), "Texture Viewer");
		}
		else if (metaData.AssetType == "Scene")
		{
			EditorScene::OpenNewScene(path);
		}
		else if (metaData.AssetType == "LuaScript")
		{
			OpenTextFile(path);
		}
		else if (metaData.AssetType == "HTML")
		{
			OpenTextFile(path);
		}
		else if (metaData.AssetType == "CSS")
		{
			OpenTextFile(path);
		}

		if (auto result = m_Handlers.find(metaData.AssetType); result != m_Handlers.end())
		{
			result->second(path);
		}
	}

	void AssetEditor::RegisterHandler(std::string type, std::function<void(const VGPath&)> handle)
	{
		m_Handlers[type] = handle;
	}

	AssetEditor& AssetEditor::Instance()
	{
		static AssetEditor editor;

		return editor;
	}

	void AssetEditor::OpenTextFile(const VGPath& path)
	{
		auto* editor = PanelManager::GetInstance();
		auto* mainWindow = dynamic_cast<EditorMainWindow*>(editor->GetPanelWithID("EditorMainWindow"));
		if (mainWindow == nullptr)
			return;

		auto* textEditor = dynamic_cast<CodeStudioPanel*>( mainWindow->GetPanelWithID("CodeStudioPanel"));
		if (textEditor == nullptr)
			return;

		if (textEditor->OpenTextFile(path))
		{
			textEditor->OpenWindow(true);
		}
		else
		{
			H_LOG_WARN("Failed to open text file: %s", path.c_str());
		}

	}
}
