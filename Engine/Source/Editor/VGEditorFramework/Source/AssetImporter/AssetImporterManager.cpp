#include "AssetImporter/AssetImporterManager.h"
#include "EditorCore/ContentBrowser.h"
#include "NNKernel/Include/Utils/HStringGenerator.h"

namespace VisionGal::Editor
{
	void AssetImporterManager::Initialize(Ref<VGWindow>& window)
	{
		m_EditorWindow = window;

		m_EditorWindow->AddWindowEventListener([this](const Horizon::Events::HWindowEvent& evt)
			{
				switch (evt.eventType)
				{
				case Horizon::Events::HWindowEventType::DROP_FILE:
					OnFileDropEvent(evt);
					break;
				//case Horizon::Events::HWindowEventType::DROP_TEXT:
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

	void AssetImporterManager::OnFileDropEvent(const Horizon::Events::HWindowEvent& evt)
	{
		Horizon::HPath fromPath = evt.file;
		Horizon::HPath toPath = ContentBrowser::GetInstance().GetCurrentBrowserDirectory();
		//String filename = fromPath.filename().string();
		//toPath = toPath / fromPath.filename();

		std::string outDirectory;
		std::string outFileName;
		std::string outFileExt;

		Horizon::HFileSystem::SplitPath(fromPath, outDirectory, outFileName, outFileExt);

		// 检测是否源文件存在
		if (Horizon::HFileSystem::ExistsFile(fromPath) == false)
			return;

		// 生成一个导入的资产路径
		Horizon::HSequenceStringGenerator gen(outFileName);
		auto nextName = outFileName;
		auto fullPath = toPath / (nextName + outFileExt);
		while (Horizon::HFileSystem::ExistsFile(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = toPath / (nextName + outFileExt);
		}

		// 复制资产
		Horizon::HFileSystem::CopyFile(fromPath, fullPath);

		// 刷新内容浏览器目录
		ContentBrowser::GetInstance().RefreshDirectory();
	}
}

