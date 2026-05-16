/**
 * @file VGNativeEngineApiStubs.cpp
 * @brief Engine Service ABI 之 **預設 Stub** 實作與行程單例表（不連結 VGEngine / VGRHI / VGUI）。
 */

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

#include "VGNativeEngineAPI/NativeEngineAPI.h"

namespace
{
	std::atomic<std::uint32_t> g_stubInvokeCount{0};

	void bump() noexcept
	{
		g_stubInvokeCount.fetch_add(1u, std::memory_order_relaxed);
	}
} // namespace

extern "C" std::uint32_t VGNativeEngineApi_GetStubInvokeCount(void)
{
	return g_stubInvokeCount.load(std::memory_order_relaxed);
}

// --- Render ---
static VGTextureHandle VG_ENGINE_ABI_STDCALL stub_render_createTexture(std::uint32_t width, std::uint32_t height)
{
	bump();
	(void)width;
	(void)height;
	return 0; // Stub：不建立真實資源
}

static void VG_ENGINE_ABI_STDCALL stub_render_uploadTexture(
	VGTextureHandle texture,
	const std::uint8_t* pixelBytes,
	std::size_t byteCount)
{
	bump();
	(void)texture;
	(void)pixelBytes;
	(void)byteCount;
}

static VGRenderTargetHandle VG_ENGINE_ABI_STDCALL stub_render_createRenderTarget(std::uint32_t width, std::uint32_t height)
{
	bump();
	(void)width;
	(void)height;
	return 0;
}

// --- UI ---
static void VG_ENGINE_ABI_STDCALL stub_ui_setDialogueText(VGElementHandle element, const char* utf8Text)
{
	bump();
	(void)element;
	(void)utf8Text;
}

static void VG_ENGINE_ABI_STDCALL stub_ui_setElementVisible(VGElementHandle element, int visible)
{
	bump();
	(void)element;
	(void)visible;
}

// --- Audio ---
static VGAudioHandle VG_ENGINE_ABI_STDCALL stub_audio_playBgm(const char* assetPathUtf8)
{
	bump();
	(void)assetPathUtf8;
	return 0;
}

static VGAudioHandle VG_ENGINE_ABI_STDCALL stub_audio_playVoice(const char* assetPathUtf8)
{
	bump();
	(void)assetPathUtf8;
	return 0;
}

// --- Asset ---
static VGAssetHandle VG_ENGINE_ABI_STDCALL stub_asset_load(const char* virtualPathUtf8)
{
	bump();
	(void)virtualPathUtf8;
	return 0;
}

static void VG_ENGINE_ABI_STDCALL stub_asset_unload(VGAssetHandle asset)
{
	bump();
	(void)asset;
}

static VGTextureHandle VG_ENGINE_ABI_STDCALL stub_asset_loadTexture(const char* virtualPathUtf8)
{
	bump();
	(void)virtualPathUtf8;
	return 0;
}

static VGAudioHandle VG_ENGINE_ABI_STDCALL stub_asset_loadAudio(const char* virtualPathUtf8)
{
	bump();
	(void)virtualPathUtf8;
	return 0;
}

// --- Input ---
static int VG_ENGINE_ABI_STDCALL stub_input_isKeyPressed(int keyCode)
{
	bump();
	(void)keyCode;
	return 0;
}

