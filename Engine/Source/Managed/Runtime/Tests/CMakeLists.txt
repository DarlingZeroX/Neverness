# =============================================================================
# Engine/Source/Managed/Runtime/Tests — 托管 Foundation 单元测试
# -----------------------------------------------------------------------------
# 项目：NevernessRuntimeManaged-Foundation.Tests.csproj
# 运行方式：dotnet test（不经过 CoreCLR Native Host）
#
# CMake 目标：neverness_managed_foundation_tests
#   - 不属于 ALL，需显式构建：
#       cmake --build <dir> --config Debug --target neverness_managed_foundation_tests
#
# 前置条件：
#   - ENABLE_TESTS=ON（根 CMakeLists.txt）
#   - NEVERNESS_BUILD_MANAGED=ON 且已安装 dotnet SDK
#
# 依赖：
#   - neverness_managed_runtime — 先 dotnet build Host 及其引用链，避免并行 dotnet 竞态
#
# 说明：测试固定使用 -c Debug；SolutionDir 与正式构建一致，输出仍在 Build/bin/Debug/
# =============================================================================

if(NOT ENABLE_TESTS)
	return()
endif()

if(NOT NEVERNESS_BUILD_MANAGED)
	return()
endif()

if(NOT NEVERNESS_DOTNET_EXECUTABLE)
	message(STATUS "未找到 dotnet，跳过 Neverness.Managed.Foundation.Tests")
	return()
endif()

set(_foundation_test_csproj
	"${CMAKE_CURRENT_SOURCE_DIR}/NevernessRuntimeManaged-Foundation.Tests.csproj"
)

# 与 NevernessDotNet.cmake 中 build 目标保持相同的 SolutionDir 约定
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" _neverness_solution_dir)
set(_neverness_solution_dir "${_neverness_solution_dir}/")

add_custom_target(neverness_managed_foundation_tests
	COMMAND "${NEVERNESS_DOTNET_EXECUTABLE}" test "${_foundation_test_csproj}"
		-c Debug
		-p:SolutionDir=${_neverness_solution_dir}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "正在运行 Neverness.Managed.Foundation.Tests (dotnet test)"
	VERBATIM
)

if(TARGET neverness_managed_runtime)
	add_dependencies(neverness_managed_foundation_tests neverness_managed_runtime)
endif()
