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
#include "../PanelInterface.h"
#include "VGEditorCore/Include/EditorCore/ContentBrowser.h"
#include <VGEngine/Include/Render/Texture2D.h>

namespace VisionGal::Editor {
	class VG_EDITOR_FRAMEWORK_API ContentBrowserPanel: public IEditorPanel
	{
		using pfsPath = Horizon::fsPath;
	public:
		ContentBrowserPanel();
		ContentBrowserPanel(const pfsPath& path);
		ContentBrowserPanel(const ContentBrowserPanel&) = delete;
		ContentBrowserPanel& operator=(const ContentBrowserPanel&) = delete;
		ContentBrowserPanel(ContentBrowserPanel&&) noexcept = default;
		ContentBrowserPanel& operator=(ContentBrowserPanel&&) noexcept = default;
		~ContentBrowserPanel() override;

		void Toggle();
		void OnGUI() override;
		void OnUpdate(float delta) override;

		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
	private:
		void RefreshDirectory();

		// 左边的目录树 Left directory tree
		void DrawRootDirectoryTree();
		void DrawDirectoryTree(ContentBrowserDirectory& node);
		// 点击打开目录
		void DirectoryItemFunction(ContentBrowserDirectory& node);

		// 右边的内容浏览器 Right content browser
		void DrawContentBrowser();
		void DrawBrowserHeader();
		void DrawBrowserList();
		void DrawBrowserListItem();

		// 单个文件的功能 Item
		void DrawPath(const pfsPath& path);
		bool ItemFunction(ContentBrowserItem& item);
		void ItemBeginDragSource(const ContentBrowserItem& item);
		void ItemBeginDropTarget(const ContentBrowserItem& item);

		// 上下文菜单 Context menu
		bool ItemContextMenu(ContentBrowserItem& item);
		void ContentBrowserContextMenu(const pfsPath& path);
	private:
	//	bool ImageButton(DLG::ITextureView* view, float width, float height);
		static void* GetAssetThumbnail(ContentBrowserItem& item);
	public:
		struct ImContentBrowserWindow;
		ContentBrowser* m_pContentBrowser;

		bool m_ContentBrowserOpenFlag = true;
		bool m_IsAnyContentBrowserItemHovered = false;
	};
}
