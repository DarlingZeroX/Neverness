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

#pragma once
#include "../../Config.h"
#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNKernel/Interface/HSingleton.h>
//#include <VGAsset/Include/HAsset.h>
#include <VGCore/Include/Core/Core.h>
#include <VGCore/Interface/VGAsset.h>

namespace VisionGal {

	struct ContentBrowserItem
	{
		bool Selected = false;
		bool Renameing = false;

		std::string Name;
		int UIFlags = 0;
		Horizon::HPath AbsolutePath;
		std::string AbsolutePathStr;
		bool IsDirectory;
		VGPath Path;

		std::string Ext;
		VGAssetMetaData MetaData;
		std::string AssetType;

		void* iconView = nullptr;
	};

	struct ContentBrowserFile: public ContentBrowserItem
	{
		ContentBrowserFile()
		{
			IsDirectory = false;
		}
	};

	struct ContentBrowserDirectory: public ContentBrowserItem
	{
		ContentBrowserDirectory()
		{
			IsDirectory = true;
		}

		std::vector<ContentBrowserDirectory> Directories;
		std::vector<ContentBrowserFile> Files;
	};

	class VG_EDITOR_FRAMEWORK_API ContentBrowser
		:
		public Horizon::HSingletonBase<ContentBrowser>
	{
	public:
		using pfsPath = Horizon::fsPath;

		ContentBrowser(const pfsPath& path);
		ContentBrowser(const ContentBrowser&) = delete;
		ContentBrowser& operator=(const ContentBrowser&) = delete;
		ContentBrowser(ContentBrowser&&) noexcept = default;
		ContentBrowser& operator=(ContentBrowser&&) noexcept = default;
		~ContentBrowser() override;
		 
		static void Create(const pfsPath& path);

		// 刷新目录
		void RefreshDirectory();

		// 打开目录
		void OpenDirectory(const pfsPath& path);

		// 删除目录或者文件
		void DeleteDirectoryItem(const ContentBrowserItem& path);

		// 重命名目录或者文件
		void RenameDirectoryItem(const ContentBrowserItem& path, const std::string& name);

		// 递归删除目录以及目录文件
		void DeleteDirectoryItemRecursion(const pfsPath& path);

		// 在文件管理器中显示
		void ShowInExplorer(const pfsPath& path);

		// 创建新目录
		void CreateNewDirectory(const pfsPath& path);

		// 复杂路径
		void CopyPath(const pfsPath& path);

		// 选择当前路径下全部目录文件
		void SelectAll(bool select);

		// 刷新根目录树文件
		void RefreshDirectoryTreeRoot();

		// 刷新目录树
		void RefreshDirectoryTree(ContentBrowserDirectory& node, const pfsPath& path);

		// 获取工程目录路径
		const pfsPath& GetProjectDirectory();

		// 获取上个访问的目录
		const pfsPath& GetPrevDirectory();

		// 获取当前文件浏览器工作目录
		const pfsPath& GetCurrentBrowserDirectory();

		//std::vector<ContentBrowserFile>& GetCurrentDirectoryItems();
		// 获取当前文件浏览器工作目录所有目录和文件的节点
		ContentBrowserDirectory& GetCurrentDirectoryNode();

		// 获取根目录节点
		ContentBrowserDirectory& GetDirectoryTreeRootNode();

		// 是否存在子目录
		bool ExistChildDirectory(ContentBrowserDirectory& node, const pfsPath& path);

		bool IsRefreshed();

		void ClearRefreshedFlag();
	private:
		pfsPath m_ProjectDirectory;
		pfsPath m_PrevDirectory;
		pfsPath m_CurrentDirectory;

		//std::vector<ContentBrowserFile> m_CurrentDirectoryItems;	//当前工作目录项
		ContentBrowserDirectory m_DirectoryTreeRootNode;	// 根目录节点
		ContentBrowserDirectory m_CurrentDirectoryNode;

		bool m_IsRefreshed = false;

		std::mutex m_RefreshDirectoryMutex;
	};
}
