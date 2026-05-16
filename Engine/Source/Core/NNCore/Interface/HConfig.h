#pragma once

#include <memory>

#ifndef NOMINMAX
#    define NOMINMAX
#endif

//#ifndef SAFE_DELETE
//#define SAFE_DELETE(p)          { if (p) { delete (p);     (p)=NULL; } }
//#endif
//#ifndef SAFE_DELETE_ARRAY
//#define SAFE_DELETE_ARRAY(p)    { if (p) { delete[] (p);   (p)=NULL; } }
//#endif
//#ifndef SAFE_RELEASE
//#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
//#endif
//#ifndef SAFE_RELEASE_ARRAY
//#define SAFE_RELEASE_ARRAY(p)   { for( int i = 0; i < _countof(p); i++ ) if (p[i]) { (p[i])->Release(); (p[i])=NULL; } }
//#endif
//
//#ifndef H_SAFE_CALL
//#define H_SAFE_CALL(x)   	try{x}catch (const std::exception& e){H_LOG_ERROR("Standard exception:{0:s},", e.what(), __FILE__, __LINE__);}
//#endif
//

#ifdef _WIN32
#define NN_KERNEL_PLATFORM_WIN32 1
#elif defined(__linux__)
#define NN_KERNEL_PLATFORM_LINUX 1
#elif defined(__ANDROID__)
#define NN_KERNEL_PLATFORM_ANDROID 1
#elif defined(__APPLE__)
#define NN_KERNEL_PLATFORM_APPLE 1
#endif

//#define NN_KERNEL_USE_GLFW
//#define NN_KERNEL_USE_SDL2
#define NN_KERNEL_USE_SDL3
//#define NN_KERNEL_USE_WIN32_NATIVE


//#ifndef ENGINE_DLL
//#    define ENGINE_DLL 1
//#endif
//
//#ifndef D3D11_SUPPORTED
//#    define D3D11_SUPPORTED 1
//#endif
//
//#ifndef D3D12_SUPPORTED
//#    define D3D12_SUPPORTED 1
//#endif
//
//#ifndef GL_SUPPORTED
//#    define GL_SUPPORTED 1
//#endif
//
//#ifndef VULKAN_SUPPORTED
//#    define VULKAN_SUPPORTED 1
//#endif

namespace NN::Core
{
	enum class PlatformType
	{
		Windows,
		Mac,
		Linux,
		Unix,
		Andriod,
		Other
	};

	enum class EWindow
	{
		WIN32_NATIVE,
		GLFW,
		SDL2,
		SDL3
	};

	enum class HTYPE_GRAPHICS_API
	{
		D3D11,
		D3D12,
		OPENGL2,
		OPENGL3,
		VULKAN,
		SDL3_RENDERER
	};

	struct PlatformWindowHandle
	{
		void* handle;
		PlatformType type;
	};
}

namespace NN
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> MakeScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> MakeRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}