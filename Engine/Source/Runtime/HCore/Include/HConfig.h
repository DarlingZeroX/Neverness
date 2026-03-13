#pragma once

// reverting back to 'unsecure' for cross-platform compatibility
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#include <stdlib.h>
#include <crtdbg.h>

#endif

#ifdef _MSC_VER
#define VA_USE_NATIVE_WINDOWS_TIMER
#endif

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning( disable : 4201 )

// warning C4239: nonstandard extension used : 'default argument'
#pragma warning( disable : 4239 )

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)    { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE_ARRAY
#define SAFE_RELEASE_ARRAY(p)   { for( int i = 0; i < _countof(p); i++ ) if (p[i]) { (p[i])->Release(); (p[i])=NULL; } }
#endif

#ifndef H_SAFE_CALL
#define H_SAFE_CALL(x)   	try{x}catch (const std::exception& e){H_LOG_ERROR("Standard exception:{0:s},", e.what(), __FILE__, __LINE__);}
#endif

#ifdef _WIN32
#define PLATFORM_WIN32 1
#elif _linux_
#define PLATFORM_LINUX 1
#elif __ANDROID__
#define PLATFORM_ANDROID 1
#elif __APPLE__
#define PLATFORM_APPLE 1
#endif

//#define GLFW_WINDOW_SUPPORTED
//#define SDL2_WINDOW_SUPPORTED
#define SDL3_WINDOW_SUPPORTED
//#define WIN32_NATIVE_WINDOW_SUPPORTED

#ifndef NOMINMAX
#    define NOMINMAX
#endif

#ifndef ENGINE_DLL
#    define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
#    define D3D11_SUPPORTED 1
#endif

#ifndef D3D12_SUPPORTED
#    define D3D12_SUPPORTED 1
#endif

//#ifndef GL_SUPPORTED
//#    define GL_SUPPORTED 1
//#endif

#ifndef VULKAN_SUPPORTED
#    define VULKAN_SUPPORTED 1
#endif

#include "Meta/Macros.h"

enum class HMetaEnable() PlatformType
{
	Windows,
	Mac,
	Linux,
	Unix,
	Andriod,
	Other
};

enum class HMetaEnable() EWindow
{
	WIN32_NATIVE,
	GLFW,
	SDL2,
	SDL3
};

enum class HMetaEnable() HTYPE_GRAPHICS_API
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

#include <memory>
#include <string>

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
constexpr Scope<T> NewScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
constexpr Ref<T> NewRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#define H_ROOT_SPACE Horizon

#define H_BEGIN_ROOT_NAMESPACE namespace H_ROOT_SPACE {
#define H_BEGIN_ROOT_SUB_NAMESPACE(Name) namespace H_ROOT_SPACE::Name {
#define H_END_NAMESPACE }

#define H_CONSTEXPR constexpr

#ifndef H_GLOBALCONST
#define H_GLOBALCONST extern const __declspec(selectany)
#endif

#ifndef _HIn
#define _HIn
#endif

#ifndef _HOut
#define _HOut
#endif

#ifndef H_NODISCARD
#define H_NODISCARD [[nodiscard]]
#endif
//#define HCONST_REF(x) const x&
//#define HREF(x) x&