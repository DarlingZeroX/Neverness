/**
 * @file SceneApiStubs.cpp
 * @brief NNSceneAPI 预设 Stub：内存内最小场景语义，供未链接 NNEngineRuntime 之测试。
 *
 * 所有 stub 函数签名对齐新 ABI（layoutVersion = 4）：
 * - 返回值统一为 NNSceneResult
 * - NNSceneHandle = uint64_t
 * - 组件操作基于 FNV-1a name hash
 */

#include <atomic>
#include <cstdint>
#include <cstring>

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/SceneAPI.h"

namespace
{
std::atomic<std::uint64_t> g_nextEntityStubId{1};

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_createScene(NNSceneHandle* outScene)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outScene != nullptr)
	{
		*outScene = 1; /* 合成场景句柄 */
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_destroyScene(NNSceneHandle scene)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_tickSystems(NNSceneHandle scene, float deltaTime)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)deltaTime;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_createEntity(NNSceneHandle scene, NNEntityHandle* outEntity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	if (outEntity != nullptr)
	{
		const std::uint64_t id = g_nextEntityStubId.fetch_add(1u, std::memory_order_relaxed);
		*outEntity = (id == 0u) ? 0u : id;
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_destroyEntity(NNSceneHandle scene, NNEntityHandle entity)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_addComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_removeComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_hasComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, int32_t* outHas)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	if (outHas != nullptr)
	{
		*outHas = 0; /* Stub 假装没有组件 */
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_getComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, void* outData, uint32_t dataSize)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	if (outData != nullptr && dataSize > 0)
	{
		std::memset(outData, 0, dataSize);
	}
	return NN_SCENE_ERR_NOT_FOUND; /* Stub 不持有组件数据 */
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_setComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, const void* data, uint32_t dataSize)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	(void)data;
	(void)dataSize;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_setParent(NNSceneHandle scene, NNEntityHandle child, NNEntityHandle parent)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)child;
	(void)parent;
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_getParent(NNSceneHandle scene, NNEntityHandle entity, NNEntityHandle* outParent)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)entity;
	if (outParent != nullptr)
	{
		*outParent = 0;
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_serializeScene(
	NNSceneHandle scene, const char* vfsPath)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)vfsPath;
	return NN_SCENE_ERR_IO; /* Stub 不支持序列化 */
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_deserializeScene(
	NNSceneHandle* outScene, const char* vfsPath)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)outScene;
	(void)vfsPath;
	return NN_SCENE_ERR_IO; /* Stub 不支持反序列化 */
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_queryEntities(
	NNSceneHandle scene, uint64_t componentTypeId,
	NNEntityHandle* outEntities, uint32_t maxCount, uint32_t* outCount)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)componentTypeId;
	(void)outEntities;
	(void)maxCount;
	if (outCount != nullptr)
	{
		*outCount = 0;
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_queryComponents(
	NNSceneHandle scene, uint64_t componentTypeId,
	const NNEntityHandle* entities, uint32_t entityCount,
	void* outData, uint32_t componentSize)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)componentTypeId;
	(void)entities;
	(void)entityCount;
	if (outData != nullptr && componentSize > 0)
	{
		std::memset(outData, 0, static_cast<std::size_t>(entityCount) * componentSize);
	}
	return NN_SCENE_OK;
}

NNSceneResult NN_ENGINE_ABI_STDCALL stub_scene_queryCount2(
	NNSceneHandle scene, uint64_t typeId1, uint64_t typeId2, uint32_t* outCount)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)scene;
	(void)typeId1;
	(void)typeId2;
	if (outCount != nullptr)
	{
		*outCount = 0;
	}
	return NN_SCENE_OK;
}
} // namespace

extern "C" void NNBuildSceneApiStubs(NNSceneAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->layoutVersion = 6;
	api->createScene = &stub_scene_createScene;
	api->destroyScene = &stub_scene_destroyScene;
	api->tickSystems = &stub_scene_tickSystems;
	api->createEntity = &stub_scene_createEntity;
	api->destroyEntity = &stub_scene_destroyEntity;
	api->addComponent = &stub_scene_addComponent;
	api->removeComponent = &stub_scene_removeComponent;
	api->hasComponent = &stub_scene_hasComponent;
	api->getComponent = &stub_scene_getComponent;
	api->setComponent = &stub_scene_setComponent;
	api->setParent = &stub_scene_setParent;
	api->getParent = &stub_scene_getParent;
	api->serializeScene = &stub_scene_serializeScene;
	api->deserializeScene = &stub_scene_deserializeScene;
	api->queryEntities = &stub_scene_queryEntities;
	api->queryComponents = &stub_scene_queryComponents;
	api->queryCount2 = &stub_scene_queryCount2;
}
