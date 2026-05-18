# CMakeList.txt: VGEditor CMake 项目，在此处包括源代码并定义
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

include(${CMAKE_CURRENT_SOURCE_DIR}/NNEditorSources.cmake)

# 文件图标
if(WIN32)
    set(APP_RESOURCES resources.rc)
endif()

add_executable(VGEditor)
target_sources(VGEditor PRIVATE ${NNEditor_SOURCES} ${APP_RESOURCES})

target_compile_definitions(VGEditor PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# 添加头文件搜索路径
target_include_directories(VGEditor PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGEditor PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(VGEditor PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)
target_include_directories(VGEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
 
# 链接库
target_link_libraries(VGEditor PRIVATE VGEditorFramework)
if(VISIONGAL_BUILD_LEGACY_GALGAME)
	target_link_libraries(VGEditor PRIVATE VGEditorGalgame)
	target_compile_definitions(VGEditor PRIVATE VISIONGAL_BUILD_LEGACY_GALGAME=1)
else()
	target_compile_definitions(VGEditor PRIVATE VISIONGAL_NO_LEGACY_GALGAME=1)
endif()
