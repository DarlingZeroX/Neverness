# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
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

add_library(NevernessRuntime-Asset SHARED)

target_sources(NevernessRuntime-Asset
  PRIVATE
    Include/AudioAsset.h
    Include/GalGameAsset.h
    Include/LuaScriptAsset.h
    Include/SceneAsset.h
    Include/SceneSerializer.h
    Include/TextureAsset.h
    Include/UIAsset.h
    Include/VideoAsset.h
    Interface/AssetFactory.h
    Interface/AssetManager.h
    Interface/AudioClip.h
    Interface/ISceneSerializer.h
    Interface/Package.h
    Interface/SceneAccessor.h
    Interface/SceneSerializerFactory.h
    Interface/VideoClip.h
    Source/AssetFactory.cpp
    Source/AssetManager.cpp
    Source/AudioAsset.cpp
    Source/AudioClip.cpp
    Source/GalGameAsset.cpp
    Source/LuaScriptAsset.cpp
    Source/Package.cpp
    Source/SceneAccessor.cpp
    Source/SceneSerializer.cpp
    Source/SceneSerializerFactory.cpp
    Source/TextureAsset.cpp
    Source/UIAsset.cpp
    Source/VideoAsset.cpp
    Source/VideoClip.cpp
)

target_compile_definitions(NevernessRuntime-Asset PRIVATE VG_ASSET_EXPORT)

target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Asset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Asset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-Asset PRIVATE SDL3::SDL3)

find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-Asset PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)

target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessCore-PlatformCore)
target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessCore-FileSystem)
target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessCore-MediaCore)
target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessRuntime-Core)
target_link_libraries(NevernessRuntime-Asset PRIVATE NevernessRuntime-VFS)
