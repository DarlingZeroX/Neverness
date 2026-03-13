# CMakeList.txt: VGEditor CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 将源代码添加到此项目的可执行文件。
file(GLOB EDITOR_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
	"${PROJECT_SOURCE_DIR}/Application/VGLauncher/Include/*.h"
)

file(GLOB EDITOR_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${PROJECT_SOURCE_DIR}/Application/VGLauncher/Source/*.cpp"
	"${PROJECT_SOURCE_DIR}/Application/VGLauncher/*.cpp"
)

add_executable (VGLauncherTest ${EDITOR_HEADER_FILES} ${EDITOR_SRC_FILES})

target_compile_definitions(VGLauncherTest PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# 添加头文件搜索路径
target_include_directories(VGLauncherTest PRIVATE ${PROJECT_SOURCE_DIR}/Runtime)
target_include_directories(VGLauncherTest PRIVATE ${PROJECT_SOURCE_DIR}/Editor)
target_include_directories(VGLauncherTest PRIVATE ${PROJECT_SOURCE_DIR}/Application/VGLauncher/Include)
target_include_directories(VGLauncherTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 寻找CMake包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# 链接库
target_link_libraries(VGLauncherTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(VGLauncherTest PRIVATE SDL3::SDL3)
target_link_libraries(VGLauncherTest PRIVATE HCore)
target_link_libraries(VGLauncherTest PRIVATE VGPackage)
target_link_libraries(VGLauncherTest PRIVATE VGImgui)
target_link_libraries(VGLauncherTest PRIVATE VGEngine)
target_link_libraries(VGLauncherTest PRIVATE VGEditorFramework)

# 测试
add_test(NAME VGLauncherTest COMMAND $<TARGET_FILE:VGLauncherTest>)

