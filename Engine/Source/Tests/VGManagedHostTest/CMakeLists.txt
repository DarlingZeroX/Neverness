set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

find_package(GTest CONFIG REQUIRED)

# 与 VGManagedHost/CMakeLists.txt 中 visiongal_managed_runtime_publish 输出目录一致；
# 供测试 exe 在未设置环境变量（例如从 VS 直接运行）时仍能解析托管产物路径。
if(NOT DEFINED VISIONGAL_MANAGED_PUBLISH_DIR)
	set(VISIONGAL_MANAGED_PUBLISH_DIR "${CMAKE_BINARY_DIR}/ManagedRuntimePublish")
endif()
set(_vg_managed_publish_dir "${VISIONGAL_MANAGED_PUBLISH_DIR}")
string(REPLACE "\\" "/" _vg_managed_publish_dir "${_vg_managed_publish_dir}")

add_executable(VGManagedHostTest
	"${CMAKE_CURRENT_SOURCE_DIR}/VGManagedHostTest.cpp"
)

target_include_directories(VGManagedHostTest PRIVATE
	"${PROJECT_SOURCE_DIR}/Engine/Source/ManagedRuntime/VGManagedHost/Include"
	"${PROJECT_SOURCE_DIR}/Engine/Source/ManagedRuntime/VGManagedCore/Include"
)

target_link_libraries(VGManagedHostTest PRIVATE
	VGManagedCore
	VGManagedHost
	GTest::gtest
	GTest::gtest_main
)

target_compile_definitions(VGManagedHostTest PRIVATE
	"VGMANAGED_TEST_ROOT_DEFAULT=\"${_vg_managed_publish_dir}\""
)

add_dependencies(VGManagedHostTest visiongal_managed_runtime_publish)

add_custom_command(TARGET VGManagedHostTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:VGManagedHost>"
		"$<TARGET_FILE_DIR:VGManagedHostTest>"
	VERBATIM
)

add_test(NAME VGManagedHostTest COMMAND $<TARGET_FILE:VGManagedHostTest>)
set_tests_properties(VGManagedHostTest PROPERTIES
	ENVIRONMENT "VGMANAGED_TEST_ROOT=${VISIONGAL_MANAGED_PUBLISH_DIR}"
)
