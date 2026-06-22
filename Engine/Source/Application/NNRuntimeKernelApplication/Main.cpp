/**
 * @file Main.cpp
 * @brief Runtime Kernel + Application 演示：SDL 窗口、事件泵、Native/Managed 双 Tick。
 */

#include "ManagedRuntimeBridge.h"
#include "NativeEngineRuntimeServices.h"

#include "Engine/WindowAPI.h"
#include "Engine/WindowTypes.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

namespace
{
float GetFrameDeltaSeconds()
{
	return 1.f / 60.f;
}
} // namespace

int main()
{
	const NNNativeEngineAPI* engineTable = NNNativeEngineApi_GetRuntimeTable();
	if (engineTable == nullptr)
	{
		std::cerr << "NNNativeEngineApi_GetRuntimeTable failed\n";
		return 1;
	}

	const NNApplicationAPI& app = engineTable->application;
	const NNWindowAPI& window = engineTable->window;
	if (app.initialize == nullptr || app.pumpEvents == nullptr || window.create == nullptr)
	{
		std::cerr << "NNApplicationAPI / NNWindowAPI is not wired\n";
		return 1;
	}

	if (!NNEngineRuntimeHost_Initialize())
	{
		std::cerr << "NNEngineRuntimeHost_Initialize failed\n";
		return 1;
	}

	std::cout << "NNRuntimeKernelApplication: 请先通过宿主完成 Entry.Bootstrap 并注册 RuntimeTick（可选）。\n";

	if (!app.initialize())
	{
		std::cerr << "application.initialize failed\n";
		NNEngineRuntimeHost_Shutdown();
		return 1;
	}

	NNWindowDesc windowDesc{};
	windowDesc.title = "Neverness Kernel";
	windowDesc.width = 1280;
	windowDesc.height = 720;
	windowDesc.resizable = true;
	windowDesc.maximized = false;
	windowDesc.hidden = false;

	const NNWindowHandle mainWindow = window.create(&windowDesc);
	if (mainWindow == NN_INVALID_WINDOW_HANDLE)
	{
		std::cerr << "window.create failed\n";
		if (app.shutdown != nullptr)
		{
			app.shutdown();
		}
		NNEngineRuntimeHost_Shutdown();
		return 1;
	}

	constexpr int kMaxFrames = 3600 * 60;
	for (int frame = 0; frame < kMaxFrames; ++frame)
	{
		if (!app.pumpEvents())
		{
			break;
		}

		const float dt = GetFrameDeltaSeconds();
		NNEngineRuntimeHost_Tick(dt);
		NNEngineRuntimeHost_TickManaged(dt);
	}

	if (app.shutdown != nullptr)
	{
		app.shutdown();
	}

	NNEngineRuntimeHost_Shutdown();
	return 0;
}
