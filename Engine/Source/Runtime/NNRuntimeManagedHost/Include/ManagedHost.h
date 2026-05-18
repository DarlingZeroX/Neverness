#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "NNRuntimeManagedHostConfig.h"

class VGManagedRuntime;

/**
 * VisionGal managed runtime host?CoreCLR ????? UCO ???Phase 1?2??
 * ??? Native/Managed ABI?NNNativeAPI?? NNRuntimeManaged ??????????????
 */
class NN_RUNTIME_MANAGED_HOST_API VGManagedHost
{
public:
	VGManagedHost();
	~VGManagedHost();

	VGManagedHost(const VGManagedHost&) = delete;
	VGManagedHost& operator=(const VGManagedHost&) = delete;

	/**
	 * Boot CoreCLR using the given runtimeconfig.json.
	 * @param assemblyPathHint Optional managed DLL next to runtimeconfig ? recommended for correct hostfxr resolution.
	 */
	bool Initialize(
		const std::filesystem::path& runtimeConfigJson,
		const std::filesystem::path* assemblyPathHint = nullptr);

	/** Record an assembly as part of the multi-assembly host (does not validate file). */
	bool LoadAssembly(const std::filesystem::path& assemblyPath);

	/**
	 * Resolve [UnmanagedCallersOnly] static method. Names are UTF-8 (ASCII recommended).
	 * @param outFn typed as void* ? cast to VGManagedVoidThunk or your own ABI.
	 */
	bool TryGetUnmanagedCallersOnly(
		const std::filesystem::path& assemblyPath,
		const char* assemblyQualifiedTypeNameUtf8,
		const char* methodNameUtf8,
		void** outFn);

	void Shutdown();

	VGManagedRuntime& Runtime();

private:
	std::unique_ptr<VGManagedRuntime> runtime_;
};

/**
 * @brief ?????????? Native `NNNativeApi_DefaultLogInfo` ???????Phase 2 ABI ????
 * @note ??????? API????????????? DLL?
 */
extern "C" NN_RUNTIME_MANAGED_HOST_API std::uint32_t VGManagedHost_GetNativeLogInfoCallCountForTest(void);
