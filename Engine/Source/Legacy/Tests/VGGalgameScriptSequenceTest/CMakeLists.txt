# VGGalgameSequenceRuntime 运行时内核单元测试（Save/Load、Tick 语义）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

find_package(GTest CONFIG REQUIRED)

add_executable(VGGalgameSequenceRuntimeTest
	"${CMAKE_CURRENT_SOURCE_DIR}/VGGalgameScriptSequenceTest.cpp"
)

target_include_directories(VGGalgameSequenceRuntimeTest PRIVATE
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
)

target_link_libraries(VGGalgameSequenceRuntimeTest PRIVATE
	VGGalgameSequenceRuntime
	GTest::gtest
	GTest::gtest_main
)

add_test(NAME VGGalgameSequenceRuntimeTest COMMAND $<TARGET_FILE:VGGalgameSequenceRuntimeTest>)
