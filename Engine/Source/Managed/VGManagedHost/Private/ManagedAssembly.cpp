#include "VGManagedHost/ManagedAssembly.h"

#include "VGManagedHost/ManagedRuntime.h"

VGManagedAssembly::VGManagedAssembly(std::filesystem::path assemblyPath)
	: path_(std::move(assemblyPath))
{
}

bool VGManagedAssembly::TryGetUnmanagedCallersOnly(
	VGManagedRuntime& runtime,
	const char* assemblyQualifiedTypeNameUtf8,
	const char* methodNameUtf8,
	void** outFunction) const
{
	return runtime.TryResolveUnmanagedCallersOnly(
		path_,
		assemblyQualifiedTypeNameUtf8,
		methodNameUtf8,
		outFunction);
}
