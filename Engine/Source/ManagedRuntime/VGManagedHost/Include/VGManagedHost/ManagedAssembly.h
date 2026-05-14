#pragma once

#include <filesystem>

#include "VGManagedHost/VGManagedHostConfig.h"

class VGManagedRuntime;

/**
 * Represents one managed assembly on disk (multi-assembly host).
 * Phase 1: bookkeeping + resolve entry points via load_assembly_and_get_function_pointer.
 */
class VG_MANAGED_HOST_API VGManagedAssembly
{
public:
	explicit VGManagedAssembly(std::filesystem::path assemblyPath);

	const std::filesystem::path& Path() const { return path_; }

	/** Resolve a static method marked [UnmanagedCallersOnly]. type / method are UTF-8. */
	bool TryGetUnmanagedCallersOnly(
		VGManagedRuntime& runtime,
		const char* assemblyQualifiedTypeNameUtf8,
		const char* methodNameUtf8,
		void** outFunction) const;

private:
	std::filesystem::path path_;
};
