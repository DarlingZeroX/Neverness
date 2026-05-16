# Minimal tests for VGEditorGalgameSequence (document, commands, clipboard)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

find_package(GTest CONFIG REQUIRED)

add_executable(VGEditorGalgameSequenceTest
	"${CMAKE_CURRENT_SOURCE_DIR}/VGEditorGalgameSequenceTest.cpp"
)

target_include_directories(VGEditorGalgameSequenceTest PRIVATE
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${PROJECT_SOURCE_DIR}/Engine/Source/Kernel
	${PROJECT_SOURCE_DIR}/Engine/Source/Editor
	${PROJECT_SOURCE_DIR}/Engine/Source/Legacy/Editor
	${PROJECT_SOURCE_DIR}/Engine/Source/Editor/VGEditorReactive/Include
)

target_link_libraries(VGEditorGalgameSequenceTest PRIVATE
	VGEditorGalgameSequence
	VGEditorReactive
	GTest::gtest
	GTest::gtest_main
)

add_test(NAME VGEditorGalgameSequenceTest COMMAND $<TARGET_FILE:VGEditorGalgameSequenceTest>)
