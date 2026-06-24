/**
 * @file WindowRegistry.cpp
 * @brief 窗口句柄注册表实现：原子句柄分配 + `unordered_map`，**禁止** 指针转句柄。
 */

#include "Core/WindowRegistry.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

namespace NN::Runtime
{
namespace
{

/** @brief 下一可用句柄（从 1 递增；0 保留为无效）。 */
std::atomic<std::uint64_t> GNextHandle{1};

std::mutex GMutex;

struct WindowEntry
{
	/** @brief 拥有权：析构时销毁 SDL 窗口与 GL 上下文。 */
	std::unique_ptr<VGWindow> window;
	/** @brief 非拥有指针，来源于 `window->GetSDLWindow()`，便于快速查找。 */
	SDL_Window* sdlWindow = nullptr;
};

std::unordered_map<NNWindowHandle, WindowEntry> GWindows;

NNWindowHandle AllocateHandle()
{
	for (;;)
	{
		const NNWindowHandle candidate = GNextHandle.fetch_add(1, std::memory_order_relaxed);
		if (candidate == NN_INVALID_WINDOW_HANDLE)
		{
			continue;
		}
		std::lock_guard lock(GMutex);
		if (GWindows.find(candidate) == GWindows.end())
		{
			return candidate;
		}
	}
}

NNWindowHandle GPrimaryHandle = NN_INVALID_WINDOW_HANDLE;

} // namespace

NNWindowHandle WindowRegistry::Create(const NNWindowDesc* desc)
{
	auto window = std::make_unique<VGWindow>();

	// Editor 路径默认无边框 + HitTest；后续可由 NNWindowDesc 扩展标志位。
	window->SetInitializeBorderless(true);
	if (!window->CreateFromDesc(desc))
	{
		return NN_INVALID_WINDOW_HANDLE;
	}

	SDL_Window* sdl = window->GetSDLWindow();
	if (sdl == nullptr)
	{
		return NN_INVALID_WINDOW_HANDLE;
	}

	const NNWindowHandle handle = AllocateHandle();
	{
		std::lock_guard lock(GMutex);
		GWindows.emplace(handle, WindowEntry{std::move(window), sdl});
	}

	AdoptPrimaryIfUnset(handle);

	return handle;
}

void WindowRegistry::Destroy(NNWindowHandle handle)
{
	if (handle == NN_INVALID_WINDOW_HANDLE)
	{
		return;
	}

	std::lock_guard lock(GMutex);
	const auto it = GWindows.find(handle);
	if (it == GWindows.end())
	{
		return;
	}

	if (GPrimaryHandle == handle)
	{
		GPrimaryHandle = NN_INVALID_WINDOW_HANDLE;
	}

	GWindows.erase(it);
}

void WindowRegistry::DestroyAll()
{
	std::lock_guard lock(GMutex);
	GWindows.clear();
	GPrimaryHandle = NN_INVALID_WINDOW_HANDLE;
}

VGWindow* WindowRegistry::Resolve(NNWindowHandle handle)
{
	std::lock_guard lock(GMutex);
	const auto it = GWindows.find(handle);
	if (it == GWindows.end())
	{
		return nullptr;
	}
	return it->second.window.get();
}

SDL_Window* WindowRegistry::TryGetSdlWindow(NNWindowHandle handle)
{
	std::lock_guard lock(GMutex);
	const auto it = GWindows.find(handle);
	if (it == GWindows.end())
	{
		return nullptr;
	}
	return it->second.sdlWindow;
}

std::size_t WindowRegistry::Count()
{
	std::lock_guard lock(GMutex);
	return GWindows.size();
}

void WindowRegistry::AdoptPrimaryIfUnset(NNWindowHandle handle)
{
	if (handle == NN_INVALID_WINDOW_HANDLE)
	{
		return;
	}
	std::lock_guard lock(GMutex);
	if (GPrimaryHandle == NN_INVALID_WINDOW_HANDLE)
	{
		GPrimaryHandle = handle;
	}
}

NNWindowHandle WindowRegistry::GetPrimaryHandle()
{
	std::lock_guard lock(GMutex);
	return GPrimaryHandle;
}

NNWindowHandle WindowRegistry::FindHandle(SDL_Window* sdlWindow)
{
	if (sdlWindow == nullptr)
	{
		return 0;
	}

	std::lock_guard lock(GMutex);
	for (const auto& [handle, entry] : GWindows)
	{
		if (entry.sdlWindow == sdlWindow)
		{
			return handle;
		}
	}
	return 0;
}

} // namespace NN::Runtime
