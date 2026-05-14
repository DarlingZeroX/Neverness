#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ManagedContext.h"
#include "VGManagedHostConfig.h"

class CoreCLRLoader;

/**
 * CoreCLR runtime lifecycle and cached hostfxr delegate (never exposed as types to consumers).
 */
class VG_MANAGED_HOST_API VGManagedRuntime
{
public:
	VGManagedRuntime();
	~VGManagedRuntime();

	VGManagedRuntime(const VGManagedRuntime&) = delete;
	VGManagedRuntime& operator=(const VGManagedRuntime&) = delete;

	/**
	 * @param runtimeConfigJson Path to a framework-dependent app's .runtimeconfig.json (e.g. publish output).
	 * @param assemblyPathHint Optional path to a managed DLL — improves hostfxr discovery via nethost.
	 */
	bool Initialize(
		const std::filesystem::path& runtimeConfigJson,
		const std::filesystem::path* assemblyPathHint = nullptr);

	void Shutdown();

	bool IsInitialized() const { return initialized_; }

	/** Cached load_assembly_and_get_function_pointer (opaque). */
	void* LoadAssemblyDelegate() const;

	void FillContextSnapshot(VGManagedRuntimeContext& out) const;

	bool RegisterLoadedAssembly(const std::filesystem::path& assemblyPath);
	const std::vector<std::filesystem::path>& LoadedAssemblies() const { return loadedAssemblies_; }

	bool TryResolveUnmanagedCallersOnly(
		const std::filesystem::path& assemblyPath,
		const char* assemblyQualifiedTypeNameUtf8,
		const char* methodNameUtf8,
		void** outFn);

private:
	bool initialized_{};
	std::unique_ptr<CoreCLRLoader> loader_;
	std::vector<std::filesystem::path> loadedAssemblies_;
	std::unordered_map<std::string, bool> loadedAssemblyPathKeys_;
};
