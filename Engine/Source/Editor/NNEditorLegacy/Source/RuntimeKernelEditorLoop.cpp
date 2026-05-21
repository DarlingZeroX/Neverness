/**
 * @file RuntimeKernelEditorLoop.cpp
 * @brief Editor Kernel 路径：NNApplicationAPI 事件循环 + Native/Managed 双 Tick。
 */

#include "RuntimeKernelEditorLoop.h"

#include "ManagedRuntimeBridge.h"
#include "NativeEngineRuntimeServices.h"
#include "NativeAPI.h"

#include "NNNativeEngineAPI/Include/WindowAPI.h"
#include "NNNativeEngineAPI/Include/WindowTypes.h"

#if defined(VISIONGAL_ENABLE_MANAGED_HOST_LEGACY) && VISIONGAL_ENABLE_MANAGED_HOST_LEGACY
#include "ManagedHost.h"
#include <filesystem>
#endif

#include <iostream>

int RunRuntimeKernelEditorMainLoop(void)
{
	const NNNativeEngineAPI* engineTable = NNNativeEngineApi_GetRuntimeTable();
	if (engineTable == nullptr)
	{
		std::cerr << "NNNativeEngineApi_GetRuntimeTable failed\n";
		return -1;
	}

	const NNApplicationAPI& app = engineTable->application;
	const NNWindowAPI& window = engineTable->window;
	if (app.initialize == nullptr || app.pumpEvents == nullptr || window.create == nullptr)
	{
		std::cerr << "NNApplicationAPI / NNWindowAPI is not wired\n";
		return -1;
	}

	if (!NNEngineRuntimeHost_Initialize())
	{
		std::cerr << "NNEngineRuntimeHost_Initialize failed\n";
		return -1;
	}

#if defined(VISIONGAL_ENABLE_MANAGED_HOST_LEGACY) && VISIONGAL_ENABLE_MANAGED_HOST_LEGACY
	{
		static VGManagedHost s_managedHost;
		const std::filesystem::path hostAssembly =
			std::filesystem::path("Neverness.Runtime.Host.dll");
		if (!VGManagedHost_BootstrapDefaultRuntime(s_managedHost, hostAssembly))
		{
			std::cerr << "Warning: Managed Bootstrap failed; continuing with Native-only Tick.\n";
		}
	}
#endif

	if (!app.initialize())
	{
		std::cerr << "application.initialize failed\n";
		NNEngineRuntimeHost_Shutdown();
		return -1;
	}

	NNWindowDesc windowDesc{};
	windowDesc.title = "Neverness Editor";
	windowDesc.width = 1280;
	windowDesc.height = 720;
	windowDesc.resizable = true;
	windowDesc.maximized = false;
	windowDesc.hidden = false;

	if (window.create(&windowDesc) == NN_INVALID_WINDOW_HANDLE)
	{
		std::cerr << "window.create failed\n";
		if (app.shutdown != nullptr)
		{
			app.shutdown();
		}
		NNEngineRuntimeHost_Shutdown();
		return -1;
	}

	constexpr float kFixedDelta = 1.f / 60.f;
	while (app.pumpEvents())
	{
		NNEngineRuntimeHost_Tick(kFixedDelta);
		NNEngineRuntimeHost_TickManaged(kFixedDelta);
	}

	if (app.shutdown != nullptr)
	{
		app.shutdown();
	}

	NNEngineRuntimeHost_Shutdown();
	return 0;
}
*/