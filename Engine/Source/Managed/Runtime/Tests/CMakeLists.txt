# Managed Foundation 單元測試（dotnet test，無 CoreCLR Host）
if(NOT ENABLE_TESTS)
	return()
endif()

find_program(VISIONGAL_DOTNET_EXECUTABLE dotnet DOC ".NET SDK CLI")
if(NOT VISIONGAL_DOTNET_EXECUTABLE)
	message(STATUS "dotnet not found — skip Neverness.Managed.Foundation.Tests")
	return()
endif()

set(_foundation_test_csproj
	"${CMAKE_CURRENT_SOURCE_DIR}/NevernessRuntimeManaged-Foundation.Tests.csproj"
)

add_custom_target(visiongal_managed_foundation_tests
	COMMAND "${VISIONGAL_DOTNET_EXECUTABLE}" test "${_foundation_test_csproj}" -c Debug
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Running Neverness.Managed.Foundation.Tests (dotnet test)"
	VERBATIM
)

# 與 **visiongal_managed_runtime_publish** 並行時曾出現多個 **dotnet** 同寫 **obj/** 之競態；強制先完成 publish 再跑 Foundation，避免 **SceneRehydrationTests** 等類在 **Dispose** 時觸發字典損壞假陽性。
if(TARGET visiongal_managed_runtime_publish)
	add_dependencies(visiongal_managed_foundation_tests visiongal_managed_runtime_publish)
endif()