// --- Scene ---
static int VG_ENGINE_ABI_STDCALL stub_scene_loadScene(const char* sceneNameUtf8)
{
	bump();
	(void)sceneNameUtf8;
	return 1;
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL stub_scene_spawn(const char* prefabVirtualPathUtf8)
{
	bump();
	(void)prefabVirtualPathUtf8;
	return 0;
}

static void VG_ENGINE_ABI_STDCALL stub_scene_destroy(VGEntityHandle entity)
{
	bump();
	(void)entity;
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL stub_scene_find(const char* entityNameUtf8)
{
	bump();
	(void)entityNameUtf8;
	return 0;
}

static void VG_ENGINE_ABI_STDCALL stub_scene_activate(VGEntityHandle entity, int active)
{
	bump();
	(void)entity;
	(void)active;
}

static int VG_ENGINE_ABI_STDCALL stub_scene_unloadScene(const char* sceneNameUtf8)
{
	bump();
	(void)sceneNameUtf8;
	return -1;
}

static int VG_ENGINE_ABI_STDCALL stub_scene_getActiveSceneName(char* outUtf8, std::size_t outCapacity)
{
	bump();
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	outUtf8[0] = '\0';
	return 0;
}

static void VG_ENGINE_ABI_STDCALL stub_scene_setParent(VGEntityHandle child, VGEntityHandle parent)
{
	bump();
	(void)child;
	(void)parent;
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL stub_scene_getParent(VGEntityHandle entity)
{
	bump();
	(void)entity;
	return 0;
}

static std::uint32_t VG_ENGINE_ABI_STDCALL stub_scene_getChildCount(VGEntityHandle entity)
{
	bump();
	(void)entity;
	return 0u;
}

static VGEntityHandle VG_ENGINE_ABI_STDCALL stub_scene_getChildAt(VGEntityHandle entity, std::uint32_t index)
{
	bump();
	(void)entity;
	(void)index;
	return 0;
}

static void VG_ENGINE_ABI_STDCALL stub_scene_getTransform(VGEntityHandle entity, VGTransform3* outTransform)
{
	bump();
	(void)entity;
	if (outTransform != nullptr)
	{
		std::memset(outTransform, 0, sizeof(VGTransform3));
		outTransform->scale[0] = outTransform->scale[1] = outTransform->scale[2] = 1.f;
	}
}

static void VG_ENGINE_ABI_STDCALL stub_scene_setTransform(VGEntityHandle entity, const VGTransform3* transform)
{
	bump();
	(void)entity;
	(void)transform;
}

static int VG_ENGINE_ABI_STDCALL stub_scene_setEntityName(VGEntityHandle entity, const char* nameUtf8)
{
	bump();
	(void)entity;
	(void)nameUtf8;
	return -1;
}

static int VG_ENGINE_ABI_STDCALL stub_scene_getEntityName(VGEntityHandle entity, char* outUtf8, std::size_t outCapacity)
{
	bump();
	(void)entity;
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	outUtf8[0] = '\0';
	return 0;
}

// --- Object（Stub：記憶體表，供未鏈結 Runtime 之單元測試）---
namespace
{
	std::mutex g_stubObjMutex;
	std::atomic<std::uint64_t> g_stubObjNext{1};
	struct StubObjSlot
	{
		std::string typeName{};
		std::uint32_t refs{1};
	};
	std::unordered_map<std::uint64_t, StubObjSlot> g_stubObjSlots;
} // namespace

static VGObjectHandle VG_ENGINE_ABI_STDCALL stub_object_createObject(const char* typeNameUtf8)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	const std::uint64_t id = g_stubObjNext.fetch_add(1u, std::memory_order_relaxed);
	if (id == 0)
	{
		return 0;
	}
	StubObjSlot s{};
	s.typeName = (typeNameUtf8 != nullptr) ? typeNameUtf8 : "";
	s.refs = 1u;
	g_stubObjSlots[id] = std::move(s);
	return static_cast<VGObjectHandle>(id);
}

static void VG_ENGINE_ABI_STDCALL stub_object_destroyObject(VGObjectHandle object)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	g_stubObjSlots.erase(static_cast<std::uint64_t>(object));
}

static void VG_ENGINE_ABI_STDCALL stub_object_retainObject(VGObjectHandle object)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	const auto it = g_stubObjSlots.find(static_cast<std::uint64_t>(object));
	if (it != g_stubObjSlots.end())
	{
		++it->second.refs;
	}
}

