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

#include "EditorCore/ContentBrowser.h"
#include <NNRuntimeAssetLegacy/Interface/Package.h>
#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNCore/Interface/HStringTools.h>
#include <NNPlatformCore/Interface/HClipboard.h>
#include <NNPlatformCore/Interface/FileSystem/HFileSystemGenerator.h>
//#include <NNRuntimeAssetLegacy/Include/HPackage.h>
//#include "Engine/Manager/AssetManager.h"
#include "NNRuntimeVFS/Include/VFSService.h"
#include "NNRuntimeVFS/Include/VFS/VFSNative.h"
#include <NNRuntimeCore/Include/Core/RuntimeCore.h>
#include "NNRuntimeAssetLegacy/Interface/AssetManager.h"

namespace NN::Editor{

	ContentBrowser::ContentBrowser(const pfsPath& path)
		:m_ProjectDirectory(path)
	{
		OpenDirectory(m_ProjectDirectory);
		RefreshDirectoryTreeRoot();
	}

	ContentBrowser::~ContentBrowser()
	{
	}

	void ContentBrowser::Create(const pfsPath& path)
	{
		if (GetInstanceValid())
			return;

		new ContentBrowser(path);
	}

	void ContentBrowser::RefreshDirectoryTreeRoot()
	{
		m_IsRefreshed = true;

		m_DirectoryTreeRootNode = {};
		m_DirectoryTreeRootNode.Name = "Content";
		m_DirectoryTreeRootNode.AbsolutePath = m_ProjectDirectory;
		m_DirectoryTreeRootNode.UIFlags = ImGuiTreeNodeFlags_SpanFullWidth;

		RefreshDirectoryTree(m_DirectoryTreeRootNode, m_ProjectDirectory);
	}

	bool ContentBrowser::ExistChildDirectory(ContentBrowserDirectory& node, const pfsPath& path)
	{
		for (auto& child : node.Directories)
		{
			if (child.AbsolutePath == path)
				return true;
		}

		return false;
	}

	bool ContentBrowser::IsRefreshed()
	{
		return m_IsRefreshed;
	}

	void ContentBrowser::ClearRefreshedFlag()
	{
		m_IsRefreshed = false;
	}

	void ContentBrowser::RefreshDirectoryTree(ContentBrowserDirectory& node, const pfsPath& path)
	{
		if (NN::Core::HFileSystem::ExistsDirectory(path) == false)
		{
			H_LOG_ERROR("[ContentBrowser::RefreshDirectoryTree] The directory tree does not exist: %s", path.string().c_str());
			return;
		}

		m_IsRefreshed = true;
		std::vector<ContentBrowserDirectory> temporaryContainer;

		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
				continue;

			// 直接使用std::filesystem::relative速度太慢
			//pfsPath relativePath = NN::Core::HFileSystem::RelativePath(entry.path(), m_ProjectDirectory);
			// 不触发文件状态检查或符号链接处理，仅做字符串解析
			//const auto absPath = std::filesystem::absolute(entry.path());
			//pfsPath relativePath = absPath.lexically_relative(m_ProjectDirectory);

			auto absPath = entry.path();

			ContentBrowserDirectory child;

			int chlidFlags = ImGuiTreeNodeFlags_SpanFullWidth;
			if (entry == m_CurrentDirectory)
				chlidFlags = chlidFlags|ImGuiTreeNodeFlags_Selected;

			if (NN::Core::HFileSystem::DirectoryEmpty(entry))
				chlidFlags |= ImGuiTreeNodeFlags_Leaf;

			std::string nodeName = ICON_FA_FOLDER " ";
			child.Name += absPath.filename().string();
			child.UIFlags = chlidFlags;
			child.AbsolutePath = entry.path();
			child.Path = NN::Runtime::RuntimeCore::GetResourcePathVFS(path.string());

			if (!ExistChildDirectory(node, entry.path()))
			{
				//temporaryContainer.emplace_back(child);
				node.Directories.emplace_back(child);
			}
		}

