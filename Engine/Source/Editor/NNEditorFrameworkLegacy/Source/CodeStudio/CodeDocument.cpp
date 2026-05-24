#include "CodeStudio/CodeStudio.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeAssetLegacy/Interface/Package.h"
#include "NNRuntimeVFS/Include/VFSService.h"

namespace NN::Editor
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
		Runtime::VGAssetMetaData metadata;
		if (Runtime::VGPackage::GetMeatData(DocPath, metadata))
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
		auto absPath = Runtime::VFS::VFSService::GetInstance()->AbsolutePath(DocPath);
		if (NN::Core::HFileSystem::ExistsFile(absPath))
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
		Runtime::VFS::VFSService::WriteTextToFile(DocPath, TexEditor.GetText());

		TexEditor.ResetTextChanged();
		Dirty = false;

		// 获取文件最后写入时间
		ReadLastWriteTime();
	}

	void CodeDocument::ReadLastWriteTime()
	{
		auto absPath = Runtime::VFS::VFSService::GetInstance()->AbsolutePath(DocPath);
		if (NN::Core::HFileSystem::ExistsFile(absPath))
		{
			FileLastWriteTime = std::filesystem::last_write_time(absPath);
		}
	}

	bool CodeDocument::OpenFile(const Runtime::VGPath& path)
	{
		std::string text;
		auto result = Runtime::VFS::VFSService::ReadTextFromFile(path, text);

		if (result == false)
			return false;

		Name = NN::Core::HFileSystem::GetFileNameFromPath(path);
		IsOpen = true;
		DocPath = path;
		Initialize();

		TexEditor.SetText(text);

		return true;
	}
}
