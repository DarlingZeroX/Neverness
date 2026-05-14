#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "VGManagedHost/VGManagedHostConfig.h"

class VGManagedRuntime;

/**
 * VisionGal managed runtime host：CoreCLR 生命周期与 UCO 解析（Phase 1–2）。
 * 业务级 Native/Managed ABI（VGNativeAPI）由 VGManagedCore 提供，不在此类的接口中展开。
 */
class VG_MANAGED_HOST_API VGManagedHost
{
public:
	VGManagedHost();
	~VGManagedHost();

	VGManagedHost(const VGManagedHost&) = delete;
	VGManagedHost& operator=(const VGManagedHost&) = delete;

	/**
	 * Boot CoreCLR using the given runtimeconfig.json.
	 * @param assemblyPathHint Optional managed DLL next to runtimeconfig — recommended for correct hostfxr resolution.
	 */
	bool Initialize(
		const std::filesystem::path& runtimeConfigJson,
		const std::filesystem::path* assemblyPathHint = nullptr);

	/** Record an assembly as part of the multi-assembly host (does not validate file). */
	bool LoadAssembly(const std::filesystem::path& assemblyPath);

	/**
	 * Resolve [UnmanagedCallersOnly] static method. Names are UTF-8 (ASCII recommended).
	 * @param outFn typed as void* — cast to VGManagedVoidThunk or your own ABI.
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
 * @brief 仅供自动化测试：返回 Native `VGNativeApi_DefaultLogInfo` 累计调用次数（Phase 2 ABI 验证）。
 * @note 非游戏发行公共 API；后续可收敛到独立测试辅助 DLL。
 */
extern "C" VG_MANAGED_HOST_API std::uint32_t VGManagedHost_GetNativeLogInfoCallCountForTest(void);
