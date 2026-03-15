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
#include "VGCore/Interface/GameInterface.h"
#include "../../Scene/Scene.h"

namespace VisionGal
{
    class VG_ENGINE_API SceneManager
    {
    public:
        SceneManager() = default;
        ~SceneManager() = default;
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        SceneManager(SceneManager&&) noexcept = default;
        SceneManager& operator=(SceneManager&&) noexcept = default;

		static SceneManager* Get();

        bool EnterPlayMode();		        // 从编辑器场景切换到运行时场景
        bool IsPlayMode() const;				// 检查当前是否处于播放模式
        bool ExitPlayMode();				        // 从运行时场景切换回编辑器场景

        bool SaveScene(Scene* scene, const String& path);	// 保存当前场景到指定路径
		Ref<IScene> LoadScene(const String& path);		// 从指定路径加载场景
		Ref<Scene> LoadNewScene();						// 加载新场景
		void LoadSceneOnUpdate(const String& path);
		 
        IScene* GetCurrentEditorScene() const;			// 获取当前编辑器场景
		IScene* GetCurrentRunningScene() const;			// 获取当前运行时场景

        void Update(float delta);

		bool SetCurrentScene(const Ref<IScene>& scene);		// 设置当前场景
    private:
        Ref<IScene> m_CurrentScene = nullptr;
		Ref<IScene> m_EditorScene = nullptr;
        bool m_IsPlayMode = false;

        std::unordered_map<String, Ref<IScene>> m_Scenes;
        Ref<IScene> m_NewScene;

		std::vector<std::function<void()>> m_UpdateTask;
    };


}
