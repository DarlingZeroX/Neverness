enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

file(GLOB VGLAUNCHER_TEST_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
)

file(GLOB VGLAUNCHER_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../Source/*.cpp"
)

add_executable(VGLauncherTest
	"${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp"
	${VGLAUNCHER_TEST_HEADERS}
	${VGLAUNCHER_SOURCES}
)

target_compile_definitions(VGLauncherTest PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

target_include_directories(VGLauncherTest PRIVATE
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${PROJECT_SOURCE_DIR}/Engine/Source/Core
	${PROJECT_SOURCE_DIR}/Engine/Source/Editor
	${CMAKE_CURRENT_SOURCE_DIR}/../Include
	${CMAKE_CURRENT_SOURCE_DIR}/Include
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(VGLauncherTest PRIVATE
	GTest::gtest
	GTest::gtest_main
	GTest::gmock
	GTest::gmock_main
	VGEditorFramework
)

add_test(NAME VGLauncherTest COMMAND $<TARGET_FILE:VGLauncherTest>)
