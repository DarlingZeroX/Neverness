#include <cstdlib>
#include <filesystem>

#include <gtest/gtest.h>

#include "VGManagedCore/ManagedExports.h"
#include "VGManagedCore/NativeAPI.h"
#include "VGManagedHost/ManagedFunction.h"
#include "VGManagedHost/ManagedHost.h"

namespace fs = std::filesystem;

namespace
{
	/**
	 * 解析托管 publish 根目录：优先环境变量 VGMANAGED_TEST_ROOT（ctest 注入），
	 * 否则使用 CMake 编译期写入的 VGMANAGED_TEST_ROOT_DEFAULT（便于从 IDE 直接运行测试 exe）。
	 */
	const char* ResolveManagedPublishRoot()
	{
		if (const char* const fromEnv = std::getenv("VGMANAGED_TEST_ROOT"))
		{
			return fromEnv;
		}
#if defined(VGMANAGED_TEST_ROOT_DEFAULT)
		return VGMANAGED_TEST_ROOT_DEFAULT;
#else
		return nullptr;
#endif
	}
} // namespace

TEST(VGManagedHost, SmokeUnmanagedCallersOnly)
{
	const char* const root = ResolveManagedPublishRoot();
	ASSERT_NE(root, nullptr)
		<< "Set VGMANAGED_TEST_ROOT to dotnet publish output, or rebuild with VGManagedHostTest "
		   "(CMake defines VGMANAGED_TEST_ROOT_DEFAULT to ManagedRuntimePublish).";

	const fs::path publish(root);
	const fs::path cfg = publish / "VisionGal.Managed.Runtime.runtimeconfig.json";
	const fs::path dll = publish / "VisionGal.Managed.Runtime.dll";

	ASSERT_TRUE(fs::exists(cfg)) << cfg;
	ASSERT_TRUE(fs::exists(dll)) << dll;

	VGManagedHost host;
	ASSERT_TRUE(host.Initialize(cfg, &dll));
	ASSERT_TRUE(host.LoadAssembly(dll));

	void* fn{};
	ASSERT_TRUE(host.TryGetUnmanagedCallersOnly(
		dll,
		"VisionGal.Managed.Runtime.Entry",
		"Smoke",
		&fn));
	ASSERT_NE(fn, nullptr);

	const auto thunk = reinterpret_cast<VGManagedVoidThunk>(fn);
	thunk();

	host.Shutdown();
}

/** Phase 2：验证托管经 VGNativeAPI 函数表调用 Native LogInfo（双向 ABI 最小闭环）。 */
TEST(VGManagedHost, BootstrapNativeApiCallsNativeLogInfo)
{
	const char* const root = ResolveManagedPublishRoot();
	ASSERT_NE(root, nullptr);

	const fs::path publish(root);
	const fs::path cfg = publish / "VisionGal.Managed.Runtime.runtimeconfig.json";
	const fs::path dll = publish / "VisionGal.Managed.Runtime.dll";

	ASSERT_TRUE(fs::exists(cfg));
	ASSERT_TRUE(fs::exists(dll));
	ASSERT_TRUE(fs::exists(publish / "VisionGal.Managed.Core.dll"));

	VGManagedHost host;
	ASSERT_TRUE(host.Initialize(cfg, &dll));
	ASSERT_TRUE(host.LoadAssembly(dll));

	const std::uint32_t logCountBefore = VGManagedHost_GetNativeLogInfoCallCountForTest();

	void* bootstrapFn{};
	ASSERT_TRUE(host.TryGetUnmanagedCallersOnly(
		dll,
		"VisionGal.Managed.Runtime.Entry",
		"BootstrapNativeApi",
		&bootstrapFn));
	ASSERT_NE(bootstrapFn, nullptr);

#if defined(_WIN32)
	using BootstrapThunk = void(__stdcall*)(const VGNativeAPI*);
#else
	using BootstrapThunk = void(*)(const VGNativeAPI*);
#endif

	const VGNativeAPI* const apiTable = VGNativeApi_GetDefaultTable();
	ASSERT_NE(apiTable, nullptr);
	ASSERT_EQ(apiTable->apiVersion, VG_NATIVE_API_VERSION);

	reinterpret_cast<BootstrapThunk>(bootstrapFn)(apiTable);

	EXPECT_GT(VGManagedHost_GetNativeLogInfoCallCountForTest(), logCountBefore);

	host.Shutdown();
}
