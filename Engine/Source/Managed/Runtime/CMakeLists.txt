# =============================================================================
# Engine/Source/Managed/Runtime — 托管 Runtime 构建
# -----------------------------------------------------------------------------
# 入口项目：Neverness.Runtime.Host
#   - 通过 ProjectReference 拉取本目录下几乎全部 Runtime 程序集及 Editor.Framework
#   - 含 CoreCLR 宿主所需的 .runtimeconfig.json（GenerateRuntimeConfigurationFiles）
#
# CMake 目标：neverness_managed_runtime（ALL，默认构建）
#
# 依赖顺序（C++ 优先）：
#   本目标依赖下列原生目标完成后，才执行 dotnet build，避免与 C++ 链接/拷贝竞态。
#   - NevernessRuntime-Managed  — Native ↔ Managed ABI 桥接 DLL
#   - VGLauncher                — 主程序启动器（拉取主要 Runtime 依赖链）
#   - VGEditor                  — 原生编辑器可执行文件
#   - NevernessRuntime-ManagedHost（可选）— CoreCLR 宿主，需 VISIONGAL_ENABLE_MANAGED_HOST=ON
# =============================================================================

include(${CMAKE_CURRENT_SOURCE_DIR}/../NevernessDotNet.cmake)

neverness_add_dotnet_build(neverness_managed_runtime
	"${CMAKE_CURRENT_SOURCE_DIR}/Neverness.Runtime.Host/Neverness.Runtime.Host.csproj")

if(TARGET neverness_managed_runtime)
	# 保证主要 C++ 产物先就绪，再编译 C#
	add_dependencies(neverness_managed_runtime
		NevernessRuntime-Managed
		VGLauncher
		VGEditor)

	# ManagedHost 为可选模块；仅当已启用且目标存在时才添加依赖
	if(VISIONGAL_ENABLE_MANAGED_HOST AND TARGET NevernessRuntime-ManagedHost)
		add_dependencies(neverness_managed_runtime NevernessRuntime-ManagedHost)
	endif()
endif()
