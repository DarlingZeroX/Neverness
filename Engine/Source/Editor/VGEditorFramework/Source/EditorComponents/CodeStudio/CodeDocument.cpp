#include "EditorComponents/CodeStudio/CodeStudio.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGCore/Interface/VGAsset.h"
#include "VGEngine/Include/Asset/Package.h"
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal::Editor
{
	void CodeDocument::Initialize()
	{
		// 保存回调
		TexEditor.OnSave = [this](ImGuiTextEditor::TextEditor* editor, const std::string& path){
				DoSave();
			};

		ReadLastWriteTime();

		TexEditor.SetPalette(ImGuiTextEditor::TextEditor::GetDarkPalette());
		TexEditor.SetActiveAutocomplete(true);
		TexEditor.SetUIScale(1.1f);

		// 根据文件类型设置语法高亮
		VGAssetMetaData metadata;
		if (VGPackage::GetMeatData(DocPath, metadata))
		{
			if (metadata.AssetType == "HTML" || metadata.AssetType == "CSS")
			{
				auto lang = ImGuiTextEditor::LanguageDefinition::RmlUI();
				TexEditor.SetLanguageDefinition(lang);
			}

			if (metadata.AssetType == "LuaScript" || metadata.AssetType == "GalGameStoryScript")
			{
				auto lang = ImGuiTextEditor::LanguageDefinition::GalGameScript();
				TexEditor.SetLanguageDefinition(lang);
			}
		}
	}

	void CodeDocument::Update()
	{
		if (FirstUseEver)
		{
			TexEditor.ResetTextChanged();
			FirstUseEver = false;
		}

		if (TexEditor.IsTextChanged() == true)
		{
			TexEditor.SetErrorMarkers({});
			Dirty = true;
		}

		if (TexEditor.IsFocused())
		{
			ImGuiViewport* imViewport = ImGui::GetWindowViewport();
			SDL_WindowID windowID = reinterpret_cast<SDL_WindowID>(imViewport->PlatformHandle);
			SDL_StartTextInput(SDL_GetWindowFromID(windowID));
		}

		// 检测文件是否被外部修改
		auto absPath = VFS::GetInstance()->AbsolutePath(DocPath);
		if (Horizon::HFileSystem::ExistsFile(absPath))
		{
			if (FileLastWriteTime != std::filesystem::last_write_time(absPath))
			{
				OpenFile(DocPath);
			}
		}
	}

	CodeDocument::CodeDocument()
	{
		IsOpen = false;
		OpenPrev = false;
		Dirty = false;
		Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void CodeDocument::DoSave()
	{
		Dirty = false;

		if (DocPath.empty())
			return;
		VFS::WriteTextToFile(DocPath, TexEditor.GetText());

		TexEditor.ResetTextChanged();
		Dirty = false;

		// 获取文件最后写入时间
		ReadLastWriteTime();
	}

	void CodeDocument::ReadLastWriteTime()
	{
		auto absPath = VFS::GetInstance()->AbsolutePath(DocPath);
		if (Horizon::HFileSystem::ExistsFile(absPath))
		{
			FileLastWriteTime = std::filesystem::last_write_time(absPath);
		}
	}

	bool CodeDocument::OpenFile(const VGPath& path)
	{
		std::string text;
		auto result = VFS::ReadTextFromFile(path, text);

		if (result == false)
			return false;

		Name = Horizon::HFileSystem::GetFileNameFromPath(path);
		IsOpen = true;
		DocPath = path;
		Initialize();

		TexEditor.SetText(text);

		return true;
	}
}
