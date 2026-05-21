# CMakeList.txt: VisionGal 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

include(${CMAKE_CURRENT_SOURCE_DIR}/NNEditorFrameworkSources.cmake)

add_library(VGEditorFramework SHARED)
target_sources(VGEditorFramework PRIVATE ${NNEditorFramework_SOURCES})

# 添加宏
target_compile_definitions(VGEditorFramework PRIVATE VG_EDITOR_FRAMEWORK_API_EXPORT)

# 添加头文件搜索路径
target_include_directories(VGEditorFramework PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGEditorFramework PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(VGEditorFramework PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)
target_include_directories(VGEditorFramework PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Bridge)
target_include_directories(VGEditorFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGEditorFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

target_link_libraries(VGEditorFramework PUBLIC NevernessRuntime-EngineLegacy)
target_link_libraries(VGEditorFramework PRIVATE NevernessBridge-ImGui)
