# =============================================================================
# Engine/Source/Managed — 托管（C#）代码 CMake 入口
# -----------------------------------------------------------------------------
# 本目录包含 Neverness 引擎的 C# Runtime / Editor 程序集。
# 与根 CMakeLists.txt 中的 C++ 模块并列；默认 cmake --build 会同时构建 C++ 与 C#。
#
# 子目录：
#   Runtime/  — 托管 Runtime 程序集（入口：Neverness.Runtime.Host）
#   Editor/   — 托管 Editor 程序集（入口：NervernessEditor）
#   Runtime/Tests/ — Foundation 单元测试（需 ENABLE_TESTS=ON，见 Tests/CMakeLists.txt）
#
# 输出目录（与 C++ 统一）：
#   Build/bin/<Config>/          — 程序集 / 可执行文件
#   Build/Intermediate/<项目名>/<Config>/ — 中间文件（由各 .csproj 指定）
# =============================================================================

# 加载 dotnet 集成模块（option、find_program、neverness_add_dotnet_build）
include(${CMAKE_CURRENT_SOURCE_DIR}/NevernessDotNet.cmake)

add_subdirectory("Runtime")
add_subdirectory("Editor")

# Foundation 测试不加入 ALL，需显式 --target neverness_managed_foundation_tests
if(ENABLE_TESTS)
	add_subdirectory("Runtime/Tests")
endif()
