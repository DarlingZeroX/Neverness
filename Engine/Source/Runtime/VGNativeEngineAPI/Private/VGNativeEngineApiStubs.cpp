/**
 * @file VGNativeEngineApiStubs.cpp
 * @brief Engine Service ABI 之 **預設 Stub** 實作與行程單例表（不連結 VGEngine / VGRHI / VGUI）。
 */

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>

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

// --- Timing ---
static float VG_ENGINE_ABI_STDCALL stub_timing_getDeltaTime(void)
{
	bump();
	return 0.f;
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

	outTable->input.isKeyPressed = &stub_input_isKeyPressed;

	outTable->scene.loadScene = &stub_scene_loadScene;

	outTable->timing.getDeltaTime = &stub_timing_getDeltaTime;

	outTable->asyncWait.createWait = &stub_async_createWait;
	outTable->asyncWait.tryComplete = &stub_async_tryComplete;
	outTable->asyncWait.releaseWait = &stub_async_releaseWait;
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
