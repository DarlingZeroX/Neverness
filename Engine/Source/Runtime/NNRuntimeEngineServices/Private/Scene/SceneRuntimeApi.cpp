/**
 * @file SceneRuntimeApi.cpp
 * @brief **NNSceneAPI** Runtime 轉發至 **NNEngineRuntime::Scene()**。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

int NN_ENGINE_ABI_STDCALL rt_scene_loadScene(const char* sceneNameUtf8)
{
	return NNEngineRuntime::Instance().Scene().LoadScene(sceneNameUtf8);
}

NNEntityHandle NN_ENGINE_ABI_STDCALL rt_scene_spawn(const char* prefabVirtualPathUtf8)
{
	return NNEngineRuntime::Instance().Scene().Spawn(prefabVirtualPathUtf8);
}

void NN_ENGINE_ABI_STDCALL rt_scene_destroy(NNEntityHandle entity)
{
	NNEngineRuntime::Instance().Scene().Destroy(entity);
}

NNEntityHandle NN_ENGINE_ABI_STDCALL rt_scene_find(const char* entityNameUtf8)
{
	return NNEngineRuntime::Instance().Scene().Find(entityNameUtf8);
}

void NN_ENGINE_ABI_STDCALL rt_scene_activate(NNEntityHandle entity, int active)
{
	NNEngineRuntime::Instance().Scene().Activate(entity, active);
}

int NN_ENGINE_ABI_STDCALL rt_scene_unloadScene(const char* sceneNameUtf8)
{
	return NNEngineRuntime::Instance().Scene().UnloadScene(sceneNameUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_scene_getActiveSceneName(char* outUtf8, std::size_t outCapacity)
{
	return NNEngineRuntime::Instance().Scene().GetActiveSceneName(outUtf8, outCapacity);
}

void NN_ENGINE_ABI_STDCALL rt_scene_setParent(NNEntityHandle child, NNEntityHandle parent)
{
	NNEngineRuntime::Instance().Scene().SetParent(child, parent);
}

NNEntityHandle NN_ENGINE_ABI_STDCALL rt_scene_getParent(NNEntityHandle entity)
{
	return NNEngineRuntime::Instance().Scene().GetParent(entity);
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_scene_getChildCount(NNEntityHandle entity)
{
	return NNEngineRuntime::Instance().Scene().GetChildCount(entity);
}

NNEntityHandle NN_ENGINE_ABI_STDCALL rt_scene_getChildAt(NNEntityHandle entity, std::uint32_t index)
{
	return NNEngineRuntime::Instance().Scene().GetChildAt(entity, index);
}

void NN_ENGINE_ABI_STDCALL rt_scene_getTransform(NNEntityHandle entity, NNTransform3* outTransform)
{
	NNEngineRuntime::Instance().Scene().GetTransform(entity, outTransform);
}

void NN_ENGINE_ABI_STDCALL rt_scene_setTransform(NNEntityHandle entity, const NNTransform3* transform)
{
	NNEngineRuntime::Instance().Scene().SetTransform(entity, transform);
}

int NN_ENGINE_ABI_STDCALL rt_scene_setEntityName(NNEntityHandle entity, const char* nameUtf8)
{
	return NNEngineRuntime::Instance().Scene().SetEntityName(entity, nameUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_scene_getEntityName(NNEntityHandle entity, char* outUtf8, std::size_t outCapacity)
{
	return NNEngineRuntime::Instance().Scene().GetEntityName(entity, outUtf8, outCapacity);
}
} // namespace

extern "C" void NNBuildSceneRuntimeApi(NNSceneAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->loadScene = &rt_scene_loadScene;
	api->spawn = &rt_scene_spawn;
	api->destroy = &rt_scene_destroy;
	api->find = &rt_scene_find;
	api->activate = &rt_scene_activate;
	api->unloadScene = &rt_scene_unloadScene;
	api->getActiveSceneName = &rt_scene_getActiveSceneName;
	api->setParent = &rt_scene_setParent;
	api->getParent = &rt_scene_getParent;
	api->getChildCount = &rt_scene_getChildCount;
	api->getChildAt = &rt_scene_getChildAt;
	api->getTransform = &rt_scene_getTransform;
	api->setTransform = &rt_scene_setTransform;
	api->setEntityName = &rt_scene_setEntityName;
	api->getEntityName = &rt_scene_getEntityName;
	api->serializeScene = nullptr;
	api->deserializeScene = nullptr;
}
