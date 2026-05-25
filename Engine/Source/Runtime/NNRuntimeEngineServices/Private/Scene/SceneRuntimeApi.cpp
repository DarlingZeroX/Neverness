/**
 * @file SceneRuntimeApi.cpp
 * @brief NNSceneAPI Runtime 转发至 NNEngineRuntime::Scene()。
 *
 * 函数签名对齐新 ABI（layoutVersion = 4）：NNSceneResult 返回值、NNSceneHandle = uint64。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/SceneAPI.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_createScene(NNSceneHandle* outScene)
{
	return NNEngineRuntime::Instance().Scene().CreateScene(outScene);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_destroyScene(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().DestroyScene(scene);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_tickSystems(NNSceneHandle scene, float deltaTime)
{
	return NNEngineRuntime::Instance().Scene().TickSystems(scene, deltaTime);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_createEntity(NNSceneHandle scene, NNEntityHandle* outEntity)
{
	return NNEngineRuntime::Instance().Scene().CreateEntity(scene, outEntity);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_destroyEntity(NNSceneHandle scene, NNEntityHandle entity)
{
	return NNEngineRuntime::Instance().Scene().DestroyEntity(scene, entity);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_addComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId)
{
	return NNEngineRuntime::Instance().Scene().AddComponent(scene, entity, componentTypeId);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_removeComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId)
{
	return NNEngineRuntime::Instance().Scene().RemoveComponent(scene, entity, componentTypeId);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_hasComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, int32_t* outHas)
{
	return NNEngineRuntime::Instance().Scene().HasComponent(scene, entity, componentTypeId, outHas);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_getComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, void* outData, uint32_t dataSize)
{
	return NNEngineRuntime::Instance().Scene().GetComponent(scene, entity, componentTypeId, outData, dataSize);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_setComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, const void* data, uint32_t dataSize)
{
	return NNEngineRuntime::Instance().Scene().SetComponent(scene, entity, componentTypeId, data, dataSize);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_setParent(NNSceneHandle scene, NNEntityHandle child, NNEntityHandle parent)
{
	return NNEngineRuntime::Instance().Scene().SetParent(scene, child, parent);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_getParent(NNSceneHandle scene, NNEntityHandle entity, NNEntityHandle* outParent)
{
	return NNEngineRuntime::Instance().Scene().GetParent(scene, entity, outParent);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_serializeScene(NNSceneHandle scene, const char* vfsPath)
{
	return NNEngineRuntime::Instance().Scene().SerializeScene(scene, vfsPath);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_deserializeScene(NNSceneHandle* outScene, const char* vfsPath)
{
	return NNEngineRuntime::Instance().Scene().DeserializeScene(outScene, vfsPath);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_queryEntities(
	NNSceneHandle scene, uint64_t componentTypeId,
	NNEntityHandle* outEntities, uint32_t maxCount, uint32_t* outCount)
{
	return NNEngineRuntime::Instance().Scene().QueryEntities(scene, componentTypeId, outEntities, maxCount, outCount);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_queryComponents(
	NNSceneHandle scene, uint64_t componentTypeId,
	const NNEntityHandle* entities, uint32_t entityCount,
	void* outData, uint32_t componentSize)
{
	return NNEngineRuntime::Instance().Scene().QueryComponents(scene, componentTypeId, entities, entityCount, outData, componentSize);
}

NNSceneResult NN_ENGINE_ABI_STDCALL rt_scene_queryCount2(
	NNSceneHandle scene, uint64_t typeId1, uint64_t typeId2, uint32_t* outCount)
{
	return NNEngineRuntime::Instance().Scene().QueryCount2(scene, typeId1, typeId2, outCount);
}
} // namespace

extern "C" void NNBuildSceneRuntimeApi(NNSceneAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->layoutVersion = 6;
	api->createScene = &rt_scene_createScene;
	api->destroyScene = &rt_scene_destroyScene;
	api->tickSystems = &rt_scene_tickSystems;
	api->createEntity = &rt_scene_createEntity;
	api->destroyEntity = &rt_scene_destroyEntity;
	api->addComponent = &rt_scene_addComponent;
	api->removeComponent = &rt_scene_removeComponent;
	api->hasComponent = &rt_scene_hasComponent;
	api->getComponent = &rt_scene_getComponent;
	api->setComponent = &rt_scene_setComponent;
	api->setParent = &rt_scene_setParent;
	api->getParent = &rt_scene_getParent;
	api->serializeScene = &rt_scene_serializeScene;
	api->deserializeScene = &rt_scene_deserializeScene;
	api->queryEntities = &rt_scene_queryEntities;
	api->queryComponents = &rt_scene_queryComponents;
	api->queryCount2 = &rt_scene_queryCount2;
}

// ── Editor Scene API 转发 ──

namespace
{
uint64_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getHierarchyVersion(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().GetHierarchyVersion(scene);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getSnapshotSize(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().GetSnapshotSize(scene);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getHierarchySnapshot(
	NNSceneHandle scene, void* outBuffer, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetHierarchySnapshot(scene, outBuffer, capacity);
}

uint64_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getTransformVersion(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().GetTransformVersion(scene);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getTransformSnapshot(
	NNSceneHandle scene, const uint64_t* entities,
	uint32_t entityCount, NNEditorTransformData* outArray)
{
	return NNEngineRuntime::Instance().Scene().GetTransformSnapshot(scene, entities, entityCount, outArray);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getIncrementalSnapshot(
	NNSceneHandle scene, void* outBuffer, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetIncrementalSnapshot(scene, outBuffer, capacity);
}

// ── Reflection API 转发（layoutVersion = 3）──

uint64_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getReflectionVersion(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().GetReflectionVersion(scene);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getTypeInfoSnapshotSize(NNSceneHandle scene)
{
	return NNEngineRuntime::Instance().Scene().GetTypeInfoSnapshotSize(scene);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getTypeInfoSnapshot(
	NNSceneHandle scene, void* outBuffer, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetTypeInfoSnapshot(scene, outBuffer, capacity);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getEntityComponentCount(
	NNSceneHandle scene, uint64_t entity)
{
	return NNEngineRuntime::Instance().Scene().GetEntityComponentCount(scene, entity);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getEntityComponents(
	NNSceneHandle scene, uint64_t entity, NNEditorComponentInfo* outInfos, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetEntityComponents(scene, entity, outInfos, capacity);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getComponentFieldInfos(
	NNSceneHandle scene, uint64_t componentTypeId, NNEditorFieldInfo* outFields, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetComponentFieldInfos(scene, componentTypeId, outFields, capacity);
}

uint32_t NN_ENGINE_ABI_STDCALL rt_editor_scene_getComponentRawData(
	NNSceneHandle scene, uint64_t entity, uint64_t componentTypeId, void* outData, uint32_t capacity)
{
	return NNEngineRuntime::Instance().Scene().GetComponentRawData(scene, entity, componentTypeId, outData, capacity);
}
} // namespace

extern "C" void NNBuildEditorSceneRuntimeApi(NNEditorSceneAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->layoutVersion              = 3;
	api->getHierarchyVersion        = &rt_editor_scene_getHierarchyVersion;
	api->getSnapshotSize            = &rt_editor_scene_getSnapshotSize;
	api->getHierarchySnapshot       = &rt_editor_scene_getHierarchySnapshot;
	api->getTransformVersion        = &rt_editor_scene_getTransformVersion;
	api->getTransformSnapshot       = &rt_editor_scene_getTransformSnapshot;
	api->getIncrementalSnapshot     = &rt_editor_scene_getIncrementalSnapshot;
	api->getReflectionVersion       = &rt_editor_scene_getReflectionVersion;
	api->getTypeInfoSnapshotSize    = &rt_editor_scene_getTypeInfoSnapshotSize;
	api->getTypeInfoSnapshot        = &rt_editor_scene_getTypeInfoSnapshot;
	api->getEntityComponentCount    = &rt_editor_scene_getEntityComponentCount;
	api->getEntityComponents        = &rt_editor_scene_getEntityComponents;
	api->getComponentFieldInfos     = &rt_editor_scene_getComponentFieldInfos;
	api->getComponentRawData        = &rt_editor_scene_getComponentRawData;
}
