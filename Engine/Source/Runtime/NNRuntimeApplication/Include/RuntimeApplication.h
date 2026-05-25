#pragma once

#include "Core/EventQueue.h"
#include "Core/SDL3EventTranslator.h"
#include "Core/Window.h"
#include "Engine/ImGuiLayer.h"

#include "NNNativeEngineAPI/Include/WindowTypes.h"

namespace NN::Runtime::Application
{

/**
 * @brief SDL3 Runtime Host：子系统生命周期、事件泵、主窗口帧钩子（ImGui）。
 * 窗口 CRUD 见 **WindowRegistry** / **NNWindowAPI**。
 */
class RuntimeApplication
{
public:
	~RuntimeApplication();
	int Initialize();
	bool PumpEvents();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	/** @brief 首个窗口创建后由 BuildWindowApi 调用，挂接 ImGui 与主窗口句柄。 */
	void OnPrimaryWindowCreated(NNWindowHandle handle);

	/** @brief 获取事件队列引用（供 BuildEventApi 绑定函数指针）。 */
	NN::Runtime::EventQueue& GetEventQueue() noexcept { return m_eventQueue; }

private:
	void AddImguiLayer(VGWindow* window);

	bool m_sdlInitialized = false;
	bool m_shouldQuit = false;
	bool m_imguiAttached = false;

	NNWindowHandle m_primaryWindowHandle = NN_INVALID_WINDOW_HANDLE;
	Scope<ImguiOpengl3Layer> m_ImguiOpengl3Layer;

	/* ── 事件系统 ── */
	NN::Runtime::EventQueue m_eventQueue;
	Scope<SDL3EventTranslator> m_eventTranslator;
};

} // namespace NN::Runtime::Application
