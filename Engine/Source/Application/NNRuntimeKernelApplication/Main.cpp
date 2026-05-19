/**
 * @file Main.cpp
 * @brief Runtime Kernel + Application 演示：SDL 窗口、事件泵、Native/Managed 双 Tick。
 */

#include "ManagedRuntimeBridge.h"
#include "NativeEngineRuntimeServices.h"

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
	if (app.initialize == nullptr || app.openWindow == nullptr || app.pumpEvents == nullptr)
	{
		std::cerr << "NNApplicationAPI is not wired\n";
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

	if (!app.openWindow("Neverness Kernel", 1280, 720))
	{
		std::cerr << "application.openWindow failed\n";
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
