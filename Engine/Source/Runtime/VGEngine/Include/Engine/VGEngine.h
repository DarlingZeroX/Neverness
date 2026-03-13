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
#include "../Project/ProjectSettings.h"
#include "../EngineConfig.h"
#include "../Scene/Scene.h"
#include "../Interface/EngineInterface.h"
#include <HCore/Interface/HCore.h>

namespace VisionGal
{
	class VG_ENGINE_API VGEngine
	{
	public:
		VGEngine();
		VGEngine(const VGEngine&) = delete;
		VGEngine& operator=(const VGEngine&) = delete;
		VGEngine(VGEngine&&) noexcept = default;
		VGEngine& operator=(VGEngine&&) noexcept = default;
		~VGEngine() = default;

		static VGEngine* Get();
		bool LoadProject();
		ProjectSettings& GetProjectConfig();

		void LoadEditorMainScene();
		void LoadProjectMainScene();

		void Run();
		void AddApplication(Ref<IEngineApplication> layer);
		void OnApplicationUpdate(float deltaTime);
		bool ProcessEvents();

		void Shutdown();
		void RequestExit();
	private:
		void OnUpdateSubSystem(float deltaTime);
		void Initialize();
	private:
		ProjectSettings m_ProjectConfig;
	//	Ref<GameApplication> m_GameApplication;

		std::vector<Ref<IEngineApplication>> m_Applications;
		Ref<IScene> m_Scene;
		bool m_Running = true;
		bool m_PowerSave = false;

		// 在循环外部定义时间点变量（例如类成员变量）
		std::chrono::high_resolution_clock::time_point m_LastUpdateTime;
	};
}