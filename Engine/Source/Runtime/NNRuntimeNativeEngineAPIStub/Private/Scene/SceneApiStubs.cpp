/**
 * @file SceneApiStubs.cpp
 * @brief **NNSceneAPI** 預設 Stub：記憶體內最小場景語意，供未鏈結 **NNEngineRuntime** 之測試。
 */

#include <atomic>
#include <cstdint>
#include <cstring>

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
std::atomic<std::uint64_t> g_nextEntityStubId{1};

int NN_ENGINE_ABI_STDCALL stub_scene_loadScene(const char* sceneNameUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)sceneNameUtf8;
	return 1;
}

NNEntityHandle NN_ENGINE_ABI_STDCALL stub_scene_spawn(const char* prefabVirtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)prefabVirtualPathUtf8;
	const std::uint64_t id = g_nextEntityStubId.fetch_add(1u, std::memory_order_relaxed);
	return id == 0u ? static_cast<NNEntityHandle>(0) : static_cast<NNEntityHandle>(id);
}

void NN_ENGINE_ABI_STDCALL stub_scene_destroy(NNEntityHandle entity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
}

NNEntityHandle NN_ENGINE_ABI_STDCALL stub_scene_find(const char* entityNameUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entityNameUtf8;
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_scene_activate(NNEntityHandle entity, int active)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	(void)active;
}

int NN_ENGINE_ABI_STDCALL stub_scene_unloadScene(const char* sceneNameUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)sceneNameUtf8;
	return -1;
}

int NN_ENGINE_ABI_STDCALL stub_scene_getActiveSceneName(char* outUtf8, std::size_t outCapacity)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	outUtf8[0] = '\0';
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_scene_setParent(NNEntityHandle child, NNEntityHandle parent)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)child;
	(void)parent;
}

NNEntityHandle NN_ENGINE_ABI_STDCALL stub_scene_getParent(NNEntityHandle entity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	return 0;
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_scene_getChildCount(NNEntityHandle entity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	return 0u;
}

NNEntityHandle NN_ENGINE_ABI_STDCALL stub_scene_getChildAt(NNEntityHandle entity, std::uint32_t index)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	(void)index;
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_scene_getTransform(NNEntityHandle entity, NNTransform3* outTransform)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	if (outTransform != nullptr)
	{
		std::memset(outTransform, 0, sizeof(NNTransform3));
		outTransform->scale[0] = outTransform->scale[1] = outTransform->scale[2] = 1.f;
	}
}

void NN_ENGINE_ABI_STDCALL stub_scene_setTransform(NNEntityHandle entity, const NNTransform3* transform)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	(void)transform;
}

int NN_ENGINE_ABI_STDCALL stub_scene_setEntityName(NNEntityHandle entity, const char* nameUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	(void)nameUtf8;
	return -1;
}

int NN_ENGINE_ABI_STDCALL stub_scene_getEntityName(NNEntityHandle entity, char* outUtf8, std::size_t outCapacity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)entity;
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	outUtf8[0] = '\0';
	return 0;
}
} // namespace

extern "C" void NNBuildSceneApiStubs(NNSceneAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->loadScene = &stub_scene_loadScene;
	api->spawn = &stub_scene_spawn;
	api->destroy = &stub_scene_destroy;
	api->find = &stub_scene_find;
	api->activate = &stub_scene_activate;
	api->unloadScene = &stub_scene_unloadScene;
	api->getActiveSceneName = &stub_scene_getActiveSceneName;
	api->setParent = &stub_scene_setParent;
	api->getParent = &stub_scene_getParent;
	api->getChildCount = &stub_scene_getChildCount;
	api->getChildAt = &stub_scene_getChildAt;
	api->getTransform = &stub_scene_getTransform;
	api->setTransform = &stub_scene_setTransform;
	api->setEntityName = &stub_scene_setEntityName;
	api->getEntityName = &stub_scene_getEntityName;
}
