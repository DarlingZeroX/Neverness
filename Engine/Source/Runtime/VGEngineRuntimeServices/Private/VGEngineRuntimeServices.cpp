/**
 * @file VGEngineRuntimeServices.cpp
 * @brief Runtime 版 Engine Service 表：先以 Stub 表為基底，再覆寫與 **VGEngineRuntime** 狀態相關之欄位。
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

static VGTextureHandle VG_ENGINE_ABI_STDCALL rt_asset_loadTexture(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().Asset().LoadTexture(virtualPathUtf8);
}

static VGAudioHandle VG_ENGINE_ABI_STDCALL rt_asset_loadAudio(const char* virtualPathUtf8)
{
	return VGEngineRuntime::Instance().Asset().LoadAudio(virtualPathUtf8);
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

	outTable->asset.loadTexture = &rt_asset_loadTexture;
	outTable->asset.loadAudio = &rt_asset_loadAudio;
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
