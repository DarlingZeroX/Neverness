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
#include "Interface.h"
#include "DrawerManager.h"
#include <VGEngine/Include/Scene/Scene.h>

namespace VisionGal::Editor {

	class VG_EDITOR_FRAMEWORK_API DetailBrowserPanel : public IEditorPanel
	{
	public:
		DetailBrowserPanel();
		DetailBrowserPanel(const DetailBrowserPanel&) = default;
		DetailBrowserPanel& operator=(const DetailBrowserPanel&) = default;
		DetailBrowserPanel(DetailBrowserPanel&&) noexcept = default;
		DetailBrowserPanel& operator=(DetailBrowserPanel&&) noexcept = default;
		~DetailBrowserPanel() override = default;

		void OnGUI() override;

		void SubscribeEngineEvent();
		void SubscribeSceneEvent();
		void BeginDropTarget();

		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
	private:
		//GameActor* m_pSelectedEntity;
		EntityID m_SelectEntityID;
		Scene* m_pScene;
		bool m_IsOpen = true;

		ComponentDrawerManager m_DrawerManager;
	};
}