static void VG_ENGINE_ABI_STDCALL stub_object_releaseObject(VGObjectHandle object)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	const auto it = g_stubObjSlots.find(static_cast<std::uint64_t>(object));
	if (it == g_stubObjSlots.end())
	{
		return;
	}
	if (it->second.refs > 0u)
	{
		--it->second.refs;
	}
	if (it->second.refs == 0u)
	{
		g_stubObjSlots.erase(it);
	}
}

static std::uint32_t VG_ENGINE_ABI_STDCALL stub_object_getRefCount(VGObjectHandle object)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	const auto it = g_stubObjSlots.find(static_cast<std::uint64_t>(object));
	return it != g_stubObjSlots.end() ? it->second.refs : 0u;
}

static int VG_ENGINE_ABI_STDCALL stub_object_isAlive(VGObjectHandle object)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	return g_stubObjSlots.find(static_cast<std::uint64_t>(object)) != g_stubObjSlots.end() ? 1 : 0;
}

static int VG_ENGINE_ABI_STDCALL stub_object_getTypeName(VGObjectHandle object, char* outUtf8, std::size_t outCapacity)
{
	bump();
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_stubObjMutex);
	const auto it = g_stubObjSlots.find(static_cast<std::uint64_t>(object));
	if (it == g_stubObjSlots.end())
	{
		return -1;
	}
	const std::string& tn = it->second.typeName;
	if (tn.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, tn.data(), tn.size());
	outUtf8[tn.size()] = '\0';
	return static_cast<int>(tn.size());
}

// --- AssetRegistry（Stub）---
namespace
{
	std::mutex g_stubRegMutex;
	std::unordered_map<std::string, VGGuid> g_stubPathToGuid;
	std::unordered_map<std::uint64_t, std::string> g_stubGuidLowToPath;
} // namespace

static int VG_ENGINE_ABI_STDCALL stub_reg_registerAsset(const char* virtualPathUtf8, VGGuid guid)
{
	bump();
	if (virtualPathUtf8 == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_stubRegMutex);
	const std::string p(virtualPathUtf8);
	g_stubPathToGuid[p] = guid;
	g_stubGuidLowToPath[guid.low] = p;
	return 0;
}

static int VG_ENGINE_ABI_STDCALL stub_reg_unregisterByGuid(VGGuid guid)
{
	bump();
	std::lock_guard<std::mutex> lock(g_stubRegMutex);
	const auto it = g_stubGuidLowToPath.find(guid.low);
	if (it == g_stubGuidLowToPath.end())
	{
		return -1;
	}
	g_stubPathToGuid.erase(it->second);
	g_stubGuidLowToPath.erase(it);
	return 0;
}

static int VG_ENGINE_ABI_STDCALL stub_reg_unregisterByPath(const char* virtualPathUtf8)
{
	bump();
	if (virtualPathUtf8 == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_stubRegMutex);
	const std::string p(virtualPathUtf8);
	const auto it = g_stubPathToGuid.find(p);
	if (it == g_stubPathToGuid.end())
	{
		return -1;
	}
	g_stubGuidLowToPath.erase(it->second.low);
	g_stubPathToGuid.erase(it);
	return 0;
}

static int VG_ENGINE_ABI_STDCALL stub_reg_resolvePathByGuid(VGGuid guid, char* outUtf8, std::size_t outCapacity)
{
	bump();
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_stubRegMutex);
	const auto it = g_stubGuidLowToPath.find(guid.low);
	if (it == g_stubGuidLowToPath.end())
	{
		return -1;
	}
	const std::string& path = it->second;
	if (path.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, path.data(), path.size());
	outUtf8[path.size()] = '\0';
	return static_cast<int>(path.size());
}

static int VG_ENGINE_ABI_STDCALL stub_reg_resolveGuidByPath(const char* virtualPathUtf8, VGGuid* outGuid)
{
	bump();
	if (virtualPathUtf8 == nullptr || outGuid == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_stubRegMutex);
	const auto it = g_stubPathToGuid.find(std::string(virtualPathUtf8));
	if (it == g_stubPathToGuid.end())
	{
		return -1;
	}
	*outGuid = it->second;
	return 0;
}