		//// 多线程合并优化
		//m_RefreshDirectoryMutex.lock();
		//node.Directories.insert(node.Directories.end(), temporaryContainer.begin(), temporaryContainer.end());
		//m_RefreshDirectoryMutex.unlock();
		//
		//if (node.Directories.empty())
		//	return;
		//
		//// 用多线程进行加速
		//std::vector<std::thread> threads;
		//
		//for (auto& subchild : node.Directories)
		//{
		//	threads.emplace_back([this, &subchild]()
		//		{
		//			RefreshDirectoryTree(subchild, subchild.AbsolutePath);
		//		});
		//}
		//
		//// 等待所有线程完成
		//for (auto& t : threads) {
		//	if (t.joinable()) {
		//		t.join();
		//	}
		//}

		for (auto& subchild : node.Directories)
		{
			RefreshDirectoryTree(subchild, subchild.AbsolutePath);
		}

		return;
	}

	const NN::Core::fsPath& ContentBrowser::GetProjectDirectory()
	{
		return m_ProjectDirectory;
	}

	const NN::Core::fsPath& ContentBrowser::GetPrevDirectory()
	{
		return m_PrevDirectory;
	}

	const NN::Core::fsPath& ContentBrowser::GetCurrentBrowserDirectory()
	{
		return m_CurrentDirectory;
	}

	ContentBrowserDirectory& ContentBrowser::GetCurrentDirectoryNode()
	{
		return m_CurrentDirectoryNode;
	}

	ContentBrowserDirectory& ContentBrowser::GetDirectoryTreeRootNode()
	{
		return m_DirectoryTreeRootNode;
	}

	////////////////////////////////////////////////////////////////
	///		Content	Browser	Function
	////////////////////////////////////////////////////////////////
	void ContentBrowser::RefreshDirectory()
	{
		// 需要刷新虚拟文件系统
		auto fsList = Runtime::VFS::VFSService::GetInstance()->GetFilesystems(Runtime::RuntimeCore::GetAssetsPathVFS());
		if (fsList.size() == 1)
		{
			auto fs = fsList.begin()->get();
			NN::Runtime::VFS::NativeFileSystem* nfs = dynamic_cast<NN::Runtime::VFS::NativeFileSystem*>(fs);
			nfs->RebuildFileList();
		}

		m_IsRefreshed = true;
		OpenDirectory(m_CurrentDirectory);
	}

	void ContentBrowser::OpenDirectory(const pfsPath& path)
	{
		if (NN::Core::HFileSystem::ExistsDirectory(path) == false)
		{
			H_LOG_ERROR("[ContentBrowser::OpenDirectory] The directory does not exist: %s", path.string().c_str());
			return;
		}

		m_PrevDirectory = m_CurrentDirectory;
		m_CurrentDirectory = path;
		m_CurrentDirectoryNode = {};

		for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			//pfsPath relativePath = NN::Core::HFileSystem::RelativePath(entry.path(), m_ProjectDirectory);

			if (entry.is_directory())
			{
				ContentBrowserDirectory directory;
				auto path = entry.path();
				int uiFlags = ImGuiTreeNodeFlags_SpanFullWidth;

				// 目录是空的
				if (NN::Core::HFileSystem::DirectoryEmpty(entry))
				{
					uiFlags |= ImGuiTreeNodeFlags_Leaf;
				}

				std::string nodeName = ICON_FA_FOLDER " ";
				directory.Name += path.filename().string();
				directory.UIFlags = uiFlags;
				directory.AbsolutePath = entry.path();
				directory.AbsolutePathStr = directory.AbsolutePath.string();
				directory.IsDirectory = true;
				directory.Path = Runtime::RuntimeCore::GetResourcePathVFS(directory.AbsolutePathStr);

				m_CurrentDirectoryNode.Directories.emplace_back(directory);
			}
			else
			{
				ContentBrowserFile file;

				std::string outDirectory;
				std::string name;
				std::string ext;

				NN::Core::HFileSystem::SplitPath(entry.path(), &outDirectory, &name, &ext);

				file.AbsolutePath = entry.path();
				file.AbsolutePathStr = file.AbsolutePath.string();
				file.Ext = file.AbsolutePath.extension().string();
				file.Name = name;
				file.UIFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;
				file.IsDirectory = false;
				file.Path = Runtime::RuntimeCore::GetResourcePathVFS(file.AbsolutePathStr);

				Runtime::VGAssetMetaData data;
				if (Runtime::VGPackage::GetMeatData(file.Path, data))
				{
					file.MetaData = data;
					file.AssetType = data.AssetType;

					m_CurrentDirectoryNode.Files.emplace_back(file);
				}
			}
		}

		return;
	}

	void ContentBrowser::DeleteDirectoryItem(const ContentBrowserItem& item)
	{
		// 目录 Directory
		if (item.IsDirectory)
		{
			if (!NN::Core::HFileSystem::DirectoryEmpty(item.AbsolutePath))
			{
				DeleteDirectoryItemRecursion(item.AbsolutePath);
			}

			NN::Core::HFileSystem::RemoveDirectory(item.AbsolutePath);

			RefreshDirectory();
			RefreshDirectoryTreeRoot();

			return;
		}

		// 文件 File
		Runtime::AssetManager::GetInstance()->RemoveAsset(item.Path);
		RefreshDirectory();
	}

	void ContentBrowser::RenameDirectoryItem(const ContentBrowserItem& path, const std::string& name)
	{
		auto file = name + path.Ext;
		auto metaFile = file + ".meta";
		auto newPath = path.AbsolutePath.parent_path() / file;
		auto newMetaPath = path.AbsolutePath.parent_path() / metaFile;

		if (!NN::Core::HFileSystem::ExistsDirectory(newPath))
		{
			// remove data file
			try {
				//std::filesystem::remove(aPath);
				NN::Core::HFileSystem::MoveFile(path.AbsolutePath, newPath);

				pfsPath oMetaPath = path.AbsolutePath.string() + ".meta";
				if (NN::Core::HFileSystem::ExistsDirectory(oMetaPath))
				{
					NN::Core::HFileSystem::MoveFile(oMetaPath, newMetaPath);
				}
			}
			catch (const std::filesystem::filesystem_error& ex) {
				H_LOG_ERROR("std::filesystem::remove error: %s", ex.what());
				return;
			}
			catch (const std::exception& ex) {
				H_LOG_ERROR("Standard exception: %s", ex.what());
				return;
			}
			catch (...) {
				H_LOG_ERROR("Unknown std::filesystem::remove exception occurred");
				return;
			}

			RefreshDirectory();
			RefreshDirectoryTreeRoot();
		}

		return;
	}

	void ContentBrowser::DeleteDirectoryItemRecursion(const pfsPath& path)
	{
		for (auto& directoryEntry : std::filesystem::directory_iterator(path))
		{
			if (directoryEntry.is_directory())
			{
				DeleteDirectoryItemRecursion(directoryEntry.path());

				NN::Core::HFileSystem::RemoveDirectory(directoryEntry.path());
			}
			else
			{
				NN::Core::HFileSystem::RemoveFile(directoryEntry.path());
			}
		}
	}

	void ContentBrowser::ShowInExplorer(const pfsPath& path)
	{
		NN::Core::HFileSystem::OpenSystemExplorerFolder(path);
	}

	void ContentBrowser::CreateNewDirectory(const pfsPath& path)
	{
		NN::Core::HSequenceGenerator::GenerateDirectory(path / L"New Folder ");

		RefreshDirectory();
		RefreshDirectoryTreeRoot();
	}

	void ContentBrowser::CopyPath(const pfsPath& path)
	{
		NN::Core::HClipboard::SetText(path.string().c_str());
	}

	void ContentBrowser::SelectAll(bool select)
	{
		for (auto& item : m_CurrentDirectoryNode.Directories)
		{
			item.Selected = select;
		}

		for (auto& item : m_CurrentDirectoryNode.Files)
		{
			item.Selected = select;
		}
	}
}
