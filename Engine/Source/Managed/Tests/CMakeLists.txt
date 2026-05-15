# Managed Foundation 單元測試（dotnet test，無 CoreCLR Host）
if(NOT ENABLE_TESTS)
	return()
endif()

find_program(VISIONGAL_DOTNET_EXECUTABLE dotnet DOC ".NET SDK CLI")
if(NOT VISIONGAL_DOTNET_EXECUTABLE)
	message(STATUS "dotnet not found — skip VisionGal.Managed.Foundation.Tests")
	return()
endif()

set(_foundation_test_csproj
	"${CMAKE_CURRENT_SOURCE_DIR}/VisionGal.Managed.Foundation.Tests/VisionGal.Managed.Foundation.Tests.csproj"
)

add_custom_target(visiongal_managed_foundation_tests
	COMMAND "${VISIONGAL_DOTNET_EXECUTABLE}" test "${_foundation_test_csproj}" -c Debug
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Running VisionGal.Managed.Foundation.Tests (dotnet test)"
	VERBATIM
)
