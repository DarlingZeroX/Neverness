#include "VGManagedHost/ManagedRuntime.h"

#include "CoreCLRLoader.h"

VGManagedRuntime::VGManagedRuntime() = default;

VGManagedRuntime::~VGManagedRuntime()
{
	Shutdown();
}

bool VGManagedRuntime::Initialize(
	const std::filesystem::path& runtimeConfigJson,
	const std::filesystem::path* assemblyPathHint)
{
	Shutdown();
	loader_ = std::make_unique<CoreCLRLoader>();
	if (!loader_->initialize(runtimeConfigJson, assemblyPathHint))
	{
		loader_.reset();
		return false;
	}
	initialized_ = true;
	return true;
}

void VGManagedRuntime::Shutdown()
{
	if (loader_)
		loader_->shutdown();
	loader_.reset();
	initialized_ = false;
	loadedAssemblies_.clear();
	loadedAssemblyPathKeys_.clear();
}

void* VGManagedRuntime::LoadAssemblyDelegate() const
{
	if (!loader_)
		return nullptr;
	return loader_->cached_load_assembly_and_get_function_pointer();
}

void VGManagedRuntime::FillContextSnapshot(VGManagedRuntimeContext& out) const
{
	out = {};
	if (!loader_)
		return;
	out.hostfxrHostContext = loader_->host_context_opaque();
	out.loadAssemblyAndGetFunctionPointerDelegate = LoadAssemblyDelegate();
}

bool VGManagedRuntime::RegisterLoadedAssembly(const std::filesystem::path& assemblyPath)
{
	const std::string key = assemblyPath.lexically_normal().generic_string();
	if (loadedAssemblyPathKeys_.count(key))
		return true;
	loadedAssemblyPathKeys_[key] = true;
	loadedAssemblies_.push_back(assemblyPath);
	return true;
}

bool VGManagedRuntime::TryResolveUnmanagedCallersOnly(
	const std::filesystem::path& assemblyPath,
	const char* assemblyQualifiedTypeNameUtf8,
	const char* methodNameUtf8,
	void** outFn)
{
	if (!loader_ || !initialized_)
		return false;
	return loader_->resolve_unmanaged_callers_only(
		assemblyPath,
		assemblyQualifiedTypeNameUtf8,
		methodNameUtf8,
		outFn);
}
