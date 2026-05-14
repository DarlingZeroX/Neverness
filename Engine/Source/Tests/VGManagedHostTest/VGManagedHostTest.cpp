#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <thread>

#include <gtest/gtest.h>

#include "VGNativeEngineAPI/NativeEngineAPI.h"
#include "VGManagedCore/ManagedExports.h"
#include "VGManagedCore/NativeAPI.h"
#include "VGManagedHost/ManagedFunction.h"
#include "VGManagedHost/ManagedHost.h"

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
#include "VGEngineRuntimeServices/NativeEngineRuntimeServices.h"
#endif

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

/**
 * Phase 1：UCO Smoke；Phase 2：BootstrapNativeApi → Native LogInfo。
 * 合并为单测、单次 Initialize/Shutdown：同一进程内第二次 hostfxr_initialize_for_runtime_config
 * 在部分 hostfxr 版本上不稳定，避免依赖 CLR 重复冷启动。
 */
TEST(VGManagedHost, InteropSmokeAndBootstrapNativeApi)
{
	const char* const root = ResolveManagedPublishRoot();
	std::cout << "[Resolved managed publish root]: " << (root ? root : "(null)") << std::endl;
	ASSERT_NE(root, nullptr)
		<< "Set VGMANAGED_TEST_ROOT to dotnet publish output, or rebuild with VGManagedHostTest "
		   "(CMake defines VGMANAGED_TEST_ROOT_DEFAULT to ManagedRuntimePublish).";

	const fs::path publish(root);
	const fs::path cfg = publish / "VisionGal.Managed.Runtime.runtimeconfig.json";
	const fs::path dll = publish / "VisionGal.Managed.Runtime.dll";

	ASSERT_TRUE(fs::exists(cfg)) << cfg;
	ASSERT_TRUE(fs::exists(dll)) << dll;
	ASSERT_TRUE(fs::exists(publish / "VisionGal.Managed.Core.dll"));
	ASSERT_TRUE(fs::exists(publish / "VisionGal.Managed.Engine.dll"));
	ASSERT_TRUE(fs::exists(publish / "VisionGal.Managed.Engine.Runtime.dll"));

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	ASSERT_TRUE(VGEngineRuntimeHost_Initialize());
	VGEngineRuntimeHost_Tick(1.f / 60.f);
#endif

	VGManagedHost host;
	ASSERT_TRUE(host.Initialize(cfg, &dll));
	ASSERT_TRUE(host.LoadAssembly(dll));

	// Phase 1
	void* smokeFn{};
	ASSERT_TRUE(host.TryGetUnmanagedCallersOnly(
		dll,
		"VisionGal.Managed.Runtime.Entry, VisionGal.Managed.Runtime",
		"Smoke",
		&smokeFn));
	ASSERT_NE(smokeFn, nullptr);
	reinterpret_cast<VGManagedVoidThunk>(smokeFn)();

	// Phase 2：計數必須與 VGNativeApi_GetDefaultTable() 同源（exe 內嵌的 VGManagedCore）。
	// 若改呼叫 VGManagedHost_GetNativeLogInfoCallCountForTest，會讀到 VGManagedHost.dll 內另一份靜態副本，永遠為 0。
	const std::uint32_t logCountBefore = VGNativeApi_GetLogInfoCallCount();
	const std::uint32_t engineStubCountBefore = VGNativeEngineApi_GetStubInvokeCount();
	void* bootstrapFn{};
	ASSERT_TRUE(host.TryGetUnmanagedCallersOnly(
		dll,
		"VisionGal.Managed.Runtime.Entry, VisionGal.Managed.Runtime",
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
	ASSERT_NE(apiTable->engineServices, nullptr);
	ASSERT_EQ(apiTable->engineServices->layoutVersion, VG_NATIVE_ENGINE_API_LAYOUT_VERSION);

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	{
		const VGNativeEngineAPI* const eng = apiTable->engineServices;
		ASSERT_NE(eng->timing.getFrameIndex, nullptr);
		const std::uint64_t frame = eng->timing.getFrameIndex();
		ASSERT_GE(frame, 1u);
		ASSERT_NE(eng->timing.getTotalTime, nullptr);
		EXPECT_GT(eng->timing.getTotalTime(), 0.f);

		ASSERT_NE(eng->asyncWait.createWait, nullptr);
		ASSERT_NE(eng->asyncWait.tryComplete, nullptr);
		ASSERT_NE(eng->asyncWait.releaseWait, nullptr);
		const VGAsyncWaitHandle wait = eng->asyncWait.createWait();
		ASSERT_NE(wait, 0u);
		int sawComplete = 0;
		for (int i = 0; i < 4000; ++i)
		{
			sawComplete = eng->asyncWait.tryComplete(wait);
			if (sawComplete != 0)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(500));
		}
		ASSERT_EQ(sawComplete, 1);
		eng->asyncWait.releaseWait(wait);
	}
#endif

	reinterpret_cast<BootstrapThunk>(bootstrapFn)(apiTable);

	EXPECT_GT(VGNativeApi_GetLogInfoCallCount(), logCountBefore);
#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	(void)engineStubCountBefore;
#else
	EXPECT_GT(VGNativeEngineApi_GetStubInvokeCount(), engineStubCountBefore);
#endif

	host.Shutdown();

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	VGEngineRuntimeHost_Shutdown();
#endif
}
