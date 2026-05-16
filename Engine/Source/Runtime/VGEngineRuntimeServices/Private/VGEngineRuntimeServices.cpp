/**
 * @file VGEngineRuntimeServices.cpp
 * @brief Runtime 版 Engine Service 表：先以 Stub 表為基底，再覆寫與 **VGEngineRuntime** 狀態相關之欄位（含 Phase 5 Object / AssetRegistry / Scene 擴充）。
 *
 * **VGEntityAPI（layout v5+）**
 * - **`VGNativeEngineApiTable_BuildRuntime`** 覆寫 **`outTable->entity.getServiceAbiToken`** 與 **`getRuntimeTick`**，转发至 **`VGEngineRuntime::Instance().Entity()`**（**EntitySubsystem**）。
 * - **`VGNativeEngineApiTable_BuildDefault`** / 未链接 Runtime 服务时仍使用 Stub 函数指针（`getRuntimeTick` 恒为 **0**）。
 * - 与 **`VGSceneAPI`** 之 **`VGEntityHandle`** 语义分离不变（见 **`EntityAPI.h`**）。
 */

#include <cstdint>
#include <mutex>

#include "VGNativeEngineAPI/EngineAPIRegistry.h"
#include "VGEngineRuntime/VGEngineRuntime.h"
#include "VGEngineRuntimeServices/NativeEngineRuntimeServices.h"

