#pragma once

struct SDL_Window;

namespace NN::Runtime::Application
{

/** @brief SDL3 主窗口与事件泵（不含 Gameplay / Render Backend）。 */
class RuntimeApplication
{
public:
	bool Initialize();
	bool OpenWindow(const char* title, int width, int height);
	bool PumpEvents();
	void Shutdown();

private:
	SDL_Window* m_window = nullptr;
	bool m_sdlInitialized = false;
	bool m_shouldQuit = false;
};

} // namespace NN::Runtime::Application
