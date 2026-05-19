#include "ManagedHost.h"

#include "NativeAPI.h"
#include "ManagedRuntime.h"

VGManagedHost::VGManagedHost()
	: runtime_(std::make_unique<VGManagedRuntime>())
{
}

VGManagedHost::~VGManagedHost() = default;

bool VGManagedHost::Initialize(
	const std::filesystem::path& runtimeConfigJson,
	const std::filesystem::path* assemblyPathHint)
{
	return runtime_->Initialize(runtimeConfigJson, assemblyPathHint);
}

bool VGManagedHost::LoadAssembly(const std::filesystem::path& assemblyPath)
{
	return runtime_->RegisterLoadedAssembly(assemblyPath);
}

bool VGManagedHost::TryGetUnmanagedCallersOnly(
	const std::filesystem::path& assemblyPath,
	const char* assemblyQualifiedTypeNameUtf8,
	const char* methodNameUtf8,
	void** outFn)
{
	return runtime_->TryResolveUnmanagedCallersOnly(
		assemblyPath,
		assemblyQualifiedTypeNameUtf8,
		methodNameUtf8,
		outFn);
}

void VGManagedHost::Shutdown()
{
	runtime_->Shutdown();
}

VGManagedRuntime& VGManagedHost::Runtime()
{
	return *runtime_;
}

extern "C" NN_RUNTIME_MANAGED_HOST_API std::uint32_t VGManagedHost_GetNativeLogInfoCallCountForTest(void)
{
	return NNNativeApi_GetLogInfoCallCount();
}
