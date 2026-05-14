#include "CoreCLRLoader.h"

#include <cstring>
#include <limits>
#include <string>
#include <vector>

#include <nethost.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
#include "VGManagedHost/ManagedInterop.h"
#else
#include <dlfcn.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
#endif

#undef max

namespace
{
#if defined(_WIN32)

void* load_native_library(const wchar_t* path)
{
	return static_cast<void*>(::LoadLibraryW(path));
}

void* get_export(void* mod, const char* name)
{
	return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(mod), name));
}

void free_native_library(void* mod)
{
	if (mod)
		::FreeLibrary(static_cast<HMODULE>(mod));
}

#else

void* load_native_library(const char* path)
{
	return ::dlopen(path, RTLD_NOW | RTLD_LOCAL);
}

void* get_export(void* mod, const char* name)
{
	return ::dlsym(mod, name);
}

void free_native_library(void* mod)
{
	if (mod)
		::dlclose(mod);
}

#endif
} // namespace

struct CoreCLRLoader::Impl
{
#if defined(_WIN32)
	void* hostfxr_dll{};
#else
	void* hostfxr_dll{};
#endif
	hostfxr_handle host_context{};
	hostfxr_initialize_for_runtime_config_fn init_for_config{};
	hostfxr_get_runtime_delegate_fn get_runtime_delegate{};
	hostfxr_close_fn close_fn{};
	load_assembly_and_get_function_pointer_fn load_assembly_and_get_fn{};

	void reset()
	{
		if (host_context && close_fn)
		{
			close_fn(host_context);
			host_context = nullptr;
		}
		if (hostfxr_dll)
		{
			free_native_library(hostfxr_dll);
			hostfxr_dll = nullptr;
		}
		init_for_config = nullptr;
		get_runtime_delegate = nullptr;
		close_fn = nullptr;
		load_assembly_and_get_fn = nullptr;
	}
};

CoreCLRLoader::CoreCLRLoader()
	: impl_(std::make_unique<Impl>())
{
}

CoreCLRLoader::~CoreCLRLoader()
{
	shutdown();
}

bool CoreCLRLoader::initialize(
	const std::filesystem::path& runtime_config_json,
	const std::filesystem::path* assembly_path_hint)
{
	shutdown();

#if defined(_WIN32)
	std::vector<char_t> hostfxr_path;
	{
		get_hostfxr_parameters params{};
		params.size = sizeof(params);
		static thread_local std::wstring asm_hint_wide;
		if (assembly_path_hint && !assembly_path_hint->empty())
		{
			asm_hint_wide = VGManagedInterop::PathToWide(*assembly_path_hint);
			params.assembly_path = asm_hint_wide.c_str();
		}
		else
		{
			asm_hint_wide.clear();
			params.assembly_path = nullptr;
		}

		size_t buffer_size = MAX_PATH;
		hostfxr_path.resize(buffer_size);
		int tr = get_hostfxr_path(hostfxr_path.data(), &buffer_size, &params);
		while (tr == static_cast<int>(0x80008098))
		{
			if (buffer_size > static_cast<size_t>(std::numeric_limits<int>::max()))
				return false;
			hostfxr_path.resize(buffer_size);
			tr = get_hostfxr_path(hostfxr_path.data(), &buffer_size, &params);
		}
		if (tr != 0)
			return false;
		hostfxr_path.resize(buffer_size);
	}

	void* lib = load_native_library(hostfxr_path.data());
	if (!lib)
		return false;

	auto init = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
		get_export(lib, "hostfxr_initialize_for_runtime_config"));
	auto getd = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(get_export(lib, "hostfxr_get_runtime_delegate"));
	auto closef = reinterpret_cast<hostfxr_close_fn>(get_export(lib, "hostfxr_close"));
	if (!init || !getd || !closef)
	{
		free_native_library(lib);
		return false;
	}

	const std::wstring cfgW = VGManagedInterop::PathToWide(runtime_config_json);
	hostfxr_handle ctx{};
	const int32_t rc = init(cfgW.c_str(), nullptr, &ctx);
	if ((rc != 0) && (rc != 1) && (rc != 2))
	{
		free_native_library(lib);
		return false;
	}

	void* load_del{};
	const int32_t rd = getd(ctx, hdt_load_assembly_and_get_function_pointer, &load_del);
	if (rd != 0 || !load_del)
	{
		closef(ctx);
		free_native_library(lib);
		return false;
	}

	impl_->hostfxr_dll = lib;
	impl_->host_context = ctx;
	impl_->init_for_config = init;
	impl_->get_runtime_delegate = getd;
	impl_->close_fn = closef;
	impl_->load_assembly_and_get_fn =
		reinterpret_cast<load_assembly_and_get_function_pointer_fn>(load_del);
	return true;