static std::uint32_t VG_ENGINE_ABI_STDCALL stub_reg_getDependencyCount(VGGuid guid)
{
	bump();
	(void)guid;
	return 0u;
}

static int VG_ENGINE_ABI_STDCALL stub_reg_getDependencyAt(VGGuid guid, std::uint32_t index, VGGuid* outDependency)
{
	bump();
	(void)guid;
	(void)index;
	(void)outDependency;
	return -1;
}

static VGGuid VG_ENGINE_ABI_STDCALL stub_reg_importAsset(const char* virtualPathUtf8)
{
	bump();
	VGGuid z{};
	if (virtualPathUtf8 == nullptr)
	{
		return z;
	}
	z.high = 1u;
	z.low = 2u;
	(void)stub_reg_registerAsset(virtualPathUtf8, z);
	return z;
}

// --- Timing ---
static float VG_ENGINE_ABI_STDCALL stub_timing_getDeltaTime(void)
{
	bump();
	return 0.f;
}

static float VG_ENGINE_ABI_STDCALL stub_timing_getTotalTime(void)
{
	bump();
	return 0.f;
}

static std::uint64_t VG_ENGINE_ABI_STDCALL stub_timing_getFrameIndex(void)
{
	bump();
	return 0;
}

// --- Async wait：建立即視為可一次 tryComplete 讀取之「已完成」物件 ---
static VGAsyncWaitHandle VG_ENGINE_ABI_STDCALL stub_async_createWait(void)
{
	bump();
	// 非零即代表有效等待控制代碼；Stub 不維護狀態機，tryComplete 對任意非零一律回報完成一次語意由下方實作鬆弛處理。
	return 1;
}

static int VG_ENGINE_ABI_STDCALL stub_async_tryComplete(VGAsyncWaitHandle wait)
{
	bump();
	return (wait != 0) ? 1 : 0;
}

static void VG_ENGINE_ABI_STDCALL stub_async_releaseWait(VGAsyncWaitHandle wait)
{
	bump();
	(void)wait;
}

/**
 * @brief **VGEntityAPI** 首包 Stub：`getServiceAbiToken`。
 *
 * 語義（與 MANAGED **§2.7.1** 一致）：
 * - 僅用於宿主／**VGManagedHostTest**／託管 **ExerciseStubInteropPath** 驗證「**`VGNativeEngineAPI::entity`** 已接線」；
 *   返回值固定為 **VG_ENTITY_SERVICE_ABI_TOKEN**，與 **VisionGal.Managed.Engine.VGNativeEngineApiConstants.EntityServiceAbiToken** 對齊。
 * - **不**表示已存在 Native ECS 世界、**不**與 **VGSceneAPI** 之 **VGEntityHandle**（場景圖）互通；真實實體子系統見路線圖 **VGEntitySystem**。
 * - 每次呼叫仍執行 **bump()**，納入 **VGNativeEngineApi_GetStubInvokeCount** 統計，便於觀測測試是否觸達本路徑。
 */
static std::uint32_t VG_ENGINE_ABI_STDCALL stub_entity_getServiceAbiToken(void)
{
	bump();
	return VG_ENTITY_SERVICE_ABI_TOKEN;
}

/**
 * @brief Stub：`getRuntimeTick` 恒返回 **0**（无 **EntitySubsystem** 驱动）；仍 **bump()** 便于统计。
 */
static std::uint64_t VG_ENGINE_ABI_STDCALL stub_entity_getRuntimeTick(void)
{
	bump();
	return 0u;
}

