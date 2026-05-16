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
#include <unordered_map>
#include <vector>
#include "../Config.h"
#include <NNCore/Interface/HConfig.h>
#include <NNRuntimeImGui/IncludeImgui.h>

namespace NN::Editor
{
	class VG_EDITOR_FRAMEWORK_API PanelManager: public IPanel
	{
		PanelManager();
	public:
		PanelManager(const PanelManager&) = default;
		PanelManager& operator=(const PanelManager&) = default;
		PanelManager(PanelManager&&) noexcept = default;
		PanelManager& operator=(PanelManager&&) noexcept = default;
		~PanelManager() override = default;

		/// <summary>
		/// 添加一个UI面板
		/// </summary>
		/// <param name="panel">UI面板</param>
		void AddPanel(const Ref<IEditorPanel>& panel);

		/// <summary>
		/// 添加一个有ID的UI面板
		/// </summary>
		/// <param name="id">ID标识符</param>
		/// <param name="panel">UI面板</param>
		void AddPanelWithID(const std::string& id, const Ref<IEditorPanel>& panel);

		/// <summary>
		/// 获取一个带有ID的面板
		/// </summary>
		/// <param name="id">ID标识符</param>
		/// <returns></returns>
		IEditorPanel* GetPanelWithID(const std::string& id);

		bool OpenPanel(const std::string& id);

		// 继承至IPanel
		void OnGUI() override;
		void TickAsyncUI();
		void OnUpdate(float delta) override;
		void OnFixedUpdate() override;

		ImGuiWindowClass& GetImGuiWindowClass();
		ImGuiWindow* GetWindow();
		ImGuiID GetWindowDockID();

		static PanelManager* GetInstance();
	public:
		std::vector<Ref<IEditorPanel>> m_Panels;
		std::unordered_map<std::string, Ref<IEditorPanel>> m_IDPanels;

		ImGuiWindowClass m_TopLevelClass;
	};
}