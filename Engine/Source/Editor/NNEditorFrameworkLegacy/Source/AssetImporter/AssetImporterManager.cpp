#include "AssetImporter/AssetImporterManager.h"
#include "EditorCore/ContentBrowser.h"
#include "NNCore/Include/Utils/HStringGenerator.h"

namespace NN::Editor
{
	void AssetImporterManager::Initialize(Ref<Runtime::VGWindow>& window)
	{
		m_EditorWindow = window;

		m_EditorWindow->AddWindowEventListener([this](const NN::Core::Events::HWindowEvent& evt)
			{
				switch (evt.eventType)
				{
				case NN::Core::Events::HWindowEventType::DROP_FILE:
					OnFileDropEvent(evt);
					break;
				//case NN::Core::Events::HWindowEventType::DROP_TEXT:
				//	OnFileDropEvent(evt);
				//	break;
				}
			});
	}

	AssetImporterManager& AssetImporterManager::GetInstance()
	{
		static AssetImporterManager s_AssetImporterManager;
		return s_AssetImporterManager;
	}

	void AssetImporterManager::OnFileDropEvent(const NN::Core::Events::HWindowEvent& evt)
	{
		NN::Core::HPath fromPath = evt.file;
		NN::Core::HPath toPath = ContentBrowser::GetInstance().GetCurrentBrowserDirectory();
		//String filename = fromPath.filename().string();
		//toPath = toPath / fromPath.filename();

		std::string outDirectory;
		std::string outFileName;
		std::string outFileExt;

		NN::Core::HFileSystem::SplitPath(fromPath, outDirectory, outFileName, outFileExt);

		// 检测是否源文件存在
		if (NN::Core::HFileSystem::ExistsFile(fromPath) == false)
			return;

		// 生成一个导入的资产路径
		NN::Core::HSequenceStringGenerator gen(outFileName);
		auto nextName = outFileName;
		auto fullPath = toPath / (nextName + outFileExt);
		while (NN::Core::HFileSystem::ExistsFile(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = toPath / (nextName + outFileExt);
		}

		// 复制资产
		NN::Core::HFileSystem::CopyFile(fromPath, fullPath);

		// 刷新内容浏览器目录
		ContentBrowser::GetInstance().RefreshDirectory();
	}
}

