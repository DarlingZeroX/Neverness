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

#include "Engine/Manager/SceneManager.h"

#include "Asset/Package.h"
#include "Asset/Accessor/SceneAccessor.h"
#include "Core/EventBus.h"
#include "Scene/GameActorFactory.h"

namespace VisionGal
{
	//bool SceneManager::EnterPlayModeApplicaiotn()
	//{
	//	// 已经在播放模式中，不需要再次进入
	//	if (IsPlayMode())
	//		return true;
	//
	//	if (m_CurrentScene == nullptr)
	//		return false;
	//
	//	// 开始播放场景事件
	//	if (IsPlayMode())
	//	{
	//		EngineEvent evt;
	//		evt.EventType = EngineEventType::EnterScenePlayMode;
	//		evt.Scene = m_CurrentScene.get();
	//		EngineEventBus::Get().OnEngineEvent.Invoke(evt);
	//	}
	//
	//	m_IsPlayMode = true;
	//}

	bool SceneManager::EnterPlayMode()
	{
		// 已经在播放模式中，不需要再次进入
		if (IsPlayMode())
			return true;

		if (m_CurrentScene == nullptr)
			return false;

		m_IsPlayMode = true;
		m_EditorScene = m_CurrentScene;
		SaveScene(dynamic_cast<Scene*>(m_CurrentScene.get()),  Core::GetProjectIntermediatePathVFS() + "runtimeScene.vgasset");
		LoadScene(Core::GetProjectIntermediatePathVFS() + "runtimeScene.vgasset");

		return true;
	}

	bool SceneManager::IsPlayMode() const
	{
		return m_IsPlayMode;
	}

	bool SceneManager::ExitPlayMode()
	{
		// 如果不是播放模式，则不需要退出
		if (!IsPlayMode())
			return true;

		if (m_CurrentScene == nullptr)
			return false;

		// 退出播放场景事件
		EngineEvent evt;
		evt.EventType = EngineEventType::ExitScenePlayMode;
		evt.Scene = m_CurrentScene.get();
		EngineEventBus::Get().OnEngineEvent.Invoke(evt);

		m_IsPlayMode = false;
		SetCurrentScene(m_EditorScene);

		return true;
	}

	bool SceneManager::SaveScene(Scene* scene, const String& path)
	{
		if (scene == nullptr)
			return false;

		SceneAssetWriter writer;
		SceneAsset asset;
		asset.WriteScene = scene;

		if ( writer.Write(path, &asset))
		{
			return true;
		}

		return false;
	}

	bool SceneManager::SetCurrentScene(const Ref<IScene>& scene)
	{
		// 如果传入的场景为空，则不进行任何操作
		if (scene == nullptr)
			return false;

		// 如果传入的场景与当前场景相同，则不进行任何操作
		if (m_CurrentScene == scene)
			return true;

		if (IsPlayMode() == false)
		{
			m_EditorScene = scene;
		}

		// 这里传值，因为当前场景下一条命令会赋值
		auto prevScene = m_CurrentScene;
		m_CurrentScene = scene;

		// 场景切换事件
		{
			EngineEvent evt;
			evt.EventType = EngineEventType::MainSceneChanged;
			evt.Scene = m_CurrentScene.get();
			evt.PrevScene = prevScene.get();
			EngineEventBus::Get().OnEngineEvent.Invoke(evt);
		}

		// 开始播放场景事件
		if (IsPlayMode())
		{
			EngineEvent evt;
			evt.EventType = EngineEventType::EnterScenePlayMode;
			evt.Scene = m_CurrentScene.get();
			EngineEventBus::Get().OnEngineEvent.Invoke(evt);
		}

		return true;
	}

	Ref<IScene> SceneManager::LoadScene(const String& path)
	{
		if (m_CurrentScene && m_CurrentScene->GetResourcePath() == path)
		{
			return m_Scenes[path];
		}

		Ref<VGAsset> asset;
		SceneAssetLoader loader;
		loader.Read(path, asset);

		Ref<SceneAsset> sceneAsset = std::dynamic_pointer_cast<SceneAsset>(asset);

		if (sceneAsset == nullptr || sceneAsset->LoadedScene == nullptr)
		{
			H_LOG_ERROR("Failed to load the scene: %s", path.c_str());
			return nullptr;
		}

		auto lastScene = m_CurrentScene;

		H_LOG_INFO("The scene was loaded successfully: %s", path.c_str());
		sceneAsset->LoadedScene->SetResourcePath(path);
		m_Scenes[path] = sceneAsset->LoadedScene;
		SetCurrentScene(sceneAsset->LoadedScene);

		if (IsPlayMode())
		{
			if (lastScene != nullptr)
			{
				auto path = lastScene->GetResourcePath();
				m_UpdateTask.push_back([this, path]()
					{
						m_Scenes.erase(path);
					});
			}
		}

		return sceneAsset->LoadedScene;
	}

	Ref<Scene> SceneManager::LoadNewScene()
	{
		Ref<Scene> scene = CreateRef<Scene>();
		GetGameActorFactory()->CreateActor(scene.get(), "camera");

		m_NewScene = scene;
		SetCurrentScene(scene);
		return scene;
	}

	IScene* SceneManager::GetCurrentEditorScene() const
	{
		if (m_EditorScene)
			return m_EditorScene.get();

		return nullptr;
	}

	IScene* SceneManager::GetCurrentRunningScene() const
	{
		if (m_CurrentScene)
			return m_CurrentScene.get();

		return nullptr;
	}

	SceneManager* SceneManager::Get()
	{
		static SceneManager manager;
		return &manager;
	}

	void SceneManager::Update(float delta)
	{
		if (m_CurrentScene == nullptr)
			return;

		for (auto& task: m_UpdateTask)
		{
			task();
		}
	}
}