extern "C" void VGNativeEngineApiTable_BuildDefault(VGNativeEngineAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	std::memset(outTable, 0, sizeof(VGNativeEngineAPI));
	outTable->layoutVersion = VG_NATIVE_ENGINE_API_LAYOUT_VERSION;
	outTable->reserved0 = 0;

	outTable->render.createTexture = &stub_render_createTexture;
	outTable->render.uploadTexture = &stub_render_uploadTexture;
	outTable->render.createRenderTarget = &stub_render_createRenderTarget;

	outTable->ui.setDialogueText = &stub_ui_setDialogueText;
	outTable->ui.setElementVisible = &stub_ui_setElementVisible;

	outTable->audio.playBgm = &stub_audio_playBgm;
	outTable->audio.playVoice = &stub_audio_playVoice;

	outTable->asset.loadAsset = &stub_asset_load;
	outTable->asset.unloadAsset = &stub_asset_unload;
	outTable->asset.loadTexture = &stub_asset_loadTexture;
	outTable->asset.loadAudio = &stub_asset_loadAudio;

	outTable->input.isKeyPressed = &stub_input_isKeyPressed;

	outTable->scene.loadScene = &stub_scene_loadScene;
	outTable->scene.spawn = &stub_scene_spawn;
	outTable->scene.destroy = &stub_scene_destroy;
	outTable->scene.find = &stub_scene_find;
	outTable->scene.activate = &stub_scene_activate;
	outTable->scene.unloadScene = &stub_scene_unloadScene;
	outTable->scene.getActiveSceneName = &stub_scene_getActiveSceneName;
	outTable->scene.setParent = &stub_scene_setParent;
	outTable->scene.getParent = &stub_scene_getParent;
	outTable->scene.getChildCount = &stub_scene_getChildCount;
	outTable->scene.getChildAt = &stub_scene_getChildAt;
	outTable->scene.getTransform = &stub_scene_getTransform;
	outTable->scene.setTransform = &stub_scene_setTransform;
	outTable->scene.setEntityName = &stub_scene_setEntityName;
	outTable->scene.getEntityName = &stub_scene_getEntityName;

	outTable->timing.getDeltaTime = &stub_timing_getDeltaTime;
	outTable->timing.getTotalTime = &stub_timing_getTotalTime;
	outTable->timing.getFrameIndex = &stub_timing_getFrameIndex;

	outTable->asyncWait.createWait = &stub_async_createWait;
	outTable->asyncWait.tryComplete = &stub_async_tryComplete;
	outTable->asyncWait.releaseWait = &stub_async_releaseWait;

	outTable->object.createObject = &stub_object_createObject;
	outTable->object.destroyObject = &stub_object_destroyObject;
	outTable->object.retainObject = &stub_object_retainObject;
	outTable->object.releaseObject = &stub_object_releaseObject;
	outTable->object.getRefCount = &stub_object_getRefCount;
	outTable->object.isAlive = &stub_object_isAlive;
	outTable->object.getTypeName = &stub_object_getTypeName;

	outTable->assetRegistry.registerAsset = &stub_reg_registerAsset;
	outTable->assetRegistry.unregisterByGuid = &stub_reg_unregisterByGuid;
	outTable->assetRegistry.unregisterByPath = &stub_reg_unregisterByPath;
	outTable->assetRegistry.resolvePathByGuid = &stub_reg_resolvePathByGuid;
	outTable->assetRegistry.resolveGuidByPath = &stub_reg_resolveGuidByPath;
	outTable->assetRegistry.getDependencyCount = &stub_reg_getDependencyCount;
	outTable->assetRegistry.getDependencyAt = &stub_reg_getDependencyAt;
	outTable->assetRegistry.importAsset = &stub_reg_importAsset;

	outTable->entity.getServiceAbiToken = &stub_entity_getServiceAbiToken;
	outTable->entity.getRuntimeTick = &stub_entity_getRuntimeTick;
}

namespace
{
	std::once_flag g_engineTableOnce;
	VGNativeEngineAPI g_engineTable{};
	const VGNativeEngineAPI* g_engineTablePtr = nullptr;
} // namespace

extern "C" const VGNativeEngineAPI* VGNativeEngineApi_GetDefaultTable(void)
{
	std::call_once(g_engineTableOnce, [] {
		VGNativeEngineApiTable_BuildDefault(&g_engineTable);
		g_engineTablePtr = &g_engineTable;
	});
	return g_engineTablePtr;
}