#else
	std::vector<char_t> hostfxr_path;
	{
		get_hostfxr_parameters params{};
		params.size = sizeof(params);
		std::string asm_storage;
		if (assembly_path_hint && !assembly_path_hint->empty())
		{
			asm_storage = assembly_path_hint->string();
			params.assembly_path = asm_storage.c_str();
		}
		else
			params.assembly_path = nullptr;

		size_t buffer_size = 4096;
		hostfxr_path.resize(buffer_size);
		int tr = get_hostfxr_path(hostfxr_path.data(), &buffer_size, &params);
		while (tr == static_cast<int>(0x80008098))
		{
			hostfxr_path.resize(buffer_size);
			tr = get_hostfxr_path(hostfxr_path.data(), &buffer_size, &params);
		}
		if (tr != 0)
			return false;
		hostfxr_path.resize(buffer_size);
	}

	void* lib = load_native_library(hostfxr_path.data());
	if (!lib)
		return false;

	auto init = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
		get_export(lib, "hostfxr_initialize_for_runtime_config"));
	auto getd = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(get_export(lib, "hostfxr_get_runtime_delegate"));
	auto closef = reinterpret_cast<hostfxr_close_fn>(get_export(lib, "hostfxr_close"));
	if (!init || !getd || !closef)
	{
		free_native_library(lib);
		return false;
	}

	const std::string cfg = runtime_config_json.string();
	hostfxr_handle ctx{};
	const int32_t rc = init(cfg.c_str(), nullptr, &ctx);
	if ((rc != 0) && (rc != 1) && (rc != 2))
	{
		free_native_library(lib);
		return false;
	}

	void* load_del{};
	const int32_t rd = getd(ctx, hdt_load_assembly_and_get_function_pointer, &load_del);
	if (rd != 0 || !load_del)
	{
		closef(ctx);
		free_native_library(lib);
		return false;
	}

	impl_->hostfxr_dll = lib;
	impl_->host_context = ctx;
	impl_->init_for_config = init;
	impl_->get_runtime_delegate = getd;
	impl_->close_fn = closef;
	impl_->load_assembly_and_get_fn =
		reinterpret_cast<load_assembly_and_get_function_pointer_fn>(load_del);
	return true;
#endif
}

void CoreCLRLoader::shutdown()
{
	if (!impl_)
		return;
	impl_->reset();
}

bool CoreCLRLoader::is_initialized() const
{
	return impl_ && impl_->load_assembly_and_get_fn != nullptr;
}

void* CoreCLRLoader::host_context_opaque() const
{
	if (!impl_)
		return nullptr;
	return impl_->host_context;
}

void* CoreCLRLoader::cached_load_assembly_and_get_function_pointer() const
{
	if (!impl_ || !impl_->load_assembly_and_get_fn)
		return nullptr;
	return reinterpret_cast<void*>(impl_->load_assembly_and_get_fn);
}

bool CoreCLRLoader::resolve_unmanaged_callers_only(
	const std::filesystem::path& assembly_path,
	const char* assemblyQualifiedTypeNameUtf8,
	const char* methodNameUtf8,
	void** out_function) const
{
	if (!is_initialized() || !out_function || !assemblyQualifiedTypeNameUtf8 || !methodNameUtf8)
		return false;
	*out_function = nullptr;

#if defined(_WIN32)
	const std::wstring asmW = VGManagedInterop::PathToWide(assembly_path);
	const std::wstring typeW = VGManagedInterop::Utf8ToWide(assemblyQualifiedTypeNameUtf8);
	const std::wstring methodW = VGManagedInterop::Utf8ToWide(methodNameUtf8);
	void* delegate{};
	const int hr = impl_->load_assembly_and_get_fn(
		asmW.c_str(),
		typeW.c_str(),
		methodW.c_str(),
		UNMANAGEDCALLERSONLY_METHOD,
		nullptr,
		&delegate);
	if (hr != 0 || !delegate)
		return false;
	*out_function = delegate;
	return true;
#else
	const std::string asm_u8 = assembly_path.string();
	void* delegate{};
	const int hr = impl_->load_assembly_and_get_fn(
		asm_u8.c_str(),
		assemblyQualifiedTypeNameUtf8,
		methodNameUtf8,
		UNMANAGEDCALLERSONLY_METHOD,
		nullptr,
		&delegate);
	if (hr != 0 || !delegate)
		return false;
	*out_function = delegate;
	return true;
#endif
}