namespace
{
using visiongal::engine::VGEngineRuntime;

static float VG_ENGINE_ABI_STDCALL rt_timing_getDeltaTime(void)
{
	return VGEngineRuntime::Instance().Timing().GetDeltaTime();
}

static float VG_ENGINE_ABI_STDCALL rt_timing_getTotalTime(void)
{
	return VGEngineRuntime::Instance().Timing().GetTotalTime();
}

static std::uint64_t VG_ENGINE_ABI_STDCALL rt_timing_getFrameIndex(void)
{
	return VGEngineRuntime::Instance().Timing().GetFrameIndex();
}

static VGAsyncWaitHandle VG_ENGINE_ABI_STDCALL rt_async_createWait(void)
{
	return static_cast<VGAsyncWaitHandle>(VGEngineRuntime::Instance().Async().CreateWait());
}

static int VG_ENGINE_ABI_STDCALL rt_async_tryComplete(VGAsyncWaitHandle wait)
{
	return VGEngineRuntime::Instance().Async().TryComplete(static_cast<std::uint64_t>(wait));
}

static void VG_ENGINE_ABI_STDCALL rt_async_releaseWait(VGAsyncWaitHandle wait)
{
	VGEngineRuntime::Instance().Async().ReleaseWait(static_cast<std::uint64_t>(wait));
}

static int VG_ENGINE_ABI_STDCALL rt_scene_loadScene(const char* sceneNameUtf8)
{
	return VGEngineRuntime::Instance().Scene().LoadScene(sceneNameUtf8);
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL rt_scene_spawn(const char* prefabVirtualPathUtf8)
{
	return VGEngineRuntime::Instance().Scene().Spawn(prefabVirtualPathUtf8);
}

static void VG_ENGINE_ABI_STDCALL rt_scene_destroy(VGEntityHandle entity)
{
	VGEngineRuntime::Instance().Scene().Destroy(entity);
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL rt_scene_find(const char* entityNameUtf8)
{
	return VGEngineRuntime::Instance().Scene().Find(entityNameUtf8);
}

static void VG_ENGINE_ABI_STDCALL rt_scene_activate(VGEntityHandle entity, int active)
{
	VGEngineRuntime::Instance().Scene().Activate(entity, active);
}

static int VG_ENGINE_ABI_STDCALL rt_scene_unloadScene(const char* sceneNameUtf8)
{
	return VGEngineRuntime::Instance().Scene().UnloadScene(sceneNameUtf8);
}

static int VG_ENGINE_ABI_STDCALL rt_scene_getActiveSceneName(char* outUtf8, std::size_t outCapacity)
{
	return VGEngineRuntime::Instance().Scene().GetActiveSceneName(outUtf8, outCapacity);
}

static void VG_ENGINE_ABI_STDCALL rt_scene_setParent(VGEntityHandle child, VGEntityHandle parent)
{
	VGEngineRuntime::Instance().Scene().SetParent(child, parent);
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL rt_scene_getParent(VGEntityHandle entity)
{
	return VGEngineRuntime::Instance().Scene().GetParent(entity);
}

static std::uint32_t VG_ENGINE_ABI_STDCALL rt_scene_getChildCount(VGEntityHandle entity)
{
	return VGEngineRuntime::Instance().Scene().GetChildCount(entity);
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL rt_scene_getChildAt(VGEntityHandle entity, std::uint32_t index)
{
	return VGEngineRuntime::Instance().Scene().GetChildAt(entity, index);
}

static void VG_ENGINE_ABI_STDCALL rt_scene_getTransform(VGEntityHandle entity, VGTransform3* outTransform)
{
	VGEngineRuntime::Instance().Scene().GetTransform(entity, outTransform);
}

static void VG_ENGINE_ABI_STDCALL rt_scene_setTransform(VGEntityHandle entity, const VGTransform3* transform)
{
	VGEngineRuntime::Instance().Scene().SetTransform(entity, transform);
}

static int VG_ENGINE_ABI_STDCALL rt_scene_setEntityName(VGEntityHandle entity, const char* nameUtf8)
{
	return VGEngineRuntime::Instance().Scene().SetEntityName(entity, nameUtf8);
}

static int VG_ENGINE_ABI_STDCALL rt_scene_getEntityName(VGEntityHandle entity, char* outUtf8, std::size_t outCapacity)
{
	return VGEngineRuntime::Instance().Scene().GetEntityName(entity, outUtf8, outCapacity);
}

static VGTextureHandle VG_ENGINE_ABI_STDCALL rt_asset_loadTexture(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().Asset().LoadTexture(virtualPathUtf8);
}

static VGAudioHandle VG_ENGINE_ABI_STDCALL rt_asset_loadAudio(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().Asset().LoadAudio(virtualPathUtf8);
}

static VGObjectHandle VG_ENGINE_ABI_STDCALL rt_object_createObject(const char* typeNameUtf8)
{
	return VGEngineRuntime::Instance().Object().CreateObject(typeNameUtf8);
}

static void VG_ENGINE_ABI_STDCALL rt_object_destroyObject(VGObjectHandle object)
{
	VGEngineRuntime::Instance().Object().DestroyObject(object);
}

static void VG_ENGINE_ABI_STDCALL rt_object_retainObject(VGObjectHandle object)
{
	VGEngineRuntime::Instance().Object().RetainObject(object);
}

static void VG_ENGINE_ABI_STDCALL rt_object_releaseObject(VGObjectHandle object)
{
	VGEngineRuntime::Instance().Object().ReleaseObject(object);
}

static std::uint32_t VG_ENGINE_ABI_STDCALL rt_object_getRefCount(VGObjectHandle object)
{
	return VGEngineRuntime::Instance().Object().GetRefCount(object);
}

static int VG_ENGINE_ABI_STDCALL rt_object_isAlive(VGObjectHandle object)
{
	return VGEngineRuntime::Instance().Object().IsAlive(object);
}

static int VG_ENGINE_ABI_STDCALL rt_object_getTypeName(VGObjectHandle object, char* outUtf8, std::size_t outCapacity)
{
	return VGEngineRuntime::Instance().Object().GetTypeName(object, outUtf8, outCapacity);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_registerAsset(const char* virtualPathUtf8, VGGuid guid)
{
	return VGEngineRuntime::Instance().AssetRegistry().RegisterAsset(virtualPathUtf8, guid);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_unregisterByGuid(VGGuid guid)
{
	return VGEngineRuntime::Instance().AssetRegistry().UnregisterByGuid(guid);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_unregisterByPath(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().AssetRegistry().UnregisterByPath(virtualPathUtf8);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_resolvePathByGuid(VGGuid guid, char* outUtf8, std::size_t outCapacity)
{
	return VGEngineRuntime::Instance().AssetRegistry().ResolvePathByGuid(guid, outUtf8, outCapacity);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_resolveGuidByPath(const char* virtualPathUtf8, VGGuid* outGuid)
{
	return VGEngineRuntime::Instance().AssetRegistry().ResolveGuidByPath(virtualPathUtf8, outGuid);
}

static std::uint32_t VG_ENGINE_ABI_STDCALL rt_reg_getDependencyCount(VGGuid guid)
{
	return VGEngineRuntime::Instance().AssetRegistry().GetDependencyCount(guid);
}

static int VG_ENGINE_ABI_STDCALL rt_reg_getDependencyAt(VGGuid guid, std::uint32_t index, VGGuid* outDependency)
{
	return VGEngineRuntime::Instance().AssetRegistry().GetDependencyAt(guid, index, outDependency);
}

static VGGuid VG_ENGINE_ABI_STDCALL rt_reg_importAsset(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().AssetRegistry().ImportAsset(virtualPathUtf8);
}

// --- VGEntityAPI（layout v5）：ABI 冒烟魔数 + 可观测 **runtimeTick**（与 Stub 行为区分见 **VGNativeEngineApiStubs.cpp**）。---

static std::uint32_t VG_ENGINE_ABI_STDCALL rt_entity_getServiceAbiToken(void)
{
	return VGEngineRuntime::Instance().Entity().GetServiceAbiToken();
}

static std::uint64_t VG_ENGINE_ABI_STDCALL rt_entity_getRuntimeTick(void)
{
	return VGEngineRuntime::Instance().Entity().GetRuntimeTick();
}
} // namespace

extern "C" void VGNativeEngineApiTable_BuildRuntime(VGNativeEngineAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	VGNativeEngineApiTable_BuildDefault(outTable);

	outTable->timing.getDeltaTime = &rt_timing_getDeltaTime;
	outTable->timing.getTotalTime = &rt_timing_getTotalTime;
	outTable->timing.getFrameIndex = &rt_timing_getFrameIndex;

	outTable->asyncWait.createWait = &rt_async_createWait;
	outTable->asyncWait.tryComplete = &rt_async_tryComplete;
	outTable->asyncWait.releaseWait = &rt_async_releaseWait;

	outTable->scene.loadScene = &rt_scene_loadScene;
	outTable->scene.spawn = &rt_scene_spawn;
	outTable->scene.destroy = &rt_scene_destroy;
	outTable->scene.find = &rt_scene_find;
	outTable->scene.activate = &rt_scene_activate;
	outTable->scene.unloadScene = &rt_scene_unloadScene;
	outTable->scene.getActiveSceneName = &rt_scene_getActiveSceneName;
	outTable->scene.setParent = &rt_scene_setParent;
	outTable->scene.getParent = &rt_scene_getParent;
	outTable->scene.getChildCount = &rt_scene_getChildCount;
	outTable->scene.getChildAt = &rt_scene_getChildAt;
	outTable->scene.getTransform = &rt_scene_getTransform;
	outTable->scene.setTransform = &rt_scene_setTransform;
	outTable->scene.setEntityName = &rt_scene_setEntityName;
	outTable->scene.getEntityName = &rt_scene_getEntityName;

	outTable->asset.loadTexture = &rt_asset_loadTexture;
	outTable->asset.loadAudio = &rt_asset_loadAudio;

	outTable->object.createObject = &rt_object_createObject;
	outTable->object.destroyObject = &rt_object_destroyObject;
	outTable->object.retainObject = &rt_object_retainObject;
	outTable->object.releaseObject = &rt_object_releaseObject;
	outTable->object.getRefCount = &rt_object_getRefCount;
	outTable->object.isAlive = &rt_object_isAlive;
	outTable->object.getTypeName = &rt_object_getTypeName;

	outTable->assetRegistry.registerAsset = &rt_reg_registerAsset;
	outTable->assetRegistry.unregisterByGuid = &rt_reg_unregisterByGuid;
	outTable->assetRegistry.unregisterByPath = &rt_reg_unregisterByPath;
	outTable->assetRegistry.resolvePathByGuid = &rt_reg_resolvePathByGuid;
	outTable->assetRegistry.resolveGuidByPath = &rt_reg_resolveGuidByPath;
	outTable->assetRegistry.getDependencyCount = &rt_reg_getDependencyCount;
	outTable->assetRegistry.getDependencyAt = &rt_reg_getDependencyAt;
	outTable->assetRegistry.importAsset = &rt_reg_importAsset;

	// §2.7.1：Runtime 覆写 **entity.***，与 **VGNativeEngineApiTable_BuildDefault** 之 Stub 分流（Stub 的 **getRuntimeTick** 恒为 0）。
	// 转发至 **VGEngineRuntime::Entity()**（**EntitySubsystem**）；**不**表示托管 **EntityWorld** 已与 Native ECS 数据镜像。
	outTable->entity.getServiceAbiToken = &rt_entity_getServiceAbiToken;
	outTable->entity.getRuntimeTick = &rt_entity_getRuntimeTick;
}

namespace
{
	std::once_flag g_runtimeTableOnce;
	VGNativeEngineAPI g_runtimeTable{};
	const VGNativeEngineAPI* g_runtimeTablePtr = nullptr;
} // namespace

extern "C" const VGNativeEngineAPI* VGNativeEngineApi_GetRuntimeTable(void)
{
	std::call_once(g_runtimeTableOnce, [] {
		VGNativeEngineApiTable_BuildRuntime(&g_runtimeTable);
		g_runtimeTablePtr = &g_runtimeTable;
	});
	return g_runtimeTablePtr;
}

extern "C" bool VGEngineRuntimeHost_Initialize(void)
{
	return VGEngineRuntime::Instance().Initialize();
}

extern "C" void VGEngineRuntimeHost_Tick(float deltaTimeSeconds)
{
	VGEngineRuntime::Instance().Tick(deltaTimeSeconds);
}

extern "C" void VGEngineRuntimeHost_Shutdown(void)
{
	VGEngineRuntime::Instance().Shutdown();
}
