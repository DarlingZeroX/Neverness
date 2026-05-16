#include "CodeStudio/DocumentManager.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeAsset/Interface/Package.h"
#include "NNRuntimeCore/Include/Core/VFS.h"

namespace VisionGal::Editor
{
	bool DocumentManager::OpenTextFile(const VGPath& path)
	{
		if (ExistDocument(path))
		{
			SetDocumentActive(path);
			return true;
		}

		auto doc = MakeRef<CodeDocument>();
		if (doc->OpenFile(path) == false)
		{
			return false;
		}

		AddDocument(doc);
		SetDocumentActive(path);

		return true;
	}

	CodeDocument* DocumentManager::GetDocument(const VGPath& path)
	{
		for (auto& doc : Documents)
		{
			if (doc->DocPath == path)
			{
				return doc.get();
			}
		}
		return nullptr;
	}

	void DocumentManager::AddDocument(const Ref<CodeDocument>& document)
	{
		FileList.insert(document->DocPath);

		document->UID = NextUID++;
		Documents.push_back(document);
	}

	bool DocumentManager::ExistDocument(const std::string& path)
	{
		if (FileList.find(path) == FileList.end())
			return false;

		for (auto it = Documents.begin(); it != Documents.end(); ++it)
		{
			// 安全的路径比较
			if ((*it)->DocPath == path)
			{
				return true;
			}
		}

		return false;
	}

	bool DocumentManager::CloseDocument(const std::string& path)
	{
		// 从 Documents 中移除匹配的文档（按路径或按指针）
		for (auto it = Documents.begin(); it != Documents.end(); ++it)
		{
			// 安全的路径比较
			if ((*it)->DocPath == path)
			{
				it = Documents.erase(it);
				break;
			}
		}

		return true;
	}

	bool DocumentManager::CloseDocument(CodeDocument* docPtr)
	{
		if (docPtr == nullptr)
			return false;

		return CloseDocument(docPtr->DocPath);
	}

	bool DocumentManager::SaveDocument(const std::string& path)
	{
		// 若文档存在且被修改，则保存
		for (auto& doc : Documents)
		{
			if (doc->DocPath == path && doc->Dirty)
			{
				doc->DoSave();
				break;
			}
		}

		return true;
	}

	bool DocumentManager::SaveDocument(CodeDocument* docPtr)
	{
		if (docPtr == nullptr)
			return false;

		return SaveDocument(docPtr->DocPath);
	}

	bool DocumentManager::ForceRemoveDocument(const std::string& path)
	{
		CloseDocument(path);

		// 移除文件记录
		FileList.erase(path);

		return true;
	}

	bool DocumentManager::ForceRemoveDocument(CodeDocument* docPtr)
	{
		if (docPtr == nullptr)
			return false;

		return ForceRemoveDocument(docPtr->DocPath);
	}

	void DocumentManager::SaveAllDocument()
	{
		for (auto& doc : Documents)
		{
			if (doc->Dirty)
			{
				doc->DoSave();
			}
		}
	}

	bool DocumentManager::SetDocumentActive(const VGPath& path)
	{
		for (auto& doc : Documents)
		{
			if (doc->DocPath == path)
			{
				doc->IsOpen = true;
				doc->NeedFocus = true;
				return true;
			}
		}

		return false;
	}
}
